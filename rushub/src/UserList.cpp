/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz
 *
 * modified: 27 Aug 2009
 * Copyright (C) 2009-2013 by Setuper
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
#include "DcCmd.h"

#include <algorithm>

namespace dcserver {


/** Unary function for sending data to users with profile */
struct ufSendTmp : public unary_function<void, HashTable<UserBase *>::iterator> {
	const string & mData; /** Data for sending */
	const unsigned long * mProfile;
	bool mAddSep;

	ufSendTmp(const string & data, bool addSep, const unsigned long * profile = NULL) : 
		mData(data), mProfile(profile), mAddSep(addSep)
	{
	}

	void operator() (UserBase * userBase) {
		if (userBase && userBase->isCanSend()) {
			if (mProfile) {
				int profile = userBase->getProfile() + 1;
				if (profile < 0) {
					profile = -profile;
				}
				if (profile > 31) {
					profile = (profile % 32) - 1;
				}
				if ((*mProfile) & static_cast<unsigned long> (1 << profile)) {
					userBase->send(mData, mAddSep);
				}
			} else {
				userBase->send(mData, mAddSep);
			}
		}
	}

	const ufSendTmp & operator = (const ufSendTmp &) { // for_each
		mAddSep = false;
		mProfile = NULL;
		return *this;
	}
}; // struct ufSendTmp



/** Unary function for sending data to users */
struct ufSend : public unary_function<void, HashTable<UserBase *>::iterator> {
	string * mData; /** Data for sending */
	const unsigned long * mProfile;
	bool mAddSep;

	ufSend(string * data, bool addSep, const unsigned long * profile = NULL) : 
		mData(data),
		mProfile(profile),
		mAddSep(addSep)
	{
	}

	void operator() (UserBase * userBase) {
		if (userBase && userBase->isCanSend()) {
			if (mProfile) {
				int profile = userBase->getProfile() + 1;
				if (profile < 0) {
					profile = -profile;
				}
				if (profile > 31) {
					profile = (profile % 32) - 1;
				}
				if ((*mProfile) & static_cast<unsigned long> (1 << profile)) {
					userBase->send(mData[userBase->getProtocolType()], mAddSep);
				}
			} else {
				userBase->send(mData[userBase->getProtocolType()], mAddSep);
			}
		}
	}

