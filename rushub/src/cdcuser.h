/*
 * RusHub - hub server for Direct Connect peer to peer network.

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

#ifndef CUSER_H
#define CUSER_H

#include "cobj.h"
#include "ctime.h"
#include "cuserbase.h"
#include "cplugin.h"

#include <string>
#ifndef _WIN32
	#ifndef __int64
		#define __int64 long long
	#endif
#endif

using namespace std;
using namespace nUtils;

namespace nDCServer {

class cDCConn;
class cDCServer; /** for mDCServer */

/** Extended class of the user */
class cDCUser : public cObj, public cDCUserBase, public cUserBase {

public:
	cDCServer * mDCServer;
	cDCConn * mDCConn; /** Connection for current user */

	string msNick; /** User's nick */

	cTime mTimeEnter;

	bool mbInUserList; /** User in user-list */
	bool mbInOpList; /** User in op-list */
	bool mbInIpList; /** User in ip-list */
	bool mbHide; /** User was hide */
	bool mbForceMove; /** User can redirect other users */
	bool mbKick; /** User can kick other users */

private:

	string msIp; /** IP address of user/bot */
	char mTagSep; /** Tag separator */

protected:

	string msMyINFO;

	string msDesc; /** User's description */
	string msEmail; /** User's e-mail */
	string msConnection; /** User's connection */
	unsigned miByte; /** User's magic byte */
	bool mbPassive; /** Passive mode flag */
	__int64 miShare; /** Share size */

	string msTag;
	string msClient;
	string msVersion;
	int miUnRegHubs;
	int miRegHubs;
	int miOpHubs;
	int miSlots;
	int miLimit;
	int miOpen;
	int miBandwidth;
	int miDownload;
	string msFraction;
	string msMode;

public:

	cDCUser();
	cDCUser(const string & sNick);
	virtual ~cDCUser();
	bool CanSend();
	void Send(const string & sData, bool bAddSep = false, bool bFlush = true);
	const string & GetIp() const { return msIp; } /** Get IP address of user */
	const string & GetNick() const { return msNick; } /** Get nick (for plugins) */
	const string & Nick() const { return msNick; }
	const string & GetMyINFO() const { return msMyINFO; } /** Get MyINFO */
	const string & MyINFO() const { return msMyINFO; }
	bool GetInUserList() const { return mbInUserList; } /** (for plugins) */
	bool GetInOpList() const { return mbInOpList; } /** (for plugins) */
	bool GetInIpList() const { return mbInIpList; } /** (for plugins) */
	bool GetHide() const { return mbHide; } /** (for plugins) */
	bool Hide() const { return mbHide; }
	bool GetForceMove() const { return mbForceMove; } /** (for plugins) */
	bool GetKick() const { return mbKick; } /** (for plugins) */
	int GetProfile() const;
	void SetIp(const string & sIP);

	const string & GetDesc() const { return msDesc; } /** Get description (for plugins) */
	const string & GetEmail() const { return msEmail; } /** Get e-mail (for plugins) */
	const string & GetConnection() const { return msConnection; } /** Get connection (for plugins) */
	unsigned GetByte() const { return miByte; } /** Get byte (for plugins) */
	__int64 GetShare() const { return miShare; } /** Get share (for plugins) */
	bool IsPassive() const { return mbPassive; }

	const string & GetTag() const { return msTag; }
	const string & GetClient() const { return msClient; }
	const string & GetVersion() const { return msVersion; }
	unsigned GetUnRegHubs() const { return miUnRegHubs; }
	unsigned GetRegHubs() const { return miRegHubs; }
	unsigned GetOpHubs() const { return miOpHubs; }
	unsigned GetSlots() const { return miSlots; }
	unsigned GetLimit() const { return miLimit; }
	unsigned GetOpen() const { return miOpen; }
	unsigned GetBandwidth() const { return miBandwidth; }
	unsigned GetDownload() const { return miDownload; }
	const string & GetFraction() const { return msFraction; }
	const string & GetMode() const { return msMode; }

	// check myinfo format
	bool SetMyINFO(const string & sMyINFO, const string & sNick); /** (for plugins) */
	bool SetMyINFO(cDCParserBase * Parser);
	void ParseMyINFO(cDCParserBase * Parser);

	void SetOpList(bool bInOpList); /** (for plugins) */
	void SetIpList(bool bInIpList); /** (for plugins) */
	void SetHide(bool bHide); /** (for plugins) */
	void SetForceMove(bool bForceMove); /** (for plugins) */
	void SetKick(bool bKick); /** (for plugins) */

private:

	inline void FindIntParam(const string find, int & param, unsigned);
	inline void ParseTag();

}; // cDCUser

}; // nDCServer

#endif // CUSER_H
