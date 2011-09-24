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
#include "NmdcParser.h"
#include "HashMap.h"

#include <string>

using namespace ::std;
using namespace ::utils;
using namespace ::dcserver::protoenums;
using namespace ::dcserver::protocol;

#if (!defined _WIN32) && (!defined __int64)
	#define __int64 long long
#endif

namespace dcserver {

class DcConn;
class DcServer;



/** Extended class of the user */
class DcUser : public Obj, public DcUserBase, public UserBase {

public:

	DcServer * mDcServer;
	DcConn * mDcConn; ///< Connection for current user

	Time mTimeEnter; ///< Enter time on the hub

public:

	DcUser();
	virtual ~DcUser();

	virtual void send(const string & data, bool addSep = false, bool flush = true);
	virtual void send(const char * data, size_t len, bool addSep = false, bool flush = true);
	virtual void disconnect();


	virtual const void * getParam(unsigned int key) const;
	virtual void setParam(unsigned int key, const char * value);


	virtual const string & getStringParam(unsigned int key) const;
	virtual void setStringParam(unsigned int key, const string & value);
	virtual void setStringParam(unsigned int key, const char * value);

	virtual bool getBoolParam(unsigned int key) const;
	virtual void setBoolParam(unsigned int key, bool value);

	virtual int getIntParam(unsigned int key) const;
	virtual void setIntParam(unsigned int key, int value);


	virtual const string & getUid() const;
	void setUid(const string & uid);
	unsigned long getUidHash() const;


	virtual const string & getInfo() const;
	virtual bool setInfo(const string & myInfo);
	bool setInfo(NmdcParser * parser);

	virtual const string & getInf() const;
	void setInf(const string & inf);

	virtual const string & getIp() const;
	void setIp(const string & ip);
	virtual int getPort() const;
	virtual int getPortConn() const;


	virtual __int64 getShare() const;


	virtual int getProfile() const;
	virtual void setProfile(int profile);

	virtual bool isCanSend() const;
	void setCanSend(bool canSend);

	virtual bool isInUserList() const;
	void setInUserList(bool);

	virtual bool isInOpList() const;
	virtual void setInOpList(bool inOpList);

	virtual bool isInIpList() const;
	virtual void setInIpList(bool inIpList);

	virtual bool isHide() const;
	virtual void setHide(bool hide);
	
	bool isPassive() const;

	virtual long getConnectTime() const;

private:

	DcUser & operator = (const DcUser &) { return *this; }

	string & updateParam(unsigned long key, const char * value);
	void parseDesc(string & description);
	void parseTag();
	void findParam(const string & tag, const char * find, unsigned long key);

private:

	string mUid; ///< UserID
	unsigned long mUidHash; ///< UserID Hash
	int mProfile; ///< Profile

	HashMap<string *> mParams;

	string myInfo;
	string mInf; // ADC
	string mIp; ///< IP address of user/bot
	string mData;
	string mSupports; // NMDC
	string mNmdcVersion; // NMDC

	bool mInOpList; ///< User in op-list
	bool mInIpList; ///< User in ip-list
	bool mHide; ///< User was hide
	bool mInUserList; ///< User in user-list
	bool mCanSend; ///< Can send to user

	bool mCanKick;
	bool mCanForceMove;

	__int64 mShare; ///< Share size

}; // DcUser

}; // namespace dcserver

#endif // DC_USER_H

/**
 * $Id$
 * $HeadURL$
 */
