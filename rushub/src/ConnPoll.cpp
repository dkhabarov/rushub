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
	if (mMaxSocket >= static_cast<tSocket> (mvFD.size())) {
		mvFD.resize(static_cast<unsigned int> (mMaxSocket + (mMaxSocket >> 1)));
	}
	return true;
}



int ConnPoll::choose(Time & timeout) {
	int ret = 0, n = 0, miliSec = static_cast<int> (timeout.msec());
	unsigned int tmp, done = 0u, size = mvFD.size();
	while (size) {
		// poll 1024 socks max
		tmp = size > 1024u ? 1024u : size;
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
	PollFd & pollFd = mvFD[static_cast<unsigned int> (sock)];
 	short events = pollFd.events;
	if (!events && mask) {
		pollFd.fd = sock;
	}

	if (mask & EF_CLOSE) {
		pollFd.events = short(0);
	} else {
		if (mask & EF_INPUT) {
			events = short(POLLIN | POLLPRI);
		}
		if (mask & EF_OUTPUT) {
			events |= short(POLLOUT);
		}
		if (mask & EF_ERROR) {
			events |= short(POLLERR | POLLHUP | POLLNVAL);
		}
		pollFd.events = short(pollFd.events | events);
	}
	return true;
}



void ConnPoll::optOut(tSocket sock, EventFlag mask) {
	PollFd & pollFd = mvFD[static_cast<unsigned int> (sock)];
 	short events = ~short(0);
	if (mask & EF_INPUT) {
		events = ~short(POLLIN | POLLPRI);
	}
	if (mask & EF_OUTPUT) {
		events &= ~short(POLLOUT);
	}
	if (mask & EF_ERROR) {
		events &= ~short(POLLERR | POLLHUP | POLLNVAL);
	}
	pollFd.events = short(pollFd.events & events);
	if (!pollFd.events) {
		pollFd.reset();
	}
}



int ConnPoll::optGet(tSocket sock) {
	PollFd & pollFd = mvFD[static_cast<unsigned int> (sock)];
	short events = pollFd.events;
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
	PollFd & pollFd = mvFD[static_cast<unsigned int> (sock)];
	short events = pollFd.revents;
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
	PollFd & pollFd = mvFD[static_cast<unsigned int> (sock)];
 	if (SOCK_INVALID(pollFd.fd)) {
		return false;
	}
	if (!pollFd.events) {
		return true;
	}
 	short events = pollFd.revents;
 	if (!events) {
		return false;
	}
	if (events & (POLLIN | POLLPRI | POLLOUT | POLLERR | POLLHUP | POLLNVAL)) {
		return true;
	}
	return false;
}


} // namespace server

#endif // _WIN32

/**
 * $Id$
 * $HeadURL$
 */
