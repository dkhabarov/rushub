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

#ifndef PARAM_H
#define PARAM_H

#include "Plugin.h"
#include "stringutils.h"
#include "Any.h"
#include "stdinc.h"

#include <string>
#include <set>

using namespace ::std;
using namespace ::utils;

namespace dcserver {

class DcUser;

typedef int (DcUser::*Action)(const string &, const string &);


/// Param class
class Param : public ParamBase {

public:

	enum {
		MODE_NONE = 0,
		MODE_NOT_CHANGE_TYPE = 1 << 0,
		MODE_NOT_MODIFY = 1 << 1,
		MODE_NOT_REMOVE = 1 << 2
	};

public:

	Param(DcUser * dcUser, const char * name);
	virtual ~Param();

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

	virtual const int64_t & getInt64() const;
	virtual int setInt64(int64_t value);
	int setInt64(int64_t * value);

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

} // namespace dcserver

#endif // PARAM_H

/**
 * $Id$
 * $HeadURL$
 */
