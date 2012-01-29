/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * Copyright (C) 2009-2012 by Setuper
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
#include "Param.h"
#include "HashMap.h"

#include <string>

using namespace ::std;
using namespace ::utils;
using namespace ::dcserver::protoenums;
using namespace ::dcserver::protocol;

#ifdef _WIN32
	#if defined(_MSC_VER) && (_MSC_VER >= 1400)
		#pragma warning(disable:4996) // Disable "This function or variable may be unsafe."
	#endif
#endif

#if (!defined _WIN32) && (!defined __int64)
	#define __int64 long long
#endif

namespace dcserver {

class DcConn;
class DcServer;


/** Extended class of the user */
class DcUser : public Obj, public DcUserBase, public UserBase, private NonCopyable {

public:

	DcServer * mDcServer;
	DcConn * mDcConn; ///< Connection for current user

	Time mTimeEnter; ///< Enter time on the hub

public:

	DcUser(DcConn *);
	virtual ~DcUser();

	virtual void send(const string & data, bool addSep = false, bool flush = true);
	virtual void send(const char * data, size_t len, bool addSep = false, bool flush = true);

	virtual void sendToChat(const string & data, const string & uid, bool flush = true);
	virtual void sendToPm(const string & data, const string & uid, const string & from, bool flush = true);

	virtual void disconnect();

	virtual bool hasFeature(int feature) const;


	virtual ParamBase * getParam(const char * name) const;
	virtual ParamBase * getParamForce(const char * name);
	virtual bool removeParam(const char * name);

	virtual const string & getUid() const;
	void setUid(const string & uid);
	unsigned long getUidHash() const;

	virtual const string & getNmdcTag();

	virtual const string & getInfo();
	virtual bool setInfo(const string & info);
	bool setInfo(NmdcParser * parser);

	virtual const string & getIp() const;
	void setIp(const string & ip);


	virtual int getProfile() const;

	virtual bool isHide() const;
	virtual bool isCanSend() const;
	void setCanSend(bool canSend);
	void setInUserList(bool);

	bool isPassive() const;
	bool isTrueBoolParam(const char * name) const;

private:

	Param * getParamForce(const char * name, bool setRules);

	int onSetShare(const string & old, const string & now);
	int onSetInOpList(const string & old, const string & now);
	int onSetInIpList(const string & old, const string & now);
	int onSetHide(const string & old, const string & now);
	int onSetInfo(const string & old, const string & now);

private:

	unsigned long mUidHash; ///< UserID Hash

	string mUid; ///< UserID
	string mNmdcTag; ///< NMDC tag
	string mInfo; ///< User Info
	string mIp; ///< IP address of user/bot

	bool mInUserList; ///< User in user-list
	bool mCanSend; ///< Can send to user
	bool mInfoChanged; ///< Can send to user

	HashMap<Param *, string> mParamList;
	set<string> mInfoNames;
	set<int> mFeatures;

}; // DcUser


}; // namespace dcserver

#endif // DC_USER_H

/**
 * $Id$
 * $HeadURL$
 */
