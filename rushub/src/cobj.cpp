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
const string cObj::msEmpty;
char * cObj::msPath = NULL;

ofstream cObj::mOfs;

cObj::cObj(const char *name) : mClassName(name), mToLog(&cout) { ++mCounterObj; }
cObj::cObj() : mClassName("Obj"), mToLog(&cout) { ++mCounterObj; }
cObj::~cObj(){ --mCounterObj; }

/** Set class name */
void cObj::SetClassName(const char *sName) {
  mClassName = sName;
}

/** Main function putting log in stream */
int cObj::StrLog(ostream &ostr, int iLevel, int iMaxLevel) {
  cTime now;
  if(iLevel <= iMaxLevel) {
    ostr << "[" << now.AsDateMS() << "] (" << iLevel << ") " << mClassName << ": ";
    return 1;
  }
  return 0;
}

/** Return log straem */
int cObj::Log(int iLevel) {
  mToLog = &Log();
  return StrLog(*mToLog, iLevel, miMaxLevel);
}

/** Return errlog stream */
int cObj::ErrLog(int iLevel) {
  mToLog = &ErrLog();
  *mToLog << "(error)";
  return StrLog(*mToLog, iLevel, miMaxErrLevel);
}

/** Virtual log function. Return log straem */
ostream & cObj::Log() {
  if(!mOfs.is_open()) {
    string sPath;
    if(msPath == NULL)
      ExecPath(sPath);
    else
      sPath = msPath;
    sPath += "system.log";
    mOfs.open(sPath.c_str());
  }
  if(!mOfs.is_open()) return cout;
  return mOfs;
}

/** Virtual errlog function. Return errlog straem */
ostream & cObj::ErrLog() {
  if(!mOfs.is_open()) {
    string sPath;
    if(msPath == NULL)
      ExecPath(sPath);
    else
      sPath = msPath;
    sPath += "system.log";
    mOfs.open(sPath.c_str());
  }
  if(!mOfs.is_open()) return cerr;
  return mOfs;
}

/** Return current log stream */
ostream & cObj::LogStream() {
  return *mToLog;
}
