/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
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

#ifndef THREAD_H
#define THREAD_H

#ifdef _WIN32
	#include <winsock2.h>
	#include <mswsock.h>
	#include <ws2tcpip.h>
	#include <ipexport.h>
	#include <process.h>
#else
	#include <pthread.h>
#endif

namespace utils {

typedef void (ThreadCall) (void *);

// Class encapsulating OS thread. Thread initiation/termination is done
// using special functions rather than in constructor/destructor so that
// thread isn't created during object construction by accident, causing
// newly created thread to access half-initialised object. Same applies
// to the destruction process: Thread should be terminated before object
// destruction begins, otherwise it can access half-destructed object.

class Thread {

public:

	inline Thread() {
	}

	// Creates OS thread. 'func' is main thread function. It'll be passed
	// 'arg' as an argument.
	void start(ThreadCall * func, void * arg);

	// Waits for thread termination.
	void stop();

	// These are internal members. They should be private, however then
	// they would not be accessible from the main C routine of the thread.
	ThreadCall * mFunc;
	void * mArg;

private:

	#ifdef _WIN32
		HANDLE mHandle;
	#else
		pthread_t mHandle;
	#endif

	Thread(const Thread &);
	const Thread & operator = (const Thread &);

}; // class Thread

} // namespace utils

#endif // THREAD_H

/**
 * $Id$
 * $HeadURL$
 */
