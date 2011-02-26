/*
 * RusHub - hub server for Direct Connect peer to peer network.

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

#ifndef EXCEPTION_H
#define EXCEPTION_H

#ifdef _WIN32

#include "DcServer.h"

#include <iostream>
#include <string>

using ::std::string;
using namespace ::dcserver;

#define FILE_NAME "exception.txt"
#define BUFFERSIZE 0x200
#define MAX_RECURSIONS 30

class Exception {

public:

	static int recursion;
	static bool first;

public:

	Exception() {
	}

	~Exception() {
	}

	static long __stdcall ExceptionFilter(LPEXCEPTION_POINTERS e);
	static int Init(const char * path);
	static int Uninit();
	static int GetFunctionInfo(unsigned long functionAddress, unsigned long stackAddress, char * buff);
	static int GetSourceInfo(unsigned address, char * buff);
	static int GetModuleName(unsigned address, char * buff);
	static void StackTrace(std::ostream & f, unsigned long eip, unsigned long esp, unsigned long ebp);
	static void StackTrace(void * hThread, char * lpszMessage, std::ostream & f, unsigned long eip, unsigned long esp, unsigned long ebp);

}; // Exception


#endif // _WIN32

#endif // EXCEPTION_H