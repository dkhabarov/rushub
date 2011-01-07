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

#ifndef CLUAINTERPRETER_H
#define CLUAINTERPRETER_H

#include "lua/lua.hpp"

#define HAVE_LUA_5_1

#include "cplugin.h" /** nDCServer */
#include "api.h"
#include "ctimerlist.h"
#include <list>
#include <iostream>
using namespace std;

namespace nLua {

#define MT_USER_CONN "User object"
#define MT_CONFIG "Config object"

class cLuaInterpreter {

public:
	string msName;
	string & msPath;
	lua_State * mL;
	bool mbEnabled;

	typedef list<string> tBotList;
	tBotList mBotList;

private:
	struct sParam {
		void * data;
		lua_Number num;
		int type;
		sParam(void * d, int t):data(d),type(t){}
		sParam(lua_Number n, int t):num(n),type(t){}
	};
	typedef vector<sParam*> tvCallParams;
	tvCallParams mCallParams;

	void CreateUserMT();
	void CreateConfigMT();

	cTimerList mTimerList;

public:
	cLuaInterpreter(const string & sName, string & sFullName);
	~cLuaInterpreter();

	int Start(); // (-1 - run already)
	int Stop();

	int CallFunc(const char*);
	void RegFunc(const char*, int (*)(lua_State*));
	void RegStrField(const char*, const char*);

	void Timer(int iId, const char * sFunc);
	bool OnError(const char* sFunc, const char* sErrMsg, bool bStop = false);
	void NewCallParam(void * data, int type = 0);
	void NewCallParam(lua_Number data, int type = 0);

	inline void OnTimer() { mTimerList.OnTimer(); }
	inline int AddTmr(cTimer * timer) { return mTimerList.AddTimer(timer); }
	inline int Size() { return mTimerList.Size(); }
	inline int DelTmr(int tm) { return mTimerList.DelTimer(tm); }
	inline void DelTmr() { mTimerList.DelTimer(); }

}; // cLuaInterpreter

}; // nLua

#endif // CLUAINTERPRETER_H
