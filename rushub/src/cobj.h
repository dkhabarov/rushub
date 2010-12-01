/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * Copyright (C) 2009-2010 by Setuper
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

#ifndef COBJ_H
#define COBJ_H

#include <iostream>
#include <fstream>
#include <sstream> /** operation << for string */
#ifndef _WIN32
	#define __int64 long long
	#include <memory.h>
	#include <syslog.h> 
#endif

using namespace std;

#ifdef _WIN32
template<class T1, class T2> inline
	T2 For_each(T1 first, T1 last, T2 func) {
		try {
			for( ; first != last; ++first) func(*first);
		} catch(...) { throw "for_each fatal error"; }
		return func;
	}
#else
template<class T1, class T2> inline
	T2 For_each(T1 first, T1 last, T2 func) {
		return for_each(first, last, func);
	}
#endif // _WIN32


/** Main object class (log class) */
class cObj {

public:

	/** Class name */
	const char *mClassName;

	/** output log stream */
	ostream *mToLog;

	/** Empty string */
	static const string msEmpty;

	/** Max log level of events */
	static int miMaxLevel;

	/** Max log level of errors */
	static int miMaxErrLevel;

	static ofstream mOfs;
	static char * msPath;

	static bool mbSysLogOn;

public:

	cObj();
	cObj(const char *name);
	virtual ~cObj();

	/** Set class name */
	void SetClassName(const char *name);

	/** Get counts of objects */
	static int GetCount() { return mCounterObj; }

	/** Main function putting log in stream */
	virtual int StrLog(ostream &ostr, int iLevel, int iMaxLevel, bool bIsError = false);

	/** Return log straem */
	int Log(int level);

	/** Return errlog stream */
	int ErrLog(int level);

	/** Virtual log function */
	virtual ostream &Log();

	/** Virtual errlog function */
	virtual ostream &ErrLog();

	/** Return current log stream */
	ostream &LogStream();

	/** Return level for syslog */
	static int SysLogLevel(int iLevel, bool bIsError = false);

private:

	/** Objects counter */
	static int mCounterObj;
	static int miLevel;
	static ostringstream mOss;
	static bool mbIsErrorLog;

}; // cObj

#endif // COBJ_H
