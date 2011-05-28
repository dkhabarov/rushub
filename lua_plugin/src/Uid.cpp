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



int Uid::userIndex(lua_State * L) {

	DcConnBase * dcConnBase = getDcConnBase(L, 1);
	if (!dcConnBase) {
		ERR_TYPEMETA(1, "UID", "userdata");
	}

	const char * str = lua_tostring(L, 2);
	if (!str) {
		ERR_TYPEMETA(2, "UID", "string");
	}

	DcUserBase * dcUserBase = dcConnBase->mDcUserBase;

	void ** userdata = NULL;
	switch(getHash(str)) {

		case PARAM_HASH_NICK :
			lua_pushstring(L, dcUserBase->getNick().c_str());
			break;

		case PARAM_HASH_IP :
			lua_pushstring(L, dcConnBase->getIp().c_str());
			break;

		case PARAM_HASH_PROFILE :
			lua_pushnumber(L, dcUserBase->getProfile());
			break;

		case PARAM_HASH_MYINFO :
			lua_pushstring(L, dcUserBase->getMyINFO().c_str());
			break;

		case PARAM_HASH_SHARE :
			lua_pushnumber(L, (double) dcUserBase->getShare());
			break;

		case PARAM_HASH_MODE :
			if (dcUserBase->getTagNil() & TAGNIL_MODE) {
				lua_pushstring(L, dcUserBase->getMode().c_str());
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_DESC :
			lua_pushstring(L, dcUserBase->getDesc().c_str());
			break;

		case PARAM_HASH_EMAIL :
			lua_pushstring(L, dcUserBase->getEmail().c_str());
			break;

		case PARAM_HASH_TAG :
			if (dcUserBase->getTagNil() & TAGNIL_TAG) {
				lua_pushstring(L, dcUserBase->getTag().c_str());
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_CONN :
			lua_pushstring(L, dcUserBase->getConnection().c_str());
			break;

		case PARAM_HASH_BYTE :
			lua_pushnumber(L, (double) dcUserBase->getByte());
			break;

		case PARAM_HASH_CLIENTN :
			if (dcUserBase->getTagNil() & TAGNIL_CLIENT) {
				lua_pushstring(L, dcUserBase->getClient().c_str());
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_CLIENTV :
			if (dcUserBase->getTagNil() & TAGNIL_VERSION) {
				lua_pushstring(L, dcUserBase->getClientVersion().c_str());
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_SLOTS :
			if (dcUserBase->getTagNil() & TAGNIL_SLOT) {
				lua_pushnumber(L, (double) dcUserBase->getSlots());
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_USHUBS :
			if (dcUserBase->getTagNil() & TAGNIL_UNREG) {
				lua_pushnumber(L, (double) dcUserBase->getUnregHubs());
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_REGHUBS :
			if (dcUserBase->getTagNil() & TAGNIL_REG) {
				lua_pushnumber(L, (double) dcUserBase->getRegHubs());
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_OPHUBS :
			if (dcUserBase->getTagNil() & TAGNIL_OP) {
				lua_pushnumber(L, (double) dcUserBase->getOpHubs());
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_LIMIT :
			if (dcUserBase->getTagNil() & TAGNIL_LIMIT) {
				lua_pushnumber(L, (double) dcUserBase->getLimit());
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_OPEN :
			if (dcUserBase->getTagNil() & TAGNIL_OPEN) {
				lua_pushnumber(L, (double) dcUserBase->getOpen());
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_BANDWIDTH :
			if (dcUserBase->getTagNil() & TAGNIL_BANDWIDTH) {
				lua_pushnumber(L, (double) dcUserBase->getBandwidth());
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_DOWNLOAD :
			if (dcUserBase->getTagNil() & TAGNIL_DOWNLOAD) {
				lua_pushnumber(L, (double) dcUserBase->getDownload());
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_FRACTION :
			if (dcUserBase->getTagNil() & TAGNIL_FRACTION) {
				lua_pushstring(L, dcUserBase->getFraction().c_str());
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_INOPLIST :
			lua_pushboolean(L, dcUserBase->getInOpList() ? 1 : 0);
			break;

		case PARAM_HASH_INIPLIST :
			lua_pushboolean(L, dcUserBase->getInIpList() ? 1 : 0);
			break;

		case PARAM_HASH_INUSERLIST :
			lua_pushboolean(L, dcUserBase->getInUserList() ? 1 : 0);
			break;

		case PARAM_HASH_KICK :
			lua_pushboolean(L, dcUserBase->getKick() ? 1 : 0);
			break;

		case PARAM_HASH_REDIRECT :
			lua_pushboolean(L, dcUserBase->getForceMove() ? 1 : 0);
			break;

		case PARAM_HASH_HIDE :
			lua_pushboolean(L, dcUserBase->getHide() ? 1 : 0);
			break;

		case PARAM_HASH_PORT :
			lua_pushnumber(L, dcConnBase->getPort());
			break;

		case PARAM_HASH_PORTCONN :
			lua_pushnumber(L, dcConnBase->getPortConn());
			break;

		case PARAM_HASH_IPCONN :
			lua_pushstring(L, dcConnBase->getIpConn().c_str());
			break;

		case PARAM_HASH_MACADDRESS :
			lua_pushstring(L, dcConnBase->getMacAddress().size() ? dcConnBase->getMacAddress().c_str() : "n/a");
			break;

		case PARAM_HASH_SUPPORTS :
			lua_pushstring(L, dcUserBase->getSupports().c_str());
			break;

		case PARAM_HASH_VERSION :
			lua_pushstring(L, dcUserBase->getVersion().c_str());
			break;

		case PARAM_HASH_DATA :
			lua_pushstring(L, dcUserBase->getData().c_str());
			break;

		case PARAM_HASH_ENTERTIME :
			lua_pushnumber(L, (lua_Number)dcConnBase->getConnectTime());
			break;

		case PARAM_HASH_UID :
			userdata = (void **) lua_newuserdata(L, sizeof(void *));
			*userdata = (void *) dcConnBase;
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

	DcConnBase * dcConnBase = getDcConnBase(L, 1);
	if (!dcConnBase) {
		ERR_TYPEMETA(1, "UID", "userdata");
	}

	const char * s = lua_tostring(L, 2);
	if (!s) {
		ERR_TYPEMETA(2, "UID", "string");
	}

	DcUserBase * dcUserBase = dcConnBase->mDcUserBase;

	switch(getHash(s)) {

		case PARAM_HASH_PROFILE :
			dcUserBase->setProfile(luaL_checkint(L, 3));
			break;

		case PARAM_HASH_MYINFO :
			s = lua_tostring(L, 3);
			if (!s) {
				ERR_TYPEMETA(3, "UID", "string");
			}
			dcUserBase->setMyINFO(s);
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
			dcUserBase->setKick(lua_toboolean(L, 3) != 0);
			break;

		case PARAM_HASH_REDIRECT :
			if (lua_type(L, 3) != LUA_TBOOLEAN) {
				ERR_TYPEMETA(3, "UID", "boolean");
			}
			dcUserBase->setForceMove(lua_toboolean(L, 3) != 0);
			break;

		case PARAM_HASH_DATA :
			s = lua_tostring(L, 3);
			if (!s) {
				ERR_TYPEMETA(3, "UID", "string");
			}
			dcUserBase->setData(s);
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
