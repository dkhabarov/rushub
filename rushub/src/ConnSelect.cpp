/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz

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

#include "ConnSelect.h"

#if USE_SELECT

namespace server {



ConnSelect::ConnSelect() :
	ConnChoose(),
	Obj("ConnSelect")
{
}



ConnSelect::~ConnSelect() {
	tResList::iterator it;
	sChooseRes * ChR = NULL;
	tSocket sock;
	for (it = mResList.begin(); it != mResList.end(); ) {
		ChR = *it;
		++it;
		if (ChR) {
			sock = ChR->mFd;
			delete ChR;
			mResList.remove(sock);
		}
	}
}



bool ConnSelect::optIn(tSocket sock, tEventFlag mask) {
	if (
		(mask & eEF_INPUT && !mReadFS.set(sock)) ||
		(mask & eEF_OUTPUT && !mWriteFS.set(sock)) ||
		(mask & eEF_ERROR && !mExceptFS.set(sock)) ||
		(mask & eEF_CLOSE && !mCloseFS.set(sock))
	) {
		return false;
	}
	sChooseRes * ChR = mResList.find(sock);
	if (!ChR) {
		mResList.add(sock, new sChooseRes(sock, mask));
	} else {
		ChR->mEvents |= mask;
	}
	return true;
}



void ConnSelect::optOut(tSocket sock, tEventFlag mask) {
	if (mask & eEF_INPUT ) {
		mReadFS.clr(sock);
	}
	if (mask & eEF_OUTPUT) {
		mWriteFS.clr(sock);
	}
	if (mask & eEF_ERROR ) {
		mExceptFS.clr(sock);
	}
	if (mask & eEF_CLOSE ) {
		mCloseFS.clr(sock);
	}

	sChooseRes * ChR = mResList.find(sock);
	if (ChR) {
		ChR->mEvents -= (ChR->mEvents & mask);
		if (!ChR->mEvents) {
			delete ChR;
			mResList.remove(sock);
		}
	}
}



int ConnSelect::optGet(tSocket sock) {
	int mask = 0;
	if (mReadFS.isSet(sock)) {
		mask |= eEF_INPUT;
	}
	if (mWriteFS.isSet(sock)) {
		mask |= eEF_OUTPUT;
	}
	if (mExceptFS.isSet(sock)) {
		mask |= eEF_ERROR;
	}
	if (mCloseFS.isSet(sock)) {
		mask |= eEF_CLOSE;
	}
	return mask;
}



int ConnSelect::revGet(tSocket sock) {
	int mask = 0;
	if (mResReadFS.isSet(sock)) {
		mask |= eEF_INPUT;
	}
	if (mResWriteFS.isSet(sock)) {
		mask |= eEF_OUTPUT;
	}
	if (mResExceptFS.isSet(sock)) {
		mask |= eEF_ERROR;
	}
	if (mCloseFS.isSet(sock)) {
		mask |= eEF_CLOSE;
	}
	return mask;
}



bool ConnSelect::revTest(tSocket sock) {
	return mResWriteFS.isSet(sock) ||
		mResReadFS.isSet(sock) ||
		mResExceptFS.isSet(sock) ||
		mCloseFS.isSet(sock);
}



/** Do select */
int ConnSelect::choose(Time & timeout) {
	mResReadFS = mReadFS;
	mResWriteFS = mWriteFS;
	mResExceptFS = mExceptFS;

	/** select */
	int ret = ::select(mMaxSocket, &mResReadFS, &mResWriteFS, &mResExceptFS, (timeval *)(&timeout));
	if (SOCK_ERROR(ret)) {
		return -1;
	}

	clearRevents();
	setRevents(mResReadFS, eEF_INPUT);
	setRevents(mResWriteFS, eEF_OUTPUT);
	setRevents(mResExceptFS, eEF_ERROR);
	setRevents(mCloseFS, eEF_CLOSE);
	return ret;
}



void ConnSelect::clearRevents(void) {
	sChooseRes * ChR = NULL;
	for (tResList::iterator it = mResList.begin(); it != mResList.end(); ++it) {
		ChR = (*it);
		if (ChR != NULL) {
			ChR->mRevents = 0;
		}
	}
}



void ConnSelect::setRevents(cSelectFD & fdset, unsigned mask) {
	tSocket sock;
	sChooseRes * ChR = NULL;
	#ifdef _WIN32
	for (unsigned i = 0; i < fdset.fd_count; ++i) {
		sock = fdset.fd_array[i];
		ChR = mResList.find(sock);
		if (!ChR) {
			mResList.add(sock, new sChooseRes(sock, 0, mask));
		} else {
			ChR->mRevents |= mask;
		}
	}
	#else
	for (unsigned i = 0; i < FD_SETSIZE; ++i) {
		sock = i;
		if (FD_ISSET(sock, &fdset)) {
			ChR = mResList.find(sock);
			if (!ChR) {
				mResList.add(sock, new sChooseRes(sock, 0, mask));
			} else {
				ChR->mRevents |= mask;
			}
		}
	}
	#endif
}


}; // server

#endif // USE_SELECT

/**
 * $Id$
 * $HeadURL$
 */
