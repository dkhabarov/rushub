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



/** Unary function for sending data to users */
struct ufSend : public unary_function<void, UserList::iterator> {
	const string & mData; /** Data for sending */
	bool mAddSep;

	ufSend(const string & data, bool addSep) : mData(data), mAddSep(addSep) {
	}

	ufSend & operator = (const ufSend &) {
		return *this;
	}

	void operator() (UserBase * userBase) {
		if (userBase && userBase->isCanSend()) {
			userBase->send(mData, mAddSep);
		}
	}

}; // struct ufSend



/** Unary function for sending data to users with profile */
struct ufSendProfile : public unary_function<void, UserList::iterator> {
	const string & mData; /** Data for sending */
	unsigned long mProfile;
	bool mAddSep;

	ufSendProfile(const string & data, unsigned long profile, bool addSep) : 
		mData(data), mProfile(profile), mAddSep(addSep)
	{
	}

	ufSendProfile & operator = (const ufSendProfile &) {
		return *this;
	}

	void operator() (UserBase * userBase) {
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

}; // struct ufSendProfile



/** Unary function for sending data dataS + nick + dataE to each user */
struct ufSendWithNick : public unary_function<void, UserList::iterator> {
	const string &mDataStart, &mDataEnd; /** Data for sending */

	ufSendWithNick(const string & dataStart, const string & dataEnd) : 
		mDataStart(dataStart),
		mDataEnd(dataEnd)
	{
	}

	ufSendWithNick & operator = (const ufSendWithNick &) {
		return *this;
	}

	void operator() (UserBase * userBase) {
		if (userBase && userBase->isCanSend() && !userBase->uid().empty()) {
			userBase->send(mDataStart, false, false);
			userBase->send(userBase->uid(), false, false);
			userBase->send(mDataEnd, true);
		}
	}

}; // struct ufSendWithNick



/** Unary function for sending data dataS + nick + dataE to each user with profile */
struct ufSendWithNickProfile : public unary_function<void, UserList::iterator> {
	const string &mDataStart, &mDataEnd; /** Data for sending */
	unsigned long mProfile;

	ufSendWithNickProfile(const string & dataStart, const string & dataEnd, unsigned long profile) : 
		mDataStart(dataStart),
		mDataEnd(dataEnd),
		mProfile(profile)
	{
	}

	ufSendWithNickProfile & operator = (const ufSendWithNickProfile &) {
		return *this;
	}

	void operator() (UserBase * userBase) {
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

}; // struct ufSendWithNickProfile



UserList::UserList(const string & name, bool keepNickList) :
	Obj("UserList"),
	HashTable<UserBase *> (1024), // 1024 for big hubs and big check interval of resize
	mKeepNickList(keepNickList),
	mRemakeNextNickList(true),
	mName(name),
	mNickListMaker(mNickList)
{
}



const string & UserList::getNickList() {
	if (mRemakeNextNickList && mKeepNickList) {
		mNickListMaker.clear();
		for_each(begin(), end(), mNickListMaker);
		mRemakeNextNickList = false;
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
		if (log(4)) {
			logStream() << "sendToAll begin" << endl;
		}

		if (mCache.size()) {
			mCache.append(data);
			if (addSep) {
				if (mCache.find(NMDC_SEPARATOR, mCache.size() - NMDC_SEPARATOR_LEN, NMDC_SEPARATOR_LEN)) {
					mCache.append(NMDC_SEPARATOR);
				}
			}
			for_each(begin(), end(), ufSend(mCache, false));
			mCache.erase(0, mCache.size());
		} else {
			for_each(begin(), end(), ufSend(data, addSep));
		}

		if (log(4)) {
			logStream() << "sendToAll end" << endl;
		}
	} else {
		mCache.append(data);
		if (addSep) {
			if (mCache.find(NMDC_SEPARATOR, mCache.size() - NMDC_SEPARATOR_LEN, NMDC_SEPARATOR_LEN)) {
				mCache.append(NMDC_SEPARATOR);
			}
		}
	}
}



/** Sending data to profiles */
void UserList::sendToProfiles(unsigned long profile, const string & data, bool addSep) {
	if (log(4)) {
		logStream() << "sendToProfiles begin" << endl;
	}

	for_each(begin(), end(), ufSendProfile(data, profile, addSep));

	if (log(4)) {
		logStream() << "sendToProfiles end" << endl;
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
		string str;
		sendToAll(str, false, false);
	}
}



/** Redefining log level function */
bool UserList::strLog() {
	Obj::strLog();
	logStream() << "(" << size() << ")" << "[" << mName << "] ";
	return true;
}



void UserList::ufDoNickList::operator() (UserBase * userBase) {
	if (!userBase->hide() && !userBase->uid().empty()) {
		mList.append(userBase->uid());
		mList.append(mSep);
	}
}



void FullUserList::ufDoInfoList::operator() (UserBase * userBase) {
	if (!userBase->hide()) {
		msListComplete.append(userBase->myInfoString());
		msListComplete.append(mSep);
	}
}



void FullUserList::ufDoIpList::operator() (UserBase * userBase) {
	if (!userBase->hide() && userBase->ip().size() && !userBase->uid().empty()) {
		mList.append(userBase->uid());
		mList.append(" ", 1);
		mList.append(userBase->ip());
		mList.append(mSep);
	}
}



FullUserList::FullUserList(const string & name, bool keepNickList, bool keepInfoList, bool keepIpList) :
	UserList(name, keepNickList),
	mKeepInfoList(keepInfoList),
	mRemakeNextInfoList(true),
	mKeepIpList(keepIpList),
	mRemakeNextIpList(true),
	mInfoListMaker(mInfoListComplete),
	mIpListMaker(mIpList)
{
	setClassName("FullUserList");
}



const string & FullUserList::getInfoList() {
	if (mRemakeNextInfoList && mKeepInfoList) {
		mInfoListMaker.clear();
		for_each(begin(), end(), mInfoListMaker);
		mRemakeNextInfoList = false;
	}
	return mInfoListComplete;
}



const string & FullUserList::getIpList() {
	if (mRemakeNextIpList && mKeepIpList) {
		mIpListMaker.clear();
		for_each(begin(), end(), mIpListMaker);
		mRemakeNextIpList = false;
	}
	return mIpList;
}



void UserList::nmdcNickList(string & list, UserBase * userBase) {
	// $NickList nick1$$nick2$$
	if (!userBase->hide() && !userBase->uid().empty()) {
		list.append(userBase->uid());
		list.append("$$");
	}
}



void UserList::nmdcInfoList(string & list, UserBase * userBase) {
	// $MyINFO nick1 ...|$MyINFO nick2 ...|
	if (!userBase->hide()) {
		list.append(userBase->myInfoString());
		list.append(NMDC_SEPARATOR);
	}
}



void UserList::nmdcIpList(string & list, UserBase * userBase) {
	// $UserIP nick1 ip1$$nick2 ip2$$
	if (!userBase->hide() && userBase->ip().size() && !userBase->uid().empty()) {
		list.append(userBase->uid());
		list.append(" ", 1);
		list.append(userBase->ip());
		list.append("$$");
	}
}


}; // namespace dcserver

/**
 * $Id$
 * $HeadURL$
 */
