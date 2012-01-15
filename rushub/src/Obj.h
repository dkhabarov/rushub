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

#include <iostream>
#include <fstream>
#include <sstream> // operation << for string
#include <vector>
#ifndef _WIN32
	#ifndef __int64
		#define __int64 long long
	#endif
	#include <memory.h>
	#include <syslog.h> 
#endif

using namespace ::std;

namespace utils {

enum {
	FATAL, // falat error
	ERR,   // simple error
	WARN,  // warning
	INFO,  // information
	DEBUG, // debug
	TRACE  // tracing
};

/** Main stream of log system */
#define logStream() logStreamLine(__LINE__)


/** NonCopyable class */
class NonCopyable {
	protected:
		NonCopyable() {}
		~NonCopyable() {}
	private:
		NonCopyable(const NonCopyable &);
		const NonCopyable & operator = (const NonCopyable &);
}; // class NonCopyable


/** Main object class (logger class) */
class Obj {

public:
	static bool mSysLogOn;

public:

	Obj();
	Obj(const char * name);
	Obj(const char * name, bool); // Without Count Control (use only if you control this object)
	virtual ~Obj();

	/** Get counts of objects */
	static int getCount();

	/** Return log straem */
	int log(int level);

	/** Return current log stream with line */
	ostream & logStreamLine(const int line);

	/** Return class name */
	const char * getClassName();

protected:

	/** Class name */
	const char * mClassName;

	/** Max log level of events */
	static int mMaxLevel;

	/** Max log level of errors */
	static int mMaxErrLevel;

	static ofstream mOfs;
	static string * mLogsPath;

protected:

	/** Set class name */
	void setClassName(const char * name);

	/** Main function putting log in stream */
	virtual bool strLog();

	/** Return a simple log stream */
	ostream & simpleLogStream();

private:

	/** Objects counter */
	static int mCounterObj;
	static int mLevel;
	static bool mCout;
	static const char * mLevelNames[];

	/** output log stream */
	ostream * mToLog;

	static ostringstream mSysLogOss;
	static ostringstream mBufOss;

	// Loading buffer
	typedef pair<int, string> Pair;
	static vector<Pair> mLoadBuf;

private:

	/** log function */
	ostream & log();

	ostream & openLog();
	bool saveInBuf();
	void loadFromBuf(ostream &);

	/** Return level for syslog */
	int sysLogLevel(int level);

}; // class Obj

}; // namespace utils

#endif // OBJ_H

/**
 * $Id$
 * $HeadURL$
 */
