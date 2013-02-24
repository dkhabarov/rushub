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

#ifndef LOGGER_H
#define LOGGER_H

#include "Singleton.h"
#include "Thread.h"
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
using namespace ::utils;

namespace utils {

/**
 * Logger class
 */
class Logger : public Singleton<Logger> {

	friend class Singleton<Logger>;

public:

	bool mSysLogOn;

	///< Max log level of events
	int mMaxLevel;

	string * mLogsPath;

public:

	void log(const string & msg);

	///< log function
	void log(int level);

	///< Return a simple log stream
	ostream & simpleLogStream();

	///< Return max log level
	int getMaxLevel();

	///< Return log level name
	const char * getLevelName(int level) const;

	void close();

private:

	Mutex mMutex;

	///< output log stream
	ostream * mToLog;

	ofstream mOfs;
	bool mCout;
	static const char * mLevelNames[];

	ostringstream mSysLogOss;
	ostringstream mBufOss;

	// Loading buffer
	typedef pair<int, string> Pair;
	vector<Pair> mLoadBuf;

private:

	Logger();
	~Logger();

	bool saveInBuf(int level);

	ostream & openLog();
	void loadFromBuf(ostream &);

	///< Return level for syslog
	int sysLogLevel(int level);

}; // class Logger

} // namespace utils

#endif // LOGGER_H

/**
 * $Id$
 * $HeadURL$
 */
