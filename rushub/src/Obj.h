/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz
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

#ifndef OBJ_H
#define OBJ_H

#include <iostream>
#include <fstream>
#include <sstream> // operation << for string
#include <vector>
#ifndef _WIN32
	#define __int64 long long
	#include <memory.h>
	#include <syslog.h> 
#endif

using namespace ::std;



/** Main object class (log class) */
class Obj {

public:

	static bool mbSysLogOn;

public:

	Obj();
	Obj(const char * name);
	virtual ~Obj();

	/** Get counts of objects */
	static int GetCount();

	/** Return log straem */
	int Log(int level);

	/** Return errlog stream */
	int ErrLog(int level);

	/** Return current log stream */
	inline ostream & LogStream() {
		return *mToLog;
	}

protected:

	/** Class name */
	const char * mClassName;

	/** Max log level of events */
	static int miMaxLevel;

	/** Max log level of errors */
	static int miMaxErrLevel;

	static ofstream mOfs;
	static string * msPath;

protected:

	/** Set class name */
	void SetClassName(const char * name);

	/** Main function putting log in stream */
	virtual bool strLog();

private:

	/** Objects counter */
	static int mCounterObj;
	static int miLevel;
	static bool mbIsErrorLog;

	/** output log stream */
	ostream * mToLog;

	static ostringstream mSysLogOss;
	static ostringstream mBufOss;

	// Loading buffer
	typedef pair<pair<int, bool>, string> Pair;
	static vector<Pair> mLoadBuf;

private:

	/** Log function */
	ostream & log();

	/** Errlog function */
	ostream & errLog();

	ostream & openLog();
	bool saveInBuf();
	void loadFromBuf(ostream &);

	/** Return level for syslog */
	int sysLogLevel(int iLevel, bool bIsError = false);

}; // class Obj

#endif // OBJ_H

/**
 * $Id$
 * $HeadURL$
 */
