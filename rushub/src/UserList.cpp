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

void UserList::ufSend::operator() (UserBase * userBase) {
	if (userBase && userBase->isCanSend()) {
		userBase->send(mData, false); // newPolitic
	}
}

void UserList::ufSendProfile::operator() (UserBase * userBase) {
	if (userBase && userBase->isCanSend()) {
		int profile = userBase->getProfile() + 1;
		if (profile < 0) {
			profile = -profile;
		}
		if (profile > 31) {
			profile = (profile % 32) - 1;
		}
		if (mProfile & (1 << profile)) {
			userBase->send(mData, false); // newPolitic
		}
	}
}

void UserList::ufSendWithNick::operator() (UserBase * userBase) {
	if (userBase && userBase->isCanSend()) {
		userBase->send(mDataStart, false, false);
		userBase->send(userBase->nick(), false, false);
		userBase->send(mDataEnd, true); // newPolitic
	}
}

void UserList::ufSendWithNickProfile::operator() (UserBase * userBase) {
	if (userBase && userBase->isCanSend()) { 
		int profile = userBase->getProfile() + 1;
		if (profile < 0) {
			profile = -profile;
		}
		if (profile > 31) {
			profile = (profile % 32) - 1;
		}
		if (mProfile & (1 << profile)) {
			userBase->send(mDataStart, false, false);
			userBase->send(userBase->nick(), false, false);
			userBase->send(mDataEnd, true); // newPolitic
		}
	}
}

void UserList::ufDoNickList::operator() (UserBase * userBase) {
	if (!userBase->hide()) {
		msList.append(userBase->nick());
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

bool UserList::add(UserBase * userBase) {
	if (userBase) {
		return List_t::add(nick2Key(userBase->nick()), userBase);
	}
	return false;
}

bool UserList::remove(UserBase * userBase) {
	if (userBase) {
		return List_t::remove(nick2Key(userBase->nick()));
	}
	return false;
}

string & UserList::getNickList() {
	if (mbRemakeNextNickList && mbKeepNickList) {
		mNickListMaker.clear();
		for_each(begin(), end(), mNickListMaker);
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
		for_each(begin(), end(), ufSend(msCache));
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

	for_each(begin(), end(), ufSendProfile(sMsg, profile));

	if (Log(4)) {
		LogStream() << "sendToProfiles end" << endl;
	}
}

/** Sending data start+Nick+end to all
    Nick - user's nick
    Use for private send to all */
void UserList::sendToWithNick(string & s, string & e) {
	for_each(begin(), end(), ufSendWithNick(s, e));
}

void UserList::sendToWithNick(string & s, string & e, unsigned long profile) {
	for_each(begin(), end(), ufSendWithNickProfile(s, e, profile));
}

/** Flush user cache */
void UserList::flushForUser(UserBase * userBase) {
	if (msCache.size()) {
		ufSend(msCache).operator() (userBase);
	}
}

/** Flush common cache */
void UserList::flushCache() {
	static string sStr;
	if (msCache.size()) {
		sendToAll(sStr, false, false);
	}
}

/** Redefining log level function */
bool UserList::strLog() {
	Obj::strLog();
	LogStream() << "(" << size() << ")" << "[" << mName << "] ";
	return true;
}


void FullUserList::ufDoINFOList::operator() (UserBase * userBase) {
	if (!userBase->hide()) {
		msListComplete.append(userBase->myInfoString());
		msListComplete.append(msSep);
	}
}

void FullUserList::ufDoIpList::operator() (UserBase * userBase) {
	if (!userBase->hide() && userBase->getIp().size()) {
		msList.append(userBase->nick());
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

string & FullUserList::getNickList() {
	if (mbKeepNickList) {
		msCompositeNickList = UserList::getNickList();
		mbOptRemake = false;
	}
	return msCompositeNickList;
}

string & FullUserList::getInfoList(bool complete) {
	if (mbKeepInfoList) {
		if (mbRemakeNextInfoList && mbKeepInfoList) {
			mINFOListMaker.clear();
			for_each(begin(), end(), mINFOListMaker);
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

string & FullUserList::getIpList() {
	if (mbRemakeNextIpList && mbKeepIpList) {
		mIpListMaker.clear();
		for_each(begin(), end(), mIpListMaker);
		mbRemakeNextIpList = false;
	}
	return msIpList;
}

}; // namespace dcserver

/**
* $Id$
* $HeadURL$
*/
