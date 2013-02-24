/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz
 *
 * modified: 27 Aug 2009
 * Copyright (C) 2009-2013 by Setuper
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

#include "Obj.h"
#include "Times.h"
#include "Logger.h"

using namespace ::std;
using namespace ::utils;


namespace utils {

volatile long Obj::mCounterObj = 0; /** Objects counter */

Obj::Obj(const char * name) :
	mClassName(name)
{
	Thread::safeInc(mCounterObj);
	//LOG(LEVEL_WARN, "+ " << mClassName);
}



// Without Count Control (use only if you control this object). For owner objects!
Obj::Obj(const char * name, bool) :
	mClassName(name)
{
}



Obj::Obj() :
	mClassName("Obj")
{
	Thread::safeInc(mCounterObj);
	//LOG(LEVEL_WARN, "+ " << mClassName);
}



Obj::~Obj() {
	Thread::safeDec(mCounterObj);
	//if (string(mClassName) != "DcServer") LOG(LEVEL_WARN, "- " << mClassName);
}



/** Get counts of objects */
long Obj::getCount() {
	return mCounterObj;
}



/** Return log straem */
int Obj::log(int level, ostream & os) {
	if (level <= Logger::getInstance()->getMaxLevel()) {
		Logger::getInstance()->log(level);
		return strLog(level, os);
	}
	return 0;
}



/** Return class name */
const char * Obj::getClassName() const {
	return mClassName;
}



/** Set class name */
void Obj::setClassName(const char * name) {
	//LOG(LEVEL_WARN, "r " << mClassName << " -> " << name);
	mClassName = name;
}



/** Main function putting log in stream */
bool Obj::strLog(int level, ostream & os) {
	utils::Time now(true);
	os << now.asDateMsec() << " " << Logger::getInstance()->getLevelName(level) << " ";
	return true;
}



/** Thread safe logger */
void Obj::log(const string & msg) {
	Logger::getInstance()->log(msg);
}







} // namespace utils

/**
 * $Id$
 * $HeadURL$
 */
