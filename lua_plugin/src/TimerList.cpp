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

#include "TimerList.h"
#include "LuaPlugin.h"

#ifndef _WIN32
	#include <cstdlib> // abs
#endif

namespace luaplugin {

int TmrCnt::mCount = 0;


Timer::Timer(int id, int interval, const char * func, LuaInterpreter * script) :
	mInterval(interval),
	mFunc(func),
	mId(id),
	mScript(script)
{
	mTime = LuaPlugin::mCurServer->getMSec();
}



Timer::~Timer() {
}



void Timer::check(int time) {
	int msec = abs(time - mTime);
	if (msec >= mInterval) {
		mScript->timer(mId, mFunc.c_str());
		if (msec >= 2 * mInterval) {
			mTime = time; // swallowing
		} else {
			mTime += mInterval;
		}
	}
}






TimerList::TimerList() {
}



TimerList::~TimerList() {
}



static void Checker(void * value) {
	((Timer *) value)->check(LuaPlugin::mCurServer->getMSec());
}



void TimerList::onTimer() {
	loop(Checker);
}



int TimerList::addTimer(Timer * timer) {
	TmrCnt::mCount = 0;
	loop(TmrCnt(timer->mId));
	add(timer);
	return ++TmrCnt::mCount;
}



int TimerList::delTimer(int id) {
	TmrCnt::mCount = 0;
	removeIf(TmrCnt(id));
	return TmrCnt::mCount;
}



void TimerList::delTimer() {
	clear();
}



void TimerList::onRemove(Timer * timer) {
	delete timer;
}


}; // namespace luaplugin

/**
 * $Id$
 * $HeadURL$
 */
