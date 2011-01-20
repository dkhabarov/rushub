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
	const string & getIp() const { return msIp; } /** Get IP address of user */
	const string & getNick() const { return msNick; } /** Get nick (for plugins) */
	const string & Nick() const { return msNick; }
	const string & MyINFO() const { return myInfo.getMyInfo(); }
	bool getInUserList() const { return mbInUserList; } /** (for plugins) */
	bool getInOpList() const { return mbInOpList; } /** (for plugins) */
	bool getInIpList() const { return mbInIpList; } /** (for plugins) */
	bool getHide() const { return mbHide; } /** (for plugins) */
	bool Hide() const { return mbHide; }
	bool getForceMove() const { return mbForceMove; } /** (for plugins) */
	bool getKick() const { return mbKick; } /** (for plugins) */
	int getProfile() const;
	void SetIp(const string & sIP);

	void setInOpList(bool inOpList); /** (for plugins) */
	void setInIpList(bool inIpList); /** (for plugins) */
	void setHide(bool hide); /** (for plugins) */
	void setForceMove(bool forceMove); /** (for plugins) */
	void setKick(bool kick); /** (for plugins) */



	const string & GetMyINFO(/*bool real = false*/) const;
	bool SetMyINFO(const string & myInfo, const string & nick); /** (for plugins) */
	bool SetMyINFO(cDCParserBase * parser);

	const string & getDesc(/*bool real = false*/) const;
	const string & getEmail(/*bool real = false*/) const;
	const string & getConnection(/*bool real = false*/) const;
	unsigned getByte(/*bool real = false*/) const;

	// setShare ?
	__int64 getShare(/*bool real = false*/) const;

	// setPassive ?
	bool IsPassive() const;

	const string & getTag(/*bool real = false*/) const;
	const string & getClient(/*bool real = false*/) const;
	const string & getVersion(/*bool real = false*/) const;
	unsigned getUnregHubs(/*bool real = false*/) const;
	unsigned getRegHubs(/*bool real = false*/) const;
	unsigned getOpHubs(/*bool real = false*/) const;
	unsigned getSlots(/*bool real = false*/) const;
	unsigned getLimit(/*bool real = false*/) const;
	unsigned getOpen(/*bool real = false*/) const;
	unsigned getBandwidth(/*bool real = false*/) const;
	unsigned getDownload(/*bool real = false*/) const;
	const string & getFraction(/*bool real = false*/) const;
	const string & getMode(/*bool real = false*/) const;

	unsigned int getTagNil(/*bool real = false*/) const;

private:

	MyInfo myInfo;
	string msIp; /** IP address of user/bot */

}; // cDCUser

}; // nDCServer

#endif // CUSER_H
