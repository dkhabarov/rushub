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

ConnSelect::~ConnSelect() {
	tResList::iterator it;
	sChooseRes *ChR;
	tSocket sock;
	for(it = mResList.begin(); it != mResList.end(); ) {
		ChR = *it;
		++it;
		if(ChR) {
			sock = ChR->mFd;
			delete ChR;
			mResList.Remove(sock);
		}
	}
}

bool ConnSelect::OptIn(tSocket sock, tEventFlag eMask) {
	if(eMask & eEF_INPUT && !mReadFS.Set(sock)) return false;
	if(eMask & eEF_OUTPUT && !mWriteFS.Set(sock)) return false;
	if(eMask & eEF_ERROR && !mExceptFS.Set(sock)) return false;
	if(eMask & eEF_CLOSE && !mCloseFS.Set(sock)) return false;
	sChooseRes *ChR = mResList.Find(sock);
	if(!ChR)
		mResList.Add(sock, new sChooseRes(sock, eMask));
	else
		ChR->mEvents |= eMask;
	return true;
}

void ConnSelect::OptOut(tSocket sock, tEventFlag eMask) {
	if(eMask & eEF_INPUT ) mReadFS.Clr(sock);
	if(eMask & eEF_OUTPUT) mWriteFS.Clr(sock);
	if(eMask & eEF_ERROR ) mExceptFS.Clr(sock);
	if(eMask & eEF_CLOSE ) mCloseFS.Clr(sock);

	sChooseRes *ChR = mResList.Find(sock);
	if(ChR) {
		ChR->mEvents -= (ChR->mEvents & eMask);
		if(!ChR->mEvents) {
			delete ChR;
			mResList.Remove(sock);
		}
	}
}

int ConnSelect::OptGet(tSocket sock) {
	int eMask = 0;
	if(mReadFS.IsSet(sock)  ) eMask |= eEF_INPUT;
	if(mWriteFS.IsSet(sock) ) eMask |= eEF_OUTPUT;
	if(mExceptFS.IsSet(sock)) eMask |= eEF_ERROR;
	if(mCloseFS.IsSet(sock) ) eMask |= eEF_CLOSE;
	return eMask;
}

int ConnSelect::RevGet(tSocket sock) {
	int eMask = 0;
	if(mResReadFS.IsSet(sock)  ) eMask |= eEF_INPUT;
	if(mResWriteFS.IsSet(sock) ) eMask |= eEF_OUTPUT;
	if(mResExceptFS.IsSet(sock)) eMask |= eEF_ERROR;
	if(mCloseFS.IsSet(sock)    ) eMask |= eEF_CLOSE;
	return eMask;
}

bool ConnSelect::RevTest(tSocket sock) {
	if(mResWriteFS.IsSet(sock) ) return true;
	if(mResReadFS.IsSet(sock)  ) return true;
	if(mResExceptFS.IsSet(sock)) return true;
	if(mCloseFS.IsSet(sock)    ) return true;
	return false;
}

/** Do select */
int ConnSelect::Choose(Time &timeout) {
	mResReadFS = mReadFS;
	mResWriteFS = mWriteFS;
	mResExceptFS = mExceptFS;

	/** select */
	int ret = ::select(mMaxSocket, &mResReadFS, &mResWriteFS, &mResExceptFS, (timeval *)(&timeout));
	if(SOCK_ERROR(ret)) return -1;

	ClearRevents();
	SetRevents(mResReadFS, eEF_INPUT);
	SetRevents(mResWriteFS, eEF_OUTPUT);
	SetRevents(mResExceptFS, eEF_ERROR);
	SetRevents(mCloseFS, eEF_CLOSE);
	return ret;
}

void ConnSelect::ClearRevents(void) {
	sChooseRes *ChR;
	for(tResList::iterator it = mResList.begin(); it != mResList.end(); ++it)
		if((ChR = (*it)) != NULL) ChR->mRevents = 0;
}


void ConnSelect::SetRevents(cSelectFD &fdset, unsigned eMask) {
	tSocket sock;
	sChooseRes *ChR;
	#ifdef _WIN32
	for(unsigned i = 0; i < fdset.fd_count; ++i) {
		sock = fdset.fd_array[i];
		ChR = mResList.Find(sock);
		if(!ChR)
			mResList.Add(sock, new sChooseRes(sock, 0, eMask));
		else
			ChR->mRevents |= eMask;
	}
	#else
	for(unsigned i = 0; i < FD_SETSIZE; ++i) {
		sock = i;
		if(FD_ISSET(sock, &fdset)) {
			ChR = mResList.Find(sock);
			if(!ChR)
				mResList.Add(sock, new sChooseRes(sock, 0, eMask));
			else
				ChR->mRevents |= eMask;
		}
	}
	#endif
}

}; // server

#endif // USE_SELECT