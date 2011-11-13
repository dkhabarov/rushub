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

#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <iostream>
#include <string.h>
#ifdef _WIN32
	#include <basetsd.h>
#else
	#define UINT_PTR unsigned long
#endif

using namespace ::std;

namespace utils {



/// Hash method
template <class T = unsigned long> struct Hash {
	T operator() (const char * s) const {
		T h = 0;
		for(; *s; ++s) {
			h = 33 * h + *s;
		}
		return h;
	}
	inline T operator() (const string & s) const {
		return this->operator() (s.c_str());
	}
};



/// Usual array data
template <class V> class Array {

protected:

	size_t mCapacity; /** Reserved number cell (element of the array) */
	size_t mSize; /** Number contributed not zero element in array */
	V * mData; /** Array data */

public:

	Array(size_t capacity = 1024);

	virtual ~Array();

	/** Returns number not zero element in array (mSize) */
	virtual size_t size() const {
		return mSize;
	}

	/** Returns reserved number a cell (mCapacity) */
	virtual size_t capacity() const {
		return mCapacity;
	}

	/** Inserts not zero data in array. Returns NULL in the event of successful charting, 
	    otherwise returns data, which were already contributed earlier in cell with this number */
	virtual V insert(size_t num, const V data = 0);

	/** Updates not zero data in cell with specified by number. 
	    Returns old data or NULL */
	virtual V update(size_t num, const V data);

	/** Deletes not zero data from specified cells of the array. 
	    Returns remote data or NULL in the event of their absences */
	virtual V remove(size_t num);

	/** Returns data specified cells of the array or NULL in the event of their absences */
	virtual V find(size_t num);


	/** Iterator for container */
	struct iterator {
		V * mData;
		size_t i;
		size_t end;

		iterator() : mData((V *) NULL), i(0), end(0) {
		}

		iterator(V * Data, size_t _i, size_t _end) : mData(Data), i(_i), end(_end) {
		}

		iterator & operator = (const iterator & it) {
			mData = it.mData;
			i = it.i;
			end = it.end;
			return *this;
		}

		iterator(const iterator & it) {
			(*this) = it;
		}

		inline bool operator == (const iterator & it) {
			return i == it.i;
		}

		inline bool operator != (const iterator & it) {
			return i != it.i;
		}

		iterator & operator ++() {
			while (++i != end && mData[i] == (V)NULL) {
			}
			return *this;
		}

		V operator * () {
			if (i < end && mData) {
				return mData[i];
			}
			return NULL;
		}

		inline bool isEnd() const {
			return i >= end;
		}
	};

	/** Initial iterator */
	iterator begin() {
		iterator begin_it(mData, 0, mCapacity);
		if (mData[0] == (V)NULL) {
			++begin_it;
		}
		return begin_it;
	}

	/** Final iterator */
	iterator end() {
		return iterator(mData, mCapacity, mCapacity);
	}

}; // Array


template <class V> Array<V>::Array(size_t capacity) : mCapacity(capacity), mSize(0) {
	mData = new V[mCapacity];
	memset(mData, 0, sizeof(V) * mCapacity);
}

template <class V> Array<V>::~Array() {
	if (mData) {
		delete [] mData;
		mData = NULL;
	}
};

/** Inserts not zero data in array. Returns NULL in the event of successful charting, 
    otherwise returns data, which were already contributed earlier in cell with this number */
template <class V> V Array<V>::insert(size_t num, const V data) {
	num %= mCapacity;
	V oldData = mData[num];
	if (!oldData && data) {
		mData[num] = data;
		++mSize;
	}
	return oldData;
}

/** Updates not zero data in cell with specified by number. 
    Returns old data or NULL */
template <class V> V Array<V>::update(size_t num, const V data) {
	num %= mCapacity;
	V oldData = mData[num];
	if (oldData && data) {
		mData[num] = data;
		return oldData;
	}
	return (V) NULL;
}

/** Deletes not zero data from specified cells of the array. 
    Returns remote data or NULL in the event of their absences */
template <class V> V Array<V>::remove(size_t num) {
	num %= mCapacity;
	V oldData = mData[num];
	if (oldData) {
		mData[num] = (V)NULL;
		--mSize;
	}
	return oldData;
}

/** Returns data specified cells of the array or NULL in the event of their absences */
template <class V> V Array<V>::find(size_t num) {
	return mData[num % mCapacity];
}



/** List with one communication (struct: key, val, next). The key is unique! */
template <class K, class V>
class List {

public:

	K mKey; /** Key */
	V mData; /** Data */
	List * mNext; /** Pointer on the next element */

public:

	/** Constructor */
	List(K key = (K)NULL, V data = (V)NULL, List * next = NULL) :
		mKey(key), mData(data), mNext(next)
	{
	}

