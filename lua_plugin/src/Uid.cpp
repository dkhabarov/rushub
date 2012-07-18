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

#include "Uid.h"

#include <string.h> // strlen
#include <cstdlib> // atof

namespace luaplugin {


void Uid::createMetaTable(lua_State * L) {
	if (!luaL_newmetatable(L, MT_USER_CONN)) {
		return;
	}

	lua_pushliteral(L, "__index");
	lua_pushstring(L, "userIndex");
	lua_pushcclosure(L, userIndex, 1);
	lua_settable(L, -3);

	lua_pushliteral(L, "__newindex");
	lua_pushstring(L, "userNewIndex");
	lua_pushcclosure(L, userNewIndex, 1);
	lua_settable(L, -3);

	lua_pushliteral(L, "__tostring");
	lua_pushstring(L, MT_USER_CONN);
	lua_pushcclosure(L, uidToString, 1);
	lua_settable(L, -3);

	lua_pushliteral(L, "__metatable");
	lua_pushliteral(L, "You're not allowed to get this metatable");
	lua_settable(L, -3);

	lua_settop(L, 0);
}



int Uid::uidToString(lua_State * L) {
	char buf[9] = { '\0' };
	sprintf(buf, "%p", *(static_cast<void **> (lua_touserdata(L, 1))));
	lua_pushfstring(L, "%s (%s)", lua_tostring(L, lua_upvalueindex(1)), buf);
	return 1;
}



int Uid::userIndex(lua_State * L) {

	DcUserBase * dcUserBase = getDcUserBase(L, 1);
	if (!dcUserBase) {
		ERR_TYPEMETA(1, "UID", "userdata");
	}

	const char * str = lua_tostring(L, 2);
	if (!str) {
		ERR_TYPEMETA(2, "UID", "string");
	}

	void ** userdata = NULL;
	switch(getHash(str)) {

		case PARAM_HASH_NICK :
			lua_pushstring(L, dcUserBase->getUid().c_str());
			break;

		case PARAM_HASH_MYINFO :
			lua_pushstring(L, dcUserBase->getInfo().c_str());
			break;

		case PARAM_HASH_TAG :
			lua_pushstring(L, dcUserBase->getNmdcTag().c_str());
			break;

		case PARAM_HASH_UID :
			userdata = static_cast<void **> (lua_newuserdata(L, sizeof(void *)));
			*userdata = static_cast<void *> (dcUserBase);
			luaL_getmetatable(L, MT_USER_CONN);
			lua_setmetatable(L, -2);
			break;

		default :
			pushValue(L, dcUserBase, str);
			break;

	}
	return 1;
}



int Uid::userNewIndex(lua_State * L) {

	DcUserBase * dcUserBase = getDcUserBase(L, 1);
	if (!dcUserBase) {
		ERR_TYPEMETA(1, "UID", "userdata");
	}

	const char * str = lua_tostring(L, 2);
	if (!str) {
		ERR_TYPEMETA(2, "UID", "string");
	}

	int type = lua_type(L, 3);

	switch(getHash(str)) {

		case PARAM_HASH_PROFILE :
			return setValue(L, dcUserBase, str, ParamBase::TYPE_INT);

		case PARAM_HASH_MYINFO :
			str = lua_tostring(L, 3);
			if (!str) {
				ERR_TYPEMETA(3, "UID", "string");
			}
			dcUserBase->setInfo(str);
			break;

		case PARAM_HASH_TAG :
			break;

		default :
			if (type == LUA_TSTRING) {
				return setValue(L, dcUserBase, str, ParamBase::TYPE_STRING);
			} else if (type == LUA_TNUMBER) {
				return setValue(L, dcUserBase, str, ParamBase::TYPE_DOUBLE);
			} else if (type == LUA_TBOOLEAN) {
				return setValue(L, dcUserBase, str, ParamBase::TYPE_BOOL);
			} else if (type == LUA_TNIL) {
				dcUserBase->removeParam(str);
			}
			break;

	}
	lua_settop(L, 0);
	return 0;
}



void Uid::pushValue(lua_State * L, DcUserBase * dcUserBase, const char * name) {
	ParamBase * paramBase = dcUserBase->getParam(name);
	if (paramBase == NULL) {
		lua_pushnil(L);
	} else {
		switch (paramBase->getType()) {
			case ParamBase::TYPE_STRING :
				lua_pushstring(L, paramBase->getString().c_str());
				break;

			case ParamBase::TYPE_BOOL :
				lua_pushboolean(L, paramBase->getBool() ? 1 : 0);
				break;

			case ParamBase::TYPE_DOUBLE :
				lua_pushnumber(L, paramBase->getDouble());
				break;

			case ParamBase::TYPE_INT :
				lua_pushnumber(L, paramBase->getInt());
				break;

			case ParamBase::TYPE_INT64 :
				lua_pushnumber(L, static_cast<double> (paramBase->getInt64()));
				break;

			case ParamBase::TYPE_LONG :
				lua_pushnumber(L, paramBase->getLong());
				break;

			default :
				lua_pushnil(L);
				break;
		}
	}
}



int Uid::setValue(lua_State * L, DcUserBase * dcUserBase, const char * name, int type) {
	ParamBase * paramBase = dcUserBase->getParamForce(name);
	const char * str = NULL;
	size_t len;
	switch (type) {
		case ParamBase::TYPE_STRING :
			str = lua_tolstring(L, 3, &len);
			if (!str) {
				ERR_TYPEMETA(3, "UID", "string");
			}
			paramBase->setString(string(str, len));
			break;

		case ParamBase::TYPE_BOOL :
			paramBase->setBool(lua_toboolean(L, 3) != 0);
			break;

		case ParamBase::TYPE_DOUBLE :
			paramBase->setDouble(lua_tonumber(L, 3));
			break;

		case ParamBase::TYPE_INT :
			paramBase->setInt(lua_tointeger(L, 3));
			break;

		case ParamBase::TYPE_INT64 :
			paramBase->setInt64(static_cast<int64_t> (lua_tonumber(L, 3)));
			break;

		case ParamBase::TYPE_LONG :
			paramBase->setLong(lua_tointeger(L, 3));
			break;

		default :
			dcUserBase->removeParam(name);
			break;
	}
	lua_settop(L, 0);
	return 0;
}



unsigned int Uid::getHash(const char * s) {
	size_t i, l = strlen(s);
	unsigned h = l;
	for (i = l; i > 0;) {
		h ^= ((h << 5) + (h >> 2) + static_cast<unsigned char> (s[--i]));
	}
	return h / 2;
}


} // namespace luaplugin

/**
 * $Id$
 * $HeadURL$
 */
