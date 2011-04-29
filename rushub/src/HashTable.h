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

template <class T = unsigned long> struct Hash {
	T operator() (const char * s) const {
		T h = 0;
		for(; *s; ++s) h = 33 * h + *s;
		return h;
	}
	inline T operator() (const string & s) const {
		return this->operator() (s.c_str());
	}
};

/** Usual array data */
template <class V> class Array {

protected:

	size_t miCapacity; /** Reserved number cell (element of the array) */
	size_t miSize; /** Number contributed not zero element in array */
	V *mData; /** Array data */

public:

	Array(size_t iCapacity = 1024);

	virtual ~Array();

	/** Returns number not zero element in array (miSize) */
	virtual size_t size() const {
		return miSize;
	}

	/** Returns reserved number a cell (miCapacity) */
	virtual size_t capacity() const {
		return miCapacity;
	}

	/** Inserts not zero data in array. Returns NULL in the event of successful charting, 
	    otherwise returns data, which were already contributed earlier in cell with this number */
	virtual V insert(size_t iNum, const V Data = 0);

	/** Updates not zero data in cell with specified by number. 
	    Returns old data or NULL */
	virtual V update(size_t iNum, const V Data);

	/** Deletes not zero data from specified cells of the array. 
	    Returns remote data or NULL in the event of their absences */
	virtual V remove(size_t iNum);

	/** Returns data specified cells of the array or NULL in the event of their absences */
	virtual V find(size_t iNum);


	/** Iterator for container */
	struct iterator {
		V * mData;
		size_t i;
		size_t end;

		iterator() : mData((V*)NULL), i(0), end(0) {
		}

		iterator(V * Data, size_t _i, size_t _end) : mData(Data), i(_i), end(_end) {
		}

		iterator & operator = (const iterator &it) {
			mData = it.mData;
			i = it.i;
			end = it.end;
			return *this;
		}

		iterator(const iterator &it) {
			(*this) = it;
		}

		inline bool operator == (const iterator &it) {
			return i == it.i;
		}

		inline bool operator != (const iterator &it) {
			return i != it.i;
		}

		iterator & operator ++() {
			while ((++i != end) && (mData[i] == (V)NULL)) {
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
		iterator begin_it(mData, 0, miCapacity);
		if(mData[0] == (V)NULL) {
			++begin_it;
		}
		return begin_it;
	}

	/** Final iterator */
	iterator end() {
		return iterator(mData, miCapacity, miCapacity);
	}

}; // Array


template <class V> Array<V>::Array(size_t iCapacity) : miCapacity(iCapacity), miSize(0) {
	mData = new V[miCapacity];
	memset(mData, NULL, sizeof(V) * miCapacity);
}

template <class V> Array<V>::~Array() {
	if (mData) {
		delete [] mData;
		mData = NULL;
	}
};

/** Inserts not zero data in array. Returns NULL in the event of successful charting, 
    otherwise returns data, which were already contributed earlier in cell with this number */
template <class V> V Array<V>::insert(size_t iNum, const V Data) {
	iNum %= miCapacity;
	V OldData = mData[iNum];
	if (!OldData && Data) {
		mData[iNum] = Data;
		++miSize;
	}
	return OldData;
}

/** Updates not zero data in cell with specified by number. 
    Returns old data or NULL */
template <class V> V Array<V>::update(size_t iNum, const V Data) {
	iNum %= miCapacity;
	V OldData = mData[iNum];
	if (OldData && Data) {
		mData[iNum] = Data;
		return OldData;
	}
	return (V) NULL;
}

/** Deletes not zero data from specified cells of the array. 
    Returns remote data or NULL in the event of their absences */
template <class V> V Array<V>::remove(size_t iNum) {
	iNum %= miCapacity;
	V OldData = mData[iNum];
	if (OldData) {
		mData[iNum] = (V)NULL;
		--miSize;
	}
	return OldData;
}

/** Returns data specified cells of the array or NULL in the event of their absences */
template <class V> V Array<V>::find(size_t iNum) {
	return mData[iNum % miCapacity];
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
	List(K key = (K)NULL, V Data = (V)NULL, List *Next = NULL) :
		mKey(key), mData(Data), mNext(Next) {}

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
	V add(K key, V Data) {
		if (mKey == key) {
			return mData; /** Element with such key already exists in list */
		}
		List *it = mNext, *prev = this;
		while ((it != NULL) && (it->mKey != key)) {
			prev = it;
			it = it->mNext;
		}
		if (it == NULL) { /** Nothing have not found. Add in list key and data */
			prev->mNext = new List(key, Data);
			return Data;
		}
		return it->mData; /** Something have found. Return that found */
	}

	/** Update data in list with specified by key. 
	    Returns previous key data or NULL */
	V update(K key, const V & Value) {
		V Data = NULL;
		if (mKey == key) { /** Update the first element of the list */
			Data = mData;
			mData = Value;
			return Data;
		}
		List * it = mNext;
		while ((it != NULL) && (it->mKey != key)) {
			it = it->mNext;
		}
		if (it != NULL) { /** Something have found. Update */
			Data = it->mData;
			it->mData = Value;
			return Data;
		}
		return Data; /** Have not found element with such key */
	}

	/** Removing the element of the list. 
	    Returns data of removed element or NULL.
	    Start - a pointer on initial element, received after removing. */
	V remove(K key, List *&Start) {
		if (mKey == key) { /** First element of the list */
			Start = mNext;
			mNext = NULL;
			return mData;
		}
		V Data = (V)NULL;
		List *it = mNext, *prev = this;

		while ((it != NULL) && (it->mKey != key)) {
			prev = it;
			it = it->mNext;
		}
		if (it != NULL) { /** Removing the element from list */
			Data = it->mData;
			prev->mNext = it->mNext;
			it->mNext = NULL;
			delete it;
			it = NULL;
		}
		return Data;
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


/** Hesh table or hash array (Array from list with hash) */
template <class V>
class HashTable {

public:

	typedef UINT_PTR Key;
	class iterator;
	Hash<Key> mHash; /** Hesh function */

private:

	typedef List<Key, V> tItem;

public:

	/** Data type - array of tItem* */
	typedef Array<tItem*> tData;

protected:

	tData * mData; /** Array with 1024 elements (on default) with type tItem* */
	size_t miSize; /** Size */
	bool mbIsResizing; /** Resizing flag */

public:

	HashTable(size_t iCapacity = 1024) : 
		miSize(0),
		mbIsResizing(false)
	{
		mData = new tData(iCapacity);
	}

	~HashTable() {
		clear();
		delete mData;
		mData = NULL;
	}

	/** Size */
	inline size_t size() const { return miSize; }

	/** Clear */
	void clear() {
		tItem * Item = NULL;
		for (size_t it = 0; it < mData->capacity(); ++it) {
			Item = mData->find(it); /** Find data by iterator */
			if (Item != NULL) {
				delete Item; /** Remove list */
			}
			mData->remove(it); /** Remove element */
		}
		miSize = 0;
	}

	/** Adds not zero data and key.
	Returns true if successful accompaniment data and key (key is unique) */
	bool add(const Key & key, V Data) {
		if(Data == (V)NULL) return false; /** No data */
		size_t iHash = key % mData->capacity(); /** Get hash */
		tItem *Items, *Item = NULL;
		Items = mData->find(iHash); /** Get cell data of the array */
		if (Items == NULL) { /** Check presence of the list in cell of the array */
			Item = new tItem(key, Data); /** Create new list with data and key */
			mData->insert(iHash, Item); /** Insert created list in cell of the array */

			if (!mbIsResizing) {
				onAdd(Data);
				++miSize;
			}
			return true;
		}
		/** Add data in existing list of the cell of the array */
		if (Data == Items->add(key, Data)) {
			if (!mbIsResizing) {
				onAdd(Data);
				++miSize;
			}
			return true;
		}
		else return false;
	}

	/** Returns true if successful removing */
	bool remove(const Key & key) {
		size_t iHash = key % mData->capacity(); /** Get hash */
		tItem *Item = NULL, *Items = mData->find(iHash); /** Get cell data of the array */
		if (Items == NULL) { /** Check presence of the list in cell of the array */
			return false;
		}
		Item = Items;
		V Data = Items->remove(key, Item); /** Removing from list */
		if (Item != Items) { /** Checking for removing the first element */
			if (Item) {
				mData->update(iHash, Item); /** Update list in cell */
			} else {
				mData->remove(iHash); /** Removing cell */
			}
			delete Items; /** Removing the old start element of the list */
			Items = NULL;
		}
		if ((V)NULL != Data) { /** Removing has occurred */
			onRemove(Data);
			--miSize;
			return true;
		}
		else return false;
	}

	/** Contain by key */
	bool contain(const Key & key) {
		tItem * Items = mData->find(key % mData->capacity()); /** Get cell data of the array */
		if (Items == NULL) { /** Check presence of the list in cell of the array */
			return false;
		}
		return ((V)NULL != Items->find(key)); /** Search key in list */
	}

	/** Find by key */
	V find(const Key & key) {
		tItem * Items = mData->find(key % mData->capacity()); /** Get cell data of the array */
		if (Items == NULL) { /** Check presence of the list in cell of the array */
			return (V)NULL;
		}
		return Items->find(key); /** Search data by key */
	}

	/** Update data (not NULL) to key. Returns true if successful */
	bool update(const Key & key, const V &Data) {
		if (Data == (V)NULL) {
			return false; /** No data */
		}
		tItem * Items = mData->find(key % mData->capacity());
		if (Items == NULL) { /** Check presence of the list in cell of the array */
			return false;
		}
		return (Items->update(key, Data) != (V)NULL); /** Update data in list */
	}

	virtual void onAdd(V) {}
	virtual void onRemove(V) {}


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

		iterator & operator = (const iterator &it) {
			mItem = it.mItem;
			i = it.i;
			return *this;
		}

		iterator(const iterator &it) {
			(*this) = it;
		}

		inline bool operator == (const iterator &it) {
			return mItem == it.mItem;
		}

		inline bool operator != (const iterator &it) {
			return mItem != it.mItem;
		}

		iterator & operator ++() {
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
		return iterator();
	}

	/** autoResize */
	bool autoResize(size_t & size, size_t & capacity, size_t & newSize) {
		capacity = this->mData->capacity();
		size = this->miSize;
		if ((size > (capacity << 1)) || (((size << 1) + 1) < capacity)) {
			newSize = size + (size >> 1) + 1;
			this->resize(newSize);
			return true;
		}
		return false;
	}

	bool autoResize() {
		size_t capacity = this->mData->capacity();
		if ((this->miSize > (capacity << 1)) || (((this->miSize << 1) + 1) < capacity)) {
			this->resize(this->miSize + (this->miSize >> 1) + 1);
			return true;
		}
		return false;
	}

	/** Resize */
	size_t resize(size_t newSize) {
		tData * newData = new tData(newSize);
		tData * oldData = this->mData;

		iterator it = this->begin();
		mbIsResizing = true;
		this->mData = newData;
		while (!it.isEnd()) {
			this->add(it.mItem->mKey, it.mItem->mData);
			++it;
		}
		delete oldData;
		mbIsResizing = false;
		return 0;
	}

	void dump(std::ostream & os) const {
		os << "Size = " << miSize << " Capacity = " << mData->capacity() << endl;
		tItem * items = NULL;
		for (size_t i = 0; i < mData->capacity(); ++i) {
			items = mData->find(i);
			if (items != NULL) {
				os << "i = " << i << " count = " << items->size() << endl;
				do {
					os << "Key = " << items->mKey << " Data = " << items->mData << endl;
				} while ((items = items->mNext) != NULL);
			}
			/*else {
				os << "i = " << i << " count = 0" << endl;
			}*/
		}
	}

}; // HashTable

}; // namespace utils

#endif // HASH_TABLE_H
