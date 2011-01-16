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

#include <string>

#include "cobj.h"
#include "ctime.h"
#include "cuserbase.h"
#include "cplugin.h"
#include "cmyinfo.h"

using namespace std;
using namespace nUtils;

#ifndef _WIN32
	#ifndef __int64
		#define __int64 long long
	#endif
#endif

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

public:

	cDCUser();
	cDCUser(const string & sNick);
	virtual ~cDCUser();

	bool CanSend();
	void Send(const string & sData, bool bAddSep = false, bool bFlush = true);
	const string & GetIp() const { return msIp; } /** Get IP address of user */
	const string & GetNick() const { return msNick; } /** Get nick (for plugins) */
	const string & Nick() const { return msNick; }
	const string & MyINFO() const { return myInfo.getMyInfo(); }
	bool GetInUserList() const { return mbInUserList; } /** (for plugins) */
	bool GetInOpList() const { return mbInOpList; } /** (for plugins) */
	bool GetInIpList() const { return mbInIpList; } /** (for plugins) */
	bool GetHide() const { return mbHide; } /** (for plugins) */
	bool Hide() const { return mbHide; }
	bool GetForceMove() const { return mbForceMove; } /** (for plugins) */
	bool GetKick() const { return mbKick; } /** (for plugins) */
	int GetProfile() const;
	void SetIp(const string & sIP);

	void SetOpList(bool bInOpList); /** (for plugins) */
	void SetIpList(bool bInIpList); /** (for plugins) */
	void SetHide(bool bHide); /** (for plugins) */
	void SetForceMove(bool bForceMove); /** (for plugins) */
	void SetKick(bool bKick); /** (for plugins) */



	const string & GetMyINFO(/*bool real = false*/) const;
	bool SetMyINFO(const string & myInfo, const string & nick); /** (for plugins) */
	bool SetMyINFO(cDCParserBase * parser);

	const string & GetDesc(/*bool real = false*/) const;
	const string & GetEmail(/*bool real = false*/) const;
	const string & GetConnection(/*bool real = false*/) const;
	unsigned GetByte(/*bool real = false*/) const;

	// setShare ?
	__int64 GetShare(/*bool real = false*/) const;

	// setPassive ?
	bool IsPassive() const;

	const string & GetTag(/*bool real = false*/) const;
	const string & GetClient(/*bool real = false*/) const;
	const string & GetVersion(/*bool real = false*/) const;
	unsigned GetUnRegHubs(/*bool real = false*/) const;
	unsigned GetRegHubs(/*bool real = false*/) const;
	unsigned GetOpHubs(/*bool real = false*/) const;
	unsigned GetSlots(/*bool real = false*/) const;
	unsigned GetLimit(/*bool real = false*/) const;
	unsigned GetOpen(/*bool real = false*/) const;
	unsigned GetBandwidth(/*bool real = false*/) const;
	unsigned GetDownload(/*bool real = false*/) const;
	const string & GetFraction(/*bool real = false*/) const;
	const string & GetMode(/*bool real = false*/) const;

	unsigned int getTagNil(/*bool real = false*/) const;

private:

	MyInfo myInfo;
	string msIp; /** IP address of user/bot */

}; // cDCUser

}; // nDCServer

#endif // CUSER_H
