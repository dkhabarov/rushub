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

#ifndef CTIMERLIST_H
#define CTIMERLIST_H

#include "clist.h"
#include <string>

class cLua;

using namespace std;
namespace nLua {

class cLuaInterpreter;

class cTimer {

private:
	int miTime, miInterval;
	string msFunc;

public:
	int miId;
	cLuaInterpreter * mScript;

public:
	cTimer(int iId, int iInterval, const char * sFunc, cLuaInterpreter * Script);
	~cTimer();
	void Check(int iTime);

}; // cTimer

class cTimerList {

	cList mList;

public:
	cTimerList();
	~cTimerList();

	void OnTimer();
	int AddTimer(cTimer *);
	int DelTimer(int);
	void DelTimer();
	int Size() { return mList.Size(); }

}; // cTimerList

class cTmrCnt {

private:
	int miId;

public:
	static int miCount; // static counter
	cTmrCnt(int iId = 0) : miId(iId) {}
	bool operator() (void * val) {
		if(((cTimer*)val)->miId != miId) return false;
		++ miCount;
		return true;
	}
}; // cTmrCnt

}; // nLua

#endif // CTIMERLIST_H
