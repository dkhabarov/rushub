/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz
 *
 * modified: 27 Aug 2009
 * Copyright (C) 2009-2012 by Setuper
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

#include "ConnPoll.h"

#ifndef _WIN32

namespace server {



ConnPoll::ConnPoll() {
	mvFD.reserve(1024);
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
	int miliSec = static_cast<int> (timeout.msec());
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



bool ConnPoll::optIn(tSocket sock, EventFlag mask) {
	PollFd & pollFd = mvFD[sock];
 	unsigned events = pollFd.events;
	if (!events && mask) {
		pollFd.fd = sock;
	}

	if (mask & EF_CLOSE) {
		pollFd.events = 0;
	} else {
		if (mask & EF_INPUT) {
			events = unsigned(POLLIN | POLLPRI);
		}
		if (mask & EF_OUTPUT) {
			events |= unsigned(POLLOUT);
		}
		if (mask & EF_ERROR) {
			events |= unsigned(POLLERR | POLLHUP | POLLNVAL);
		}
		pollFd.events |= events;
	}
	return true;
}



void ConnPoll::optOut(tSocket sock, EventFlag mask) {
	PollFd & pollFd = mvFD[sock];
 	unsigned events = ~(0u);
	if (mask & EF_INPUT) {
		events = ~unsigned(POLLIN | POLLPRI);
	}
	if (mask & EF_OUTPUT) {
		events &= ~unsigned(POLLOUT);
	}
	if (mask & EF_ERROR) {
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
		mask = EF_CLOSE;
	} else {
		if (events & (POLLIN | POLLPRI)) {
			mask |= EF_INPUT;
		}
		if (events & POLLOUT) {
			mask |= EF_OUTPUT;
		}
		if (events & (POLLERR | POLLHUP | POLLNVAL)) {
			mask |= EF_ERROR;
		}
	}
	return mask;
}



int ConnPoll::revGet(tSocket sock) {
	PollFd & pollFd = mvFD[sock];
	unsigned events = pollFd.revents;
	int mask = 0;
	if (!pollFd.events && (pollFd.fd == sock)) {
		mask = EF_CLOSE;
	}
	if (events & (POLLIN | POLLPRI)) {
		mask |= EF_INPUT;
	}
	if (events & POLLOUT) {
		mask |= EF_OUTPUT;
	}
	if (events & (POLLERR | POLLHUP | POLLNVAL)) {
		mask |= EF_ERROR;
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

/**
 * $Id$
 * $HeadURL$
 */
