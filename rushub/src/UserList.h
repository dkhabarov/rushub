/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz
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

#ifndef USER_LIST_H
#define USER_LIST_H

//#define WITHOUT_PLUGINS 1

#include "Obj.h"
#include "HashTable.h"
#include "Plugin.h" // for NMDC_SEPARATOR

#include <string>
#include <functional>
#include <algorithm> // for ::std::transform
#include <vector>

using namespace ::std;
using namespace ::utils; /** for HashTable */


namespace dcserver {

class UserBase;



/** The structure, allowing add and delete users. Quick iterations cycle for sending */
class UserList : public Obj, public HashTable<UserBase *> {

public:

	/** Unary function for sending data to users */
	struct ufSend : public unary_function<void, iterator> {
		const string & mData; /** Data for sending */
		bool mAddSep;

		ufSend(const string & data, bool addSep) : mData(data), mAddSep(addSep) {
		}

		ufSend & operator = (const ufSend &) {
			return *this;
		}

		void operator() (UserBase *); /** Sending operator */
	};

	/** Unary function for sending data to users with profile */
	struct ufSendProfile : public unary_function<void, iterator> {
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

		void operator() (UserBase *); /** Sending operator */
	};

	/** Unary function for sending data dataS + nick + dataE to each user */
	struct ufSendWithNick : public unary_function<void, iterator> {
		const string &mDataStart, &mDataEnd; /** Data for sending */

		ufSendWithNick(const string & dataStart, const string & dataEnd) : 
			mDataStart(dataStart),
			mDataEnd(dataEnd)
		{
		}

		ufSendWithNick & operator = (const ufSendWithNick &) {
			return *this;
		}

		void operator() (UserBase *); /** Sending operator */
	};

	/** Unary function for sending data dataS + nick + dataE to each user with profile */
	struct ufSendWithNickProfile : public unary_function<void, iterator> {
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

		void operator() (UserBase *); /** Sending operator */
	};



	/** Unary function for constructing nick-list */
	struct ufDoNickList : public unary_function<void, iterator> {
		string & mList; /** List */
		string mStart; /** Prefix */
		string mSep; /** Separator */

		ufDoNickList(string & list) : mList(list){
		}

		virtual ~ufDoNickList() {
		}

		virtual ufDoNickList & operator = (const ufDoNickList &) {
			return *this;
		}

		virtual void clear() { /** Clear var and record prefix */
			mList.erase(0, mList.size());
			mList.append(mStart);
		}

		virtual void operator() (UserBase *);
	};

public:

	explicit UserList(const string & name, bool keepNickList = false);

	virtual ~UserList() {
	}

	virtual UserList & operator = (const UserList &) {
		return *this;
	}

	static Key nick2Key(const string & nick) {
		string key;
		key.resize(nick.length());
		::transform(nick.begin(), nick.end(), key.begin(), ::tolower);
		return mHash(key);
	}

	UserBase * getUserBaseByNick(const string & nick) {
		return List_t::find(nick2Key(nick));
	}

	virtual const string & getNickList();

	void setNickListStart(const char * start) {
		mNickListMaker.mStart = start;
	}

	void setNickListSeparator(const char * sep) {
		mNickListMaker.mSep = sep;
	}
	
	void remake() {
		mRemakeNextNickList = (!mOptRemake);
	}

	/** Sending data to all from the list */
	void sendToAll(const string & data, bool useCache = false, bool addSep = true);

	/** Sending data to profiles */
	void sendToProfiles(unsigned long profile, const string & data, bool addSep = true);

	/** Sending data sStart+Nick+sEnd to all list */
	void sendWithNick(const string & dataStart, const string & dataEnd);

	/** Sending data sStart+Nick+sEnd to profiles */
	void sendWithNick(const string & dataStart, const string & dataEnd, unsigned long profile);

	/** Sending data from cache to user */
	void flushForUser(UserBase * userBase);

