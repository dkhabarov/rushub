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

#ifndef LUA_UTILS_H
#define LUA_UTILS_H

#include "LuaPlugin.h"

namespace luaplugin {

class LuaUtils {

public:
	static const size_t minMsgLen;
	static const size_t maxMsgLen;
	static const size_t minNickLen;
	static const size_t maxNickLen;
	static const size_t minFileLen;
	static const size_t maxFileLen;

public:

	static int errCount(lua_State *, const char * countStr);
	static int deprecatedFunc(lua_State *, const char * instead);
	static bool checkCount(lua_State *, int argNumber);
	static int pushError(lua_State *, const char * msg);
	static bool checkMsgLen(lua_State *, size_t len);
	static bool checkNickLen(lua_State *, size_t len);
	static bool checkFileLen(lua_State *, size_t len);
	static bool checkScriptName(lua_State *, string & name);
	static LuaInterpreter * findInterpreter(lua_State *, const string & name);


}; // LuaUtils

} // namespace luaplugin

#endif // LUA_UTILS_H

/**
 * $Id$
 * $HeadURL$
 */
