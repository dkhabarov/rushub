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

#ifndef TIMER_LIST_H
#define TIMER_LIST_H

#include "List.h"

#include <string>

#if (!defined _WIN32) && (!defined __int64)
	#define __int64 long long
#endif

using namespace ::std;

class LuaPlugin;

namespace luaplugin {

class LuaInterpreter;



class Timer {

public:

	int mId;
	LuaInterpreter * mScript;

public:

	Timer(int id, int interval, const char * func, LuaInterpreter *);

	~Timer();

	void check(__int64 time);

private:

	__int64 mTime;
	int mInterval;
	string mFunc;

}; // Timer



class TimerList : public List<Timer *> {

public:

	TimerList();
	virtual ~TimerList();

	void onTimer();
	int addTimer(Timer *);
	int delTimer(int);
	void delTimer();

	virtual void onRemove(Timer *);

}; // class TimerList



class TmrCnt {

public:

	static int mCount; // static counter

public:

	TmrCnt(int id = 0) : mId(id) {
	}

	bool operator() (void * value) {
		if (((Timer *) value)->mId != mId) {
			return false;
		}
		++ mCount;
		return true;
	}

private:

	int mId;

}; // class TmrCnt

}; // namespace luaplugin

#endif // TIMER_LIST_H

/**
 * $Id$
 * $HeadURL$
 */
