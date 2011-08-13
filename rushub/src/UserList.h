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
			mList.erase(0, mList.size());
			mList.append(mStart);
			for_each(begin, end, mMaker);
			mRemake = false;
		}
		return mList;
	}

private:

	struct Maker : public unary_function<void, I> {
		Func mFunc;
		string & mList;

		Maker(Func func, string & list) : mFunc(func), mList(list) {
		}

		void operator() (T t) {
			mFunc(mList, t);
		}

	private:
		Maker & operator = (const Maker &) {
			return *this;
		}
	}; // struct ListMaker

	string mList;
	string mStart;

	bool mRemake;

	Maker mMaker;

private:

	ListItem & operator = (const ListItem &) {
		return *this;
	}

}; // class ListItem



/// User List Item for quickly get list
typedef ListItem<HashTable<UserBase *>::iterator, UserBase *> UserListItem;



/** The structure, allowing add and delete users. Quick iterations cycle for sending */
class UserList : public Obj, public HashTable<UserBase *> {

public:

	explicit UserList(const string & name);
	virtual ~UserList();

	static Key uidToLowerHash(const string & uid);

	UserBase * getUserBaseByUid(const string & uid);

	void addUserListItem(UserListItem::Func func, const char * start = "");
	const string & getList(int number = 0);

	inline void remake() {
		onRemove(NULL);
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

	virtual void onAdd(UserBase * userBase);
	virtual void onRemove(UserBase *);
	virtual void onResize(size_t & currentSize, size_t & oldCapacity, size_t & newCapacity);

private:

	string mName; ///< Name of list
	string mCache;

	vector<UserListItem*> mListItems;

private:

	UserList & operator = (const UserList &);

}; // UserList


}; // namespace dcserver

#endif // USER_LIST_H

/**
 * $Id$
 * $HeadURL$
 */
