/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
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

#include "Thread.h"

#ifndef _WIN32
	#include <signal.h>
#else
	#include <mswsock.h>
	#include <ws2tcpip.h>
	#include <ipexport.h>
	#include <process.h>
#endif

namespace utils {

Mutex Thread::mMutex;


extern "C" {
	#ifdef _WIN32
		static unsigned int __stdcall call(void * arg) {
			Thread * self = (Thread *) arg;
			self->mFunc(self->mArg);
			return 0;
		}
	#else
		static void * call(void * arg) {
			// Following code will guarantee more predictable latecnies as it'll
			// disallow any signal handling in the I/O thread.
			sigset_t sigset;
			/*int res =*/ sigfillset(&sigset);
			//assert(res == 0);
			/*res =*/ pthread_sigmask(SIG_BLOCK, &sigset, NULL);
			//assert(res == 0);

			Thread * self = (Thread *) arg;
			self->mFunc(self->mArg);
			return NULL;
		}
	#endif
}



Thread::~Thread() {
}



void Thread::start(ThreadCall * func, void * arg) {
	mFunc = func;
	mArg = arg;
	#ifdef _WIN32
		mHandle = (HANDLE) _beginthreadex(NULL, 0, &::utils::call, this, 0, NULL);
		//assert(mHandle != NULL);
	#else
		/*int res =*/ pthread_create(&mHandle, NULL, call, this);
		//assert(res == 0);
	#endif
}



void Thread::stop() {
	#ifdef _WIN32
		/*DWORD res =*/ WaitForSingleObject(mHandle, INFINITE);
		//assert(res != WAIT_FAILED);
		/*BOOL res2 =*/ CloseHandle(mHandle);
		//assert(res2 != 0);
	#else
		/*int res =*/ pthread_join(mHandle, NULL);
		//assert(res == 0);
	#endif
}



long Thread::safeInc(volatile long & value) {
	#ifdef _WIN32
		return InterlockedIncrement(&value);
	#else
		Mutex::Lock l(mMutex);
		long res = ++value;
		return res;
	#endif
}



long Thread::safeDec(volatile long & value) {
	#ifdef _WIN32
		return InterlockedDecrement(&value);
	#else
		Mutex::Lock l(mMutex);
		long res = --value;
		return res;
	#endif
}



long Thread::safeExc(volatile long & target, long value) {
	#ifdef _WIN32
		return InterlockedExchange(&target, value);
	#else
		Mutex::Lock l(mMutex);
		long res = target;
		target = value;
		return res;
	#endif
}

} // namespace utils

/**
 * $Id$
 * $HeadURL$
 */
