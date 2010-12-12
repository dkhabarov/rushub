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

#include "cobj.h"
#include "ctime.h"
#include "cdir.h" // for ExecPath

using namespace std;
using namespace nUtils;

#define LOG_FILE "system.%Y-%m-%d.log"
#define ERR_LABEL "(error)"

/** 
  * The event level. All events above limit are not considered
  * 0 - Information messages of the start and stops hub
  * 1 - Failures lower level, and information records
  * 2 - Events
  * 3 - Good actions and process actions
  * 4 - Enough frequent events
  * 5 - io actions
  */
int cObj::miMaxLevel = 0;

/** 
  * The error level. All errors above limit are not considered
  * 0 - Log with critical errors and exceptions. The errors bring about fall of the server
  * 1 - The suspicious errors. End usually disconnection. If such errors appear then with server not okay
  * 2 - Errors, which can be connected with incorrect call function from outside (from plugins)
  */
int cObj::miMaxErrLevel = 2;

int cObj::mCounterObj = 0; /** Objects counter */
int cObj::miLevel = 0;
const string cObj::msEmpty;
string * cObj::msPath = NULL; /** Logs path */

bool cObj::mbSysLogOn = false;
bool cObj::mbIsErrorLog = false;
ostringstream cObj::mOss;
ofstream cObj::mOfs;

cObj::cObj(const char *name) : mClassName(name), mToLog(&cout) { ++mCounterObj; /*if(Log(0)) LogStream() << "+ " << mClassName << endl;*/ }
cObj::cObj() : mClassName("Obj"), mToLog(&cout) { ++mCounterObj; /*if(Log(0)) LogStream() << "+ " << mClassName << endl;*/ }
cObj::~cObj(){ --mCounterObj; /*if(string(mClassName) != "cDCServer" && Log(0)) LogStream() << "- " << mClassName << endl;*/ }

/** Set class name */
void cObj::SetClassName(const char *sName) {
	//if(Log(0)) LogStream() << "r " << mClassName << " -> " << sName << endl;
	mClassName = sName;
}

/** Main function putting log in stream */
int cObj::StrLog(ostream &ostr, int iLevel, int iMaxLevel, bool bIsError /* = false */) {
	if(iLevel <= iMaxLevel) {
		cTime now;
		#ifndef _WIN32
		if(mbSysLogOn) {
			const string & buf = mOss.str();
			if(!buf.empty()) {
				syslog(SysLogLevel(miLevel, bIsError), buf.c_str());
				mOss.str("");
			}
		}
		#endif
		miLevel = iLevel;
		mbIsErrorLog = bIsError;
		ostr << "[" << now.AsDateMS() << "] (" << iLevel << ") " << mClassName << ": ";
		return 1;
	}
	return 0;
}

/** Return log straem */
int cObj::Log(int iLevel) {
	mToLog = &Log();
	return StrLog(*mToLog, iLevel, miMaxLevel, false);
}

/** Return errlog stream */
int cObj::ErrLog(int iLevel) {
	mToLog = &ErrLog();
	*mToLog << ERR_LABEL;
	return StrLog(*mToLog, iLevel, miMaxErrLevel, true);
}

/** Virtual log function. Return log straem */
ostream & cObj::Log() {
#ifndef _WIN32
	if(mbSysLogOn)
		return mOss;
	else
#endif
	{
		if(!mOfs.is_open()) {
			string sPath;
			if(msPath == NULL)
				ExecPath(sPath);
			else
				sPath = *msPath;

			char buf[64];
			time_t rawtime;
			time(&rawtime);
			struct tm * tmr = localtime(&rawtime);
			strftime(buf, 64, LOG_FILE, tmr);

			sPath.append(buf);
			mOfs.open(sPath.c_str(), ios_base::app);
		}
		if(!mOfs.is_open()) return cout;
		return mOfs;
	}
}

/** Virtual errlog function. Return errlog straem */
ostream & cObj::ErrLog() {
#ifndef _WIN32
	if(mbSysLogOn)
		return mOss;
	else
#endif
	{
		if(!mOfs.is_open()) {
			string sPath;
			if(msPath == NULL)
				ExecPath(sPath);
			else
				sPath = *msPath;

			char buf[64];
			time_t rawtime;
			time(&rawtime);
			struct tm * tmr = localtime(&rawtime);
			strftime(buf, 64, LOG_FILE, tmr);

			sPath.append(buf);
			mOfs.open(sPath.c_str(), ios_base::app);
		}
		if(!mOfs.is_open()) return cerr;
		return mOfs;
	}
}

/** Return current log stream */
ostream & cObj::LogStream() {
	return *mToLog;
}

/** Return level for syslog */
int cObj::SysLogLevel(int iLevel, bool bIsError /* = false */) {
	#ifndef _WIN32
	if (bIsError) {
		switch(iLevel) {
			case 0:
				return LOG_USER | LOG_CRIT;
			case 1:
				return LOG_USER | LOG_ERR;
			default:
				return 0;
		}
	} else {
		switch(iLevel) {
			case 0:
			case 1:
			case 2:
				return LOG_USER | LOG_NOTICE;
			case 3:
			case 4:
				return LOG_USER | LOG_INFO;
			case 5:
			case 6:
				return LOG_USER | LOG_DEBUG;
			default:
				return 0;
		}
	}
	#endif
	return 0;
}
