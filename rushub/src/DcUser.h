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

#ifndef DC_USER_H
#define DC_USER_H

#include "Obj.h"
#include "Times.h"
#include "UserBase.h"
#include "Plugin.h"
#include "MyInfo.h"

#include <string>

using namespace ::std;
using namespace ::utils;

#ifndef _WIN32
	#ifndef __int64
		#define __int64 long long
	#endif
#endif

namespace dcserver {

class DcConn;
class DcServer; /** for mDcServer */



/** Extended class of the user */
class DcUser : public Obj, public DcUserBase, public UserBase {

public:

	DcServer * mDcServer;
	DcConn * mDcConn; /** Connection for current user */

	string msNick; /** User's nick */

	Time mTimeEnter;

	bool mbInOpList; /** User in op-list */
	bool mbInIpList; /** User in ip-list */
	bool mbHide; /** User was hide */
	bool mbForceMove; /** User can redirect other users */
	bool mbKick; /** User can kick other users */

public:

	DcUser();
	DcUser(const string & nick);
	virtual ~DcUser();

	void send(const string & data, bool addSep = false, bool flush = true);

	void setInUserList(bool);
	
	void setCanSend(bool canSend);
	
	inline bool isCanSend() {
		return mCanSend;
	}
	
	
	/** Get IP address of user */
	const string & getIp() const {
		return mIp;
	}

	/** Get nick (for plugins) */
	const string & getNick() const {
		return msNick;
	}

	const string & Nick() const {
		return msNick;
	}
	
	const string & MyINFO() const {
		return myInfo.getMyInfo();
	}
	
	bool getInUserList() const {
		return mbInUserList;
	}

	bool getInOpList() const {
		return mbInOpList;
	}

	bool getInIpList() const {
		return mbInIpList;
	}

	bool getHide() const {
		return mbHide;
	}

	bool Hide() const {
		return mbHide;
	}

	bool getForceMove() const {
		return mbForceMove;
	}

	bool getKick() const {
		return mbKick;
	}
	
	int getProfile() const;
	void SetIp(const string & ip);

	void setInOpList(bool inOpList);
	void setInIpList(bool inIpList);
	void setHide(bool hide);
	void setForceMove(bool forceMove);
	void setKick(bool kick);



	const string & getMyINFO(/*bool real = false*/) const;
	bool setMyINFO(const string & myInfo, const string & nick);
	bool setMyINFO(DcParserBase * parser);

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

	bool mbInUserList; /** User in user-list */
	bool mCanSend;
	MyInfo myInfo;
	string mIp; /** IP address of user/bot */
	

}; // DcUser

}; // namespace dcserver

#endif // DC_USER_H
