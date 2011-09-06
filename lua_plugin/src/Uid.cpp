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
	sprintf(buf, "%p", *((void **)lua_touserdata(L, 1)));
	lua_pushfstring(L, "%s (%s)", lua_tostring(L, lua_upvalueindex(1)), buf);
	return 1;
}



void Uid::pushStringOrNil(lua_State * L, DcUserBase * dcUserBase, unsigned long key) {
	const string * param = dcUserBase->getParam(key);
	if (param != NULL) {
		lua_pushstring(L, (*param).c_str());
	} else {
		lua_pushnil(L);
	}
}



void Uid::pushNumberOrNil(lua_State * L, DcUserBase * dcUserBase, unsigned long key) {
	const string * param = dcUserBase->getParam(key);
	if (param != NULL) {
		lua_pushnumber(L, atof((*param).c_str()));
	} else {
		lua_pushnil(L);
	}
}



int Uid::userIndex(lua_State * L) {

	DcConnBase * dcConnBase = getDcConnBase(L, 1); // TODO refactoring
	if (!dcConnBase) {
		ERR_TYPEMETA(1, "UID", "userdata");
	}

	const char * str = lua_tostring(L, 2);
	if (!str) {
		ERR_TYPEMETA(2, "UID", "string");
	}

	DcUserBase * dcUserBase = dcConnBase->mDcUserBase; // TODO refactoring

	void ** userdata = NULL;
	switch(getHash(str)) {

		case PARAM_HASH_NICK :
			lua_pushstring(L, dcUserBase->getUid().c_str());
			break;

		case PARAM_HASH_IP :
			lua_pushstring(L, dcUserBase->getIp().c_str()); // TODO refactoring
			break;

		case PARAM_HASH_PROFILE :
			lua_pushnumber(L, dcUserBase->getProfile());
			break;

		case PARAM_HASH_MYINFO :
			lua_pushstring(L, dcUserBase->getInfo().c_str());
			break;

		case PARAM_HASH_SHARE :
			lua_pushnumber(L, (double) dcUserBase->getShare());
			break;

		case PARAM_HASH_MODE :
			pushStringOrNil(L, dcUserBase, USER_PARAM_MODE);
			break;

		case PARAM_HASH_DESC :
			pushStringOrNil(L, dcUserBase, USER_PARAM_DESC);
			break;

		case PARAM_HASH_EMAIL :
			pushStringOrNil(L, dcUserBase, USER_PARAM_EMAIL);
			break;

		case PARAM_HASH_TAG :
			pushStringOrNil(L, dcUserBase, USER_PARAM_TAG);
			break;

		case PARAM_HASH_CONN :
			pushStringOrNil(L, dcUserBase, USER_PARAM_CONNECTION);
			break;

		case PARAM_HASH_BYTE :
			pushNumberOrNil(L, dcUserBase, USER_PARAM_BYTE);
			break;

		case PARAM_HASH_CLIENTN :
			pushStringOrNil(L, dcUserBase, USER_PARAM_CLIENT_NAME);
			break;

		case PARAM_HASH_CLIENTV :
			pushStringOrNil(L, dcUserBase, USER_PARAM_CLIENT_VERSION);
			break;

		case PARAM_HASH_SLOTS :
			pushNumberOrNil(L, dcUserBase, USER_PARAM_SLOTS);
			break;

		case PARAM_HASH_USHUBS :
			pushNumberOrNil(L, dcUserBase, USER_PARAM_UNREG_HUBS);
			break;

		case PARAM_HASH_REGHUBS :
			pushNumberOrNil(L, dcUserBase, USER_PARAM_REG_HUBS);
			break;

		case PARAM_HASH_OPHUBS :
			pushNumberOrNil(L, dcUserBase, USER_PARAM_OP_HUBS);
			break;

		case PARAM_HASH_LIMIT :
			pushNumberOrNil(L, dcUserBase, USER_PARAM_LIMIT);
			break;

		case PARAM_HASH_OPEN :
			pushNumberOrNil(L, dcUserBase, USER_PARAM_OPEN);
			break;

		case PARAM_HASH_BANDWIDTH :
			pushNumberOrNil(L, dcUserBase, USER_PARAM_BANDWIDTH);
			break;

		case PARAM_HASH_DOWNLOAD :
			pushNumberOrNil(L, dcUserBase, USER_PARAM_DOWNLOAD);
			break;

		case PARAM_HASH_FRACTION :
			pushStringOrNil(L, dcUserBase, USER_PARAM_FRACTION);
			break;

		case PARAM_HASH_INOPLIST :
			lua_pushboolean(L, dcUserBase->isInOpList() ? 1 : 0);
			break;

		case PARAM_HASH_INIPLIST :
			lua_pushboolean(L, dcUserBase->isInIpList() ? 1 : 0);
			break;

		case PARAM_HASH_INUSERLIST :
			lua_pushboolean(L, dcUserBase->isInUserList() ? 1 : 0);
			break;

		case PARAM_HASH_KICK :
			lua_pushboolean(L, dcUserBase->getParam(USER_PARAM_CAN_KICK) ? 1 : 0);
			break;

		case PARAM_HASH_REDIRECT :
			lua_pushboolean(L, dcUserBase->getParam(USER_PARAM_CAN_FORCE_MOVE) ? 1 : 0);
			break;

		case PARAM_HASH_HIDE :
			lua_pushboolean(L, dcUserBase->isHide() ? 1 : 0);
			break;

		case PARAM_HASH_PORT :
			lua_pushnumber(L, dcUserBase->getPort()); // TODO refactoring
			break;

		case PARAM_HASH_PORTCONN :
			lua_pushnumber(L, dcUserBase->getPortConn()); // TODO refactoring
			break;

		case PARAM_HASH_IPCONN :
			lua_pushstring(L, dcUserBase->getIpConn().c_str()); // TODO refactoring
			break;

		case PARAM_HASH_MACADDRESS :
			lua_pushstring(L, dcUserBase->getMacAddress().size() ? dcUserBase->getMacAddress().c_str() : "n/a"); // TODO refactoring
			break;

		case PARAM_HASH_SUPPORTS :
			pushStringOrNil(L, dcUserBase, USER_PARAM_SUPPORTS);
			break;

		case PARAM_HASH_VERSION :
			pushStringOrNil(L, dcUserBase, USER_PARAM_NMDC_VERSION);
			break;

		case PARAM_HASH_DATA :
			pushStringOrNil(L, dcUserBase, USER_PARAM_DATA);
			break;

		case PARAM_HASH_ENTERTIME :
			lua_pushnumber(L, (lua_Number)dcUserBase->getConnectTime()); // TODO refactoring
			break;

		case PARAM_HASH_UID :
			userdata = (void **) lua_newuserdata(L, sizeof(void *));
			*userdata = (void *) dcConnBase; // TODO refactoring
			luaL_getmetatable(L, MT_USER_CONN);
			lua_setmetatable(L, -2);
			break;

		default :
			lua_pushnil(L);
			break;

	}
	return 1;
}