	/** Sending data from cache to all and clear cache */
	void flushCache();

	/** Redefining log level function */
	virtual bool strLog();

protected:

	string mNickList;
	bool mKeepNickList;
	bool mRemakeNextNickList;
	bool mOptRemake; /** Flag of the absence of the change between remake */

protected:

	virtual void onAdd(UserBase * userBase) {
		if (!mRemakeNextNickList && mKeepNickList) {
			mNickListMaker(userBase);
		}
	}

	virtual void onRemove(UserBase *) {
		mRemakeNextNickList = mKeepNickList;
		mOptRemake = false;
	}

	virtual void onResize(size_t & currentSize, size_t & oldCapacity, size_t & newCapacity) {
		if (Log(3)) {
			LogStream() << "Autoresizing: size = " << currentSize << 
			", capacity = " << oldCapacity << " -> " << newCapacity << endl;
		}
	}

private:

	typedef HashTable<UserBase *> List_t;

	string mName; ///< Name of list
	string mCache;

	ufDoNickList mNickListMaker;


}; // UserList


/** Full user list */
class FullUserList : public UserList {

public:

	/** Unary function for constructing MyINFO list */
	struct ufDoINFOList : public UserList::ufDoNickList {
		string & msListComplete;
		ufDoINFOList(string & list, string & listComplete) :
			ufDoNickList(list),
			msListComplete(listComplete)
		{
			mSep = NMDC_SEPARATOR;
			mStart = "";
		}

		virtual ~ufDoINFOList() {
		}

		virtual ufDoINFOList & operator = (const ufDoINFOList &) {
			return *this;
		}

		virtual void clear() { /** Clear var and record prefix */
			ufDoNickList::clear();
			msListComplete.erase(0, msListComplete.size());
			msListComplete.append(mStart);
		}

		virtual void operator() (UserBase *);
	};

	/** Unary function for constructing ip-list */
	struct ufDoIpList : public UserList::ufDoNickList {
		ufDoIpList(string & list) : ufDoNickList(list) {
			mSep = "$$";
			mStart = "$UserIP ";
		}

		virtual ~ufDoIpList() {
		}

		virtual ufDoIpList & operator = (const ufDoIpList &) {
			return *this;
		}

		virtual void operator() (UserBase *);
	};

public:

	explicit FullUserList(const string & name, bool keepNickList = false, bool keepInfoList = false, bool keepIpList = false);

	virtual ~FullUserList() {
	}

	virtual FullUserList & operator = (const FullUserList &) {
		return *this;
	}

	virtual const string & getNickList();
	virtual const string & getInfoList(bool complete = false);
	virtual const string & getIpList();

	inline void remake() {
		if (!mOptRemake) {
			mRemakeNextNickList = mRemakeNextInfoList = mRemakeNextIpList = true;
		} else {
			mRemakeNextNickList = mRemakeNextInfoList = mRemakeNextIpList = false;
		}
	}


protected:

	bool mKeepInfoList;
	bool mRemakeNextInfoList;

	bool mKeepIpList;
	bool mRemakeNextIpList;

protected:

	virtual void onAdd(UserBase * userBase) {
		UserList::onAdd(userBase);
		if(!mRemakeNextInfoList && mKeepInfoList) mInfoListMaker(userBase);
		if(!mRemakeNextIpList && mKeepIpList) mIpListMaker(userBase);
	}

	virtual void onRemove(UserBase * userBase) {
		UserList::onRemove(userBase);
		mRemakeNextInfoList = mKeepInfoList;
		mRemakeNextIpList = mKeepIpList;
	}

private:

	string mInfoList;
	string mInfoListComplete;
	string mIpList;
	ufDoINFOList mInfoListMaker;
	ufDoIpList mIpListMaker;


}; // FullUserList

}; // namespace dcserver

#endif // USER_LIST_H

/**
 * $Id$
 * $HeadURL$
 */
