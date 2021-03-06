/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * Copyright (C) 2009-2013 by Setuper
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
#include "stdinc.h"

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

	DcUser(int type, DcConn *);
	virtual ~DcUser();

	virtual ParamBase * getParam(const char * name) const;
	virtual ParamBase * getParamForce(const char * name);
	virtual bool removeParam(const char * name);

	virtual void disconnect();

	virtual const string & getSid() const; // User ID
	virtual const string & getNick() const;
	void setSid(const string & sid);
	void setNick(const string & nick);
	unsigned long getSidHash() const;
	unsigned long getNickHash() const;

	virtual const string & getNmdcTag();

	virtual const string & getInfo();
	virtual bool setInfo(const string & info);

	virtual bool parseCommand(const char * cmd);

	virtual const char * getCommand();

	virtual const string & getIp() const;
	void setIp(const string & ip);

	virtual int getProfile() const;
	virtual int getProtocolType() const;

	virtual bool isHide() const;
	virtual bool isCanSend() const;

	virtual bool hasFeature(unsigned int feature) const;

	void send(const char * data, size_t len, bool addSep = false, bool flush = true);
	virtual void send(const string & data, bool addSep = false, bool flush = true);

	/// Chat Direct
	virtual void sendToChat(const string & data, const string & nick, bool flush = true);

	/// Chat Broadcast
	virtual void sendToChatAll(const string & data, const string & nick, bool flush = true);

	/// Private Message
	virtual void sendToPm(const string & data, const string & nick, const string & from, bool flush = true);


	void setCanSend(bool canSend);
	void setInUserList(bool);

	bool isPassive() const;
	bool isTrueBoolParam(const char * name) const;

	set<string> & getInfoNames() { // ADC
		return mInfoNames;
	}

	set<unsigned int> & getFeatures() { // ADC
		return mFeatures;
	}

private:

	Param * getParamForce(const char * name, bool setRules);

	int onSetShare(const string & old, const string & now);
	int onSetInOpList(const string & old, const string & now);
	int onSetInIpList(const string & old, const string & now);
	int onSetHide(const string & old, const string & now);
	int onSetInfo(const string & old, const string & now);

	DcUser(const DcUser &);
	DcUser & operator = (const DcUser &);

private:

	unsigned long mNickHash; ///< Nick Hash
	unsigned long mSidHash; ///< User SID Hash

	string mNick; ///< User nick
	string mSid; ///< User SID (Session ID)
	string mNmdcTag; ///< NMDC tag
	string mInfo; ///< User Info
	string mIp; ///< IP address of user/bot

	bool mInUserList; ///< User in user-list
	bool mCanSend; ///< Can send to user
	bool mInfoChanged; ///< Can send to user

	HashMap<Param *, string> mParamList;
	set<string> mInfoNames;
	set<unsigned int> mFeatures;

}; // class DcUser


} // namespace dcserver

#endif // DC_USER_H

/**
 * $Id$
 * $HeadURL$
 */
