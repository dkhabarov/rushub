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

#ifndef CONN_CHOOSE_H
#define CONN_CHOOSE_H

#include "ConnBase.h" // first (def winsock2.h)
#include "Times.h"
#include "HashTable.h"

using namespace ::utils; // HashTable, Time

#ifdef _WIN32
	#define USE_SELECT 1
#else
	//#define USE_EPOLL 1
#endif

namespace server {

class ConnChoose {

protected:

	tSocket mMaxSocket;

public:

	/// Hash-table for connections
	typedef HashTable<ConnBase *> tConnBaseList;

	/// Hash-table
	tConnBaseList mConnBaseList;

	/// Events flags
	enum tEventFlag {
		eEF_INPUT = 1 << 0, ///< For read (1)
		eEF_OUTPUT= 1 << 1, ///< For write (2)
		eEF_ERROR = 1 << 2, ///< Socket contains errors (4)
		eEF_CLOSE = 1 << 3, ///< Closed socket (8)
		eEF_ALL   = eEF_INPUT | eEF_OUTPUT | eEF_ERROR,
		eEF_ALL_AND_CLOSE = eEF_ALL | eEF_CLOSE
	};

public:

	ConnChoose() : mMaxSocket(0) {
	}

	virtual ~ConnChoose() {
	}

	/// virtual function for select limit detect
	virtual size_t size() {
		return 0;
	}

	virtual bool addConn(ConnBase *);
	virtual bool deleteConn(ConnBase *);
	virtual bool hasConn(ConnBase *);

	virtual int choose(Time &) = 0;

	virtual bool optIn(tSocket, tEventFlag) = 0;
	virtual void optOut(tSocket, tEventFlag) = 0;
	virtual int optGet(tSocket) = 0;
	virtual int revGet(tSocket) = 0;
	virtual bool revTest(tSocket) = 0;

	inline bool optIn(ConnBase *, tEventFlag);
	inline void optOut(ConnBase *, tEventFlag);
	inline int optGet(ConnBase *);
	inline int revGet(ConnBase *);
	inline bool revTest(ConnBase *);

	inline ConnBase * operator [] (tSocket);

	/** The Structure of the result of the choice, which returns the iterator.
	Contains the structure, which defines the type socket and pointer on structure, prestored in structure of the choice */
	struct ChooseRes {

		/// Socket descriptor
		tSocket mFd;

		/// Events
		int mEvents;

		/// Revents
		int mRevents;

		/// Connection
		ConnBase * mConnBase;

		ChooseRes(tSocket fd = 0, int events = 0, int revents = 0) : 
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
		ConnChoose * mChoose; ///< Pointer on ConnChoose (for operator [])
		tSocket * mMax; ///< Max descriptor
		ChooseRes mRes; ///< ChooseRes

		iterator(ConnChoose * ch, tSocket * max) : mChoose(ch), mMax(max) {
		}

		iterator() : mChoose(0), mMax(0) {
		}

		iterator & operator ++() {
			while ((++mRes.mFd < *mMax) && !(mChoose->revTest(mRes.mFd))) {
			}
			return *this;
		};

		ChooseRes & operator * () {
			mRes.mEvents = mChoose->optGet(mRes.mFd);
			mRes.mRevents = mChoose->revGet(mRes.mFd);
			mRes.mConnBase = mChoose->operator[] (mRes.mFd);
			return mRes;
		};

		inline bool operator != (const iterator & it) const {
			return mRes.mFd != it.mRes.mFd;
		}

		inline bool operator == (const iterator & it) const {
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
		if (!revTest(tSocket(0))) {
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

bool ConnChoose::optIn(ConnBase * connBase, ConnChoose::tEventFlag eMask) {
	return connBase != NULL ? this->optIn(tSocket(*connBase), eMask) : false;
}

void ConnChoose::optOut(ConnBase * connBase, ConnChoose::tEventFlag eMask) {
	if (!connBase) {
		return;
	}
	this->optOut(tSocket(*connBase), eMask);
}

int ConnChoose::optGet(ConnBase * connBase) {
	return connBase != NULL ? optGet(tSocket(*connBase)) : 0;
}

int ConnChoose::revGet(ConnBase * connBase) {
	return connBase != NULL ? revGet(tSocket(*connBase)) : 0;
}

bool ConnChoose::revTest(ConnBase * connBase) {
	return connBase != NULL ? revTest(tSocket(*connBase)) : false;
}

ConnBase * ConnChoose::operator [] (tSocket sock) {
	return mConnBaseList.find(sock);
}

}; // namespace server

#endif // CONN_CHOOSE_H

/**
 * $Id$
 * $HeadURL$
 */
