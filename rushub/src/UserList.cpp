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
struct ufSend : public unary_function<void, HashTable<UserBase *>::iterator> {
	const string & mData; /** Data for sending */
	bool mAddSep;

	ufSend(const string & data, bool addSep) : mData(data), mAddSep(addSep) {
	}

	void operator() (UserBase * userBase) {
		if (userBase && userBase->isCanSend()) {
			userBase->send(mData, mAddSep);
		}
	}

	const ufSend & operator = (const ufSend &) { // for_each
		mAddSep = false;
		return *this;
	}

}; // struct ufSend



/** Unary function for sending data to users with features */
struct ufSendFeature : public unary_function<void, HashTable<UserBase *>::iterator> {
	const string & mData; /** Data for sending */
	bool mAddSep;
	const vector<unsigned int> & mPositive;
	const vector<unsigned int> & mNegative;

	ufSendFeature(const string & data, bool addSep,
		const vector<unsigned int> & positive, const vector<unsigned int> & negative) : 
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

	const ufSendFeature & operator = (const ufSendFeature &) { // for_each
		mAddSep = false;
		return *this;
	}

}; // struct ufSendFeature



/** Unary function for sending data to users with profile */
struct ufSendProfile : public unary_function<void, HashTable<UserBase *>::iterator> {
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
			if (mProfile & static_cast<unsigned long> (1 << profile)) {
				userBase->send(mData, mAddSep);
			}
		}
	}

	const ufSendProfile & operator = (const ufSendProfile &) { // for_each
		mProfile = 0;
		mAddSep = false;
		return *this;
	}

}; // struct ufSendProfile



/** Unary function for sending chat data to each user */
struct ufSendChat : public unary_function<void, HashTable<UserBase *>::iterator> {
	const string & mData;
	const string & mUid;

	ufSendChat(const string & data, const string & uid) : 
		mData(data),
		mUid(uid)
	{
	}

	void operator() (UserBase * userBase) {
		if (userBase && userBase->isCanSend() && !userBase->getUid().empty()) {
			userBase->sendToChatAll(mData, mUid, true);
		}
	}

	const ufSendChat & operator = (const ufSendChat &) { // for_each
		return *this;
	}

}; // struct ufSendChat



/** Unary function for sending chat data to each user with profile */
struct ufSendChatProfile : public unary_function<void, HashTable<UserBase *>::iterator> {
	const string & mData;
	const string & mUid;
	unsigned long mProfile;

	ufSendChatProfile(const string & data, const string & uid, unsigned long profile) : 
		mData(data),
		mUid(uid),
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
			if (mProfile & static_cast<unsigned long> (1 << profile) && !userBase->getUid().empty()) {
				userBase->sendToChat(mData, mUid, true);
			}
		}
	}

	const ufSendChatProfile & operator = (const ufSendChatProfile &) { // for_each
		mProfile = 0;
		return *this;
	}

}; // struct ufSendChatProfile



/** Unary function for sending pm data to each user */
struct ufSendPm : public unary_function<void, HashTable<UserBase *>::iterator> {
	const string & mData;
	const string & mUid;
	const string & mFrom;

	ufSendPm(const string & data, const string & uid, const string & from) : 
		mData(data),
		mUid(uid),
		mFrom(from)
	{
	}

	void operator() (UserBase * userBase) {
		if (userBase && userBase->isCanSend() && !userBase->getUid().empty()) {
			userBase->sendToPm(mData, mUid, mFrom, true);
		}
	}

	const ufSendPm & operator = (const ufSendPm &) { // for_each
		return *this;
	}

}; // struct ufSendPm



/** Unary function for sending pm data to each user with profile */
struct ufSendPmProfile : public unary_function<void, HashTable<UserBase *>::iterator> {
	const string & mData;
	const string & mUid;
	const string & mFrom;
	unsigned long mProfile;

	ufSendPmProfile(const string & data, const string & uid, const string & from, unsigned long profile) : 
		mData(data),
		mUid(uid),
		mFrom(from),
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
			if (mProfile & static_cast<unsigned long> (1 << profile) && !userBase->getUid().empty()) {
				userBase->sendToPm(mData, mUid, mFrom, true);
			}
		}
	}

	const ufSendPmProfile & operator = (const ufSendPmProfile &) { // for_each
		mProfile = 0;
		return *this;
	}

}; // struct ufSendPmProfile



UserList::UserList(const string & name) :
	Obj("UserList"),
	HashTable<UserBase *> (1024), // 1024 for big hubs and big check interval of resize
	mName(name)
{
}



UserList::~UserList() {
	Mutex::Lock l(mMutex); // sync
	for (size_t i = 0; i < mListItems.size(); ++i) {
		delete mListItems[i];
	}
}



size_t UserList::size() const {
	return tParent::size();
}



void UserList::clear() {
	Mutex::Lock l(mMutex); // sync
	tParent::clear();
}



bool UserList::add(const unsigned long & key, UserBase * data) {
	Mutex::Lock l(mMutex); // sync
	return tParent::add(key, data);
}



bool UserList::remove(const unsigned long & key) {
	Mutex::Lock l(mMutex); // sync
	return tParent::remove(key);
}



