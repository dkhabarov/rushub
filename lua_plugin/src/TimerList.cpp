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

#include "TimerList.h"
#include "LuaPlugin.h"

#ifndef _WIN32
	#include <cstdlib> // abs
#endif

namespace luaplugin {

int cTmrCnt::miCount = 0;

cTimer::cTimer(int iId, int iInterval, const char * sFunc, LuaInterpreter * Script) :
	miInterval(iInterval),
	msFunc(sFunc),
	miId(iId),
	mScript(Script)
{
	miTime = LuaPlugin::mCurServer->getMSec();
}

cTimer::~cTimer() {
}

void cTimer::check(int iTime) {
	if (abs(iTime - miTime) >= miInterval) {
		mScript->Timer(miId, msFunc.c_str());
		miTime = iTime;
	}
}

cTimerList::cTimerList() {
}

cTimerList::~cTimerList() {
}

static void Checker(void * val) {
	((cTimer *)val)->check(LuaPlugin::mCurServer->getMSec());
}
void cTimerList::onTimer() {
	mList.Loop(Checker);
}

int cTimerList::AddTimer(cTimer * timer) {
	cTmrCnt::miCount = 0;
	mList.Loop(cTmrCnt(timer->miId));
	mList.add(timer);
	return ++cTmrCnt::miCount;
}
int cTimerList::DelTimer(int iId) {
	cTmrCnt::miCount = 0;
	mList.DelIf(cTmrCnt(iId));
	return cTmrCnt::miCount;
}
void cTimerList::DelTimer() {
	mList.clear();
}

}; // namespace luaplugin
