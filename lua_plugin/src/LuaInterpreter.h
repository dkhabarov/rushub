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

#ifndef LUA_INTERPRETER_H
#define LUA_INTERPRETER_H

#include "Plugin.h" // ::dcserver
#include "TimerList.h"
#include "api.h"

#include <list>
#include <iostream>

using namespace ::std;

namespace luaplugin {

class LuaInterpreter {

public:

	string mName;
	string & mPath;
	lua_State * mL;
	bool mEnabled;

	typedef list<string> BotList;
	BotList mBotList;

public:

	LuaInterpreter(const string & name, string & fullName);

	~LuaInterpreter();

	LuaInterpreter & operator = (const LuaInterpreter &) {
		return *this;
	}

	int start(); // (-1 - is running already)
	int stop();

	int callFunc(const char * funcName);

	bool onError(const char * funcName, const char * errMsg, bool stop = false);
	void timer(int id, const char * funcName);

	static void logError(const char * msg);

	void newCallParam(void * data, int type = 0);
	void newCallParam(lua_Number data, int type = 0);

	inline void onTimer() {
		mTimerList.onTimer();
	}
	inline int addTmr(Timer * timer) {
		return mTimerList.addTimer(timer);
	}
	inline int size() {
		return mTimerList.size();
	}
	inline int delTmr(int tm) {
		return mTimerList.delTimer(tm);
	}
	inline void delTmr() {
		mTimerList.delTimer();
	}

private:

	struct Param {
		void * data;
		lua_Number num;
		int type;
		Param(void * d, int t) : data(d), type(t) {
		}
		Param(lua_Number n, int t) : num(n), type(t) {
		}
	}; // struct Param

	typedef vector<Param *> CallParams;
	CallParams mCallParams;

	TimerList mTimerList;

private:

	void regFunc(const char * funcName, int (*)(lua_State *));
	void regStrField(const char * name, const char * value);

}; // class LuaInterpreter

}; // namespace luaplugin

#endif // LUA_INTERPRETER_H

/**
 * $Id$
 * $HeadURL$
 */
