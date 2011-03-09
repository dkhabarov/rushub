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
//		for (; first != last; ++first) {
//			f(*first);
//		}
//		return f;
//	}
#else
#define For_each for_each
//template<class T1, class T2> inline
//	T2 For_each(T1 first, T1 last, T2 f) {
//		return for_each (first, last, f);
//	}
#endif // _WIN32

void UserList::ufSend::operator() (UserBase * userBase) {
	if (userBase && userBase->isCanSend()) {
		if (!mbProfile) {
			userBase->send(msData, false); // newPolitic
		} else {
			int profile = userBase->getProfile() + 1;
			if (profile < 0) {
				profile = -profile;
			}
			if (profile > 31) {
				profile = (profile % 32) - 1;
			}
			if (miProfile & (1 << profile)) {
				userBase->send(msData, false); // newPolitic
			}
		}
	}
}

void UserList::ufSendWithNick::operator() (UserBase * userBase) {
	if (userBase && userBase->isCanSend()) { 
		if (!mbProfile) {
			userBase->send(msDataStart, false, false);
			userBase->send(userBase->Nick(), false, false);
			userBase->send(msDataEnd, true); // newPolitic
		} else {
			int profile = userBase->getProfile() + 1;
			if (profile < 0) {
				profile = -profile;
			}
			if (profile > 31) {
				profile = (profile % 32) - 1;
			}
			if (miProfile & (1 << profile)) {
				userBase->send(msDataStart, false, false);
				userBase->send(userBase->Nick(), false, false);
				userBase->send(msDataEnd, true); // newPolitic
			}
		}
	}
}

void UserList::ufDoNickList::operator() (UserBase * userBase) {
	if (!userBase->Hide()) {
		msList.append(userBase->Nick());
		msList.append(msSep);
	}
}

UserList::UserList(string name, bool keepNickList) :
	Obj("UserList"),
	HashTable<UserBase *> (1024), // 1024 for big hubs and big check interval of resize
	mName(name),
	mNickListMaker(msNickList),
	mbKeepNickList(keepNickList),
	mbRemakeNextNickList(true),
	mbOptRemake(false)
{
}

bool UserList::Add(UserBase * userBase) {
	if (userBase) {
		return List_t::Add(Nick2Key(userBase->Nick()), userBase);
	}
	return false;
}

bool UserList::Remove(UserBase * userBase) {
	if (userBase) {
		return List_t::Remove(Nick2Key(userBase->Nick()));
	}
	return false;
}

string & UserList::GetNickList() {
	if (mbRemakeNextNickList && mbKeepNickList) {
		mNickListMaker.Clear();
		For_each(begin(), end(), mNickListMaker);
		mbRemakeNextNickList = mbOptRemake = false;
	}
	return msNickList;
}

/**
 Sendind data to all users from the list
 data - sending data
 useCache - true - not send and save to cache, false - send data and send cache
 addSep - add sep to end of list
 */
void UserList::sendToAll(const string & data, bool useCache, bool addSep) {
	msCache.append(data.c_str(), data.size());
	if (addSep) {
		msCache.append(NMDC_SEPARATOR);
	}
	if (!useCache) {
		if (Log(4)) {
			LogStream() << "sendToAll begin" << endl;
		}
		For_each(begin(), end(), ufSend(msCache));
		if (Log(4)) {
			LogStream() << "sendToAll end" << endl;
		}
		msCache.erase(0, msCache.size());
	}
}

/** Sending data to profiles */
void UserList::sendToProfiles(unsigned long profile, const string & data, bool addSep) {
	string sMsg(data);
	if (addSep) {
		sMsg.append(NMDC_SEPARATOR);
	}

	if (Log(4)) {
		LogStream() << "sendToProfiles begin" << endl;
	}

	For_each(begin(), end(), ufSend(sMsg, profile));

	if (Log(4)) {
		LogStream() << "sendToProfiles end" << endl;
	}
}

/** Sending data start+Nick+end to all
    Nick - user's nick
    Use for private send to all */
void UserList::SendToWithNick(string & s, string & e) {
	For_each(begin(), end(), ufSendWithNick(s, e));
}

void UserList::SendToWithNick(string & s, string & e, unsigned long profile) {
	For_each(begin(), end(), ufSendWithNick(s, e, profile));
}

/** Flush user cache */
void UserList::FlushForUser(UserBase * userBase) {
	if (msCache.size()) {
		ufSend(msCache).operator() (userBase);
	}
}

/** Flush common cache */
void UserList::FlushCache() {
	static string sStr;
	if (msCache.size()) {
		sendToAll(sStr, false, false);
	}
}

/** Redefining log level function */
bool UserList::strLog() {
	Obj::strLog();
	LogStream() << "(" << Size() << ")" << "[" << mName << "] ";
	return true;
}


void FullUserList::ufDoINFOList::operator() (UserBase * userBase) {
	if (!userBase->Hide()) {
		msListComplete.append(userBase->MyINFO());
		msListComplete.append(msSep);
	}
}

void FullUserList::ufDoIpList::operator() (UserBase * userBase) {
	if (!userBase->Hide() && userBase->getIp().size()) {
		msList.append(userBase->Nick());
		msList.append(" ");
		msList.append(userBase->getIp());
		msList.append(msSep);
	}
}

FullUserList::FullUserList(string name, bool keepNickList, bool keepInfoList, bool keepIpList) :
	UserList(name, keepNickList),
	mINFOListMaker(msINFOList, msINFOListComplete),
	mIpListMaker(msIpList),
	mbKeepInfoList(keepInfoList),
	mbRemakeNextInfoList(true),
	mbKeepIpList(keepIpList),
	mbRemakeNextIpList(true)
{
	SetClassName("FullUserList");
}

string & FullUserList::GetNickList() {
	if (mbKeepNickList) {
		msCompositeNickList = UserList::GetNickList();
		mbOptRemake = false;
	}
	return msCompositeNickList;
}

string & FullUserList::GetInfoList(bool complete) {
	if (mbKeepInfoList) {
		if (mbRemakeNextInfoList && mbKeepInfoList) {
			mINFOListMaker.Clear();
			For_each(begin(), end(), mINFOListMaker);
			mbRemakeNextInfoList = mbOptRemake = false;
		}
		if (complete) {
			msCompositeINFOList = msINFOListComplete;
		} else {
			msCompositeINFOList = msINFOList;
		}
	}
	return msCompositeINFOList;
}

string & FullUserList::GetIpList() {
	if (mbRemakeNextIpList && mbKeepIpList) {
		mIpListMaker.Clear();
		For_each(begin(), end(), mIpListMaker);
		mbRemakeNextIpList = false;
	}
	return msIpList;
}

}; // namespace dcserver
