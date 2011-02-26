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

#ifndef HASH_LIST_MAP_H
#define HASH_LIST_MAP_H

#include "HashTable.h"
#include "Obj.h"

#include <list>
#include <map>
#include <string>

using namespace ::std;

namespace utils {

/** Hash map with list */
template <class V, class K = unsigned long>
class HashMap : public Obj {

public:

	typedef list<V, allocator<V> > List_t; /** Iteration list with data */
	typedef typename List_t::iterator iterator; /** Iterator for the data-list */
	typedef map<K, iterator> tHashMap; /** Map [K]=iterator */
	typedef typename tHashMap::iterator tUHIt; /** Iterator for tHashMap */
	typedef pair<K, iterator> tHashPair; /** pair (for insert) */

	Hash<K> mHash; /** Hash function */

private:

	List_t mList; /** List with data */
	tHashMap mHashMap; /** HashMap */

public:

	HashMap() : Obj("HashMap") {}
	~HashMap() {}
	size_t Size() const { return mHashMap.size(); } /** Number element in container HashMap */

	iterator begin() { return mList.begin(); } /** Befin iterator of data list */
	iterator end() { return mList.end(); } /** End iterator of data list */

	bool Add(const K & hash, V Data); /** Adding data and hash-key (true - ok, false - fail) */
	bool Remove(const K & hash); /** Removing data by hash-key (true - ok, false - fail) */
	bool Contain(const K & hash); /** Check existed this hash-key (true - yes, false - no) */
	V Find(const K & hash); /** Return data by hash-key. Return val else NULL */
	virtual void OnAdd(V) {};
	virtual void OnRemove(V) {};

}; // HashMap


/** Adding data and hash-key (true - ok, false - fail) */
template <class V, class K>
bool HashMap<V, K>::Add(const K & hash, V Data) {

	/** Check */
	if(Contain(hash)) { if(Log(1)) LogStream() << "Hash " << hash << " is contains already" << endl; return false; }

	/** Insert data */
	iterator ulit = mList.insert(mList.begin(), Data); /** Insert in begin list */
	if(ulit == mList.end()) { if(Log(1)) LogStream() << "Don't add " << hash << " into the list" << endl; return false; }

	/** Insert hash-key */
	pair<tUHIt, bool> P = mHashMap.insert(tHashPair(hash, ulit));
	if(P.second) {
		OnAdd(Data);
		if(Log(4)) LogStream() << "Added: " << hash << " (size: " << mHashMap.size() << ")" << endl;
	} else {
		if(Log(1)) LogStream() << "Don't add " << hash << endl;
		mList.erase(ulit); /** Removing data fron list */
		return false;
	}
	return true;
}

/** Removing data by hash-key (true - ok, false - fail) */
template <class V, class K>
bool HashMap<V, K>::Remove(const K & hash) {
	tUHIt uhit = mHashMap.find(hash);
	if( uhit != mHashMap.end()) {
		OnRemove(*(uhit->second));
		mList.erase(uhit->second); /** Remove data */
		mHashMap.erase(uhit); /** Remove key */
		if(Log(4)) LogStream() << "Removed: " << hash << " (size: " << mHashMap.size() << ")" << endl;
		return true;
	}
	if(Log(3)) LogStream() << "Don't exist: " << hash << endl;
	return false;
}

/** Check existed this hash-key (true - yes, false - no) */
template <class V, class K>
bool HashMap<V, K>::Contain(const K & hash) {
	return mHashMap.find(hash) != mHashMap.end();
}

/** Return data by hash-key. Return val else NULL */
template <class V, class K>
V HashMap<V, K>::Find(const K & hash) {
	tUHIt uhit = mHashMap.find(hash);
	if(uhit != mHashMap.end()) return *(uhit->second); /** Pointer to iterator = val */
	return NULL;
}

}; // namespace utils

#endif // HASH_LIST_MAP_H
