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
#include "stringutils.h"
#include "Any.h"

#include <stdlib.h> // atoi unix
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


/// Param class
class Param : public ParamBase {

public:

	Param(const string & name) : mName(name), mType(TYPE_NONE) {
	}

	const string & getName() const {
		return mName;
	}
	int getType() const {
		return mType;
	}

	const string & getString() const {
		return mValue;
	}

	void setString(const string & value) {
		mType = TYPE_STRING;
		mValue = value;
	}

	int getInt() const {
		return mValue;
	}

	void setInt(int value) {
		mType = TYPE_INT;
		mValue = value;
	}

	bool getBool() const {
		return mValue;
	}

	void setBool(bool value) {
		mType = TYPE_BOOL;
		mValue = value;
	}

	double getDouble() const {
		return mValue;
	}

	void setDouble(double value) {
		mType = TYPE_DOUBLE;
		mValue = value;
	}

	long getLong() const {
		return mValue;
	}

	void setLong(long value) {
		mType = TYPE_LONG;
		mValue = value;
	}

	__int64 getInt64() const {
		return mValue;
	}

	void setInt64(__int64 value) {
		mType = TYPE_INT64;
		mValue = value;
	}

private:

	string mName;
	int mType;

	Any mValue;

}; // class ParamBase


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


	virtual ParamBase * getParam(const string & name) const;
	virtual ParamBase * getParamForce(const string & name);


	virtual const string * getParamOld(unsigned int key) const;
	virtual void setParamOld(unsigned int key, const char * value);


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


	virtual const string & getInfo();
	virtual bool setInfo(const string & myInfo);
	bool setInfo(NmdcParser * parser);

	virtual const string & getInf() const;
	void setInf(const string & inf);

	virtual const string & getIp() const;
	void setIp(const string & ip);
	int getPort() const;
	int getPortConn() const;


	virtual int getProfile() const;
	void setProfile(int profile);

	virtual bool isCanSend() const;
	void setCanSend(bool canSend);

	virtual bool isHide() const;
	
	bool isPassive() const;

	virtual long getConnectTime() const;

private:

	DcUser & operator = (const DcUser &) { return *this; }

	string & updateParamOld(unsigned long key, const char * value);
	void parseDesc(string & description);
	void parseTag();
	void findParam(const string & tag, const char * find, unsigned long key);

	void appendParam(string & dst, const char * prefix, unsigned long key);


	bool isInUserList() const;
	void setInUserList(bool);

	bool isInOpList() const;
	void setInOpList(bool inOpList);

	bool isInIpList() const;
	void setInIpList(bool inIpList);

	void setHide(bool hide);

	static unsigned long DcUser::getHash(const char * s);

private:

	string mUid; ///< UserID
	unsigned long mUidHash; ///< UserID Hash
	int mProfile; ///< Profile

	HashMap<string *> mParams;

	HashMap<Param *> mParamList;

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

}; // DcUser


}; // namespace dcserver

#endif // DC_USER_H

/**
 * $Id$
 * $HeadURL$
 */
