/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * Copyright (C) 2009-2011 by Setuper
 * E-Mail: setuper at gmail dot com (setuper@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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
	DcConn * mDcConn; //< Connection for current user

	Time mTimeEnter; //< Enter time on the hub


	// TODO add param => hash for nick
	string mNick; //< User's nick

	bool mInOpList; //< User in op-list
	bool mInIpList; //< User in ip-list
	bool mHide; //< User was hide
	bool mForceMove; //< User can redirect other users
	bool mKick; //< User can kick other users

	string mSupports; //< Support cmd param (PROTOCOL NMDC)
	string mVersion; //< DC version (PROTOCOL NMDC)

public:

	DcUser();
	virtual ~DcUser();

	virtual void send(const string & data, bool addSep = false, bool flush = true);

	void setInUserList(bool);

	void setCanSend(bool canSend);

	inline bool isCanSend() {
		return mCanSend;
	}


	/** Get nick */
	virtual const string & getNick() const;
	virtual const string & nick() const;
	virtual const string & myInfoString() const;
	virtual bool getInUserList() const;
	virtual bool getInOpList() const;
	virtual bool getInIpList() const;
	virtual bool getHide() const;
	virtual bool hide() const;
	virtual bool getForceMove() const;
	virtual bool getKick() const;

	virtual int getProfile() const;
	virtual void setProfile(int iProfile);

	const string & getIp() const;
	void setIp(const string & ip);

	void setNick(const string & nick);

	virtual void setInOpList(bool inOpList);
	virtual void setInIpList(bool inIpList);
	virtual void setHide(bool hide);
	virtual void setForceMove(bool forceMove);
	virtual void setKick(bool kick);



	virtual const string & getMyINFO(/*bool real = false*/) const;
	virtual bool setMyINFO(const string & myInfo);
	bool setMyINFO(DcParser * parser);

	virtual const string & getDesc(/*bool real = false*/) const;
	virtual const string & getEmail(/*bool real = false*/) const;
	virtual const string & getConnection(/*bool real = false*/) const;
	virtual unsigned getByte(/*bool real = false*/) const;

	// setShare ?
	virtual __int64 getShare(/*bool real = false*/) const;

	// setPassive ?
	bool isPassive() const;

	virtual const string & getTag(/*bool real = false*/) const;
	virtual const string & getClient(/*bool real = false*/) const;
	virtual const string & getClientVersion(/*bool real = false*/) const;
	virtual unsigned getUnregHubs(/*bool real = false*/) const;
	virtual unsigned getRegHubs(/*bool real = false*/) const;
	virtual unsigned getOpHubs(/*bool real = false*/) const;
	virtual unsigned getSlots(/*bool real = false*/) const;
	virtual unsigned getLimit(/*bool real = false*/) const;
	virtual unsigned getOpen(/*bool real = false*/) const;
	virtual unsigned getBandwidth(/*bool real = false*/) const;
	virtual unsigned getDownload(/*bool real = false*/) const;
	virtual const string & getFraction(/*bool real = false*/) const;
	virtual const string & getMode(/*bool real = false*/) const;

	virtual unsigned int getTagNil(/*bool real = false*/) const;

	virtual const string & getData() const;
	virtual void setData(const string & sData);

	//< Support string (PROTOCOL NMDC)
	virtual const string & getSupports() const;

	//< User's protocol version (PROTOCOL NMDC)
	virtual const string & getVersion() const;

private:

	bool mInUserList; //< User in user-list
	bool mCanSend;
	MyInfo myInfo;
	string mIp; //< IP address of user/bot
	string mData; //< Some user's data


}; // DcUser

}; // namespace dcserver

#endif // DC_USER_H

/**
 * $Id$
 * $HeadURL$
 */
