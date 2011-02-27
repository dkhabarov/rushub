/*
 * RusHub - hub server for Direct Connect peer to peer network.

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

#ifndef LIST_H
#define LIST_H

#include <stdio.h>

namespace luaplugin {

template<typename T> class Item {
	template<typename V> friend class cList;

public:

	Item() : mValue(NULL), mNext(NULL), mbDel(true) {
	}

	~Item() {
		if (mValue) {
			delete mValue;
		}
	}

	Item(T * value) : mValue(value), mNext(NULL), mbDel(false) {
	}

	T * GetValue() const {
		return mValue;
	}

	bool IsDel() const {
		return mbDel;
	}

	void SetDel() {
		mbDel = true;
	}

private:

	T * mValue;
	Item<T> * mNext;
	bool mbDel;

}; // template class Item

template<typename T> class cList {

public:

	cList() : mFirst(NULL), mLast(NULL), miSize(0), mbProcess(false) {
	}

	~cList() {
		Clear();
	}

	void Clear() {
		Item<T> *p, *q = mFirst;
		while (q) {
			p = q;
			q = q->mNext;
			delete p;
		}
		mFirst = mLast = NULL;
		miSize = 0;
	}

	int Size() {
		return miSize;
	}

	void Add(T * value) {
		if (value != NULL) {
			if (mLast == NULL) {
				mFirst = mLast = new Item<T>(value);
			} else {
				mLast->mNext = new Item<T>(value);
				mLast = mLast->mNext;
			}
			++miSize;
		}
	}

	template<typename P> void DelIf(P pred) {
		Item<T> *p = mFirst;
		while (p) {
			if (!p->IsDel() && pred(p->GetValue())) {
				p->SetDel();
			}
			p = p->mNext;
		}
	}

	template<typename P> void Loop(P pred) {
		bool proc = mbProcess;
		mbProcess = true;
		Item<T> *p = mFirst, *q = NULL, *r = NULL;
		while (p) {
			if (!p->IsDel()) {
				pred(p->GetValue()); // may be Loop
			}
			p = p->mNext;
		}
		if (!proc) { // first Loop - removing all deleted elements
			p = mFirst;
			while (p) {
				r = q;
				q = p;
				p = p->mNext;
				if (q->IsDel()) {
					if (q == mFirst) {
						mFirst = p;
					}
					if (q == mLast) {
						mLast = r;
					}
					delete q;
					q = NULL;
					--miSize;
				}
			}
			mbProcess = false;
		}
	}

private:

	Item<T> * mFirst;
	Item<T> * mLast;

	int miSize;
	bool mbProcess;

}; // template class cList

}; // namespace luaplugin

#endif // LIST_H