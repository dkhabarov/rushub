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

#ifndef OBJ_H
#define OBJ_H

#include "stdinc.h"

#include <iostream>
#include <fstream>
#include <sstream> // operation << for string
#include <vector>
#ifndef _WIN32
	#include <memory.h>
	#include <syslog.h> 
#endif

using namespace ::std;

namespace utils {

enum {
	LEVEL_FATAL, // fatal error
	LEVEL_ERROR, // simple error
	LEVEL_WARN,  // warning
	LEVEL_INFO,  // information
	LEVEL_DEBUG, // debug
	LEVEL_TRACE  // tracing
};

/** Main stream of log system */
#define logStream() logStreamLine(__LINE__)


#ifndef STR_LEN
# define STR_LEN(S) S , sizeof(S) / sizeof(S[0]) - 1
#endif


/**
 * NonCopyable class
 */
class NonCopyable {

protected:

	NonCopyable() {}
	virtual ~NonCopyable() {}

private:

	NonCopyable(const NonCopyable &);
	const NonCopyable & operator = (const NonCopyable &);

}; // class NonCopyable


/**
 * Main object class (logger class)
 */
class Obj {

public:

	static bool mSysLogOn;

public:

	Obj();
	Obj(const char * name);
	Obj(const char * name, bool); // Without Count Control (use only if you control this object)
	virtual ~Obj();

	///< Get counts of objects
	static const long getCount();

	///< Return log straem
	int log(int level);

	///< Return current log stream with line
	ostream & logStreamLine(const int line);

	///< Return class name
	const char * getClassName();

protected:

	///< Max log level of events
	static int mMaxLevel;

	static ofstream mOfs;
	static string * mLogsPath;

protected:

	///< Set class name
	void setClassName(const char * name);

	///< Main function putting log in stream
	virtual bool strLog();

	///< Return a simple log stream
	ostream & simpleLogStream();

private:

	///< Class name
	const char * mClassName;

	///< output log stream
	ostream * mToLog;

	///< Objects counter
	static volatile long mCounterObj;
	static int mLevel;
	static bool mCout;
	static const char * mLevelNames[];

	static ostringstream mSysLogOss;
	static ostringstream mBufOss;

	// Loading buffer
	typedef pair<int, string> Pair;
	static vector<Pair> mLoadBuf;

private:

	///< log function
	static ostream & log();

	static ostream & openLog();
	static bool saveInBuf();
	static void loadFromBuf(ostream &);

	///< Return level for syslog
	static int sysLogLevel(int level);

	Obj(const Obj &);
	Obj & operator = (const Obj &);

}; // class Obj

} // namespace utils

#endif // OBJ_H

/**
 * $Id$
 * $HeadURL$
 */
