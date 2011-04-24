/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz

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

class DcUser;
class UserBase;



/** The structure, allowing add and delete users. Quick iterations cycle for sending */
class UserList : public Obj, public HashTable<UserBase *> {

friend class DcServer;

public:

	/** Unary function for sending data to users */
	struct ufSend : public unary_function<void, iterator> {
		string & mData; /** Data for sending */

		ufSend(string & data) : mData(data) {
		}

		ufSend & operator = (const ufSend &) {
			return *this;
		}

		void operator() (UserBase *); /** Sending operator */
	};

	/** Unary function for sending data to users with profile */
	struct ufSendProfile : public unary_function<void, iterator> {
		string & mData; /** Data for sending */
		unsigned long mProfile;

		ufSendProfile(string & data, unsigned long profile) : mData(data), mProfile(profile) {
		}

		ufSendProfile & operator = (const ufSendProfile &) {
			return *this;
		}

		void operator() (UserBase *); /** Sending operator */
	};

	/** Unary function for sending data sDataS+sNick+sDataE to each user */
	struct ufSendWithNick : public unary_function<void, iterator> {
		string &mDataStart, &mDataEnd; /** Data for sending */

		ufSendWithNick(string & dataStart, string & dataEnd) : 
			mDataStart(dataStart),
			mDataEnd(dataEnd)
		{
		}

		ufSendWithNick & operator = (const ufSendWithNick &) {
			return *this;
		}

		void operator() (UserBase *); /** Sending operator */
	};

	/** Unary function for sending data sDataS+sNick+sDataE to each user with profile */
	struct ufSendWithNickProfile : public unary_function<void, iterator> {
		string &mDataStart, &mDataEnd; /** Data for sending */
		unsigned long mProfile;

		ufSendWithNickProfile(string & dataStart, string & dataEnd, unsigned long profile) : 
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
		string & msList; /** List */
		string msStart; /** Prefix */
		string msSep; /** Separator */

		ufDoNickList(string & sList) : msList(sList){
		}

		virtual ~ufDoNickList() {
		}

		virtual ufDoNickList & operator = (const ufDoNickList &) {
			return *this;
		}

		virtual void clear() { /** Clear var and record prefix */
			msList.erase(0, msList.size());
			msList.append(msStart.c_str(), msStart.size());
		}

		virtual void operator() (UserBase *);
	};

private:

	typedef HashTable<UserBase *> List_t;

	string mName;
	string msCache;

	string msNickList;
	ufDoNickList mNickListMaker;

protected:

	bool mbKeepNickList;
	bool mbRemakeNextNickList;
	bool mbOptRemake; /** Flag of the absence of the change between remake */

public:

	explicit UserList(string sName, bool bKeepNickList = false);

	virtual ~UserList() {
	}

	virtual UserList & operator = (const UserList &) {
		return *this;
	}

	Key nick2Key(const string & nick) {
		string key;
		key.resize(nick.length());
		::transform(nick.begin(), nick.end(), key.begin(), ::tolower);
		return List_t::mHash(key);
	}

	UserBase * getUserBaseByNick(const string & nick) {
		return List_t::find(nick2Key(nick));
	}

	UserBase * getUserBaseByKey(const Key & key) {
		return List_t::find(key);
	}

	bool containsNick(const string & nick) {
		return List_t::contain(nick2Key(nick));
	}

	bool containsKey(const Key &key) {
		return List_t::contain(key);
	}

	bool addWithNick(const string & nick, UserBase * userBase) {
		return List_t::add(nick2Key(nick), userBase);
	}

	bool addWithKey(const Key & key, UserBase * userBase) {
		return List_t::add(key, userBase);
	}

	bool removeByNick(const string & nick) {
		return List_t::remove(nick2Key(nick));
	}

	bool removeByKey(const Key & key) {
		return List_t::remove(key);
	}

	bool add(UserBase * userBase);

	bool remove(UserBase * userBase);

	virtual string & getNickList();

	void setNickListStart(const char * start) {
		mNickListMaker.msStart = start;
	}

	void setNickListSeparator(const char * sep) {
		mNickListMaker.msSep = sep;
	}
	
	void remake() {
		mbRemakeNextNickList = (!mbOptRemake);
	}

