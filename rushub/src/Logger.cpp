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

#include "Logger.h"
#include "Obj.h"

#include <time.h> // time

using namespace ::std;
using namespace ::utils;


namespace utils {

#define LOG_FILE "system.%Y-%m-%d.log"

const char * Logger::mLevelNames[] = {"FATAL", "ERROR", "WARN ", "INFO ", "DEBUG", "TRACE"};

Logger::Logger() :
	mSysLogOn(false),
	mMaxLevel(LEVEL_TRACE), // set max level for log before load config
	mLogsPath(NULL),
	mToLog(&cout),
	mCout(false)
{
}



Logger::~Logger() {
}



void Logger::close() {
	if (mOfs.is_open()) {
		mOfs.close();
	}
	freeInstance();
}


void Logger::log(const string & msg) {
	Mutex::Lock l(mMutex); // sync
	ostream & os = (*mToLog);
	os << msg;
	os.flush();
}



/** log function. Set log straem */
void Logger::log(int level) {

#ifndef _WIN32
	if (mSysLogOn) {
		if (saveInBuf(level)) {
			loadFromBuf(mSysLogOss);
		}
		const string & buf = mSysLogOss.str();
		if (!buf.empty()) {
			syslog(sysLogLevel(level), "%s", buf.c_str());
			mSysLogOss.str("");
		}
		mToLog = &mSysLogOss;
	} else
#endif
	{
		if (mOfs.is_open()) {
			mToLog = &mOfs;
		} else if (mCout == true) {
			mToLog = &cout;
		} else {
			// save in buff when the config is not loaded
			saveInBuf(level);

			if (mLogsPath == NULL) {
				mToLog = &mBufOss;
			} else {
				mToLog = &openLog();
			}
		}
	}
}



/** Return a simple log stream */
ostream & Logger::simpleLogStream() {
	return *mToLog;
}



/** Return log level name */
const char * Logger::getLevelName(int level) const {
	return mLevelNames[level];
}



///< Return max log level
int Logger::getMaxLevel() {
	return mMaxLevel;
}



ostream & Logger::openLog() {
	time_t rawtime;
	time(&rawtime);
	struct tm tmr;
	#ifndef _WIN32
		tmr = *localtime(&rawtime);
	#else
		localtime_s(&tmr, &rawtime);
	#endif

	char buf[64] = { '\0' };
	strftime(buf, 64, LOG_FILE, &tmr);

	ostream * ret = &mOfs;
	// If mLogsPath is empty, then using cout
	if (mLogsPath->empty()) {
		ret = &cout;
		mCout = true;
	} else {
		string path(*mLogsPath);
		mOfs.open(path.append(buf).c_str(), ios_base::app);
		if (!mOfs.is_open()) {
			ret = &cout;
			mCout = true;
		}
	}

	loadFromBuf(*ret);
	return *ret;
}



void Logger::loadFromBuf(ostream & os) {
	for (vector<Pair>::iterator it = mLoadBuf.begin(); it != mLoadBuf.end(); ++it) {
		Pair & p = (*it);
		if (p.first <= getMaxLevel()) {
			os << p.second;
		}
	}
}



// Saving in buffer
bool Logger::saveInBuf(int level) {
	const string & buff = mBufOss.str();
	if (!buff.empty()) {
		mLoadBuf.push_back(pair<int, string>(level, buff));
		mBufOss.str("");
		return true;
	}
	return false;
}



/** Return level for syslog */
#ifndef _WIN32

int Logger::sysLogLevel(int level) {

	switch (level) {

		case LEVEL_FATAL :
			return LOG_USER | LOG_CRIT;

		case LEVEL_ERROR :
			return LOG_USER | LOG_ERR;

		case LEVEL_WARN :
			// Fallthrough

		case LEVEL_INFO :
			return LOG_USER | LOG_NOTICE;

		case LEVEL_DEBUG :
			return LOG_USER | LOG_INFO;

		case LEVEL_TRACE :
			return LOG_USER | LOG_DEBUG;

		default :
			return 0;

	}
}

#else

int Logger::sysLogLevel(int) {
	return 0;
}

#endif


} // namespace utils

/**
 * $Id$
 * $HeadURL$
 */
