/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * Copyright (C) 2009-2010 by Setuper
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

#include "cuserbase.h"
#include "cuserlist.h"

#include <algorithm>

namespace nDCServer {

void cUserList::ufSend::operator()(cUserBase *User) {
	if(User && User->CanSend()) {
		if(!mbProfile) {
			User->Send(msData, false); // newPolitic
		} else {
			int iProfile = User->GetProfile() + 1;
			if(iProfile < 0) iProfile = -iProfile;
			if(iProfile > 31) iProfile = (iProfile % 32) - 1;
			if(miProfile & (1 << iProfile))
				User->Send(msData, false); // newPolitic
		}
	}
}

void cUserList::ufSendWithNick::operator()(cUserBase *User) {
	if(User && User->CanSend()) { 
		if(!mbProfile) {
			User->Send(msDataStart, false, false);
			User->Send(User->msNick, false, false);
			User->Send(msDataEnd, true); // newPolitic
		} else {
			int iProfile = User->GetProfile() + 1;
			if(iProfile < 0) iProfile = -iProfile;
			if(iProfile > 31) iProfile = (iProfile % 32) - 1;
			if(miProfile & (1 << iProfile)) {
				User->Send(msDataStart, false, false);
				User->Send(User->msNick, false, false);
				User->Send(msDataEnd, true); // newPolitic
			}
		}
	}
}

void cUserList::ufDoNickList::operator()(cUserBase *User) {
	if(!User->mbHide) {
		msList.append(User->msNick);
		msList.append(msSep);
	}
}

cUserList::cUserList(string sName, bool bKeepNickList) :
	cObj("cUserList"),
	tcHashTable<cUserBase*>(2048), // 2048 for big hubs and big check interval of resize
	msName(sName),
	mNickListMaker(msNickList),
	mbKeepNickList(bKeepNickList),
	mbRemakeNextNickList(true),
	mbOptRemake(false)
{
}

bool cUserList::Add(cUserBase *User) {
	if(User) return List_t::Add(Nick2Key(User->msNick), User);
	else return false;
}

bool cUserList::Remove(cUserBase *User) {
	if(User) return List_t::Remove(Nick2Key(User->msNick));
	else return false;
}

string &cUserList::GetNickList() {
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
void cUserList::SendToAll(const string &sData, bool bUseCache, bool bAddSep) {
	msCache.append(sData.c_str(), sData.size());
	if(bAddSep) msCache.append(DC_SEPARATOR);
	if(!bUseCache) {
		if(Log(4)) LogStream() << "SendToAll begin" << endl;
		For_each(begin(), end(), ufSend(msCache));
		if(Log(4)) LogStream() << "SendToAll end" << endl;
		msCache.erase(0, msCache.size());
	}
}

/** Sending data to profiles */
void cUserList::SendToProfiles(unsigned long iProfile, const string &sData, bool bAddSep) {
	string sMsg(sData);
	if(bAddSep) sMsg.append(DC_SEPARATOR);
	if(Log(4)) LogStream() << "SendToProfiles begin" << endl;
	For_each(begin(), end(), ufSend(sMsg, iProfile));
	if(Log(4)) LogStream() << "SendToProfiles end" << endl;
}

/** Sending data sStart+msNick+sEnd to all
    msNick - user's nick
    Use for private send to all */
void cUserList::SendToWithNick(string &sStart, string &sEnd) {
	For_each(begin(), end(), ufSendWithNick(sStart, sEnd));
}

void cUserList::SendToWithNick(string &sStart, string &sEnd, unsigned long iProfile) {
	For_each(begin(), end(), ufSendWithNick(sStart, sEnd, iProfile));
}

/** Flush user cache */
void cUserList::FlushForUser(cUserBase *User) {
	if(msCache.size())
		ufSend(msCache).operator()(User);
}

/** Flush common cache */
void cUserList::FlushCache() {
	static string sStr;
	if(msCache.size())
		SendToAll(sStr, false, false);
}

/** Redefining log level function */
int cUserList::StrLog(ostream & ostr, int iLevel, int iMaxLevel, bool bIsError /* = false */) {
	if(cObj::StrLog(ostr, iLevel, iMaxLevel, bIsError)) {
		LogStream() << "(" << Size() << ")" << "[" << msName << "] ";
		return 1;
	}
	return 0;
}


void cFullUserList::ufDoINFOList::operator()(cUserBase *User) {
	if(!User->mbHide) {
		msListComplete.append(User->MyINFO());
		msListComplete.append(msSep);
	}
}

void cFullUserList::ufDoIpList::operator()(cUserBase *User) {
	if(!User->mbHide && User->GetIp().size()) {
		msList.append(User->msNick);
		msList.append(" ");
		msList.append(User->GetIp());
		msList.append(msSep);
	}
}

cFullUserList::cFullUserList(string sName, bool bKeepNickList, bool bKeepInfoList, bool bKeepIpList) :
	cUserList(sName, bKeepNickList),
	mINFOListMaker(msINFOList, msINFOListComplete),
	mIpListMaker(msIpList),
	mbKeepInfoList(bKeepInfoList),
	mbRemakeNextInfoList(true),
	mbKeepIpList(bKeepIpList),
	mbRemakeNextIpList(true)
{
	SetClassName("cFullUserList");
}

string &cFullUserList::GetNickList() {
	if(mbKeepNickList) {
		msCompositeNickList = cUserList::GetNickList();
		mbOptRemake = false;
	}
	return msCompositeNickList;
}

string &cFullUserList::GetInfoList(bool bComplete) {
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

string &cFullUserList::GetIpList() {
	if(mbRemakeNextIpList && mbKeepIpList) {
		mIpListMaker.Clear();
		For_each(begin(), end(), mIpListMaker);
		mbRemakeNextIpList = false;
	}
	return msIpList;
}

}; // nDCServer
