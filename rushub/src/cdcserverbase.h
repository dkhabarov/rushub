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

#ifndef CDCSERVERBASE_H
#define CDCSERVERBASE_H

#ifndef _WIN32
	#define __int64 long long
#endif

#include <string>
#include <iostream>
#include <vector>

using namespace std;

namespace nDCServer {

class cDCUserBase;
class cDCConnBase;

class cDCConnListIterator {
public:
	cDCConnListIterator(){}
	virtual ~cDCConnListIterator(){}
	virtual cDCConnBase * operator() (void) = 0;
}; // cDCConnListIterator

class cDCServerBase {

public:

	typedef vector<string> List_t;

	virtual const string & GetMainDir() const = 0;
	virtual const string & GetTime() = 0;
	virtual const string & GetHubInfo() const = 0; /** Name and version of the hub */
	virtual const string & GetLocale() const = 0;
	virtual const string & GetSystemVersion() const = 0;
	virtual int GetMSec() const = 0;
	virtual int GetUpTime() const = 0; /** Work time (sec) */
	virtual int GetUsersCount() const = 0;
	virtual __int64 GetTotalShare() const = 0;

	virtual int CheckCmd(const string &) = 0;

	virtual bool SendToUser(cDCConnBase *DCConn, const char *sData, char *sNick = NULL, char *sFrom = NULL) = 0;
	virtual bool SendToNick(const char *sTo, const char *sData, char *sNick = NULL, char *sFrom = NULL) = 0;
	virtual bool SendToAll(const char *sData, char *sNick = NULL, char *sFrom = NULL) = 0;
	virtual bool SendToProfiles(unsigned long iProfile, const char *sData, char *sNick = NULL, char *sFrom = NULL) = 0;
	virtual bool SendToIP(const char *sIP, const char *sData, unsigned long iProfile = 0, char *sNick = NULL, char *sFrom = NULL) = 0;
	virtual bool SendToAllExceptNicks(List_t & NickList, const char *sData, char *sNick = NULL, char *sFrom = NULL) = 0;
	virtual bool SendToAllExceptIps(List_t & IPList, const char *sData, char *sNick = NULL, char *sFrom = NULL) = 0;

	virtual void GetDCConnBase(const char * sIP, vector<cDCConnBase *> & vconn) = 0;
	virtual cDCUserBase * GetDCUserBase(const char *sNick) = 0;
	virtual cDCConnListIterator * GetDCConnListIterator() = 0;

	virtual const char * GetConfig(const string & sName) = 0;
	virtual const char * GetLang(const string & sName) = 0;
	virtual bool SetConfig(const string & sName, const string & sValue) = 0;
	virtual bool SetLang(const string & sName, const string & sValue) = 0;

	virtual int RegBot(const string & sNick, const string & sMyINFO, const string & sIP, bool bKey = true) = 0;
	virtual int UnregBot(const string & sNick) = 0;

	virtual void StopHub() = 0;
	virtual void RestartHub() = 0;

}; // cDCServerBase

}; // nDCServer

#endif // CDCSERVERBASE_H
