/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz

 * modified: 27 Aug 2009
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

#include "Obj.h"
#include "Times.h"

using namespace ::std;
using namespace ::utils;

#define LOG_FILE "system.%Y-%m-%d.log"
#define ERR_LABEL "[ERROR]"
#define MAX_LEVEL 6 // Max level for log
#define MAX_ERR_LEVEL 2 // Max error level for log


bool Obj::mbSysLogOn = false;


/** 
  * The event level. All events above limit are not considered
  * 0 - Information messages of the start and stops hub
  * 1 - Failures lower level, and information records
  * 2 - Events
  * 3 - Good actions and process actions
  * 4 - Enough frequent events
  * 5 - io actions
  */
int Obj::miMaxLevel = MAX_LEVEL; // set max level for log before load config

/** 
  * The error level. All errors above limit are not considered
  * 0 - Log with critical errors and exceptions. The errors bring about fall of the server
  * 1 - The suspicious errors. End usually disconnection. If such errors appear then with server not okay
  * 2 - Errors, which can be connected with incorrect call function from outside (from plugins)
  */
int Obj::miMaxErrLevel = MAX_ERR_LEVEL;

ofstream Obj::mOfs;
string * Obj::msPath = NULL; /** Logs path */


int Obj::mCounterObj = 0; /** Objects counter */
int Obj::miLevel = 0;
bool Obj::mbIsErrorLog = false;

ostringstream Obj::mSysLogOss;
ostringstream Obj::mBufOss;

vector<Obj::Pair> Obj::mLoadBuf;



Obj::Obj(const char * name) :
	mClassName(name),
	mToLog(&cout)
{
	++mCounterObj;
	//if (Log(0)) LogStream() << "+ " << mClassName << endl;
}



Obj::Obj() :
	mClassName("Obj"),
	mToLog(&cout)
{
	++mCounterObj;
	//if (Log(0)) LogStream() << "+ " << mClassName << endl;
}



Obj::~Obj() {
	--mCounterObj;
	//if (string(mClassName) != "DcServer" && Log(0)) LogStream() << "- " << mClassName << endl;
}



/** Get counts of objects */
int Obj::GetCount() {
	return mCounterObj;
}



/** Return log straem */
int Obj::Log(int level) {
	if (level <= miMaxLevel) {
		mToLog = &log();
		miLevel = level;
		mbIsErrorLog = false;
		return strLog();
	}
	return 0;
}



/** Return errlog stream */
int Obj::ErrLog(int level) {
	if (level <= miMaxErrLevel) {
		mToLog = &errLog();
		miLevel = level;
		mbIsErrorLog = true;
		return strLog();
	}
	return 0;
}



/** Set class name */
void Obj::SetClassName(const char * name) {
	//if (Log(0)) LogStream() << "r " << mClassName << " -> " << name << endl;
	mClassName = name;
}



/** Main function putting log in stream */
bool Obj::strLog() {
	Time now;
	LogStream() << "[" << now.AsDateMS() << "] " << ((mbIsErrorLog) ? ERR_LABEL " " : "")
		<< "(" << miLevel << ") " << mClassName << ": ";
	return true;
}



/** Log function. Return log straem */
ostream & Obj::log() {

#ifndef _WIN32
	if (mbSysLogOn) {
		if (saveInBuf()) {
			loadFromBuf(mSysLogOss);
		}
		const string & buf = mSysLogOss.str();
		if (!buf.empty()) {
			syslog(sysLogLevel(miLevel, mbIsErrorLog), buf.c_str());
			mSysLogOss.str("");
		}
		return mSysLogOss;
	}
#endif

	if (mOfs.is_open()) {
		return mOfs;
	}

	// save in buff when the config is not loaded
	saveInBuf();

	if (msPath == NULL) {
		return mBufOss;
	}
	return openLog();
}



/** Errlog function. Return errlog straem */
ostream & Obj::errLog() {
	return log();
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

	string path = *msPath;
	path.append(buf);
	mOfs.open(path.c_str(), ios_base::app);

	ostream * ret = &mOfs;
	if (!mOfs.is_open()) {
		ret = &cout;
	}

	loadFromBuf(*ret);
	return *ret;
}



// Saving in buffer
bool Obj::saveInBuf() {
	const string & buff = mBufOss.str();
	if (!buff.empty()) {
		pair<int, bool> first(miLevel, mbIsErrorLog);
		mLoadBuf.push_back(pair<pair<int, bool>, string>(first, buff));
		mBufOss.str("");
		return true;
	}
	return false;
}



void Obj::loadFromBuf(ostream & os) {
	for (vector<Pair>::iterator it = mLoadBuf.begin(); it != mLoadBuf.end(); ++it) {
		Pair & p = (*it);
		if ((!p.first.second && p.first.first <= miMaxLevel) || (p.first.second && p.first.first <= miMaxErrLevel)) {
			os << p.second;
		}
	}
}



/** Return level for syslog */
#ifndef _WIN32

int Obj::sysLogLevel(int level, bool isError /* = false */) {
	if (isError) {

		switch (level) {

			case 0 :
				return LOG_USER | LOG_CRIT;

			case 1 :
				return LOG_USER | LOG_ERR;

			default :
				return 0;

		}

	} else {

		switch (level) {

			case 0 :
				// Fallthrough

			case 1 :
				// Fallthrough

			case 2 :
				return LOG_USER | LOG_NOTICE;

			case 3 :
				// Fallthrough

			case 4 :
				return LOG_USER | LOG_INFO;

			case 5 :
				// Fallthrough

			case 6 :
				return LOG_USER | LOG_DEBUG;

			default :
				return 0;

		}
	}
	return 0;
}

#else

int Obj::sysLogLevel(int, bool) {
	return 0;
}

#endif

/**
 * $Id$
 * $HeadURL$
 */
