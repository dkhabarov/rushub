/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz
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

#ifndef CONN_POLL_H
#define CONN_POLL_H

#ifndef _WIN32

#include "ConnChoose.h"

#include <vector>
#include <sys/poll.h>

#if !defined _SYS_POLL_H_ && !defined _SYS_POLL_H && !defined pollfd && !defined _POLL_EMUL_H_

	/** The poll file descriptor structure (where not defined) */
	struct pollfd {
		int fd;        /** File descriptor */
		short events;  /** Requested events */
		short revents; /** Returned events */
	}; // pollfd

#endif

using std::vector;

namespace server {



/** Unix poll socket chooser */
class ConnPoll : public ConnChoose {

public:

	/// Poll file descriptor struct
	struct PollFd: public pollfd {
		PollFd() {
			reset();
		}
		inline void reset() {
			fd = -1;
			events = revents = 0;
		}
	}; // PollFd

	typedef vector<PollFd> tFDArray;

public:

	ConnPoll();
	~ConnPoll();

	bool addConn(ConnBase *);

	int choose(Time &);

	bool optIn(tSocket, EventFlag);
	void optOut(tSocket, EventFlag);
	int optGet(tSocket);
	int revGet(tSocket);
	bool revTest(tSocket);

private:

	tFDArray mvFD;

}; // class ConnPoll

} // namespace server

#endif // _WIN32

#endif // CONN_POLL_H

/**
 * $Id$
 * $HeadURL$
 */
