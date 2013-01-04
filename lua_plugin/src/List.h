/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * Copyright (C) 2009-2013 by Setuper
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

#ifndef LIST_H
#define LIST_H

#include <stdio.h>

namespace luaplugin {

template<typename T> class Item {
	template<typename V> friend class List;

public:

	Item() : mValue(NULL), mNext(NULL), mAlive(false) {
	}

	Item(T value) : mValue(value), mNext(NULL), mAlive(true) {
	}

	~Item() {
	}

	T getValue() const {
		return mValue;
	}

	bool isAlive() const {
		return mAlive;
	}

	void setDead() {
		mAlive = false;
	}

private:

	T mValue;
	Item<T> * mNext;
	bool mAlive;

}; // template class Item



template<typename T> class List {

public:

	List() : mFirst(NULL), mLast(NULL), mSize(0), mProcess(false) {
	}

	virtual ~List() {
		clear();
	}

	void clear() {
		Item<T> *p, *q = mFirst;
		while (q) {
			p = q;
			q = q->mNext;
			onRemove(p->getValue());
			delete p;
		}
		mFirst = mLast = NULL;
		mSize = 0;
	}

	virtual int size() const {
		return mSize;
	}

	virtual void add(T value) {
		if (mLast == NULL) {
			mFirst = mLast = new Item<T>(value);
		} else {
			mLast->mNext = new Item<T>(value);
			mLast = mLast->mNext;
		}
		++mSize;
	}

	template<typename P> void removeIf(P pred) {
		Item<T> *p = mFirst;
		while (p) {
			if (p->isAlive() && pred(p->getValue())) {
				p->setDead(); // fix me!
			}
			p = p->mNext;
		}
	}

	virtual void onRemove(T) {
	}

	template<typename P> void loop(P pred) {
		bool proc = mProcess;
		mProcess = true;
		Item<T> *next = mFirst, *curr = NULL, *prev = NULL;
		while (next) {
			if (next->isAlive()) {
				pred(next->getValue()); // may be Loop
			}
			next = next->mNext;
		}
		if (!proc) { // first Loop - removing all deleted elements
			next = mFirst;
			while (next) {
				prev = curr;
				curr = next;
				next = next->mNext;
				if (!curr->isAlive()) {
					if (curr == mFirst) {
						mFirst = next;
					}
					if (curr == mLast) {
						mLast = prev;
					}
					if (prev) {
						prev->mNext = next;
					}
					onRemove(curr->getValue());
					delete curr;
					curr = prev;
					--mSize;
				}
			}
			mProcess = false;
		}
	}

private:

	Item<T> * mFirst;
	Item<T> * mLast;

	int mSize;
	bool mProcess;

}; // template class List


} // namespace luaplugin

#endif // LIST_H

/**
 * $Id$
 * $HeadURL$
 */
