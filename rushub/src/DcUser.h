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

#include <stdlib.h> // atoi
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


	virtual const string * getParam(unsigned int key) const;
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

	string & updateParam(unsigned long key, const char * value);
	void parseDesc(string & description);
	void parseTag();
	void findParam(const string & tag, const char * find, unsigned long key);

	void collectInfo();
	void appendParam(string & dst, const char * prefix, unsigned long key);


	bool isInUserList() const;
	void setInUserList(bool);

	bool isInOpList() const;
	void setInOpList(bool inOpList);

	bool isInIpList() const;
	void setInIpList(bool inIpList);

	void setHide(bool hide);

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
	bool mCollectInfo;

}; // DcUser



class Param : public ParamBase {

public:

	Param(const char * name, int category = 0, bool readOnly = false) : 
		name(name),
		type(TYPE_NONE),
		category(category),
		readOnly(readOnly)
	{
	}

	~Param() {
	}


	const string & getName() const {
		return name;
	}

	int getType() const {
		return type;
	}

	int getCategory() const {
		return category;
	}

	bool isReadOnly() const {
		return readOnly;
	}

	void setReadOnly(bool readOnly) {
		this->readOnly = readOnly;
	}


	const string & getString() const {
		return value;
	}

	int getInt() const {
		return atoi(value.c_str());
	}

	bool getBool() const {
		return value == "true" || 0 != getInt();
	}

	double getDouble() const {
		return atof(value.c_str());
	}

	long getLong() const {
		return atol(value.c_str());
	}
	
	__int64 getInt64() const {
		return stringToInt64(value);
	}


	void setString(const char * data) {
		if (!readOnly) {
			value = data;
			type = TYPE_STRING;
		}
	}

	void setInt(const int & data) {
		if (!readOnly) {
			sprintf(mBuffer, "%d", data);
			value = mBuffer;
			type = TYPE_INT;
		}
	}

	void setBool(const bool & data) {
		if (!readOnly) {
			value = (data ? "1" : "0");
			type = TYPE_BOOL;
		}
	}

	void setDouble(const double & data) {
		if (!readOnly) {
			sprintf(mBuffer, "%f", data);
			value = mBuffer;
			type = TYPE_DOUBLE;
		}
	}

	void setLong(const long & data) {
		if (!readOnly) {
			sprintf(mBuffer, "%ld", data);
			value = mBuffer;
			type = TYPE_LONG;
		}
	}

	void setInt64(const __int64 & data) {
		if (!readOnly) {
			value = int64ToString(data);
			type = TYPE_INT64;
		}
	}


private:

	string name;
	string value;
	int type;
	int category;
	bool readOnly;
	static char mBuffer[32];

}; // class Param


}; // namespace dcserver

#endif // DC_USER_H

/**
 * $Id$
 * $HeadURL$
 */
