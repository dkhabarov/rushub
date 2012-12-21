/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz
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

#include "Obj.h"
#include "Times.h"
#include "Thread.h"

#include <time.h> // time

using namespace ::std;
using namespace ::utils;


namespace utils {


#define LOG_FILE "system.%Y-%m-%d.log"

bool Obj::mSysLogOn = false;
int Obj::mMaxLevel = LEVEL_TRACE; // set max level for log before load config


ofstream Obj::mOfs;
string * Obj::mLogsPath = NULL; /** Logs path */

volatile long Obj::mCounterObj = 0; /** Objects counter */
bool Obj::mCout = false;
const char * Obj::mLevelNames[] = {"FATAL", "ERROR", "WARN ", "INFO ", "DEBUG", "TRACE"};

ostringstream Obj::mSysLogOss;
ostringstream Obj::mBufOss;

vector<Obj::Pair> Obj::mLoadBuf;



Obj::Obj(const char * name) :
	mClassName(name),
	mToLog(&cout)
{
	Thread::safeInc(mCounterObj);
	//LOG(LEVEL_WARN, "+ " << mClassName);
}



// Without Count Control (use only if you control this object). For owner objects!
Obj::Obj(const char * name, bool) :
	mClassName(name),
	mToLog(&cout)
{
}



Obj::Obj() :
	mClassName("Obj"),
	mToLog(&cout)
{
	Thread::safeInc(mCounterObj);
	//LOG(LEVEL_WARN, "+ " << mClassName);
}



Obj::~Obj() {
	Thread::safeDec(mCounterObj);
	//if (string(mClassName) != "DcServer") LOG(LEVEL_WARN, "- " << mClassName);
}



/** Get counts of objects */
long Obj::getCount() {
	return mCounterObj;
}



/** Return log straem */
int Obj::log(int level, ostream & os) {
	if (level <= getMaxLevel()) {
		mToLog = &log(level);
		return strLog(level, os);
	}
	return 0;
}



/** Return class name */
const char * Obj::getClassName() const {
	return mClassName;
}



/** Return log level name */
const char * Obj::getLevelName(int level) const {
	return mLevelNames[level];
}



///< Return max log level
int Obj::getMaxLevel() {
	return mMaxLevel;
}


/** Set class name */
void Obj::setClassName(const char * name) {
	//LOG(LEVEL_WARN, "r " << mClassName << " -> " << name);
	mClassName = name;
}



/** Main function putting log in stream */
bool Obj::strLog(int level, ostream & os) {
	utils::Time now(true);
	os << now.asDateMsec() << " " << getLevelName(level) << " ";
	return true;
}



/** Return a simple log stream */
ostream & Obj::simpleLogStream() {
	return *mToLog;
}



/** log function. Return log straem */
ostream & Obj::log(int level) {

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
		return mSysLogOss;
	}
#endif

	if (mOfs.is_open()) {
		return mOfs;
	} else if (mCout == true) {
		return cout;
	}

	// save in buff when the config is not loaded
	saveInBuf(level);

	if (mLogsPath == NULL) {
		return mBufOss;
	}
	return openLog();
}



ostream & Obj::openLog() {
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



// Saving in buffer
bool Obj::saveInBuf(int level) {
	const string & buff = mBufOss.str();
	if (!buff.empty()) {
		mLoadBuf.push_back(pair<int, string>(level, buff));
		mBufOss.str("");
		return true;
	}
	return false;
}



void Obj::loadFromBuf(ostream & os) {
	for (vector<Pair>::iterator it = mLoadBuf.begin(); it != mLoadBuf.end(); ++it) {
		Pair & p = (*it);
		if (p.first <= getMaxLevel()) {
			os << p.second;
		}
	}
}



/** Return level for syslog */
#ifndef _WIN32

int Obj::sysLogLevel(int level) {

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

int Obj::sysLogLevel(int) {
	return 0;
}

#endif

} // namespace utils

/**
 * $Id$
 * $HeadURL$
 */
