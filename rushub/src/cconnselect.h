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

#ifndef CCONNSELECT_H
#define CCONNSELECT_H

#include "cconnchoose.h"
#include "tchashtable.h"

#include "cobj.h"

#if USE_SELECT

#ifndef _WIN32
	#include <sys/select.h>
	#include <memory.h>
#endif

namespace nServer {

/** cConnSelect */
class cConnSelect : public cConnChoose, public cObj {

public:

	typedef tcHashTable<sChooseRes *> tResList;
	struct iterator;

protected:

	tResList mResList; /** Res list */

public:
	cConnSelect() : cConnChoose(), cObj("cConnSelect") {};
	virtual ~cConnSelect();

	unsigned Size() { return mResList.Size(); }

	int Choose(cTime &);

	bool OptIn(tSocket, tEventFlag);
	void OptOut(tSocket, tEventFlag);
	int OptGet(tSocket);
	int RevGet(tSocket);
	bool RevTest(tSocket);

	struct cSelectFD : public fd_set {
		cSelectFD() {
			FD_ZERO(this);
		}
		cSelectFD & operator = (const cSelectFD &set) {
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
		bool IsSet(tSocket sock) { return FD_ISSET(sock, this) != 0; }
		void Clr(tSocket sock) { FD_CLR(sock, this); }
		bool Set(tSocket sock) {
			#ifdef _WIN32
				if(fd_count >= FD_SETSIZE) return false;
			#endif
			FD_SET(sock, this);
			return true;
		}
	};

	void ClearRevents();
	void SetRevents(cSelectFD &fdset, unsigned eMask);

	struct iterator {
		cConnSelect *mSel; /** for operator [] */
		tResList::iterator mIt; /** iterator for list */
		iterator() {}
		iterator(cConnSelect *sel, tResList::iterator it) : mSel(sel), mIt(it) {}

		iterator & operator = (const iterator &it) {
			mSel= it.mSel;
			mIt = it.mIt;
			return *this;
		}
		bool operator != (const iterator &it){ return mIt != it.mIt; }
		sChooseRes & operator *() {
			sChooseRes *ChR;
#ifdef _WIN32
			__try {
				if((ChR = (*mIt))->mConnBase == NULL)
					ChR->mConnBase = mSel->operator[](ChR->mFd);
			} __except(1) {
				if(mSel->ErrLog(0)) mSel->LogStream() << "Fatal error: " << endl
					<< "error in operator *()" << endl
					<< "Item = " << mIt.mItem << endl
					<< "Hash = " << mIt.i.i << endl
					<< "End = " << mIt.i.end << endl;
				if(mSel->ErrLog(0)) mSel->LogStream()	<< "ConnBase = " << ChR->mConnBase << endl
					<< "Socket = " << ChR->mFd << endl;
			}
#else
		if((ChR = (*mIt))->mConnBase == NULL)
			ChR->mConnBase = mSel->operator[](ChR->mFd);
#endif
			return *ChR;
		}

		iterator & operator ++() {
#ifdef _WIN32
			__try {
				while(!(++mIt).IsEnd() && !(*mIt)->mRevents && !((*mIt)->mEvents & eEF_CLOSE)) {}
			} __except(1) {
				if(mSel->ErrLog(0)) mSel->LogStream() << "Fatal error: " << endl
					<< "error in operator ++()" << endl
					<< "Item = " << mIt.mItem << endl
					<< "Hash = " << mIt.i.i << endl
					<< "End = " << mIt.i.end << endl
					<< "Socket = " << (*mIt)->mFd << endl; 
			}
#else
			while(!(++mIt).IsEnd() && !(*mIt)->mRevents && !((*mIt)->mEvents & eEF_CLOSE)) {}
#endif
			return *this;
		}
	}; // iterator

	iterator begin() {
		static iterator sBegin(this, mResList.begin());
		sBegin.mIt = mResList.begin();
		if(!sBegin.mIt.IsEnd() && !(*sBegin.mIt)->mRevents && !((*sBegin.mIt)->mEvents & eEF_CLOSE))
			++sBegin;
		return sBegin;
	}
	iterator end() {
		static iterator sEnd(this, mResList.end());
		return sEnd;
	}

protected:

	cSelectFD mReadFS; /** For read */
	cSelectFD mWriteFS; /** For write */
	cSelectFD mExceptFS; /** Errors */
	cSelectFD mCloseFS; /** Closed */

	// select results
	cSelectFD mResReadFS;
	cSelectFD mResWriteFS;
	cSelectFD mResExceptFS;

}; // cConnSelect

}; // nServer

#endif // USE_SELECT

#endif // CCONNSELECT_H
