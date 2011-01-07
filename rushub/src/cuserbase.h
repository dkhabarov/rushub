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

#ifndef CUSERBASE_H
#define CUSERBASE_H

#include <string>
#ifndef _WIN32
	#define __int64 long long
#endif

using namespace std;

namespace nDCServer {

/** Base user class */
class cUserBase {

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

	string msNick; /** User's nick */
	bool mbInUserList; /** User in user-list */
	bool mbInOpList; /** User in op-list */
	bool mbInIpList; /** User in ip-list */
	bool mbHide; /** User was hide */
	bool mbForceMove; /** User can redirect other users */
	bool mbKick; /** User can kick other users */

public:

	cUserBase() : miByte(0), mbPassive(false), miShare(0),
		miUnRegHubs(0),miRegHubs(0),miOpHubs(0),miSlots(0),
		miLimit(0),miOpen(0),miBandwidth(0),miDownload(0), 
		mbInUserList(false), mbInOpList(false), mbInIpList(false),
		mbHide(false), mbForceMove(false), mbKick(false) {}
	virtual ~cUserBase() {}
	virtual bool CanSend() { return false; }
	virtual void Send(string &msg, bool sep = false, bool flush = true) = 0;
	virtual const string & MyINFO() const { return msMyINFO; }
	virtual const string & GetIp() const { static const string s(""); return s; } /** Void ip for bot */
	virtual int GetProfile() const { return 30; } /** 30 profile for bot */

}; // cUserBase

}; // nDCServer

#endif // CUSERBASE_H
