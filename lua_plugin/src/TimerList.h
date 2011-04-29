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

#ifndef TIMER_LIST_H
#define TIMER_LIST_H

#include "List.h"

#include <string>

using namespace ::std;

class LuaPlugin;

namespace luaplugin {

class LuaInterpreter;

class cTimer {

private:
	int miTime, miInterval;
	string msFunc;

public:
	int miId;
	LuaInterpreter * mScript;

public:
	cTimer(int iId, int iInterval, const char * sFunc, LuaInterpreter * Script);
	~cTimer();
	void check(int iTime);

}; // cTimer

class cTimerList {

	cList<cTimer> mList;

public:
	cTimerList();
	~cTimerList();

	void onTimer();
	int AddTimer(cTimer *);
	int DelTimer(int);
	void DelTimer();
	int size() {
		return mList.size();
	}

}; // class cTimerList

class cTmrCnt {

public:

	static int miCount; // static counter

public:

	cTmrCnt(int iId = 0) : miId(iId) {
	}

	bool operator() (void * val) {
		if (((cTimer*)val)->miId != miId) {
			return false;
		}
		++ miCount;
		return true;
	}

private:

	int miId;

}; // class cTmrCnt

}; // namespace luaplugin

#endif // TIMER_LIST_H

/**
 * $Id$
 * $HeadURL$
 */
