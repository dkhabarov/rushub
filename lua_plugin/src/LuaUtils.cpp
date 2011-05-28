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

#include "LuaUtils.h"


namespace luaplugin {

const size_t LuaUtils::minMsgLen = 1;
const size_t LuaUtils::maxMsgLen = 128000;
const size_t LuaUtils::minNickLen = 1;
const size_t LuaUtils::maxNickLen = 64;
const size_t LuaUtils::minFileLen = 1;
const size_t LuaUtils::maxFileLen = 255;

int LuaUtils::errCount(lua_State * L, const char * countStr) {
	lua_Debug ar;
	if (!lua_getstack(L, 0, &ar)) {
		return luaL_error(L, "bad argument count (%s expected, got %d)", countStr, lua_gettop(L));
	}
	lua_getinfo(L, "n", &ar);
	if (ar.name == NULL) {
		ar.name = "?";
	}
	return luaL_error(L, "bad argument count to " LUA_QS " (%s expected, got %d)", ar.name, countStr, lua_gettop(L));
}



bool LuaUtils::checkCount(lua_State * L, int argNumber) {
	if (lua_gettop(L) != argNumber) {
		char sBuf[32] = { '\0' };
		sprintf(sBuf, "%d", argNumber);
		LuaUtils::errCount(L, sBuf);
		return false;
	}
	return true;
}



int LuaUtils::pushError(lua_State * L, const char * msg) {
	lua_settop(L, 0);
	lua_pushnil(L);
	lua_pushstring(L, msg);
	return 2;
}



bool LuaUtils::checkMsgLen(lua_State * L, size_t len) {
	if (len < minMsgLen || len > maxMsgLen) {
		pushError(L, "very long string");
		return false;
	}
	return true;
}



bool LuaUtils::checkNickLen(lua_State * L, size_t len) {
	if (len < minNickLen || len > maxNickLen) {
		pushError(L, "very long nick");
		return false;
	}
	return true;
}



bool LuaUtils::checkFileLen(lua_State * L, size_t len) {
	if (len < minFileLen || len > maxFileLen) {
		pushError(L, "very long file name");
		return false;
	}
	return true;
}



bool LuaUtils::checkScriptName(lua_State * L, string & name) {
	size_t fileSize = name.size();
	if (!checkFileLen(L, fileSize)) {
		return false;
	}
	if (fileSize <= 4 || (0 != name.compare(fileSize - 4, 4, ".lua"))) {
		name.append(".lua");
	}
	return true;
}



LuaInterpreter * LuaUtils::findInterpreter(lua_State * L, const string & name) {
	LuaInterpreter * luaInterpreter = NULL;
	for (LuaPlugin::LuaInterpreterList::iterator it = LuaPlugin::mCurLua->mLua.begin();
		it != LuaPlugin::mCurLua->mLua.end();
		++it
	) {
		if ((*it) && (*it)->mName == name) {
			luaInterpreter = *it;
			break;
		}
	}
	if (!luaInterpreter || !luaInterpreter->mL) {
		pushError(L, "script was not found");
		return NULL;
	}
	return luaInterpreter;
}

}; // namespace luaplugin

/**
 * $Id$
 * $HeadURL$
 */
