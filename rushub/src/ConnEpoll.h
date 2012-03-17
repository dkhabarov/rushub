/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz
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

#ifndef CONN_EPOLL_H
#define CONN_EPOLL_H

#ifdef USE_EPOLL

#include "ConnChoose.h"

#include <sys/epoll.h>

namespace server {



/// ConnEpoll
class ConnEpoll : public ConnChoose {

public:

	ConnEpoll();
	~ConnEpoll();

	int choose(Time &);

	bool optIn(tSocket, EventFlag);
	void optOut(tSocket, EventFlag);
	int optGet(tSocket);
	int revGet(tSocket);
	bool revTest(tSocket);

private:

	int mFd; ///< epoll fd

}; // ConnEpoll

}; // server

#endif // USE_EPOLL

#endif // CONN_EPOLL_H

/**
 * $Id$
 * $HeadURL$
 */
