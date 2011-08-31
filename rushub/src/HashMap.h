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
class HashMap {

public:

	typedef list<V, allocator<V> > List_t; /** Iteration list with data */
	typedef typename List_t::iterator iterator; /** Iterator for the data-list */
	typedef map<K, iterator> tHashMap; /** Map [K]=iterator */
	typedef pair<K, iterator> tHashPair; /** pair (for insert) */

	Hash<K> mHash; /** Hash function */

private:

	List_t mList; /** List with data */
	tHashMap mHashMap; /** HashMap */

public:

	HashMap() {
	}

	~HashMap() {
	}

	/** Number element in container HashMap */
	inline size_t size() const {
		return mHashMap.size();
	}

	/** Befin iterator of data list */
	inline iterator begin() {
		return mList.begin();
	}

	/** End iterator of data list */
	inline iterator end() {
		return mList.end();
	}

	/** Adding data and hash-key (true - ok, false - fail) */
	bool add(const K & hash, V Data);

	/** Removing data by hash-key (true - ok, false - fail) */
	bool remove(const K & hash);

	/** Check existed this hash-key (true - yes, false - no) */
	bool contain(const K & hash) const;

	/** Return data by hash-key. Return val else NULL */
	V find(const K & hash) const;

	virtual void onAdd(V) {
	}

	virtual void onRemove(V) {
	}

}; // HashMap


/** Adding data and hash-key (true - ok, false - fail) */
template <class V, class K>
bool HashMap<V, K>::add(const K & hash, V Data) {

	// Check
	if (contain(hash)) {
		return false;
	}

	// Insert data
	iterator ulit = mList.insert(mList.begin(), Data); // Insert in begin list
	if (ulit == mList.end()) {
		return false;
	}

	// Insert hash-key
	pair<tHashMap::iterator, bool> P = mHashMap.insert(tHashPair(hash, ulit));
	if (P.second) {
		onAdd(Data);
	} else {
		mList.erase(ulit); // Removing data fron list
		return false;
	}
	return true;
}

/** Removing data by hash-key (true - ok, false - fail) */
template <class V, class K>
bool HashMap<V, K>::remove(const K & hash) {
	tHashMap::iterator uhit = mHashMap.find(hash);
	if (uhit != mHashMap.end()) {
		onRemove(*(uhit->second));
		mList.erase(uhit->second); // Remove data 
		mHashMap.erase(uhit); // Remove key
		return true;
	}
	return false;
}

/** Check existed this hash-key (true - yes, false - no) */
template <class V, class K>
bool HashMap<V, K>::contain(const K & hash) const {
	return mHashMap.find(hash) != mHashMap.end();
}

/** Return data by hash-key. Return val else NULL */
template <class V, class K>
V HashMap<V, K>::find(const K & hash) const {
	tHashMap::const_iterator uhit = mHashMap.find(hash);
	if (uhit != mHashMap.end()) {
		return *(uhit->second); // Pointer to iterator = val
	}
	return NULL;
}

}; // namespace utils

#endif // HASH_LIST_MAP_H

/**
 * $Id$
 * $HeadURL$
 */
