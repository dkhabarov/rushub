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

#include <list>
#include <map>
#include <string>

using namespace ::std;

namespace utils {

/** Hash map with list */
template <class V, class K = unsigned long>
class HashMap {

public:

	/// Iterator for the data-list
	typedef typename list<V, allocator<V> >::iterator iterator;

	Hash<K> mHash; ///< Hash function

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

private:

	typedef map<K, iterator> tHashMap; ///< Map [K]=iterator

	list<V, allocator<V> > mList; ///< List with data
	tHashMap mHashMap; ///< HashMap

}; // HashMap


/** Adding data and hash-key (true - ok, false - fail) */
template <class V, class K>
bool HashMap<V, K>::add(const K & hash, V data) {

	// Check
	if (contain(hash)) {
		return false;
	}

	// Insert data
	iterator it = mList.insert(mList.begin(), data);
	if (it == mList.end()) {
		return false;
	}

	// Insert hash-key
	pair<tHashMap::iterator, bool> p = mHashMap.insert(pair<K, iterator>(hash, it));
	if (p.second) {
		onAdd(data);
	} else {
		mList.erase(it); // Removing data fron list
		return false;
	}
	return true;
}

/** Removing data by hash-key (true - ok, false - fail) */
template <class V, class K>
bool HashMap<V, K>::remove(const K & hash) {
	tHashMap::iterator it = mHashMap.find(hash);
	if (it != mHashMap.end()) {
		onRemove(*(it->second));
		mList.erase(it->second); // Remove data 
		mHashMap.erase(it); // Remove key
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
	tHashMap::const_iterator it = mHashMap.find(hash);
	if (it != mHashMap.end()) {
		return *(it->second); // Pointer to iterator = val
	}
	return NULL;
}

}; // namespace utils

#endif // HASH_LIST_MAP_H

/**
 * $Id$
 * $HeadURL$
 */