	/** Sending data to all from the list */
	void sendToAll(const string & data, bool useCache = false, bool addSep = true);

	/** Sending data to profiles */
	void sendToProfiles(unsigned long profile, const string & data, bool addSep = true);

	/** Sending data sStart+Nick+sEnd to all list */
	void sendToWithNick(string & start, string & end);

	/** Sending data sStart+Nick+sEnd to profiles */
	void sendToWithNick(string & start, string & end, unsigned long profile);

	/** Sending data from cache to user */
	void flushForUser(UserBase * userBase);

	/** Sending data from cache to all and clear cache */
	void flushCache();

	bool autoResize() {
		unsigned size, capacity, newSize;
		if (List_t::autoResize(size, capacity, newSize) && Log(3)) {
			LogStream() << "Autoresizing: size = " << size << 
			", capacity = " << capacity << " -> " << newSize << endl;
			return true;
		}
		return false;
	}

	/** onAdd */
	virtual void onAdd(UserBase * userBase) {
		if (!mbRemakeNextNickList && mbKeepNickList) {
			mNickListMaker(userBase);
		}
	}
	/** onRemove */
	virtual void onRemove(UserBase *) {
		mbRemakeNextNickList = mbKeepNickList;
		mbOptRemake = false;
	}

	/** Redefining log level function */
	virtual bool strLog();

}; // UserList


/** Full user list */
class FullUserList : public UserList {

public:

	/** Unary function for constructing MyINFO list */
	struct ufDoINFOList : public UserList::ufDoNickList {
		string & msListComplete;
		ufDoINFOList(string & sList, string & sListComplete) :
			ufDoNickList(sList),
			msListComplete(sListComplete)
		{
			msSep = NMDC_SEPARATOR;
			msStart = "";
		}

		virtual ~ufDoINFOList() {
		}

		virtual ufDoINFOList & operator = (const ufDoINFOList &) {
			return *this;
		}

		virtual void clear() { /** Clear var and record prefix */
			ufDoNickList::clear();
			msListComplete.erase(0, msListComplete.size());
			msListComplete.append(msStart.data(), msStart.size());
		}

		virtual void operator() (UserBase *);
	};

	/** Unary function for constructing ip-list */
	struct ufDoIpList : public UserList::ufDoNickList {
		ufDoIpList(string & sList) : ufDoNickList(sList) {
			msSep = "$$";
			msStart = "$UserIP ";
		}

		virtual ~ufDoIpList() {
		}

		virtual ufDoIpList & operator = (const ufDoIpList &) {
			return *this;
		}

		virtual void operator() (UserBase *);
	};

private:

	string msINFOList;
	string msINFOListComplete;
	string msIpList;
	ufDoINFOList mINFOListMaker;
	ufDoIpList mIpListMaker;

protected:

	bool mbKeepInfoList;
	bool mbRemakeNextInfoList;

	bool mbKeepIpList;
	bool mbRemakeNextIpList;

	string msCompositeNickList;
	string msCompositeINFOList;

public:

	explicit FullUserList(string sName, bool bKeepNickList = false, bool bKeepInfoList = false, bool bKeepIpList = false);

	virtual ~FullUserList() {
	}

	virtual string & getNickList();
	virtual string & getInfoList(bool bComplete = false);
	virtual string & getIpList();

	inline void remake() {
		if (!mbOptRemake) {
			mbRemakeNextNickList = mbRemakeNextInfoList = mbRemakeNextIpList = true;
		} else {
			mbRemakeNextNickList = mbRemakeNextInfoList = mbRemakeNextIpList = false;
		}
	}


	/** onAdd */
	virtual void onAdd(UserBase * userBase) {
		UserList::onAdd(userBase);
		if(!mbRemakeNextInfoList && mbKeepInfoList) mINFOListMaker(userBase);
		if(!mbRemakeNextIpList && mbKeepIpList) mIpListMaker(userBase);
	}

	/** onRemove */
	virtual void onRemove(UserBase * userBase) {
		UserList::onRemove(userBase);
		mbRemakeNextInfoList = mbKeepInfoList;
		mbRemakeNextIpList = mbKeepIpList;
	}
	
}; // FullUserList

}; // namespace dcserver

#endif // USER_LIST_H
