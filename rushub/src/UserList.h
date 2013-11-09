/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz
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

#ifndef USER_LIST_H
#define USER_LIST_H

//#define WITHOUT_PLUGINS 1

#include "Obj.h"
#include "HashTable.h"
#include "Plugin.h" // for NMDC_SEPARATOR
#include "Mutex.h"

#include <string>
#include <functional>
#include <algorithm> // for ::std::transform
#include <vector>

using namespace ::std;
using namespace ::utils; // HashTable, Obj


namespace dcserver {

class UserBase;

namespace protocol {
	class DcCmd;
}

using namespace ::dcserver::protocol;



/// Base template for user list
template <class I, class T>
class ListItem {

public:

	typedef void (*Func)(string &, T);

public:

	ListItem(Func func, const char * start = "") : 
		mStart(start),
		mRemake(true),
		mMaker(func, mList)
	{
	}

	void add(T t) {
		if(!mRemake) {
			mMaker(t);
		}
	}

	void remake() {
		mRemake = true;
	}

	const string & getList(I begin, I end) {
		if (mRemake) {
			mList.erase();
			mList.append(mStart);
			for_each(begin, end, mMaker);
			mRemake = false;
		}
		return mList;
	}

private:

	/// Maker for list
	struct Maker : public unary_function<void, I> {
		Func mFunc;
		string & mList;

		Maker(Func func, string & list) : mFunc(func), mList(list) {
		}

		void operator() (T t) {
			mFunc(mList, t);
		}

		const Maker & operator = (const Maker &) { // for_each
			return *this;
		}
	}; // struct ListMaker

	string mList;
	string mStart;

	bool mRemake;

	Maker mMaker;

private:

	ListItem(const ListItem<I, T> &);
	ListItem<I, T> & operator = (const ListItem<I, T> &);

}; // class ListItem



/// User List Item for quickly get list
typedef ListItem<HashTable<UserBase *>::iterator, UserBase *> UserListItem;



/** The structure, allowing add and delete users. Quick iterations cycle for sending */
class UserList : public Obj, protected HashTable<UserBase *> {

public:

	explicit UserList(const string & name);
	virtual ~UserList();

	size_t size() const;
	void clear();
	bool add(const unsigned long & key, UserBase * data);
	bool remove(const unsigned long & key);
	bool contain(const unsigned long & key);
	UserBase * find(const unsigned long & key);
	bool update(const unsigned long & key, UserBase * const & data);
	bool autoResize();

	/** Exec func for each */
	template<class F>
	void doForEach(F f, bool notLock = false) {
		if (notLock) {
			iterator it = begin();
			iterator it_e = end();
			while (it != it_e) {
				f(*it++);
			}
		} else {
			Mutex::Lock l(mMutex); // sync
			iterator it = begin();
			iterator it_e = end();
			while (it != it_e) {
				f(*it++);
			}
		}
	}

	static Key toLowerHash(const string & nick);

	UserBase * getUserBaseByNick(const string & nick);

	void addUserListItem(UserListItem::Func func, const char * start = "");
	const string & getList(unsigned int number = 0);

	/** Remake */
	void remake();

	/** Sending data to all from the list. Adds separator to end of string, if it does not have. */
	void sendToAll(DcCmd *, bool flush = true, unsigned long profiles = 0);

	/** Sending data to all from the list */
	void sendToAll(const string & data, bool addSep = true, bool flush = true);

	/** Sending data to all from the list */
	void sendToAllAdc(const string & data, bool addSep = true, bool flush = true);

	/** Sending data to users by features (sync down) */
	void sendToFeature(const string & data, const vector<unsigned int> & positive, 
		const vector<unsigned int> & negative, bool addSep = true);

	/** Sending data to profiles (sync down) */
	void sendToProfiles(unsigned long profile, const string & data, bool addSep = true);

	/** Sending to all profiles chat (sync down) */
	void sendToAllChat(const string & data, const string & nick, unsigned long profiles = 0);

	/** Sending to all profiles pm (sync down) */
	void sendToAllPm(const string & data, const string & nick, const string & from, unsigned long profiles = 0);

	/** Sending data from cache to user */
	void flushForUser(UserBase * userBase);

	/** Sending data from cache to all and clear cache */
	void flushCache();

protected:

	Mutex mMutex;

protected:

	typedef HashTable<UserBase *> tParent;

	virtual bool strLog(int level, ostream & os); ///< Redefining log level function

	virtual void onAdd(UserBase * userBase);
	virtual void onRemove(UserBase *);
	virtual void onResize(const size_t & currentSize, const size_t & oldCapacity, const size_t & newCapacity);

private:

	string mName; ///< Name of list
	string mCache[DC_PROTOCOL_TYPE_SIZE];

	vector<UserListItem*> mListItems;

private:

	void sendToAll(const string & data, bool addSep, bool flush, string & cache, const char * sep, size_t sepLen);
	static void addInCache(string & cache, const string & data, const char * sep, size_t sepLen, bool addSep = true);

	UserList(const UserList &);
	UserList & operator = (const UserList &);

}; // class UserList


} // namespace dcserver

#endif // USER_LIST_H

/**
 * $Id$
 * $HeadURL$
 */