	virtual ~List() {
		if (mNext != NULL) {
			delete mNext;
			mNext = NULL;
		}
	}

	/** Size of list */
	size_t size() const {
		size_t i = 1;
		List * it = mNext;
		while (it != NULL) {
			it = it->mNext;
			++i;
		}
		return i;
	}

	/** The Accompaniment data and key in list. 
	    Returns the added data or data for existing key.
	    The Data can be repeated, key is unique! */
	V add(K key, V data) {
		if (mKey == key) {
			return mData; /** Element with such key already exists in list */
		}
		List *it = mNext, *prev = this;
		while ((it != NULL) && (it->mKey != key)) {
			prev = it;
			it = it->mNext;
		}
		if (it == NULL) { /** Nothing have not found. Add in list key and data */
			prev->mNext = new List(key, data);
			return data;
		}
		return it->mData; /** Something have found. Return that found */
	}

	/** Update data in list with specified by key. 
	    Returns previous key data or NULL */
	V update(K key, const V & value) {
		V data = NULL;
		if (mKey == key) { /** Update the first element of the list */
			data = mData;
			mData = value;
			return data;
		}
		List * it = mNext;
		while ((it != NULL) && (it->mKey != key)) {
			it = it->mNext;
		}
		if (it != NULL) { /** Something have found. Update */
			data = it->mData;
			it->mData = value;
			return data;
		}
		return data; /** Have not found element with such key */
	}

	/** Removing the element of the list. 
	    Returns data of removed element or NULL.
	    Start - a pointer on initial element, received after removing. */
	V remove(K key, List *& start) {
		if (mKey == key) { /** First element of the list */
			start = mNext;
			mNext = NULL;
			return mData;
		}
		V data = (V)NULL;
		List *it = mNext, *prev = this;

		while ((it != NULL) && (it->mKey != key)) {
			prev = it;
			it = it->mNext;
		}
		if (it != NULL) { /** Removing the element from list */
			data = it->mData;
			prev->mNext = it->mNext;
			it->mNext = NULL;
			delete it;
			it = NULL;
		}
		return data;
	}

	/** Finding data by key */
	V find(K key) {
		if (mKey == key) {
			return mData; /** Have found in first element of the list */
		}
		List * it = mNext;
		while ((it != NULL ) && (it->mKey != key)) {
			it = it->mNext;
		}
		if (it != NULL) {
			return it->mData; /** Have found and return */
		}
		return (V)NULL; /** Nothing have not found */
	}
}; // List


/** Hash table or hash array (Array from list with hash) */
template <class V>
class HashTable {

public:

	typedef UINT_PTR Key;
	class iterator;
	static Hash<Key> mHash; /** Hash function */

private:

	typedef List<Key, V> tItem;

public:

	/** Data type - array of tItem* */
	typedef Array<tItem*> tData;

protected:

	tData * mData; /** Array with 1024 elements (on default) with type tItem* */
	size_t mSize; /** Size */
	bool mIsResizing; /** Resizing flag */

public:

	HashTable(size_t capacity = 1024) : 
		mSize(0),
		mIsResizing(false)
	{
		mData = new tData(capacity);
	}

	~HashTable() {
		clear();
		delete mData;
		mData = NULL;
	}

	/** Size */
	inline size_t size() const {
		return mSize;
	}

	/** Clear */
	void clear() {
		tItem * item = NULL;
		for (size_t it = 0; it < mData->capacity(); ++it) {
			item = mData->find(it); /** Find data by iterator */
			if (item != NULL) {
				delete item; /** Remove list */
			}
			mData->remove(it); /** Remove element */
		}
		mSize = 0;
	}

	/** Adds not zero data and key.
	Returns true if successful accompaniment data and key (key is unique) */
	bool add(const Key & key, V data) {
		if (data == (V)NULL) {
			return false; /** No data */
		}
		size_t hash = key % mData->capacity(); /** Get hash */
		tItem *items, *item = NULL;
		items = mData->find(hash); /** Get cell data of the array */
		if (items == NULL) { /** Check presence of the list in cell of the array */

			item = new tItem(key, data); /** Create new list with data and key */
			mData->insert(hash, item); /** Insert created list in cell of the array */

			if (!mIsResizing) {
				onAdd(data);
				++mSize;
			}
			return true;
		}
		/** Add data in existing list of the cell of the array */
		if (data == items->add(key, data)) {
			if (!mIsResizing) {
				onAdd(data);
				++mSize;
			}
			return true;
		}
		return false;
	}

