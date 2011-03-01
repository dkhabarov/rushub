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

#ifndef CONN_CHOOSE_H
#define CONN_CHOOSE_H

#include "Times.h"
#include "ConnBase.h"
#include "HashTable.h" // Hash table (HashTable)

using namespace ::utils; // HashTable, Time

#ifdef _WIN32
	#define USE_SELECT 1
#else
	//#define USE_SELECT 1
/*	#if HAVE_SYS_POLL_H // for configure
		#define USE_SELECT 0
	#else
		#define USE_SELECT 1
	#endif
*/
#endif

namespace server {

class ConnChoose {

protected:

	tSocket mMaxSocket;

public:

		/** Hash-table for connections */
		typedef HashTable<ConnBase *> tConnBaseList;
/*
		#ifdef USE_OLD_CONNLIST
			typedef HashMap <ConnBase *, tSocket> tConnBaseList;
		#else
			typedef vector <ConnBase *> tConnBaseList;
		#endif
*/

	/** Hash-table */
	tConnBaseList mConnBaseList;

	/** Events flags */
	enum tEventFlag {
		eEF_INPUT = 1 << 0, /** For read (1) */
		eEF_OUTPUT= 1 << 1, /** For write (2) */
		eEF_ERROR = 1 << 2, /** Socket contains errors (4) */
		eEF_CLOSE = 1 << 3, /** Closed socket (8) */
		eEF_ALL   = eEF_INPUT | eEF_OUTPUT | eEF_ERROR,
		eEF_ALL_AND_CLOSE = eEF_ALL | eEF_CLOSE
	};

public:

	ConnChoose() : mMaxSocket(0) {
	}

	virtual ~ConnChoose() {
	}

	/** virtual function for select limit detect */
	virtual unsigned Size() {
		return 0;
	}

	virtual bool AddConn(ConnBase *);
	virtual bool DelConn(ConnBase *);
	virtual bool HasConn(ConnBase *);

	virtual int Choose(Time &) = 0;

	virtual bool OptIn(tSocket, tEventFlag) = 0;
	virtual void OptOut(tSocket, tEventFlag) = 0;
	virtual int OptGet(tSocket) = 0;
	virtual int RevGet(tSocket) = 0;
	virtual bool RevTest(tSocket) = 0;

	inline bool OptIn(ConnBase *, tEventFlag);
	inline void OptOut(ConnBase *, tEventFlag);
	inline int OptGet(ConnBase *);
	inline int RevGet(ConnBase *);
	inline bool RevTest(ConnBase *);

	inline ConnBase * operator [] (tSocket);

	/** The Structure of the result of the choice, which returns the iterator.
	Contains the structure, which defines the type socket and pointer on structure, prestored in structure of the choice */
	struct sChooseRes {

		/** Socket descriptor */
		tSocket mFd;

		/** Events */
		int mEvents;

		/** Revents */
		int mRevents;

		/** Connection */
		ConnBase * mConnBase;

		sChooseRes(tSocket fd = 0, int events = 0, int revents = 0) : 
			mFd(fd),
			mEvents(events),
			mRevents(revents),
			mConnBase(NULL)
		{
		}

	};


	/** The Iterator, which returns the object of the result of the choice.
	Return only objects, in which have entered the data! */
	struct iterator {
		ConnChoose * mChoose; /** Pointer on ConnChoose (for operator []) */
		tSocket * mMax; /** Max descriptor */
		sChooseRes mRes; /** sChooseRes */

		iterator(ConnChoose * ch, tSocket * max) : mChoose(ch), mMax(max) {
		}

		iterator() : mChoose(0), mMax(0) {
		}

		iterator & operator ++() {
			while ((++mRes.mFd < *mMax) && !(mChoose->RevTest(mRes.mFd))) {
			}
			return *this;
		};

		sChooseRes & operator * () {
			mRes.mEvents = mChoose->OptGet(mRes.mFd);
			mRes.mRevents = mChoose->RevGet(mRes.mFd);
			mRes.mConnBase = mChoose->operator[] (mRes.mFd);
			return mRes;
		};

		bool operator != (const iterator & it) const {
			return mRes.mFd != it.mRes.mFd;
		}

		bool operator == (const iterator & it) const {
			return mRes.mFd == it.mRes.mFd;
		}

		iterator & operator = (const iterator & it) {
			mRes.mFd = it.mRes.mFd;
			mMax = it.mMax;
			mChoose = it.mChoose;
			return *this;
		}
	};

	iterator & begin() {
		static iterator beginIterator(this, &mMaxSocket);
		beginIterator.mRes.mFd = 0;
		if (!RevTest(tSocket(0))) {
			++beginIterator;
		}
		return beginIterator;
	};

	iterator & end() {
		static iterator endIterator(this, &mMaxSocket);
		endIterator.mRes.mFd = mMaxSocket;
		return endIterator;
	}

}; // class ConnChoose

bool ConnChoose::OptIn(ConnBase * connBase, ConnChoose::tEventFlag eMask) {
	if (!connBase) {
		return false;
	}
	return this->OptIn(tSocket(*connBase), eMask);
}

void ConnChoose::OptOut(ConnBase * connBase, ConnChoose::tEventFlag eMask) {
	if (!connBase) {
		return;
	}
	this->OptOut(tSocket(*connBase), eMask);
}

int ConnChoose::OptGet(ConnBase * connBase) {
	if (!connBase) {
		return 0;
	}
	return this->OptGet(tSocket(*connBase));
}

int ConnChoose::RevGet(ConnBase * connBase) {
	if (!connBase) {
		return 0;
	}
	return this->RevGet(tSocket(*connBase));
}

bool ConnChoose::RevTest(ConnBase * connBase) {
	if (!connBase) {
		return false;
	}
	return this->RevTest(tSocket(*connBase));
}

ConnBase * ConnChoose::operator [] (tSocket sock) {
	return mConnBaseList.Find(sock);
}

}; // namespace server

#endif // CONN_CHOOSE_H
