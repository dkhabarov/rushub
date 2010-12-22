/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz

 * modified: 10 Dec 2009
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

#ifndef CCONNCHOOSE_H
#define CCONNCHOOSE_H

#include "ctime.h"
#include "cconnbase.h"
#include "tchashtable.h" /** Hash table (tcHashTable) */

using namespace nUtils; /** tcHashTable, cTime */

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

namespace nServer {

class cConnChoose {

protected:
	tSocket mMaxSocket;

public:
		/** Hash-table for connections */
		typedef tcHashTable<cConnBase *> tConnBaseList;
/*
		#ifdef USE_OLD_CONNLIST
			typedef tcHashMap <cConnBase *, tSocket> tConnBaseList;
		#else
			typedef vector <cConnBase *> tConnBaseList;
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

	cConnChoose() : mMaxSocket(0) {};
	virtual ~cConnChoose(){};

	/** virtual function for select limit detect */
	virtual unsigned Size() { return 0; }

	/** Add conn in mConnBaseList */
	virtual bool AddConn(cConnBase *);

	/** Del conn from mConnBaseList */
	virtual bool DelConn(cConnBase *);

	/** HaConn in mConnBaseList */
	virtual bool HasConn(cConnBase *);

	inline cConnBase * operator [] (tSocket);
	inline tSocket operator [] (cConnBase *);

	/** Choose */
	virtual int Choose(cTime &) = 0;


	inline bool OptIn(cConnBase *, tEventFlag);
	inline void OptOut(cConnBase *, tEventFlag);
	inline int OptGet(cConnBase *);
	inline int RevGet(cConnBase *);
	inline bool RevTest(cConnBase *);

	/** OptIn */
	virtual bool OptIn(tSocket, tEventFlag) = 0;

	/** OptOut */
	virtual void OptOut(tSocket, tEventFlag) = 0;

	/** OptGet */
	virtual int OptGet(tSocket) = 0;

	/** RevGet */
	virtual int RevGet(tSocket) = 0;

	/** RevTest */
	virtual bool RevTest(tSocket) = 0;

	/** The Structure of the result of the choice, which returns the iterator.
	Contains the structure, which defines the type socket and pointer on structure, prestored in structure of the choice */
	struct sChooseRes {
		tSocket mFd; /** Socket descriptor */
		int mEvents; /** Events */
		int mRevents; /** Revents */
		cConnBase *mConnBase; /** Connection */
		sChooseRes() : mFd(0), mEvents(tEventFlag(0)), mRevents(tEventFlag(0)), mConnBase(NULL){}
	};


	/** The Iterator, which returns the object of the result of the choice.
	Return only objects, in which have entered the data! */
	struct iterator {
		tSocket *mMax; /** Max descriptor */
		cConnChoose *mChoose; /** Pointer on cConnChoose (for operator []) */
		sChooseRes mRes; /** sChooseRes */

		iterator(cConnChoose *ch, tSocket *max) : mMax(max), mChoose(ch){}
		iterator() : mMax(0), mChoose(0){};

		iterator & operator ++() {
			while( (++mRes.mFd < *mMax) && !(mChoose->RevTest(mRes.mFd)) ){}
			return *this;
		};

		sChooseRes & operator *() {
			mRes.mEvents = mChoose->OptGet(mRes.mFd);
			mRes.mRevents = mChoose->RevGet(mRes.mFd);
			mRes.mConnBase = mChoose->operator[](mRes.mFd);
			return mRes;
		};

		bool operator != (const iterator &it) const {
			return mRes.mFd != it.mRes.mFd;
		}

		bool operator == (const iterator &it) const {
			return mRes.mFd == it.mRes.mFd;
		}

		iterator &operator = (const iterator &it) {
			mRes.mFd = it.mRes.mFd;
			mMax = it.mMax;
			mChoose = it.mChoose;
			return *this;
		}
	};

	iterator &begin() {
		static iterator sBegin(this, &mMaxSocket);
		sBegin.mRes.mFd = 0;
		if( !RevTest(tSocket(0)) ) ++sBegin;
		return sBegin;
	};

	iterator &end() {
		static iterator sEnd(this, &mMaxSocket);
		sEnd.mRes.mFd = mMaxSocket;
		return sEnd;
	}

protected:
	static iterator sBegin;
	static iterator sEnd;

}; // cConnBaseChoose

bool cConnChoose::OptIn(cConnBase *conn, cConnChoose::tEventFlag eMask) {
	if(!conn) return false;
	return this->OptIn(tSocket(*conn), eMask);
}


void cConnChoose::OptOut(cConnBase *conn, cConnChoose::tEventFlag eMask) {
	if(!conn) return;
	this->OptOut(tSocket(*conn), eMask);
}


int cConnChoose::OptGet(cConnBase *conn) {
	if(!conn) return 0;
	return this->OptGet(tSocket(*conn));
}


int cConnChoose::RevGet(cConnBase *conn) {
	if(!conn) return 0;
	return this->RevGet(tSocket(*conn));
}


bool cConnChoose::RevTest(cConnBase *conn) {
	if(!conn) return false;
	return this->RevTest(tSocket(*conn));
}


tSocket cConnChoose::operator [] (cConnBase *conn) {
	if(!conn) return -1;
	return (tSocket)(*conn);
}

cConnBase * cConnChoose::operator [] (tSocket sock) {
	return mConnBaseList.Find(sock);
}

}; // nServer

#endif // CCONNCHOOSE_H
