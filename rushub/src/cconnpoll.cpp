/*
 * RusHub - hub server for Direct Connect peer to peer network.

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

#include "cconnpoll.h"

#ifndef _WIN32

namespace nServer {

cConnPoll::cConnPoll() {
	mvFD.reserve(16384);
}

cConnPoll::~cConnPoll() {
}

bool cConnPoll::OptIn(tSocket Socket, tEventFlag eMask) {
 	unsigned iEvent = FD(Socket).events;
	if(!iEvent && eMask) FD(Socket).fd = Socket;

	if (eMask & eEF_CLOSE) FD(Socket).events = 0;
	else {
		if (eMask & eEF_INPUT) iEvent = unsigned(POLLIN | POLLPRI);
		if (eMask & eEF_OUTPUT) iEvent |= unsigned(POLLOUT);
		if (eMask & eEF_ERROR) iEvent |= unsigned(POLLERR | POLLHUP | POLLNVAL);
		FD(Socket).events |= iEvent;
	}
	return true;
}

void cConnPoll::OptOut(tSocket Socket, tEventFlag eMask) {
 	unsigned iEvent = ~(0u);
	if(eMask & eEF_INPUT) iEvent = ~unsigned(POLLIN | POLLPRI);
	if(eMask & eEF_OUTPUT) iEvent &= ~unsigned(POLLOUT);
	if(eMask & eEF_ERROR) iEvent &= ~unsigned(POLLERR | POLLHUP | POLLNVAL);
	if(!(FD(Socket).events &= iEvent)) FD(Socket).reset();
}

int cConnPoll::OptGet(tSocket Socket) {
	int eMask = 0;
 	unsigned iEvent = FD(Socket).events;
	if(!iEvent && (FD(Socket).fd == Socket)) eMask = eEF_CLOSE;
	else {
		if (iEvent & (POLLIN | POLLPRI)) eMask |= eEF_INPUT;
		if (iEvent & POLLOUT) eMask |= eEF_OUTPUT;
		if (iEvent & (POLLERR | POLLHUP | POLLNVAL)) eMask |= eEF_ERROR;
	}
	return eMask;
}

int cConnPoll::RevGet(tSocket Socket) {
	int eMask = 0;
	cPollFD & PollFD = FD(Socket);
 	unsigned iEvent = PollFD.revents;
	if(!PollFD.events && (PollFD.fd == Socket)) eMask = eEF_CLOSE;
	if(iEvent & (POLLIN | POLLPRI)) eMask |= eEF_INPUT;
	if(iEvent & POLLOUT) eMask |= eEF_OUTPUT;
	if(iEvent & (POLLERR | POLLHUP | POLLNVAL)) eMask |= eEF_ERROR;
	return eMask;
}

bool cConnPoll::RevTest(cPollFD & PollFD) {
 	if(SOCK_INVALID(PollFD.fd)) return false;
	if(!PollFD.events) return true;
 	unsigned iEvent = PollFD.revents;
 	if(!iEvent) return false;
	if(iEvent & POLLOUT) return true;
	if(iEvent & (POLLIN | POLLPRI)) return true;
	if(iEvent & (POLLERR | POLLHUP | POLLNVAL)) return true;
	return false;
}

bool cConnPoll::RevTest(tSocket Socket) {
	return RevTest(FD(Socket));
}

int cConnPoll::poll(int iMiliSec) {
	int tmp, n, iRet, iDone, iSize = mvFD.size();
	n = iRet = iDone = 0;
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

bool cConnPoll::AddConn(cConnBase * conn) {
	if(!cConnChoose::AddConn(conn)) return false;
	if(mMaxSocket >= (tSocket)mvFD.size()) mvFD.resize(mMaxSocket + (mMaxSocket >> 1));
	return true;
}

}; // nServer

#endif // _WIN32