bool UserList::contain(const unsigned long & key) {
	Mutex::Lock l(mMutex); // sync
	return tParent::contain(key);
}



UserBase * UserList::find(const unsigned long & key) {
	Mutex::Lock l(mMutex); // sync
	return tParent::find(key);
}



bool UserList::update(const unsigned long & key, UserBase * const & data) {
	Mutex::Lock l(mMutex); // sync
	return tParent::update(key, data);
}



bool UserList::autoResize() {
	Mutex::Lock l(mMutex); // sync
	return tParent::autoResize();
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
	Mutex::Lock l(mMutex); // sync
	mListItems.push_back(new UserListItem(func, start));
}



const string & UserList::getList(unsigned int number) {
	Mutex::Lock l(mMutex); // sync
	return mListItems[number]->getList(begin(), end());
}



/** Remake */
void UserList::remake() {
	Mutex::Lock l(mMutex); // sync
	onRemove(NULL);
}



/**
 Sendind data to all NMDC users from the list
 data - sending data
 flush - false - not send and save to cache, true - send data and send cache
 addSep - add sep to end of list
 */
void UserList::sendToAll(const string & data, bool addSep, bool flush) {
	Mutex::Lock l(mMutex); // sync
	sendToAll(data, addSep, flush, mCacheNmdc, STR_LEN(NMDC_SEPARATOR));
}



/**
 Sendind data to all ADC users from the list
 data - sending data
 flush - false - not send and save to cache, true - send data and send cache
 addSep - add sep to end of list
 */
void UserList::sendToAllAdc(const string & data, bool addSep, bool flush) {
	Mutex::Lock l(mMutex); // sync
	sendToAll(data, addSep, flush, mCacheAdc, STR_LEN(ADC_SEPARATOR));
}



/** Sending data to users by features (sync down) */
void UserList::sendToFeature(const string & data, const vector<unsigned int> & positive, 
		const vector<unsigned int> & negative, bool addSep) {

	LOG(LEVEL_TRACE, "sendToFeature begin");
	doForEach(ufSendFeature(data, addSep, positive, negative));
	LOG(LEVEL_TRACE, "sendToFeature end");
}



/** Sending data to profiles (sync down) */
void UserList::sendToProfiles(unsigned long profile, const string & data, bool addSep) {
	LOG(LEVEL_TRACE, "sendToProfiles begin");
	doForEach(ufSendProfile(data, profile, addSep));
	LOG(LEVEL_TRACE, "sendToProfiles end");
}



/** Use for chat send to all (sync down) */
void UserList::sendToAllChat(const string & data, const string & uid) {
	doForEach(ufSendChat(data, uid));
}



/** Sending to all profiles chat (sync down) */
void UserList::sendToAllChat(const string & data, const string & uid, unsigned long profile) {
	doForEach(ufSendChatProfile(data, uid, profile));
}



/** Use for private send to all (sync down) */
void UserList::sendToAllPm(const string & data, const string & uid, const string & from) {
	doForEach(ufSendPm(data, uid, from));
}



/** Sending to all profiles pm (sync down) */
void UserList::sendToAllPm(const string & data, const string & uid, const string & from, unsigned long profile) {
	doForEach(ufSendPmProfile(data, uid, from, profile));
}



/** Flush user cache */
void UserList::flushForUser(UserBase * userBase) {
	Mutex::Lock l(mMutex); // sync
	if (mCacheNmdc.size()) {
		ufSend(mCacheNmdc, false).operator() (userBase);
	}
	if (mCacheAdc.size()) {
		ufSend(mCacheAdc, false).operator() (userBase);
	}
}



/** Flush common cache */
void UserList::flushCache() {
	if (mCacheNmdc.size()) {
		string str;
		sendToAll(str, false, true);
	}
	if (mCacheAdc.size()) {
		string str;
		sendToAllAdc(str, false, true);
	}
}



/** Redefining log level function */
bool UserList::strLog(int level, ostream & os) {
	Obj::strLog(level, os);
	os << "[" << mName << ":" << size() << "] ";
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
	LOG(LEVEL_DEBUG, "Autoresizing: size = " << currentSize << 
		", capacity = " << oldCapacity << " -> " << newCapacity);
}



void UserList::sendToAll(const string & data, bool addSep, bool flush, string & cache, const char * sep, size_t sepLen) {
	if (flush) {
		LOG(LEVEL_TRACE, "sendToAll begin");

		if (!cache.empty()) {
			addInCache(cache, data, sep, sepLen, addSep);
			for_each(begin(), end(), ufSend(cache, false));
			string().swap(cache); // erase & free memory
		} else {
			for_each(begin(), end(), ufSend(data, addSep));
		}

		LOG(LEVEL_TRACE, "sendToAll end");
	} else {
		addInCache(cache, data, sep, sepLen, addSep);
	}
}



void UserList::addInCache(string & cache, const string & data, const char * sep, size_t sepLen, bool addSep /*= true*/) {
	cache.reserve(cache.size() + data.size() + sepLen);
	cache.append(data);
	if (addSep) {
		if (cache.find(sep, cache.size() - sepLen, sepLen)) {
			cache.append(sep, sepLen);
		}
	}
}


} // namespace dcserver

/**
 * $Id$
 * $HeadURL$
 */
