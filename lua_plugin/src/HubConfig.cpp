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

#include "HubConfig.h"
#include "LuaPlugin.h"
#include <string.h>

using namespace ::std;

namespace luaplugin {

#define MT_CONFIG "Config object"


void HubConfig::createMetaTable(lua_State * L) {
	if (!luaL_newmetatable(L, MT_CONFIG)) {
		return;
	}

	lua_pushliteral(L, "__index");
	lua_pushstring(L, "ConfigIndex");
	lua_pushcclosure(L, configIndex, 1);
	lua_settable(L, -3);

	lua_pushliteral(L, "__newindex");
	lua_pushstring(L, "ConfigNewindex");
	lua_pushcclosure(L, configNewindex, 1);
	lua_settable(L, -3);

	lua_pushliteral(L, "__tostring");
	lua_pushstring(L, MT_CONFIG);
	lua_pushcclosure(L, configTostring, 1);
	lua_settable(L, -3);

	lua_pushliteral(L, "__metatable");
	lua_pushliteral(L, "You're not allowed to get this metatable");
	lua_settable(L, -3);

	lua_settop(L, 0);

	Config * config = (Config *) lua_newuserdata(L, sizeof(Config));
	config->isExist = 1;
	luaL_getmetatable(L, MT_CONFIG);
	lua_setmetatable(L, -2);
	lua_setglobal(L, "Config");
}



int HubConfig::configTostring(lua_State * L) {
	char buf[9] = { '\0' };
	sprintf(buf, "%p", lua_touserdata(L, 1));
	lua_pushfstring(L, "%s (%s)", lua_tostring(L, lua_upvalueindex(1)), buf);
	return 1;
}



int HubConfig::configTable(lua_State * L) {
	static const vector<string> & conf = LuaPlugin::mCurServer->getConfig();
	int indx = 1;
	lua_newtable(L);
	for (vector<string>::const_iterator it = conf.begin(); it != conf.end(); ++it) {
		lua_pushnumber(L, indx++);
		lua_pushstring(L, (*it).c_str());
		lua_rawset(L, -3);
	}
	return 1;
}



int HubConfig::configIndex(lua_State * L) {
	Config * config = (Config *) lua_touserdata(L, 1);
	if (config->isExist != 1) {
		lua_settop(L, 0);
		lua_pushnil(L);
		return 1;
	}
	const char * name = lua_tostring(L, 2);
	if (!name) {
		return luaL_error(L, "bad argument #%d to " LUA_QS " (%s expected, got %s)", 2, "Config", "string", luaL_typename(L, 2));
	}
	if (!strcmp(name, "table")) {
		lua_settop(L, 0);
		lua_pushcfunction(L, &configTable);
		return 1;
	}
	const char * cfg = LuaPlugin::mCurServer->getConfig(name);
	lua_settop(L, 0);
	if (!cfg) {
		lua_pushnil(L);
	} else {
		lua_pushstring(L, cfg);
	}
	return 1;
}



int HubConfig::configNewindex(lua_State * L) {
	Config * config = (Config *) lua_touserdata(L, 1);
	if (config->isExist != 1) {
		lua_settop(L, 0);
		return 0;
	}
	const char * name = lua_tostring(L, 2);
	if (!name) {
		return luaL_error(L, "bad argument #%d to " LUA_QS " (%s expected, got %s)", 2, "Config", "string", luaL_typename(L, 2));
	}
	const char * value = NULL;
	if (lua_isstring(L, 3)) {
		value = lua_tostring(L, 3);
	} else if (lua_isboolean(L, 3)) {
		value = lua_toboolean(L, 3) == 0 ? "0" : "1";
	} else {
		return luaL_error(L, "bad argument #%d to " LUA_QS " (%s expected, got %s)", 3, "Config", "string or boolean", luaL_typename(L, 3));
	}
	LuaPlugin::mCurServer->setConfig(name, value);
	LuaPlugin::mCurLua->onConfigChange(name, value);
	lua_settop(L, 0);
	return 0;
}


}; // namespace luaplugin

/**
 * $Id$
 * $HeadURL$
 */
