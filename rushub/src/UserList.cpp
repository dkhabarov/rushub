/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz
 *
 * modified: 27 Aug 2009
 * Copyright (C) 2009-2012 by Setuper
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

	void operator() (UserBase * userBase) {
		if (userBase && userBase->isCanSend()) {
			userBase->send(mData, mAddSep);
		}
	}

private:

	ufSend & operator = (const ufSend &) {
		return *this;
	}

}; // struct ufSend



/** Unary function for sending data to users with features */
struct ufSendFeature : public unary_function<void, UserList::iterator> {
	const string & mData; /** Data for sending */
	bool mAddSep;
	const vector<int> & mPositive;
	const vector<int> & mNegative;

	ufSendFeature(const string & data, bool addSep,
		const vector<int> & positive, const vector<int> & negative) : 
		mData(data),
		mAddSep(addSep),
		mPositive(positive),
		mNegative(negative)
	{
	}

	void operator() (UserBase * userBase) {
		if (userBase && userBase->isCanSend()) {
			bool canSend = true;
			for (unsigned int i = 0; i < mPositive.size(); ++i) {
				if (!userBase->hasFeature(mPositive[i])) {
					canSend = false;
					break;
				}
			}
			if (canSend) {
				for (unsigned int i = 0; i < mNegative.size(); ++i) {
					if (userBase->hasFeature(mNegative[i])) {
						canSend = false;
						break;
					}
				}
				if (canSend) {
					userBase->send(mData, mAddSep, false); // not flush
				}
			}
		}
	}

private:

