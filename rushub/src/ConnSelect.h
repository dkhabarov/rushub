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

#ifndef CONN_SELECT_H
#define CONN_SELECT_H

#include "ConnChoose.h"
#include "HashTable.h"
#include "Obj.h"

#if USE_SELECT

#ifndef _WIN32
	#include <sys/select.h>
	#include <memory.h>
#else
	#pragma warning(disable:4127) // Disable "conditional expression is constant"
#endif

namespace server {

/** ConnSelect */
class ConnSelect : public ConnChoose, public Obj {

public:

	typedef HashTable<ChooseRes *> tResList;
	struct iterator;

protected:

	/** Res list */
	tResList mResList;

public:
	ConnSelect();
	virtual ~ConnSelect();

	inline size_t size() {
		return mResList.size();
	}

	int choose(Time &);

	bool optIn(tSocket, tEventFlag);
	void optOut(tSocket, tEventFlag);
	int optGet(tSocket);
	int revGet(tSocket);
	bool revTest(tSocket);

	struct SelectFd : public fd_set {
		SelectFd() {
			FD_ZERO(this);
		}
		SelectFd & operator = (const SelectFd &set) {
			#ifdef _WIN32
				fd_count = set.fd_count;
				static size_t fd_array_size = sizeof(fd_array);
				::memcpy(&fd_array, &(set.fd_array), fd_array_size);
			#else
				static size_t fds_bits_size = sizeof(fds_bits);
				memcpy(&fds_bits, &(set.fds_bits), fds_bits_size);
			#endif
			return *this;
		}
		inline bool isSet(tSocket sock) {
			return FD_ISSET(sock, this) != 0;
		}
		inline void clr(tSocket sock) {
			FD_CLR(sock, this);
		}
		bool set(tSocket sock) {
			#ifdef _WIN32
				if(fd_count >= FD_SETSIZE) {
					return false;
				}
			#endif
			FD_SET(sock, this);
			return true;
		}
	};

	void clearRevents();
	void setRevents(SelectFd & fdset, unsigned eMask);

	struct iterator {
		ConnSelect * mSel; /** for operator [] */
		tResList::iterator mIt; /** iterator for list */
		iterator() {
		}
		iterator(ConnSelect * sel, tResList::iterator it) : mSel(sel), mIt(it) {
		}

		inline iterator & operator = (const iterator & it) {
			mSel= it.mSel;
			mIt = it.mIt;
			return *this;
		}

		bool operator != (const iterator & it) {
			return mIt != it.mIt;
		}

		ChooseRes & operator * () {
			ChooseRes * chooseRes = NULL;
			
			#ifdef _WIN32
				__try {
					chooseRes = (*mIt);
					if (chooseRes->mConnBase == NULL) {
						chooseRes->mConnBase = mSel->operator[] (chooseRes->mFd);
					}
				} __except(1) {
					if (mSel->ErrLog(0)) {
						mSel->LogStream() << "Fatal error: " << endl
							<< "error in operator *()" << endl
							<< "item = " << mIt.mItem << endl
							<< "hash = " << mIt.i.i << endl
							<< "end = " << mIt.i.end << endl;
					}
					if (mSel->ErrLog(0)) {
						mSel->LogStream()	<< "connBase = " << (*mIt)->mConnBase << endl
							<< "socket = " << (*mIt)->mFd << endl;
					}
				}
			#else
				chooseRes = (*mIt);
				if (chooseRes->mConnBase == NULL) {
					chooseRes->mConnBase = mSel->operator[] (chooseRes->mFd);
				}
			#endif
			return *chooseRes;
		}

		iterator & operator ++() {
			#ifdef _WIN32
				__try {
					while (
						!(++mIt).isEnd() &&
						!(*mIt)->mRevents &&
						!((*mIt)->mEvents & eEF_CLOSE)
					) {
					}
				} __except(1) {
					if(mSel->ErrLog(0)) {
						mSel->LogStream() << "Fatal error: " << endl
							<< "error in operator ++()" << endl
							<< "item = " << mIt.mItem << endl
							<< "hash = " << mIt.i.i << endl
							<< "end = " << mIt.i.end << endl
							<< "socket = " << (*mIt)->mFd << endl;
					}
				}
			#else
				while (
					!(++mIt).isEnd() &&
					!(*mIt)->mRevents &&
					!((*mIt)->mEvents & eEF_CLOSE)
				) {
				}
			#endif
			return *this;
		}
	}; // iterator

	iterator begin() {
		static iterator begin_it(this, mResList.begin());
		begin_it.mIt = mResList.begin();
		if (
			!begin_it.mIt.isEnd() &&
			!(*begin_it.mIt)->mRevents &&
			!((*begin_it.mIt)->mEvents & eEF_CLOSE)
		) {
			++begin_it;
		}
		return begin_it;
	}
	inline iterator end() {
		static iterator end_it(this, mResList.end());
		return end_it;
	}

protected:

	/** For read */
	SelectFd mReadFS;

	/** For write */
	SelectFd mWriteFS;

	/** Errors */
	SelectFd mExceptFS;

	/** Closed */
	SelectFd mCloseFS;

	// select results
	SelectFd mResReadFS;
	SelectFd mResWriteFS;
	SelectFd mResExceptFS;

}; // ConnSelect

}; // server

#endif // USE_SELECT

#endif // CONN_SELECT_H

/**
 * $Id$
 * $HeadURL$
 */
