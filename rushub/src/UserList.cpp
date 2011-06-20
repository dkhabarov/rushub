/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz
 *
 * modified: 27 Aug 2009
 * Copyright (C) 2009-2011 by Setuper
 * E-Mail: setuper at gmail dot com (setuper@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "UserBase.h"
#include "UserList.h"

#include <algorithm>

namespace dcserver {

void UserList::ufSend::operator() (UserBase * userBase) {
	if (userBase && userBase->isCanSend()) {
		userBase->send(mData, mAddSep);
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
			userBase->send(mData, mAddSep);
		}
	}
}

void UserList::ufSendWithNick::operator() (UserBase * userBase) {
	if (userBase && userBase->isCanSend() && !userBase->uid().empty()) {
		userBase->send(mDataStart, false, false);
		userBase->send(userBase->uid(), false, false);
		userBase->send(mDataEnd, true);
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
		if (mProfile & (1 << profile) && !userBase->uid().empty()) {
			userBase->send(mDataStart, false, false);
			userBase->send(userBase->uid(), false, false);
			userBase->send(mDataEnd, true);
		}
	}
}

void UserList::ufDoNickList::operator() (UserBase * userBase) {
	if (!userBase->hide() && !userBase->uid().empty()) {
		msList.append(userBase->uid());
		msList.append(msSep);
	}
}

UserList::UserList(string name, bool keepNickList) :
	Obj("UserList"),
	HashTable<UserBase *> (1024), // 1024 for big hubs and big check interval of resize
	mName(name),
	mNickListMaker(mNickList),
	mKeepNickList(keepNickList),
	mRemakeNextNickList(true),
	mOptRemake(false)
{
}

string & UserList::getNickList() {
	if (mRemakeNextNickList && mKeepNickList) {
		mNickListMaker.clear();
		for_each(begin(), end(), mNickListMaker);
		mRemakeNextNickList = mOptRemake = false;
	}
	return mNickList;
}

/**
 Sendind data to all users from the list
 data - sending data
 useCache - true - not send and save to cache, false - send data and send cache
 addSep - add sep to end of list
 */
void UserList::sendToAll(const string & data, bool useCache, bool addSep) {
	if (!useCache) {
		if (Log(4)) {
			LogStream() << "sendToAll begin" << endl;
		}

		if (mCache.size()) {
			mCache.append(data.c_str(), data.size());
			if (addSep) {
				mCache.append(NMDC_SEPARATOR);
			}
			for_each(begin(), end(), ufSend(mCache, false));
			mCache.erase(0, mCache.size());
		} else {
			for_each(begin(), end(), ufSend(data, addSep));
		}

		if (Log(4)) {
			LogStream() << "sendToAll end" << endl;
		}
	} else {
		mCache.append(data.c_str(), data.size());
		if (addSep) {
			mCache.append(NMDC_SEPARATOR);
		}
	}
}

/** Sending data to profiles */
void UserList::sendToProfiles(unsigned long profile, const string & data, bool addSep) {
	if (Log(4)) {
		LogStream() << "sendToProfiles begin" << endl;
	}

	for_each(begin(), end(), ufSendProfile(data, profile, addSep));

	if (Log(4)) {
		LogStream() << "sendToProfiles end" << endl;
	}
}

/** Sending data start+Nick+end to all
    Nick - user's nick
    Use for private send to all */
void UserList::sendWithNick(const string & dataStart, const string & dataEnd) {
	for_each(begin(), end(), ufSendWithNick(dataStart, dataEnd));
}

void UserList::sendWithNick(const string & dataStart, const string & dataEnd, unsigned long profile) {
	for_each(begin(), end(), ufSendWithNickProfile(dataStart, dataEnd, profile));
}

/** Flush user cache */
void UserList::flushForUser(UserBase * userBase) {
	if (mCache.size()) {
		ufSend(mCache, false).operator() (userBase);
	}
}

/** Flush common cache */
void UserList::flushCache() {
	if (mCache.size()) {
		string sStr;
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
	if (!userBase->hide() && userBase->ip().size() && !userBase->uid().empty()) {
		msList.append(userBase->uid());
		msList.append(" ");
		msList.append(userBase->ip());
		msList.append(msSep);
	}
}

FullUserList::FullUserList(string name, bool keepNickList, bool keepInfoList, bool keepIpList) :
	UserList(name, keepNickList),
	mInfoListMaker(mInfoList, mInfoListComplete),
	mIpListMaker(mIpList),
	mKeepInfoList(keepInfoList),
	mRemakeNextInfoList(true),
	mKeepIpList(keepIpList),
	mRemakeNextIpList(true)
{
	SetClassName("FullUserList");
}

string & FullUserList::getNickList() {
	if (mKeepNickList) {
		UserList::getNickList();
		mOptRemake = false;
	}
	return mNickList;
}

string & FullUserList::getInfoList(bool complete) {
	if (mKeepInfoList) {
		if (mRemakeNextInfoList && mKeepInfoList) {
			mInfoListMaker.clear();
			for_each(begin(), end(), mInfoListMaker);
			mRemakeNextInfoList = mOptRemake = false;
		}
	}
	return complete ? mInfoListComplete : mInfoList;
}

string & FullUserList::getIpList() {
	if (mRemakeNextIpList && mKeepIpList) {
		mIpListMaker.clear();
		for_each(begin(), end(), mIpListMaker);
		mRemakeNextIpList = false;
	}
	return mIpList;
}

}; // namespace dcserver

/**
 * $Id$
 * $HeadURL$
 */
