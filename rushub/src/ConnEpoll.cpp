/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz
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

#include "ConnEpoll.h"

#ifdef USE_EPOLL

namespace server {



ConnEpoll::ConnEpoll() {
	mFd = epoll_create(10000);
	if (mFd == -1) {
		// errno = ENOMEM
	}
}



ConnEpoll::~ConnEpoll() {
	::close(mFd);
}



int ConnEpoll::choose(Time & timeout) {
	static struct epoll_event * events;
	return epoll_wait(mFd, events, 1024, static_cast<int> (timeout.msec()) + 1);
}



bool ConnEpoll::optIn(tSocket sock, tEventFlag mask) {

	if (mask & eEF_CLOSE) {
		int res = epoll_ctl(epfd, EPOLL_CTL_DEL, sock, &ev);
		if (res != 0) {
			// error
			return false;
		}
	}
	
	struct epoll_event ev;
	ev.data.fd = sock;

	if (mask & eEF_INPUT) {
		ev.events = EPOLLIN | POLLPRI;
	}
	if (mask & eEF_OUTPUT) {
		ev.events |= EPOLLOUT;
	}
	if (mask & eEF_ERROR) {
		ev.events |= (EPOLLERR | EPOLLHUP);
	}

	int res = epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev);
	if (res != 0) {
		// error
		return false;
	}
	return true;
}



void ConnEpoll::optOut(tSocket sock, tEventFlag mask) {

	if (mask & eEF_CLOSE) {
		int res = epoll_ctl(epfd, EPOLL_CTL_DEL, sock, &ev);
		if (res != 0) {
			// error
			return false;
		}
		return true;
	}
	
	struct epoll_event ev;
	ev.data.fd = sock;

	if (mask & eEF_INPUT) {
		ev.events = EPOLLIN | POLLPRI;
	}
	if (mask & eEF_OUTPUT) {
		ev.events |= EPOLLOUT;
	}
	if (mask & eEF_ERROR) {
		ev.events |= (EPOLLERR | EPOLLHUP);
	}

	int res = epoll_ctl(epfd, EPOLL_CTL_MOD, sock, &ev);
	if (res != 0) {
		// error
		return false;
	}
	return true;
}



int ConnEpoll::optGet(tSocket sock) {
	return eEF_INPUT | eEF_OUTPUT | eEF_ERROR | eEF_CLOSE;
}



int ConnEpoll::revGet(tSocket sock) {
	return 0;
}



bool ConnEpoll::revTest(tSocket sock) {
	return false;
}


}; // server

#endif // USE_EPOLL

/**
 * $Id$
 * $HeadURL$
 */
