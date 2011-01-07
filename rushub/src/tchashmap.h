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

#ifndef TCHASHLISTMAP_H
#define TCHASHLISTMAP_H

#include "tchashtable.h"
#include "cobj.h"

#include <list>
#include <map>
#include <string>

using namespace std;

namespace nUtils {

/** Hash map with list */
template <class tDataType, class tKeyType = unsigned long> class tcHashMap : public cObj {
public:
	typedef list<tDataType, allocator<tDataType> > List_t; /** Iteration list with data */
	typedef typename List_t::iterator iterator; /** Iterator for the data-list */
	typedef map<tKeyType, iterator> tHashMap; /** Map [tKeyType]=iterator */
	typedef typename tHashMap::iterator tUHIt; /** Iterator for tHashMap */
	typedef pair<tKeyType, iterator> tHashPair; /** pair (for insert) */

	tcHash<tKeyType> mHash; /** Hash function */

private:
	List_t mList; /** List with data */
	tHashMap mHashMap; /** HashMap */

public:
	tcHashMap() : cObj("tcHashMap") {}
	~tcHashMap() {}
	size_t Size() const { return mHashMap.size(); } /** Number element in container HashMap */

	iterator begin() { return mList.begin(); } /** Befin iterator of data list */
	iterator end() { return mList.end(); } /** End iterator of data list */

	bool Add(const tKeyType &Hash, tDataType Data); /** Adding data and hash-key (true - ok, false - fail) */
	bool Remove(const tKeyType &Hash); /** Removing data by hash-key (true - ok, false - fail) */
	bool Contain(const tKeyType &Hash); /** Check existed this hash-key (true - yes, false - no) */
	tDataType Find(const tKeyType &Hash); /** Return data by hash-key. Return val else NULL */
	virtual void OnAdd(tDataType) {};
	virtual void OnRemove(tDataType) {};

}; // tcHashMap


/** Adding data and hash-key (true - ok, false - fail) */
template <class tDataType, class tKeyType>
bool tcHashMap<tDataType, tKeyType>::Add(const tKeyType &Hash, tDataType Data) {
	/** Check */
	if(Contain(Hash)) { if(Log(1)) LogStream() << "Hash " << Hash << " is contains already" << endl; return false; }

	/** Insert data */
	iterator ulit = mList.insert(mList.begin(), Data); /** Insert in begin list */
	if(ulit == mList.end()) { if(Log(1)) LogStream() << "Don't add " << Hash << " into the list" << endl; return false; }

	/** Insert hash-key */
	pair<tUHIt, bool> P = mHashMap.insert(tHashPair(Hash, ulit));
	if(P.second) {
		OnAdd(Data);
		if(Log(4)) LogStream() << "Added: " << Hash << endl;
	} else {
		if(Log(1)) LogStream() << "Don't add " << Hash << endl;
		mList.erase(ulit); /** Removing data fron list */
		return false;
	}
	return true;
}

/** Removing data by hash-key (true - ok, false - fail) */
template <class tDataType, class tKeyType>
bool tcHashMap<tDataType, tKeyType>::Remove(const tKeyType &Hash) {
	tUHIt uhit = mHashMap.find(Hash);
	if( uhit != mHashMap.end()) {
		OnRemove(*(uhit->second));
		mList.erase(uhit->second); /** Remove data */
		mHashMap.erase(uhit); /** Remove key */
		if(Log(4)) LogStream() << "Removed: " << Hash << endl;
		return true;
	}
	if(Log(3)) LogStream() << "Don't exist: " << Hash << endl;
	return false;
}

/** Check existed this hash-key (true - yes, false - no) */
template <class tDataType, class tKeyType>
bool tcHashMap<tDataType, tKeyType>::Contain(const tKeyType &Hash) {
	return mHashMap.find(Hash) != mHashMap.end();
}

/** Return data by hash-key. Return val else NULL */
template <class tDataType, class tKeyType>
tDataType tcHashMap<tDataType, tKeyType>::Find(const tKeyType &Hash) {
	tUHIt uhit = mHashMap.find(Hash);
	if(uhit != mHashMap.end()) return *(uhit->second); /** Pointer to iterator = val */
	return NULL;
}

}; // nUtils

#endif // TCHASHMAP_H
