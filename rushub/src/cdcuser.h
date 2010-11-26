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

#ifndef CUSER_H
#define CUSER_H

#include "cobj.h"
#include "ctime.h"
#include "cuserbase.h"
#include "cdcuserbase.h"
#include "cdcparserbase.h"

#include <string>

using namespace std;
using namespace nUtils;

namespace nDCServer {

class cDCConn;
class cDCServer; /** for mDCServer */

/** Extended class of the user */
class cDCUser : public cObj, public cUserBase, public cDCUserBase {

public:
	cDCServer * mDCServer;
	cDCConn * mDCConn; /** Connection for current user */

	cTime mTimeEnter;

private:

	string msIp; /** IP address of user/bot */
	char mTagSep; /** Tag separator */

public:

	cDCUser();
	cDCUser(const string & sNick);
	virtual ~cDCUser();
	virtual bool CanSend();
	virtual void Send(string & sData, bool bAddSep = false, bool bFlush = true);
	virtual const string & GetIp() const { return msIp; } /** Get IP address of user */
	virtual const string & GetNick() const { return msNick; } /** Get nick (for plugins) */
	virtual const string & GetMyINFO() const { return msMyINFO; } /** Get MyINFO (for plugins) */
	virtual bool IsInUserList() const { return mbInUserList; } /** (for plugins) */
	virtual bool IsInOpList() const { return mbInOpList; } /** (for plugins) */
	virtual bool IsInIpList() const { return mbInIpList; } /** (for plugins) */
	virtual bool IsHide() const { return mbHide; } /** (for plugins) */
	virtual int GetProfile() const;
	virtual void SetIp(const string & sIP);

	virtual const string & GetDesc() const { return msDesc; } /** Get description (for plugins) */
	virtual const string & GetEmail() const { return msEmail; } /** Get e-mail (for plugins) */
	virtual const string & GetConnection() const { return msConnection; } /** Get connection (for plugins) */
	virtual unsigned GetByte() const { return miByte; } /** Get byte (for plugins) */
	virtual __int64 GetShare() const { return miShare; } /** Get share (for plugins) */
	virtual bool IsPassive() const { return mbPassive; }

	virtual const string & GetTag() const { return msTag; }
	virtual const string & GetClient() const { return msClient; }
	virtual const string & GetVersion() const { return msVersion; }
	virtual unsigned GetUnRegHubs() const { return miUnRegHubs; }
	virtual unsigned GetRegHubs() const { return miRegHubs; }
	virtual unsigned GetOpHubs() const { return miOpHubs; }
	virtual unsigned GetSlots() const { return miSlots; }
	virtual unsigned GetLimit() const { return miLimit; }
	virtual unsigned GetOpen() const { return miOpen; }
	virtual unsigned GetBandwidth() const { return miBandwidth; }
	virtual unsigned GetDownload() const { return miDownload; }
	virtual const string & GetFraction() const { return msFraction; }
	virtual const string & GetMode() const { return msMode; }

	// check myinfo format
	virtual bool SetMyINFO(const string & sMyINFO, const string & sNick); /** (for plugins) */
	virtual bool SetMyINFO(cDCParserBase * Parser);
	virtual void ParseMyINFO(cDCParserBase * Parser);

	virtual void SetOpList(bool bInOpList); /** (for plugins) */
	virtual void SetIpList(bool bInIpList); /** (for plugins) */
	virtual void SetHide(bool bHide); /** (for plugins) */

private:

	inline void FindIntParam(const string find, int & param, unsigned);
	inline void ParseTag();

}; // cDCUser

}; // nDCServer

#endif // CUSER_H
