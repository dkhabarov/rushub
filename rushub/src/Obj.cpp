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

#include <time.h> // time

using namespace ::std;


namespace utils {


#define LOG_FILE "system.%Y-%m-%d.log"

bool Obj::mSysLogOn = false;
int Obj::mMaxLevel = TRACE; // set max level for log before load config


ofstream Obj::mOfs;
string * Obj::mLogsPath = NULL; /** Logs path */

int Obj::mCounterObj = 0; /** Objects counter */
int Obj::mLevel = 0;
bool Obj::mCout = false;
const char * Obj::mLevelNames[] = {"FATAL", "ERROR", "WARN ", "INFO ", "DEBUG", "TRACE"};

ostringstream Obj::mSysLogOss;
ostringstream Obj::mBufOss;

vector<Obj::Pair> Obj::mLoadBuf;



Obj::Obj(const char * name) :
	mClassName(name),
	mToLog(&cout)
{
	++mCounterObj;
	//if (log(WARN)) logStream() << "+ " << mClassName << endl;
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
	++mCounterObj;
	//if (log(WARN)) logStream() << "+ " << mClassName << endl;
}



Obj::~Obj() {
	--mCounterObj;
	//if (string(mClassName) != "DcServer" && log(WARN)) logStream() << "- " << mClassName << endl;
}



/** Get counts of objects */
int Obj::getCount() {
	return mCounterObj;
}



/** Return log straem */
int Obj::log(int level) {
	if (level <= mMaxLevel) {
		mToLog = &log();
		mLevel = level;
		return strLog();
	}
	return 0;
}



/** Return current log stream */
ostream & Obj::logStreamLine(const int line) {
	simpleLogStream() << mClassName << "(" << line << "): ";
	return simpleLogStream();
}



/** Return class name */
const char * Obj::getClassName() {
	return mClassName;
}



/** Set class name */
void Obj::setClassName(const char * name) {
	//if (log(WARN)) logStream() << "r " << mClassName << " -> " << name << endl;
	mClassName = name;
}



/** Main function putting log in stream */
bool Obj::strLog() {
	utils::Time now(true);
	simpleLogStream() << now.asDateMsec() << " " << mLevelNames[mLevel] << " ";
	return true;
}



/** Return a simple log stream */
ostream & Obj::simpleLogStream() {
	return *mToLog;
}



/** log function. Return log straem */
ostream & Obj::log() {

#ifndef _WIN32
	if (mSysLogOn) {
		if (saveInBuf()) {
			loadFromBuf(mSysLogOss);
		}
		const string & buf = mSysLogOss.str();
		if (!buf.empty()) {
			syslog(sysLogLevel(mLevel), "%s", buf.c_str());
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
	saveInBuf();

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
bool Obj::saveInBuf() {
	const string & buff = mBufOss.str();
	if (!buff.empty()) {
		mLoadBuf.push_back(pair<int, string>(mLevel, buff));
		mBufOss.str("");
		return true;
	}
	return false;
}



void Obj::loadFromBuf(ostream & os) {
	for (vector<Pair>::iterator it = mLoadBuf.begin(); it != mLoadBuf.end(); ++it) {
		Pair & p = (*it);
		if (p.first <= mMaxLevel) {
			os << p.second;
		}
	}
}



/** Return level for syslog */
#ifndef _WIN32

int Obj::sysLogLevel(int level) {

	switch (level) {

		case FATAL :
			return LOG_USER | LOG_CRIT;

		case ERR :
			return LOG_USER | LOG_ERR;

		case WARN :
			// Fallthrough

		case INFO :
			return LOG_USER | LOG_NOTICE;

		case DEBUG :
			return LOG_USER | LOG_INFO;

		case TRACE :
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

}; // namespace utils

/**
 * $Id$
 * $HeadURL$
 */