	ufSendFeature & operator = (const ufSendFeature &) {
		return *this;
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

private:

	ufSendProfile & operator = (const ufSendProfile &) {
		return *this;
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

	void operator() (UserBase * userBase) {
		if (userBase && userBase->isCanSend() && !userBase->getUid().empty()) {
			userBase->send(mDataStart, false, false);
			userBase->send(userBase->getUid(), false, false);
			userBase->send(mDataEnd, true);
		}
	}

private:

	ufSendWithNick & operator = (const ufSendWithNick &) {
		return *this;
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

	void operator() (UserBase * userBase) {
		if (userBase && userBase->isCanSend()) { 
			int profile = userBase->getProfile() + 1;
			if (profile < 0) {
				profile = -profile;
			}
			if (profile > 31) {
				profile = (profile % 32) - 1;
			}
			if (mProfile & (1 << profile) && !userBase->getUid().empty()) {
				userBase->send(mDataStart, false, false);
				userBase->send(userBase->getUid(), false, false);
				userBase->send(mDataEnd, true);
			}
		}
	}

private:

	ufSendWithNickProfile & operator = (const ufSendWithNickProfile &) {
		return *this;
	}

}; // struct ufSendWithNickProfile



UserList::UserList(const string & name) :
	Obj("UserList"),
	HashTable<UserBase *> (1024), // 1024 for big hubs and big check interval of resize
	mName(name)
{
}



UserList::~UserList() {
	for (unsigned int i = 0; i < mListItems.size(); ++i) {
		delete mListItems[i];
	}
}



UserList::Key UserList::uidToLowerHash(const string & uid) {
	string tmp;
	tmp.resize(uid.size());
	::transform(uid.begin(), uid.end(), tmp.begin(), ::tolower);
	return mHash(tmp);
}



UserBase * UserList::getUserBaseByUid(const string & uid) {
	return find(static_cast<unsigned long> (uidToLowerHash(uid))); // for x64 compatibility
}



void UserList::addUserListItem(UserListItem::Func func, const char * start) {
	mListItems.push_back(new UserListItem(func, start));
}



const string & UserList::getList(int number) {
	return mListItems[number]->getList(begin(), end());
}



/**
 Sendind data to all users from the list
 data - sending data
 useCache - true - not send and save to cache, false - send data and send cache
 addSep - add sep to end of list
 */
void UserList::sendToAll(const string & data, bool useCache, bool addSep) {
	if (!useCache) {
		if (log(TRACE)) {
			logStream() << "sendToAll begin" << endl;
		}

		if (mCache.size()) {
			mCache.reserve(mCache.size() + data.size() + NMDC_SEPARATOR_LEN);
			mCache.append(data);
			if (addSep) {
				if (mCache.find(NMDC_SEPARATOR, mCache.size() - NMDC_SEPARATOR_LEN, NMDC_SEPARATOR_LEN)) {
					mCache.append(NMDC_SEPARATOR);
				}
			}
			for_each(begin(), end(), ufSend(mCache, false));
			string().swap(mCache); // erase & free memory
		} else {
			for_each(begin(), end(), ufSend(data, addSep));
		}

		if (log(TRACE)) {
			logStream() << "sendToAll end" << endl;
		}
	} else {
		mCache.reserve(mCache.size() + data.size() + NMDC_SEPARATOR_LEN);
		mCache.append(data);
		if (addSep) {
			if (mCache.find(NMDC_SEPARATOR, mCache.size() - NMDC_SEPARATOR_LEN, NMDC_SEPARATOR_LEN)) {
				mCache.append(NMDC_SEPARATOR);
			}
		}
	}
}



void UserList::sendToAllAdc(const string & data, bool useCache, bool addSep) {
	if (!useCache) {
		if (log(TRACE)) {
			logStream() << "sendToAll begin" << endl;
		}

		if (mCache.size()) {
			mCache.reserve(mCache.size() + data.size() + ADC_SEPARATOR_LEN);
			mCache.append(data);
			if (addSep) {
				if (mCache.find(ADC_SEPARATOR, mCache.size() - ADC_SEPARATOR_LEN, ADC_SEPARATOR_LEN)) {
					mCache.append(ADC_SEPARATOR);
				}
			}
			for_each(begin(), end(), ufSend(mCache, false));
			string().swap(mCache); // erase & free memory
		} else {
			for_each(begin(), end(), ufSend(data, addSep));
		}

		if (log(TRACE)) {
			logStream() << "sendToAll end" << endl;
		}
	} else {
		mCache.reserve(mCache.size() + data.size() + ADC_SEPARATOR_LEN);
		mCache.append(data);
		if (addSep) {
			if (mCache.find(ADC_SEPARATOR, mCache.size() - ADC_SEPARATOR_LEN, ADC_SEPARATOR_LEN)) {
				mCache.append(ADC_SEPARATOR);
			}
		}
	}
}



void UserList::sendToFeature(const string & data, const vector<int> & positive, 
		const vector<int> & negative, bool addSep) {

	if (log(TRACE)) {
		logStream() << "sendToFeature begin" << endl;
	}

	for_each(begin(), end(), ufSendFeature(data, addSep, positive, negative));

	if (log(TRACE)) {
		logStream() << "sendToFeature end" << endl;
	}
}



/** Sending data to profiles */
void UserList::sendToProfiles(unsigned long profile, const string & data, bool addSep) {
	if (log(TRACE)) {
		logStream() << "sendToProfiles begin" << endl;
	}

	for_each(begin(), end(), ufSendProfile(data, profile, addSep));

	if (log(TRACE)) {
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



void UserList::flushCacheAdc() {
	if (mCache.size()) {
		string str;
		sendToAllAdc(str, false, false);
	}
}



/** Redefining log level function */
bool UserList::strLog() {
	Obj::strLog();
	simpleLogStream() << "[" << mName << ":" << size() << "] ";
	return true;
}



void UserList::onAdd(UserBase * userBase) {
	for (unsigned int i = 0; i < mListItems.size(); ++i) {
		mListItems[i]->add(userBase);
	}
}



void UserList::onRemove(UserBase *) {
	for (unsigned int i = 0; i < mListItems.size(); ++i) {
		mListItems[i]->remake();
	}
}



void UserList::onResize(size_t & currentSize, size_t & oldCapacity, size_t & newCapacity) {
	if (log(DEBUG)) {
		logStream() << "Autoresizing: size = " << currentSize << 
		", capacity = " << oldCapacity << " -> " << newCapacity << endl;
	}
}



UserList & UserList::operator = (const UserList &) {
	return *this;
}


}; // namespace dcserver

/**
 * $Id$
 * $HeadURL$
 */
