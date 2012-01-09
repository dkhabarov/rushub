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
#include <set>

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
class DcUser;

typedef int (DcUser::*Action)(const string &, const string &);


/// Param class
class Param : public ParamBase {

public:

	enum {
		MODE_NONE = 0,
		MODE_NOT_CHANGE_TYPE = 1 << 0,
		MODE_NOT_MODIFY = 1 << 1,
		MODE_NOT_REMOVE = 1 << 2,
	};

public:

	Param(DcUser * dcUser, const char * name);
	~Param();

	virtual const string & getName() const;
	virtual int getType() const;
	int getMode() const;

	virtual const string & getString() const;
	virtual int setString(const string & value);
	int setString(string * value);

	virtual const int & getInt() const;
	virtual int setInt(int value);
	int setInt(int * value);

	virtual const bool & getBool() const;
	virtual int setBool(bool value);
	int setBool(bool * value);

	virtual const double & getDouble() const;
	virtual int setDouble(double value);
	int setDouble(double * value);

	virtual const long & getLong() const;
	virtual int setLong(long value);
	int setLong(long * value);

	virtual const __int64 & getInt64() const;
	virtual int setInt64(__int64 value);
	int setInt64(__int64 * value);

	virtual const string & toString();

	template <class T>
	int set(T const & value, int type) {
		int n_err = check(value, type);
		if (n_err == 0) {
			mType = type;
			mValue = value;
		}
		return n_err;
	}

	template <class T>
	int set(T * value, int type) {
		int n_err = check(*value, type);
		if (n_err == 0) {
			mType = type;
			mValue = value;
		}
		return n_err;
	}

	template <class T>
	int set(T const & value, int type, int mode, Action action = NULL) {
		int n_err = set(value, type);
		mAction = action;
		mMode = mode;
		return n_err;
	}

	template <class T>
	int set(T * value, int type, int mode, Action action = NULL) {
		int n_err = set(value, type);
		mAction = action;
		mMode = mode;
		return n_err;
	}

private:

	template <class T>
	int check(const T & now, int type) {
		if (!(mMode & MODE_NOT_MODIFY) && (!(mMode & MODE_NOT_CHANGE_TYPE) || mType == type)) {
			if (mAction != NULL) {
				string oldStr, nowStr;
				return (mDcUser->*mAction) (utils::toString(mValue, oldStr), utils::toString(now, nowStr));
			}
			return 0;
		}
		return -1;
	}

private:

	string mBuf;
	string mName;
	int mType;
	int mMode;

	DcUser * mDcUser;
	Action mAction;
	Any mValue;

}; // class Param






/** Extended class of the user */
class DcUser : public Obj, public DcUserBase, public UserBase {

public:

	DcServer * mDcServer;
	DcConn * mDcConn; ///< Connection for current user

	Time mTimeEnter; ///< Enter time on the hub

public:

	DcUser(DcConn *);
	virtual ~DcUser();

	virtual void send(const string & data, bool addSep = false, bool flush = true);
	virtual void send(const char * data, size_t len, bool addSep = false, bool flush = true);
	virtual void disconnect();

	virtual bool hasFeature(const string & feature) const;


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

	DcUser & operator = (const DcUser &) { return *this; }

	Param * getParamForce(const char * name, bool setRules);

	int onSetShare(const string & old, const string & now);
	int onSetInOpList(const string & old, const string & now);
	int onSetInIpList(const string & old, const string & now);
	int onSetHide(const string & old, const string & now);
	int onSetInfo(const string & old, const string & now);

private:

	unsigned long mUidHash; ///< UserID Hash

	string mUid; ///< UserID
	string mTag;
	string mInfo;
	string mIp; ///< IP address of user/bot

	bool mInUserList; ///< User in user-list
	bool mCanSend; ///< Can send to user
	bool mInfoChanged; ///< Can send to user

	HashMap<Param *, string> mParamList;
	set<string> mInfoNames;
	set<string> mFeatures; // TODO refactoring to int

}; // DcUser


}; // namespace dcserver

#endif // DC_USER_H

/**
 * $Id$
 * $HeadURL$
 */