	/** Returns true if successful removing */
	bool remove(const Key & key) {
		size_t hash = key % mData->capacity(); /** Get hash */
		tItem *item = NULL, *items = mData->find(hash); /** Get cell data of the array */
		if (items == NULL) { /** Check presence of the list in cell of the array */
			return false;
		}
		item = items;
		V data = items->remove(key, item); /** Removing from list */
		if (item != items) { /** Checking for removing the first element */
			if (item) {
				mData->update(hash, item); /** Update list in cell */
			} else {
				mData->remove(hash); /** Removing cell */
			}
			delete items; /** Removing the old start element of the list */
			items = NULL;
		}
		if (!mIsResizing && (V)NULL != data) { /** Removing has occurred */
			onRemove(data);
			--mSize;
			return true;
		}
		return false;
	}

	/** Contain by key */
	bool contain(const Key & key) {
		tItem * items = mData->find(key % mData->capacity()); /** Get cell data of the array */
		if (items == NULL) { /** Check presence of the list in cell of the array */
			return false;
		}
		return ((V)NULL != items->find(key)); /** Search key in list */
	}

	/** Find by key */
	V find(const Key & key) {
		tItem * items = mData->find(key % mData->capacity()); /** Get cell data of the array */
		if (items == NULL) { /** Check presence of the list in cell of the array */
			return (V)NULL;
		}
		return items->find(key); /** Search data by key */
	}

	/** Update data (not NULL) to key. Returns true if successful */
	bool update(const Key & key, const V & data) {
		if (data == (V)NULL) {
			return false; /** No data */
		}
		tItem * items = mData->find(key % mData->capacity());
		if (items == NULL) { /** Check presence of the list in cell of the array */
			return false;
		}
		return (items->update(key, data) != (V)NULL); /** Update data in list */
	}


	/** Iterator through all NON-NULL elements of the container */
	class iterator 
	#ifdef _WIN32
		#if _MSC_VER < 1600
		: public std::_Iterator_with_base<std::forward_iterator_tag, V> // for_each algorithm
		#else
		: public std::iterator<std::forward_iterator_tag, V> // for_each algorithm
		#endif
	#endif
	{
	public:
		typename tData::iterator i; /** Iterator of array (type tItem*) */
		tItem * mItem; /** Pointer to array element */

		iterator() : mItem(NULL) {
		}

		iterator(typename tData::iterator it) : i(it) {
			if (!i.isEnd()) {
				mItem = *i;
			} else {
				mItem = NULL;
			}
		}

		iterator & operator = (const iterator & it) {
			mItem = it.mItem;
			i = it.i;
			return *this;
		}

		iterator(const iterator &it) {
			(*this) = it;
		}

		inline bool operator == (const iterator & it) {
			return mItem == it.mItem;
		}

		inline bool operator != (const iterator & it) {
			return mItem != it.mItem;
		}

		iterator & operator ++ () {
			if ((mItem != NULL) && (mItem->mNext != NULL)) {
				mItem = mItem->mNext;
			} else {
				++i;
				if (!i.isEnd()) {
					mItem = (*i);
				} else {
					mItem = NULL;
				}
			}
			return *this;
		}

		iterator operator ++ (int) {
			iterator it = *this;
			this->operator ++();
			return it;
		}

		inline bool isEnd() const {
			return mItem == NULL;
		}
		
		V operator * () {
			return mItem->mData;
		}
	}; // iterator

	iterator begin() {
		return iterator(mData->begin());
	}

	iterator end() {
		static iterator end_it;
		return end_it;
	}

	/** autoResize */
	bool autoResize() {
		size_t capacity = mData->capacity();
		if ((mSize > (capacity << 1)) || (((mSize << 1) + 1) < capacity)) {
			resize(mSize + (mSize >> 1) + 1);
			return true;
		}
		return false;
	}

protected:

	virtual void onAdd(V) {
	}

	virtual void onRemove(V) {
	}

	virtual void onResize(size_t & /* currentSize */, size_t & /* oldCapacity */, size_t & /* newCapacity */) {
	}

private:

	/** Resize */
	size_t resize(size_t newCapacity) {
		tData * newData = new tData(newCapacity);
		tData * oldData = mData;

		size_t oldCapacity = mData->capacity();
		size_t size = mSize;

		mIsResizing = true; // Begin resizing

		Key key(0); // Previous key
		iterator it = begin();
		mData = newData;
		while (!it.isEnd()) {
			add(it.mItem->mKey, it.mItem->mData); // Adding
			key = it.mItem->mKey; // Save key for next del
			++it;

			// Removing old data
			mData = oldData;
			remove(key);
			mData = newData;
		}
		delete oldData;

		mIsResizing = false; // End resizing

		onResize(size, oldCapacity, newCapacity);
		return 0;
	}

}; // HashTable

template <class V>
Hash<UINT_PTR> HashTable<V>::mHash;

}; // namespace utils

#endif // HASH_TABLE_H

/**
 * $Id$
 * $HeadURL$
 */
