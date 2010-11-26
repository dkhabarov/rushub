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

#ifndef CLIST_H
#define CLIST_H

#include <stdio.h>

namespace nLua {

class Item {
	friend class cList;
	void * mValue;
	Item * mNext;
	bool mbDel;
public:
	Item() : mValue(NULL), mNext(NULL), mbDel(true) {}
	~Item() { if(mValue) delete mValue; }
	Item(void * value) : mValue(value), mNext(NULL), mbDel(false) {}
	void * GetValue() const { return mValue; }
	bool IsDel() const { return mbDel; }
	void SetDel() { mbDel = true; }
}; // Item

class cList {

	Item * mFirst;
	Item * mLast;

	int miSize;
	bool mbProcess;

public:

	cList() : mFirst(NULL), mLast(NULL), miSize(0), mbProcess(false) {}
	~cList() { Clear(); }

	void Clear() {
		Item *p, *q = mFirst;
		while(q) {
			p = q;
			q = q->mNext;
			delete p;
		}
		mFirst = mLast = NULL;
		miSize = 0;
	}
	int Size() { return miSize; }
	void Add(void * value) {
		if (value != NULL) {
			if (mLast == NULL) {
				mFirst = mLast = new Item(value);
			} else {
				mLast->mNext = new Item(value);
				mLast = mLast->mNext;
			}
			++miSize;
		}
	}
	template<typename _Predicate> void DelIf(_Predicate pred) {
		Item *p = mFirst;
		while(p) {
			if(!p->IsDel() && pred(p->GetValue()))
				p->SetDel();
			p = p->mNext;
		}
	}
	template<typename _Predicate> void Loop(_Predicate pred) {
		bool proc = mbProcess;
		mbProcess = true;
		Item *p = mFirst, *q = NULL, *r = NULL;
		while(p) {
			if(!p->IsDel())
				pred(p->GetValue()); // may be Loop
			p = p->mNext;
		}
		if(!proc) { // first Loop - removing all deleted elements
			p = mFirst;
			while(p) {
				r = q;
				q = p;
				p = p->mNext;
				if(q->IsDel()) {
					if(q == mFirst) mFirst = p;
					if(q == mLast) mLast = r;
					delete q;
					q = NULL;
					--miSize;
				}
			}
			mbProcess = false;
		}
	}

}; // cList

}; // nLua

#endif // CLIST_H
