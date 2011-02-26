/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz

 * modified: 27 Aug 2009
 * Copyright (C) 2009-2011 by Setuper
 * E-Mail: setuper at gmail dot com (setuper@gmail.com)

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "UserBase.h"
#include "UserList.h"

#include <algorithm>

namespace dcserver {

#ifdef _WIN32
#define For_each for_each
//template<class T1, class T2> inline
//	T2 For_each(T1 first, T1 last, T2 f) {
//		for(; first != last; ++first) f(*first);
//		return f;
//	}
#else
#define For_each for_each
//template<class T1, class T2> inline
//	T2 For_each(T1 first, T1 last, T2 f) {
//		return for_each(first, last, f);
//	}
#endif // _WIN32

void UserList::ufSend::operator()(UserBase *User) {
	if(User && User->CanSend()) {
		if(!mbProfile) {
			User->send(msData, false); // newPolitic
		} else {
			int iProfile = User->getProfile() + 1;
			if(iProfile < 0) iProfile = -iProfile;
			if(iProfile > 31) iProfile = (iProfile % 32) - 1;
			if(miProfile & (1 << iProfile))
				User->send(msData, false); // newPolitic
		}
	}
}

void UserList::ufSendWithNick::operator()(UserBase *User) {
	if(User && User->CanSend()) { 
		if(!mbProfile) {
			User->send(msDataStart, false, false);
			User->send(User->Nick(), false, false);
			User->send(msDataEnd, true); // newPolitic
		} else {
			int iProfile = User->getProfile() + 1;
			if(iProfile < 0) iProfile = -iProfile;
			if(iProfile > 31) iProfile = (iProfile % 32) - 1;
			if(miProfile & (1 << iProfile)) {
				User->send(msDataStart, false, false);
				User->send(User->Nick(), false, false);
				User->send(msDataEnd, true); // newPolitic
			}
		}
	}
}

void UserList::ufDoNickList::operator()(UserBase *User) {
	if(!User->Hide()) {
		msList.append(User->Nick());
		msList.append(msSep);
	}
}

UserList::UserList(string sName, bool bKeepNickList) :
	Obj("UserList"),
	HashTable<UserBase*>(1024), // 1024 for big hubs and big check interval of resize
	mName(sName),
	mNickListMaker(msNickList),
	mbKeepNickList(bKeepNickList),
	mbRemakeNextNickList(true),
	mbOptRemake(false)
{
}

bool UserList::Add(UserBase *User) {
	if(User) return List_t::Add(Nick2Key(User->Nick()), User);
	else return false;
}

bool UserList::Remove(UserBase *User) {
	if(User) return List_t::Remove(Nick2Key(User->Nick()));
	else return false;
}

string &UserList::GetNickList() {
	if(mbRemakeNextNickList && mbKeepNickList) {
		mNickListMaker.Clear();
		For_each(begin(), end(), mNickListMaker);
		mbRemakeNextNickList = mbOptRemake = false;
	}
	return msNickList;
}

/**
 Sendind data to all users from the list
 sData - sending data
 bUseCache - true - not send and save to cache, false - send data and send cache
 bAddSep - add sep to end of list
 */
void UserList::sendToAll(const string &sData, bool bUseCache, bool bAddSep) {
	msCache.append(sData.c_str(), sData.size());
	if(bAddSep) msCache.append(NMDC_SEPARATOR);
	if(!bUseCache) {
		if(Log(4)) LogStream() << "sendToAll begin" << endl;
		For_each(begin(), end(), ufSend(msCache));
		if(Log(4)) LogStream() << "sendToAll end" << endl;
		msCache.erase(0, msCache.size());
	}
}

/** Sending data to profiles */
void UserList::sendToProfiles(unsigned long iProfile, const string &sData, bool bAddSep) {
	string sMsg(sData);
	if(bAddSep) sMsg.append(NMDC_SEPARATOR);
	if(Log(4)) LogStream() << "sendToProfiles begin" << endl;
	For_each(begin(), end(), ufSend(sMsg, iProfile));
	if(Log(4)) LogStream() << "sendToProfiles end" << endl;
}

/** Sending data sStart+Nick+sEnd to all
    Nick - user's nick
    Use for private send to all */
void UserList::SendToWithNick(string &sStart, string &sEnd) {
	For_each(begin(), end(), ufSendWithNick(sStart, sEnd));
}

void UserList::SendToWithNick(string &sStart, string &sEnd, unsigned long iProfile) {
	For_each(begin(), end(), ufSendWithNick(sStart, sEnd, iProfile));
}

/** Flush user cache */
void UserList::FlushForUser(UserBase *User) {
	if(msCache.size())
		ufSend(msCache).operator()(User);
}

/** Flush common cache */
void UserList::FlushCache() {
	static string sStr;
	if(msCache.size())
		sendToAll(sStr, false, false);
}

/** Redefining log level function */
bool UserList::StrLog() {
	Obj::StrLog();
	LogStream() << "(" << Size() << ")" << "[" << mName << "] ";
	return true;
}


void FullUserList::ufDoINFOList::operator()(UserBase *User) {
	if(!User->Hide()) {
		msListComplete.append(User->MyINFO());
		msListComplete.append(msSep);
	}
}

void FullUserList::ufDoIpList::operator()(UserBase *User) {
	if(!User->Hide() && User->getIp().size()) {
		msList.append(User->Nick());
		msList.append(" ");
		msList.append(User->getIp());
		msList.append(msSep);
	}
}

FullUserList::FullUserList(string sName, bool bKeepNickList, bool bKeepInfoList, bool bKeepIpList) :
	UserList(sName, bKeepNickList),
	mINFOListMaker(msINFOList, msINFOListComplete),
	mIpListMaker(msIpList),
	mbKeepInfoList(bKeepInfoList),
	mbRemakeNextInfoList(true),
	mbKeepIpList(bKeepIpList),
	mbRemakeNextIpList(true)
{
	SetClassName("FullUserList");
}

string &FullUserList::GetNickList() {
	if(mbKeepNickList) {
		msCompositeNickList = UserList::GetNickList();
		mbOptRemake = false;
	}
	return msCompositeNickList;
}

string &FullUserList::GetInfoList(bool bComplete) {
	if(mbKeepInfoList) {
		if(mbRemakeNextInfoList && mbKeepInfoList) {
			mINFOListMaker.Clear();
			For_each(begin(), end(), mINFOListMaker);
			mbRemakeNextInfoList = mbOptRemake = false;
		}
		if(bComplete) msCompositeINFOList = msINFOListComplete;
		else msCompositeINFOList = msINFOList;
	}
	return msCompositeINFOList;
}

string &FullUserList::GetIpList() {
	if(mbRemakeNextIpList && mbKeepIpList) {
		mIpListMaker.Clear();
		For_each(begin(), end(), mIpListMaker);
		mbRemakeNextIpList = false;
	}
	return msIpList;
}

}; // namespace dcserver