int Uid::userNewIndex(lua_State * L) {

	DcConnBase * dcConnBase = getDcConnBase(L, 1); // TODO refactoring
	if (!dcConnBase) {
		ERR_TYPEMETA(1, "UID", "userdata");
	}

	const char * s = lua_tostring(L, 2);
	if (!s) {
		ERR_TYPEMETA(2, "UID", "string");
	}

	DcUserBase * dcUserBase = dcConnBase->mDcUserBase; // TODO refactoring

	switch(getHash(s)) {

		case PARAM_HASH_PROFILE :
			dcUserBase->setProfile(luaL_checkint(L, 3));
			break;

		case PARAM_HASH_MYINFO :
			s = lua_tostring(L, 3);
			if (!s) {
				ERR_TYPEMETA(3, "UID", "string");
			}
			dcUserBase->setInfo(s);
			break;

		case PARAM_HASH_INOPLIST :
			if (lua_type(L, 3) != LUA_TBOOLEAN) {
				ERR_TYPEMETA(3, "UID", "boolean");
			}
			dcUserBase->setInOpList(lua_toboolean(L, 3) != 0);
			break;

		case PARAM_HASH_INIPLIST :
			if (lua_type(L, 3) != LUA_TBOOLEAN) {
				ERR_TYPEMETA(3, "UID", "boolean");
			}
			dcUserBase->setInIpList(lua_toboolean(L, 3) != 0);
			break;

		case PARAM_HASH_HIDE :
			if (lua_type(L, 3) != LUA_TBOOLEAN) {
				ERR_TYPEMETA(3, "UID", "boolean");
			}
			dcUserBase->setHide(lua_toboolean(L, 3) != 0);
			break;

		case PARAM_HASH_KICK :
			if (lua_type(L, 3) != LUA_TBOOLEAN) {
				ERR_TYPEMETA(3, "UID", "boolean");
			}
			if (lua_toboolean(L, 3) != 0) {
				dcUserBase->setParam(USER_PARAM_CAN_KICK, "1");
			} else {
				dcUserBase->removeParam(USER_PARAM_CAN_KICK);
			}
			break;

		case PARAM_HASH_REDIRECT :
			if (lua_type(L, 3) != LUA_TBOOLEAN) {
				ERR_TYPEMETA(3, "UID", "boolean");
			}
			if (lua_toboolean(L, 3) != 0) {
				dcUserBase->setParam(USER_PARAM_CAN_FORCE_MOVE, "1");
			} else {
				dcUserBase->removeParam(USER_PARAM_CAN_FORCE_MOVE);
			}
			break;

		case PARAM_HASH_DATA :
			s = lua_tostring(L, 3);
			if (!s) {
				ERR_TYPEMETA(3, "UID", "string");
			}
			dcUserBase->setParam(USER_PARAM_DATA, s);
			break;

		default :
			break;

	}
	lua_settop(L, 0);
	return 0;
}



unsigned int Uid::getHash(const char * s) {
	size_t i, l = strlen(s);
	unsigned h = l;
	for (i = l; i > 0;) {
		h ^= ((h << 5) + (h >> 2) + (unsigned char)(s[--i]));
	}
	return h / 2;
}


}; // namespace luaplugin

/**
 * $Id$
 * $HeadURL$
 */
