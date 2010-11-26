/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * Copyright (C) 2009-2010 by Setuper
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

#ifndef TCQUEUEARRAY_H
#define TCQUEUEARRAY_H

#include <iostream>
#include <string.h>
#ifdef _WIN32
	#include <basetsd.h>
#else
	#define UINT_PTR unsigned long
#endif

using namespace std;

namespace nUtils {

template <class tHashType = unsigned long> struct tcHash {
	tHashType operator() (const char* s) const {
		tHashType h = 0;
		for(; *s; ++s) h = 33*h + *s;
		return h;
	}
	tHashType operator() (const string &s) const {
		return this->operator() (s.c_str());
	}
};

/** Usual array data */
template <class tDataType> class tcArray {

protected:

	unsigned miCapacity; /** Reserved number cell (element of the array) */
	unsigned miSize; /** Number contributed not zero element in array */
	tDataType *mData; /** Array data */

public:

	tcArray(unsigned iCapacity = 1024);
	virtual ~tcArray() { if(mData) delete [] mData; mData = NULL; };

	/** Returns number not zero element in array (miSize) */
	virtual inline unsigned Size() const { return miSize; }
	/** Returns reserved number a cell (miCapacity) */
	virtual inline unsigned Capacity() const { return miCapacity; }

	/** Inserts not zero data in array. Returns NULL in the event of successful charting, 
	    otherwise returns data, which were already contributed earlier in cell with this number */
	virtual tDataType Insert(unsigned iNum, const tDataType Data = 0);
	/** Updates not zero data in cell with specified by number. 
	    Returns old data or NULL */
	virtual tDataType Update(unsigned iNum, const tDataType Data);
	/** Deletes not zero data from specified cells of the array. 
	    Returns remote data or NULL in the event of their absences */
	virtual tDataType Remove(unsigned iNum);
	/** Returns data specified cells of the array or NULL in the event of their absences */
	virtual tDataType Find(unsigned iNum);


	/** Iterator for container */
	struct iterator {
		unsigned i;
		unsigned end;
		tDataType *mData;

		iterator() : i(0), end(0), mData((tDataType*)NULL) {}
		iterator(tDataType *Data, unsigned _i, unsigned _end) : i(_i), end(_end), mData(Data) {}
		iterator & operator = (const iterator &it) { i = it.i; end = it.end; mData = it.mData; return *this; }
		iterator(const iterator &it) { (*this) = it; }
		bool operator == (const iterator &it) { return i == it.i; }
		bool operator != (const iterator &it) { return i != it.i; }
		iterator & operator ++() { while((++i != end) && (mData[i] == (tDataType)NULL)){}; return *this; }
		tDataType operator *() { if(i < end && mData) return mData[i]; return NULL; }
		inline bool IsEnd() const { return i >= end; }
	};

	/** Initial iterator */
	iterator begin() {
		iterator Begin(mData, 0, miCapacity);
		if(mData[0] == (tDataType)NULL) ++Begin;
		return Begin;
	}

	/** Final iterator */
	iterator end() { return iterator(mData, miCapacity, miCapacity); }

}; // tcArray


template <class tDataType> tcArray<tDataType>::tcArray(unsigned iCapacity) : miCapacity(iCapacity), miSize(0) {
	mData = new tDataType[miCapacity];
	memset(mData, NULL, sizeof(tDataType) * miCapacity);
}

/** Inserts not zero data in array. Returns NULL in the event of successful charting, 
    otherwise returns data, which were already contributed earlier in cell with this number */
template <class tDataType> tDataType tcArray<tDataType>::Insert(unsigned iNum, const tDataType Data) {
	iNum %= miCapacity;
	tDataType OldData = mData[iNum];
	if(!OldData && Data) {
		mData[iNum] = Data;
		++miSize;
	}
	return OldData;
}

/** Updates not zero data in cell with specified by number. 
    Returns old data or NULL */
template <class tDataType> tDataType tcArray<tDataType>::Update(unsigned iNum, const tDataType Data) {
	iNum %= miCapacity;
	tDataType OldData = mData[iNum];
	if(OldData && Data) {
		mData[iNum] = Data;
		return OldData;
	}
	return (tDataType) NULL;
}

/** Deletes not zero data from specified cells of the array. 
    Returns remote data or NULL in the event of their absences */
template <class tDataType> tDataType tcArray<tDataType>::Remove(unsigned iNum) {
	iNum %= miCapacity;
	tDataType OldData = mData[iNum];
	if(OldData) {
		mData[iNum] = (tDataType)NULL;
		--miSize;
	}
	return OldData;
}

/** Returns data specified cells of the array or NULL in the event of their absences */
template <class tDataType> tDataType tcArray<tDataType>::Find(unsigned iNum) {
	return mData[iNum % miCapacity];
}



/** List with one communication (struct: key, val, next). The key is unique! */
template <class tKeyType, class tDataType>class tcList {
public:
	tKeyType mKey; /** Key */
	tDataType mData; /** Data */
	tcList *mNext; /** Pointer on the next element */

public:
	/** Constructor */
	tcList(tKeyType Key = (tKeyType)NULL, tDataType Data = (tDataType)NULL, tcList *Next = NULL) :
		mKey(Key), mData(Data), mNext(Next) {}

	virtual ~tcList() {
		if(mNext != NULL) {
			delete mNext;
			mNext = NULL;
		}
	}

	/** Size of list */
	unsigned Size() const {
		unsigned i = 1;
		tcList *it = mNext;
		while(it != NULL) {
			it = it->mNext;
			++i;
		}
		return i;
	}

	/** The Accompaniment data and key in list. 
	    Returns the added data or data for existing key.
	    The Data can be repeated, key is unique! */
	tDataType Add(tKeyType Key, tDataType Data) {
		if(mKey == Key) return mData; /** Element with such key already exists in list */
		tcList *it = mNext, *prev = this;
		while((it != NULL) && (it->mKey != Key)) {
			prev = it;
			it = it->mNext;
		}
		if(it == NULL) { /** Nothing have not found. Add in list key and data */
			prev->mNext = new tcList(Key, Data);
			return Data;
		}
		else return it->mData; /** Something have found. Return that found */
	}

	/** Update data in list with specified by key. 
	    Returns previous key data or NULL */
	tDataType Update(tKeyType Key, const tDataType &Value) {
		tDataType Data = NULL;
		if(mKey == Key) { /** Update the first element of the list */
			Data = mData;
			mData = Value;
			return Data;
		}
		tcList *it = mNext;
		while((it != NULL) && (it->mKey != Key)) it = it->mNext;
		if(it != NULL) { /** Something have found. Update */
			Data = it->mData;
			it->mData = Value;
			return Data;
		}
		else return Data; /** Have not found element with such key */
	}

	/** Removing the element of the list. 
	    Returns data of removed element or NULL.
	    Start - a pointer on initial element, received after removing. */
	tDataType Remove(tKeyType Key, tcList *&Start) {
		if(mKey == Key) { /** First element of the list */
			Start = mNext;
			mNext = NULL;
			return mData;
		}
		tDataType Data = (tDataType)NULL;
		tcList *it = mNext, *prev = this;

		while((it != NULL) && (it->mKey != Key)) {
			prev = it;
			it = it->mNext;
		}
		if(it != NULL) { /** Removing the element from list */
			Data = it->mData;
			prev->mNext = it->mNext;
			it->mNext = NULL;
			delete it;
			it = NULL;
		}
		return Data;
	}

	/** Finding data by key */
	tDataType Find(tKeyType Key) {
		if(mKey == Key) return mData; /** Have found in first element of the list */
		tcList *it = mNext;
		while((it != NULL ) && (it->mKey != Key)) it = it->mNext;
		if(it != NULL) return it->mData; /** Have found and return */
		else return (tDataType)NULL; /** Nothing have not found */
	}
}; // tcList


/** Hesh table or hash array (Array from list with hash) */
template <class tDataType> class tcHashTable {

public:
	typedef UINT_PTR tKeyType;
	class iterator;
	tcHash<tKeyType> mHash; /** Hesh function */

private:
	typedef tcList<tKeyType, tDataType> tItem;

public:
	/** Data type - array of tItem* */
	typedef tcArray<tItem*> tData;

protected:
	tData * mData; /** Array with 1024 elements (on default) with type tItem* */
	unsigned miSize; /** Size */
	bool mbIsResizing; /** Resizing flag */

public:

	tcHashTable(unsigned iCapacity = 1024) : 
		miSize(0),
		mbIsResizing(false)
	{
		mData = new tData(iCapacity);
	}

	~tcHashTable() {
		Clear();
		delete mData;
		mData = NULL;
	}

	/** Size */
	inline unsigned Size() const { return miSize; }

	/** Clear */
	void Clear() {
		tItem *Item = NULL;
		for(unsigned it = 0; it < mData->Capacity(); ++it) {
			Item = mData->Find(it); /** Find data by iterator */
			if(Item != NULL) { delete Item; } /** Remove list */
			mData->Remove(it); /** Remove element */
		}
		miSize = 0;
	}

	/** Adds not zero data and key.
	Returns true if successful accompaniment data and key (key is unique) */
	bool Add(const tKeyType &Key, tDataType Data) {
		if(Data == (tDataType)NULL) return false; /** No data */
		unsigned iHash = Key % mData->Capacity(); /** Get hash */
		tItem *Items, *Item = NULL;
		Items = mData->Find(iHash); /** Get cell data of the array */
		if(Items == NULL) { /** Check presence of the list in cell of the array */
			Item = new tItem(Key, Data); /** Create new list with data and key */
			mData->Insert(iHash, Item); /** Insert created list in cell of the array */

			if(!mbIsResizing) {
				OnAdd(Data);
				++miSize;
			}
			return true;
		}
		/** Add data in existing list of the cell of the array */
		if(Data == Items->Add(Key, Data)) {
			if(!mbIsResizing) {
				OnAdd(Data);
				++miSize;
			}
			return true;
		}
		else return false;
	}

	/** Returns true if successful removing */
	bool Remove(const tKeyType &Key) {
		unsigned iHash = Key % mData->Capacity(); /** Get hash */
		tItem *Item = NULL, *Items = mData->Find(iHash); /** Get cell data of the array */
		if(Items == NULL){ return false; } /** Check presence of the list in cell of the array */
		Item = Items;
		tDataType Data = Items->Remove(Key, Item); /** Removing from list */
		if(Item != Items) { /** Checking for removing the first element */
			if(Item) mData->Update(iHash, Item); /** Update list in cell */
			else mData->Remove(iHash); /** Removing cell */
			delete Items; /** Removing the old start element of the list */
			Items = NULL;
		}
		if((tDataType)NULL != Data) { /** Removing has occurred */
			OnRemove(Data);
			--miSize;
			return true;
		}
		else return false;
	}

	/** Contain by key */
	bool Contain(const tKeyType &Key) {
		tItem * Items = mData->Find(Key % mData->Capacity()); /** Get cell data of the array */
		if(Items == NULL) return false; /** Check presence of the list in cell of the array */
		return ((tDataType)NULL != Items->Find(Key)); /** Search key in list */
	}

	/** Find by key */
	tDataType Find(const tKeyType &Key) {
		tItem * Items = mData->Find(Key % mData->Capacity()); /** Get cell data of the array */
		if(Items == NULL) return (tDataType)NULL; /** Check presence of the list in cell of the array */
		return Items->Find(Key); /** Search data by key */
	}

	/** Update data (not NULL) to key. Returns true if successful */
	bool Update(const tKeyType &Key, const tDataType &Data) {
		if(Data == (tDataType)NULL) return false; /** No data */
		tItem * Items = mData->Find(Key % mData->Capacity());
		if(Items == NULL) return false; /** Check presence of the list in cell of the array */
		return (Items->Update(Key, Data) != (tDataType)NULL); /** Update data in list */
	}

	virtual void OnAdd(tDataType) {}
	virtual void OnRemove(tDataType) {}


	/** Iterator through all NON-NULL elements of the container */
	class iterator {
	public:
		typename tData::iterator i; /** Iterator of array (type tItem*) */
		tItem * mItem; /** Pointer to array element */

		iterator() : mItem(NULL) {}
		iterator(typename tData::iterator it) : i(it) { if(!i.IsEnd()) mItem = *i; else mItem = NULL; }
		iterator & operator = (const iterator &it) { mItem = it.mItem; i = it.i; return *this; }
		iterator(const iterator &it) { (*this) = it; }
		bool operator == (const iterator &it) { return mItem == it.mItem; }
		bool operator != (const iterator &it) { return mItem != it.mItem; }
		iterator & operator ++() {
			if((mItem != NULL) && (mItem->mNext != NULL)) mItem = mItem->mNext;
			else {
				++i;
				if(!i.IsEnd()) mItem = (*i);
				else mItem = NULL;
			}
			return *this;
		}
		bool IsEnd() const { return mItem == NULL; }
		tDataType operator *() { return mItem->mData; }
	}; // iterator

	iterator begin() { return iterator(mData->begin()); }
	iterator end() { return iterator(); }

	/** AutoResize */
	bool AutoResize(unsigned & iSize, unsigned & iCapacity, unsigned & iNewSize) {
		iCapacity = this->mData->Capacity();
		iSize = this->miSize;
		if((iSize > (iCapacity << 1)) || (((iSize << 1) + 1) < iCapacity)) {
			iNewSize = iSize + (iSize >> 1) + 1;
			this->Resize(iNewSize);
			return true;
		}
		return false;
	}

	bool AutoResize() {
		unsigned iCapacity = this->mData->Capacity();
		if((this->miSize > (iCapacity << 1)) || (((this->miSize << 1) + 1) < iCapacity)) {
			this->Resize(this->miSize + (this->miSize >> 1) + 1);
			return true;
		}
		return false;
	}

	/** Resize */
	int Resize(int NewSize) {
		tData *NewData = new tData(NewSize);
		tData *OldData = this->mData;

		iterator it = this->begin();
		mbIsResizing = true;
		this->mData = NewData;
		while(!it.IsEnd()) {
			this->Add(it.mItem->mKey, it.mItem->mData);
			++it;
		}
		delete OldData;
		mbIsResizing = false;
		return 0;
	}

	void Dump(std::ostream &os) const {
		os << "Size = " << miSize << " Capacity = " << mData->Capacity() << endl;
		tItem *Items;
		for(unsigned i = 0; i < mData->Capacity(); ++i) {
			Items = mData->Find(i);
			if(Items != NULL) {
				os << "i = " << i << " count = " << Items->Size() << endl;
				do {
					os << "Key = " << Items->mKey << " Data = " << Items->mData << endl;
				} while((Items = Items->mNext) != NULL);
			}
			//else os << "i = " << i << " count = 0" << endl;
		}
	}

}; // tcHashTable

}; // nUtils

#endif // TCQUEUEARRAY_H
