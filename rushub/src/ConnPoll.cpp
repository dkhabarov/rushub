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

#include "ConnPoll.h"

#ifndef _WIN32

namespace server {



ConnPoll::ConnPoll() {
	mvFD.reserve(16384);
}



ConnPoll::~ConnPoll() {
}



bool ConnPoll::addConn(ConnBase * connBase) {
	if (!ConnChoose::addConn(connBase)) {
		return false;
	}
	if (mMaxSocket >= (tSocket)mvFD.size()) {
		mvFD.resize(mMaxSocket + (mMaxSocket >> 1));
	}
	return true;
}



int ConnPoll::choose(Time & timeout) {
	int miliSec = timeout.MiliSec() + 1;
	int tmp, n = 0, ret = 0, done = 0, size = mvFD.size();
	while (size) {
		// poll 1024 socks max
		tmp = size > 1024 ? 1024 : size;
		ret = ::poll(&(mvFD[done]), tmp, miliSec);
		if (ret < 0) {
			return -1;
		}
		size -= tmp;
		done += tmp;
		n += ret;
	}
	return n;
}



bool ConnPoll::optIn(tSocket sock, tEventFlag mask) {
	PollFd & pollFd = mvFD[sock];
 	unsigned events = pollFd.events;
	if (!events && mask) {
		pollFd.fd = sock;
	}

	if (mask & eEF_CLOSE) {
		pollFd.events = 0;
	} else {
		if (mask & eEF_INPUT) {
			events = unsigned(POLLIN | POLLPRI);
		}
		if (mask & eEF_OUTPUT) {
			events |= unsigned(POLLOUT);
		}
		if (mask & eEF_ERROR) {
			events |= unsigned(POLLERR | POLLHUP | POLLNVAL);
		}
		pollFd.events |= events;
	}
	return true;
}



void ConnPoll::optOut(tSocket sock, tEventFlag mask) {
	PollFd & pollFd = mvFD[sock];
 	unsigned events = ~(0u);
	if (mask & eEF_INPUT) {
		events = ~unsigned(POLLIN | POLLPRI);
	}
	if (mask & eEF_OUTPUT) {
		events &= ~unsigned(POLLOUT);
	}
	if (mask & eEF_ERROR) {
		events &= ~unsigned(POLLERR | POLLHUP | POLLNVAL);
	}
	if (!(pollFd.events &= events)) {
		pollFd.reset();
	}
}



int ConnPoll::optGet(tSocket sock) {
	PollFd & pollFd = mvFD[sock];
	unsigned events = pollFd.events;
	int mask = 0;
	if (!events && (pollFd.fd == sock)) {
		mask = eEF_CLOSE;
	} else {
		if (events & (POLLIN | POLLPRI)) {
			mask |= eEF_INPUT;
		}
		if (events & POLLOUT) {
			mask |= eEF_OUTPUT;
		}
		if (events & (POLLERR | POLLHUP | POLLNVAL)) {
			mask |= eEF_ERROR;
		}
	}
	return mask;
}



int ConnPoll::revGet(tSocket sock) {
	PollFd & pollFd = mvFD[sock];
	unsigned events = pollFd.revents;
	int mask = 0;
	if (!pollFd.events && (pollFd.fd == sock)) {
		mask = eEF_CLOSE;
	}
	if (events & (POLLIN | POLLPRI)) {
		mask |= eEF_INPUT;
	}
	if (events & POLLOUT) {
		mask |= eEF_OUTPUT;
	}
	if (events & (POLLERR | POLLHUP | POLLNVAL)) {
		mask |= eEF_ERROR;
	}
	return mask;
}



bool ConnPoll::revTest(tSocket sock) {
	PollFd & pollFd = mvFD[sock];
 	if (SOCK_INVALID(pollFd.fd)) {
		return false;
	}
	if (!pollFd.events) {
		return true;
	}
 	unsigned events = pollFd.revents;
 	if (!events) {
		return false;
	}
	if (events & (POLLIN | POLLPRI | POLLOUT | POLLERR | POLLHUP | POLLNVAL)) {
		return true;
	}
	return false;
}


}; // server

#endif // _WIN32
