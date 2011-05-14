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

	void ** userdata = NULL;
	switch(getHash(str)) {

		case PARAM_HASH_NICK :
			if (dcConnBase->mDcUserBase != NULL) {
				lua_pushstring(L, dcConnBase->mDcUserBase->getNick().c_str());
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_IP :
			lua_pushstring(L, dcConnBase->getIp().c_str());
			break;

		case PARAM_HASH_PROFILE :
			lua_pushnumber(L, dcConnBase->getProfile());
			break;

		case PARAM_HASH_MYINFO :
			if (dcConnBase->mDcUserBase != NULL) {
				lua_pushstring(L, dcConnBase->mDcUserBase->getMyINFO().c_str());
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_SHARE :
			if (dcConnBase->mDcUserBase != NULL) {
				lua_pushnumber(L, (double) dcConnBase->mDcUserBase->getShare());
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_MODE :
			if ((dcConnBase->mDcUserBase != NULL) && (dcConnBase->mDcUserBase->getTagNil() & TAGNIL_MODE)) {
				lua_pushstring(L, dcConnBase->mDcUserBase->getMode().c_str());
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_DESC :
			if (dcConnBase->mDcUserBase != NULL) {
				lua_pushstring(L, dcConnBase->mDcUserBase->getDesc().c_str());
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_EMAIL :
			if (dcConnBase->mDcUserBase != NULL) {
				lua_pushstring(L, dcConnBase->mDcUserBase->getEmail().c_str());
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_TAG :
			if ((dcConnBase->mDcUserBase != NULL) && (dcConnBase->mDcUserBase->getTagNil() & TAGNIL_TAG)) {
				lua_pushstring(L, dcConnBase->mDcUserBase->getTag().c_str());
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_CONN :
			if (dcConnBase->mDcUserBase != NULL) {
				lua_pushstring(L, dcConnBase->mDcUserBase->getConnection().c_str());
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_BYTE :
			if (dcConnBase->mDcUserBase != NULL) {
				lua_pushnumber(L, (double) dcConnBase->mDcUserBase->getByte());
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_CLIENTN :
			if ((dcConnBase->mDcUserBase != NULL) && (dcConnBase->mDcUserBase->getTagNil() & TAGNIL_CLIENT)) {
				lua_pushstring(L, dcConnBase->mDcUserBase->getClient().c_str());
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_CLIENTV :
			if ((dcConnBase->mDcUserBase != NULL) && (dcConnBase->mDcUserBase->getTagNil() & TAGNIL_VERSION)) {
				lua_pushstring(L, dcConnBase->mDcUserBase->getVersion().c_str());
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_SLOTS :
			if ((dcConnBase->mDcUserBase != NULL) && (dcConnBase->mDcUserBase->getTagNil() & TAGNIL_SLOT)) {
				lua_pushnumber(L, (double) dcConnBase->mDcUserBase->getSlots());
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_USHUBS :
			if ((dcConnBase->mDcUserBase != NULL) && (dcConnBase->mDcUserBase->getTagNil() & TAGNIL_UNREG)) {
				lua_pushnumber(L, (double) dcConnBase->mDcUserBase->getUnregHubs());
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_REGHUBS :
			if ((dcConnBase->mDcUserBase != NULL) && (dcConnBase->mDcUserBase->getTagNil() & TAGNIL_REG)) {
				lua_pushnumber(L, (double) dcConnBase->mDcUserBase->getRegHubs());
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_OPHUBS :
			if ((dcConnBase->mDcUserBase != NULL) && (dcConnBase->mDcUserBase->getTagNil() & TAGNIL_OP)) {
				lua_pushnumber(L, (double) dcConnBase->mDcUserBase->getOpHubs());
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_LIMIT :
			if ((dcConnBase->mDcUserBase != NULL) && (dcConnBase->mDcUserBase->getTagNil() & TAGNIL_LIMIT)) {
				lua_pushnumber(L, (double) dcConnBase->mDcUserBase->getLimit());
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_OPEN :
			if ((dcConnBase->mDcUserBase != NULL) && (dcConnBase->mDcUserBase->getTagNil() & TAGNIL_OPEN)) {
				lua_pushnumber(L, (double) dcConnBase->mDcUserBase->getOpen());
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_BANDWIDTH :
			if ((dcConnBase->mDcUserBase != NULL) && (dcConnBase->mDcUserBase->getTagNil() & TAGNIL_BANDWIDTH)) {
				lua_pushnumber(L, (double) dcConnBase->mDcUserBase->getBandwidth());
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_DOWNLOAD :
			if ((dcConnBase->mDcUserBase != NULL) && (dcConnBase->mDcUserBase->getTagNil() & TAGNIL_DOWNLOAD)) {
				lua_pushnumber(L, (double) dcConnBase->mDcUserBase->getDownload());
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_FRACTION :
			if ((dcConnBase->mDcUserBase != NULL) && (dcConnBase->mDcUserBase->getTagNil() & TAGNIL_FRACTION)) {
				lua_pushstring(L, dcConnBase->mDcUserBase->getFraction().c_str());
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_INOPLIST :
			if (dcConnBase->mDcUserBase != NULL) {
				lua_pushboolean(L, dcConnBase->mDcUserBase->getInOpList() ? 1 : 0);
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_INIPLIST :
			if (dcConnBase->mDcUserBase != NULL) {
				lua_pushboolean(L, dcConnBase->mDcUserBase->getInIpList() ? 1 : 0);
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_INUSERLIST :
			if (dcConnBase->mDcUserBase != NULL) {
				lua_pushboolean(L, dcConnBase->mDcUserBase->getInUserList() ? 1 : 0);
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_KICK :
			if (dcConnBase->mDcUserBase != NULL) {
				lua_pushboolean(L, dcConnBase->mDcUserBase->getKick() ? 1 : 0);
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_REDIRECT :
			if (dcConnBase->mDcUserBase != NULL) {
				lua_pushboolean(L, dcConnBase->mDcUserBase->getForceMove() ? 1 : 0);
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_HIDE :
			if (dcConnBase->mDcUserBase != NULL) {
				lua_pushboolean(L, dcConnBase->mDcUserBase->getHide() ? 1 : 0);
			} else {
				lua_pushnil(L);
			}
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
			lua_pushstring(L, dcConnBase->getSupports().c_str());
			break;

		case PARAM_HASH_VERSION :
			lua_pushstring(L, dcConnBase->getVersion().c_str());
			break;

		case PARAM_HASH_DATA :
			lua_pushstring(L, dcConnBase->getData().c_str());
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

	switch(getHash(s)) {

		case PARAM_HASH_PROFILE :
			dcConnBase->setProfile(luaL_checkint(L, 3));
			break;

		case PARAM_HASH_MYINFO :
			if (dcConnBase->mDcUserBase) {
				s = lua_tostring(L, 3);
				if (!s) {
					ERR_TYPEMETA(3, "UID", "string");
				}
				dcConnBase->mDcUserBase->setMyINFO(s);
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_INOPLIST :
			if (dcConnBase->mDcUserBase) {
				if (lua_type(L, 3) != LUA_TBOOLEAN) {
					ERR_TYPEMETA(3, "UID", "boolean");
				}
				dcConnBase->mDcUserBase->setInOpList(lua_toboolean(L, 3) != 0);
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_INIPLIST :
			if (dcConnBase->mDcUserBase) {
				if (lua_type(L, 3) != LUA_TBOOLEAN) {
					ERR_TYPEMETA(3, "UID", "boolean");
				}
				dcConnBase->mDcUserBase->setInIpList(lua_toboolean(L, 3) != 0);
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_HIDE :
			if (dcConnBase->mDcUserBase) {
				if (lua_type(L, 3) != LUA_TBOOLEAN) {
					ERR_TYPEMETA(3, "UID", "boolean");
				}
				dcConnBase->mDcUserBase->setHide(lua_toboolean(L, 3) != 0);
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_KICK :
			if (dcConnBase->mDcUserBase) {
				if (lua_type(L, 3) != LUA_TBOOLEAN) {
					ERR_TYPEMETA(3, "UID", "boolean");
				}
				dcConnBase->mDcUserBase->setKick(lua_toboolean(L, 3) != 0);
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_REDIRECT :
			if (dcConnBase->mDcUserBase) {
				if (lua_type(L, 3) != LUA_TBOOLEAN) {
					ERR_TYPEMETA(3, "UID", "boolean");
				}
				dcConnBase->mDcUserBase->setForceMove(lua_toboolean(L, 3) != 0);
			} else {
				lua_pushnil(L);
			}
			break;

		case PARAM_HASH_DATA :
			s = lua_tostring(L, 3);
			if (!s) {
				ERR_TYPEMETA(3, "UID", "string");
			}
			dcConnBase->setData(s);
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
