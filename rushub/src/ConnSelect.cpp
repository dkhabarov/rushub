/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz
 *
 * modified: 27 Aug 2009
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

#include "ConnSelect.h"

#if USE_SELECT

namespace server {



ConnSelect::ConnSelect() :
	ConnChoose()
{
}



ConnSelect::~ConnSelect() {
	tResList::iterator it;
	ChooseRes * chooseRes = NULL;
	tSocket sock;
	for (it = mResList.begin(); it != mResList.end(); ) {
		chooseRes = *it;
		++it;
		if (chooseRes) {
			sock = chooseRes->mFd;
			delete chooseRes;
			mResList.remove(sock);
		}
	}
}



bool ConnSelect::optIn(tSocket sock, EventFlag mask) {
	if (
		(mask & EF_INPUT && !mReadFS.set(sock)) ||
		(mask & EF_OUTPUT && !mWriteFS.set(sock)) ||
		(mask & EF_ERROR && !mExceptFS.set(sock)) ||
		(mask & EF_CLOSE && !mCloseFS.set(sock))
	) {
		return false;
	}
	ChooseRes * chooseRes = mResList.find(sock);
	if (!chooseRes) {
		mResList.add(sock, new ChooseRes(sock, mask));
	} else {
		chooseRes->mEvents |= mask;
	}
	return true;
}



void ConnSelect::optOut(tSocket sock, EventFlag mask) {
	if (mask & EF_INPUT ) {
		mReadFS.clr(sock);
	}
	if (mask & EF_OUTPUT) {
		mWriteFS.clr(sock);
	}
	if (mask & EF_ERROR ) {
		mExceptFS.clr(sock);
	}
	if (mask & EF_CLOSE ) {
		mCloseFS.clr(sock);
	}

	ChooseRes * chooseRes = mResList.find(sock);
	if (chooseRes) {
		chooseRes->mEvents -= (chooseRes->mEvents & mask);
		if (!chooseRes->mEvents) {
			delete chooseRes;
			mResList.remove(sock);
		}
	}
}



int ConnSelect::optGet(tSocket sock) {
	int mask = 0;
	if (mReadFS.isSet(sock)) {
		mask |= EF_INPUT;
	}
	if (mWriteFS.isSet(sock)) {
		mask |= EF_OUTPUT;
	}
	if (mExceptFS.isSet(sock)) {
		mask |= EF_ERROR;
	}
	if (mCloseFS.isSet(sock)) {
		mask |= EF_CLOSE;
	}
	return mask;
}



int ConnSelect::revGet(tSocket sock) {
	int mask = 0;
	if (mResReadFS.isSet(sock)) {
		mask |= EF_INPUT;
	}
	if (mResWriteFS.isSet(sock)) {
		mask |= EF_OUTPUT;
	}
	if (mResExceptFS.isSet(sock)) {
		mask |= EF_ERROR;
	}
	if (mCloseFS.isSet(sock)) {
		mask |= EF_CLOSE;
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
	int ret = ::select(static_cast<int> (mMaxSocket), &mResReadFS, &mResWriteFS, &mResExceptFS, (timeval *)(&timeout));
	if (SOCK_ERROR(ret)) {
		return -1;
	}

	clearRevents();
	setRevents(mResReadFS, EF_INPUT);
	setRevents(mResWriteFS, EF_OUTPUT);
	setRevents(mResExceptFS, EF_ERROR);
	setRevents(mCloseFS, EF_CLOSE);
	return ret;
}



void ConnSelect::clearRevents(void) {
	static tResList::iterator it_e = mResList.end();
	for (tResList::iterator it = mResList.begin(); it != it_e; ++it) {
		(*it)->mRevents = 0;
	}
}



void ConnSelect::setRevents(SelectFd & fdset, unsigned mask) {
	tSocket sock;
	ChooseRes * chooseRes = NULL;
	#ifdef _WIN32
	for (unsigned i = 0; i < fdset.fd_count; ++i) {
		sock = fdset.fd_array[i];
		if (fdset.isSet(sock)) {
			chooseRes = mResList.find(sock);
			if (!chooseRes) {
				mResList.add(sock, new ChooseRes(sock, 0, mask));
			} else {
				chooseRes->mRevents |= mask;
			}
		}
	}
	#else
	for (unsigned i = 0; i < FD_SETSIZE; ++i) {
		sock = i;
		if (fdset.isSet(sock)) {
			chooseRes = mResList.find(sock);
			if (!chooseRes) {
				mResList.add(sock, new ChooseRes(sock, 0, mask));
			} else {
				chooseRes->mRevents |= mask;
			}
		}
	}
	#endif
}


} // namespace server

#endif // USE_SELECT

/**
 * $Id$
 * $HeadURL$
 */
