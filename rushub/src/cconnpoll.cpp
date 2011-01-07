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

#include "cconnpoll.h"

#ifndef _WIN32

namespace nServer {

bool cConnPoll::AddConn(cConnBase * conn) {
	if(!cConnChoose::AddConn(conn)) return false;
	if(mMaxSocket >= (tSocket)mvFD.size()) mvFD.resize(mMaxSocket + (mMaxSocket >> 1));
	return true;
}

int cConnPoll::Choose(cTime & timeout) {
	int iMiliSec = timeout.MiliSec() + 1;
	int tmp, n = 0, iRet = 0, iDone = 0, iSize = mvFD.size();
	while(iSize) {
		tmp = iSize;
		if(tmp > 1024) tmp = 1024; // poll 1024 socks max
		iRet = ::poll(&(mvFD[iDone]), tmp, iMiliSec);
		if(iRet < 0) return -1;
		iSize -= tmp;
		iDone += tmp;
		n += iRet;
	}
	return n;
}

bool cConnPoll::OptIn(tSocket Socket, tEventFlag eMask) {
	cPollFD & PollFD = mvFD[Socket];
 	unsigned iEvent = PollFD.events;
	if(!iEvent && eMask) PollFD.fd = Socket;

	if (eMask & eEF_CLOSE) PollFD.events = 0;
	else {
		if (eMask & eEF_INPUT) iEvent = unsigned(POLLIN | POLLPRI);
		if (eMask & eEF_OUTPUT) iEvent |= unsigned(POLLOUT);
		if (eMask & eEF_ERROR) iEvent |= unsigned(POLLERR | POLLHUP | POLLNVAL);
		PollFD.events |= iEvent;
	}
	return true;
}

void cConnPoll::OptOut(tSocket Socket, tEventFlag eMask) {
	cPollFD & PollFD = mvFD[Socket];
 	unsigned iEvent = ~(0u);
	if(eMask & eEF_INPUT) iEvent = ~unsigned(POLLIN | POLLPRI);
	if(eMask & eEF_OUTPUT) iEvent &= ~unsigned(POLLOUT);
	if(eMask & eEF_ERROR) iEvent &= ~unsigned(POLLERR | POLLHUP | POLLNVAL);
	if(!(PollFD.events &= iEvent)) PollFD.reset();
}

int cConnPoll::OptGet(tSocket Socket) {
	cPollFD & PollFD = mvFD[Socket];
	unsigned iEvent = PollFD.events;
	int eMask = 0;
	if(!iEvent && (PollFD.fd == Socket)) eMask = eEF_CLOSE;
	else {
		if (iEvent & (POLLIN | POLLPRI)) eMask |= eEF_INPUT;
		if (iEvent & POLLOUT) eMask |= eEF_OUTPUT;
		if (iEvent & (POLLERR | POLLHUP | POLLNVAL)) eMask |= eEF_ERROR;
	}
	return eMask;
}

int cConnPoll::RevGet(tSocket Socket) {
	cPollFD & PollFD = mvFD[Socket];
	unsigned iEvent = PollFD.revents;
	int eMask = 0;
	if(!PollFD.events && (PollFD.fd == Socket)) eMask = eEF_CLOSE;
	if(iEvent & (POLLIN | POLLPRI)) eMask |= eEF_INPUT;
	if(iEvent & POLLOUT) eMask |= eEF_OUTPUT;
	if(iEvent & (POLLERR | POLLHUP | POLLNVAL)) eMask |= eEF_ERROR;
	return eMask;
}

bool cConnPoll::RevTest(tSocket Socket) {
	cPollFD & PollFD = mvFD[Socket];
 	if(SOCK_INVALID(PollFD.fd)) return false;
	if(!PollFD.events) return true;
 	unsigned iEvent = PollFD.revents;
 	if(!iEvent) return false;
	if(iEvent & (POLLIN | POLLPRI | POLLOUT | POLLERR | POLLHUP | POLLNVAL)) return true;
	return false;
}

}; // nServer

#endif // _WIN32