	const ufSend & operator = (const ufSend &) { // for_each
		mAddSep = false;
		mProfile = NULL;
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



/** Unary function for sending chat data to each user with profile */
struct ufSendChat : public unary_function<void, HashTable<UserBase *>::iterator> {
	const string & mData;
	const string & mNick;
	const unsigned long * mProfile;

	ufSendChat(const string & data, const string & nick, const unsigned long * profile = NULL) : 
		mData(data),
		mNick(nick),
		mProfile(profile)
	{
	}

	void operator() (UserBase * userBase) {
		if (userBase && userBase->isCanSend()) {
			if (mProfile) {
				int profile = userBase->getProfile() + 1;
				if (profile < 0) {
					profile = -profile;
				}
				if (profile > 31) {
					profile = (profile % 32) - 1;
				}
				if ((*mProfile) & static_cast<unsigned long> (1 << profile) && !userBase->getNick().empty()) {
					userBase->sendToChat(mData, mNick, true);
				}
			} else if (!userBase->getNick().empty()) {
				userBase->sendToChatAll(mData, mNick, true);
			}
		}
	}

	const ufSendChat & operator = (const ufSendChat &) { // for_each
		mProfile = NULL;
		return *this;
	}

}; // struct ufSendChat



/** Unary function for sending pm data to each user with profile */
struct ufSendPm : public unary_function<void, HashTable<UserBase *>::iterator> {
	const string & mData;
	const string & mNick;
	const string & mFrom;
	const unsigned long * mProfile;

	ufSendPm(const string & data, const string & nick, const string & from, const unsigned long * profile = NULL) : 
		mData(data),
		mNick(nick),
		mFrom(from),
		mProfile(profile)
	{
	}

	void operator() (UserBase * userBase) {
		if (userBase && userBase->isCanSend()) { 
			if (mProfile) {
				int profile = userBase->getProfile() + 1;
				if (profile < 0) {
					profile = -profile;
				}
				if (profile > 31) {
					profile = (profile % 32) - 1;
				}
				if ((*mProfile) & static_cast<unsigned long> (1 << profile) && !userBase->getNick().empty()) {
					userBase->sendToPm(mData, mNick, mFrom, true);
				}
			} else if (!userBase->getNick().empty()) {
				userBase->sendToPm(mData, mNick, mFrom, true);
			}
		}
	}

	const ufSendPm & operator = (const ufSendPm &) { // for_each
		mProfile = NULL;
		return *this;
	}

}; // struct ufSendPm



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



UserList::Key UserList::toLowerHash(const string & str) {
	string tmp;
	tmp.resize(str.size());
	::transform(str.begin(), str.end(), tmp.begin(), ::tolower);
	return mHash(tmp);
}



UserBase * UserList::getUserBaseByNick(const string & nick) {
	return find(static_cast<unsigned long> (toLowerHash(nick))); // for x64 compatibility
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
 Sendind data to all users from the list.
 Adds separator to end of string, if it does not have!
 DcCmd - sending cnd
 flush - false - not send and save to cache, true - send data and send cache
 */
void UserList::sendToAll(DcCmd * dcCmd, bool flush, const unsigned long * profile) {
	Mutex::Lock l(mMutex); // sync
	if (profile) {
		LOG(LEVEL_TRACE, "sendToProfiles begin");
		string s[2] = {
			dcCmd->getChunk1(DC_PROTOCOL_TYPE_NMDC),
			dcCmd->getChunk1(DC_PROTOCOL_TYPE_ADC)
		};
		for_each(begin(), end(), ufSend(s, false, profile));
		LOG(LEVEL_TRACE, "sendToProfiles end");
	} else if (flush) {
		LOG(LEVEL_TRACE, "sendToAll begin");
		for (int i = 0; i < DC_PROTOCOL_TYPE_SIZE; ++i) {
			addInCache(mCache[i], dcCmd->getChunk1(i), DcProtocol::getSeparator(i), DcProtocol::getSeparatorLen(i));
		}

		for_each(begin(), end(), ufSend(mCache, false, profile));

		for (int i = 0; i < DC_PROTOCOL_TYPE_SIZE; ++i) {
			string().swap(mCache[i]); // erase & free memory
		}
		LOG(LEVEL_TRACE, "sendToAll end");
	} else {
		for (int i = 0; i < DC_PROTOCOL_TYPE_SIZE; ++i) {
			addInCache(mCache[i], dcCmd->getChunk1(i), DcProtocol::getSeparator(i), DcProtocol::getSeparatorLen(i));
		}
	}
}



/**
 Sendind data to all NMDC users from the list
 data - sending data
 flush - false - not send and save to cache, true - send data and send cache
 addSep - add sep to end of list
 */
void UserList::sendToAll(const string & data, bool addSep, bool flush) {
	Mutex::Lock l(mMutex); // sync
	sendToAll(data, addSep, flush, mCache[DC_PROTOCOL_TYPE_NMDC], STR_LEN(NMDC_SEPARATOR));
}



/**
 Sendind data to all ADC users from the list
 data - sending data
 flush - false - not send and save to cache, true - send data and send cache
 addSep - add sep to end of list
 */
void UserList::sendToAllAdc(const string & data, bool addSep, bool flush) {
	Mutex::Lock l(mMutex); // sync
	sendToAll(data, addSep, flush, mCache[DC_PROTOCOL_TYPE_ADC], STR_LEN(ADC_SEPARATOR));
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
	doForEach(ufSendTmp(data, addSep, &profile));
	LOG(LEVEL_TRACE, "sendToProfiles end");
}



/** Sending to all profiles chat (sync down) */
void UserList::sendToAllChat(const string & data, const string & nick, const unsigned long * profile) {
	doForEach(ufSendChat(data, nick, profile));
}



/** Sending to all profiles pm (sync down) */
void UserList::sendToAllPm(const string & data, const string & nick, const string & from, const unsigned long * profile) {
	doForEach(ufSendPm(data, nick, from, profile));
}



/** Flush user cache */
void UserList::flushForUser(UserBase * userBase) {
	Mutex::Lock l(mMutex); // sync
	bool send = false;
	for (int i = 0; i < DC_PROTOCOL_TYPE_SIZE; ++i) {
		if (mCache[i].size()) {
			send = true;
			break;
		}
	}
	if (send) {
		ufSend(mCache, false).operator() (userBase);
	}
}



/** Flush common cache */
void UserList::flushCache() {
	if (mCache[DC_PROTOCOL_TYPE_NMDC].size()) {
		string str;
		sendToAll(str, false, true);
	}
	if (mCache[DC_PROTOCOL_TYPE_ADC].size()) {
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
			for_each(begin(), end(), ufSendTmp(cache, false));
			string().swap(cache); // erase & free memory
		} else {
			for_each(begin(), end(), ufSendTmp(data, addSep));
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
