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

#ifndef MUTEX_H
#define MUTEX_H

#ifdef _WIN32
	#include <windows.h>
#else
	#include <pthread.h>
#endif


template<typename M>
class ScopedLock {

public:

	ScopedLock(M & m) : mutex(m) {
		mutex.lock();
	}
	~ScopedLock() {
		mutex.unlock();
	}

private:

	M & mutex;

};

class Mutex {

public:

	inline Mutex() {
		#ifdef _WIN32
			InitializeCriticalSection(&mCs);
		#else
			pthread_mutex_init(&mMutex, NULL);
		#endif
	}

	inline ~Mutex() {
		#ifdef _WIN32
			DeleteCriticalSection(&mCs);
		#else
			pthread_mutex_destroy(&mMutex);
		#endif
	}

	inline void lock() {
		#ifdef _WIN32
			EnterCriticalSection(&mCs);
		#else
			pthread_mutex_lock(&mMutex);
		#endif
	}

	inline void tryLock() {
		#ifdef _WIN32
			TryEnterCriticalSection(&mCs);
		#else
			pthread_mutex_trylock(&mMutex);
		#endif
	}

	inline void unlock() {
		#ifdef _WIN32
			LeaveCriticalSection(&mCs);
		#else
			pthread_mutex_unlock(&mMutex);
		#endif
	}

	typedef ScopedLock<Mutex> Lock;

private:

	#ifdef _WIN32
		CRITICAL_SECTION mCs;
	#else
		pthread_mutex_t mMutex;
	#endif

private:

	Mutex(const Mutex &);
	void operator = (const Mutex &);

}; // class Mutex


#endif // MUTEX_H

/**
 * $Id$
 * $HeadURL$
 */
