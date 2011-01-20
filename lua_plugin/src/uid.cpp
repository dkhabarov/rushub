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

#include <string.h> // strlen

#include "uid.h"

namespace nLua {


unsigned int GetHash(const char * s) {
	size_t i, l = strlen(s);
	unsigned h = l;
	for(i = l; i > 0;)
		h ^= ((h << 5) + (h >> 2) + (unsigned char)(s[--i]));
	return h / 2;
}


int UidTostring(lua_State *L) {
	char buf[9] = { '\0' };
	sprintf(buf, "%p", *((void**)lua_touserdata(L, 1)));
	lua_pushfstring(L, "%s (%s)", lua_tostring(L, lua_upvalueindex(1)), buf);
	return 1;
}


int UserIndex(lua_State *L) {
	cDCConnBase * Conn = GetDCConnBase(L, 1);
	if(!Conn) ERR_TYPEMETA(1, "UID", "userdata");
	const char * s = lua_tostring(L, 2);
	if(!s) ERR_TYPEMETA(2, "UID", "string");
	void ** userdata = NULL;
	switch(GetHash(s)) {
		case ePH_NICK:       if(Conn->mDCUserBase)lua_pushstring(L, Conn->mDCUserBase->getNick().c_str());else lua_pushnil(L); break;
		case ePH_IP:         lua_pushstring (L, Conn->getIp().c_str()); break;
		case ePH_PROFILE:    lua_pushnumber (L, Conn->getProfile()); break;
		case ePH_MYINFO:     if(Conn->mDCUserBase)lua_pushstring(L, Conn->mDCUserBase->GetMyINFO().c_str());else lua_pushnil(L); break;
		case ePH_SHARE:      if(Conn->mDCUserBase)lua_pushnumber(L, (double)Conn->mDCUserBase->getShare());else lua_pushnil(L); break;
		case ePH_MODE:       if(Conn->mDCUserBase && (Conn->mDCUserBase->getTagNil() & TAGNIL_MODE))lua_pushstring(L, Conn->mDCUserBase->getMode().c_str());else lua_pushnil(L); break;
		case ePH_DESC:       if(Conn->mDCUserBase)lua_pushstring(L, Conn->mDCUserBase->getDesc().c_str());else lua_pushnil(L); break;
		case ePH_EMAIL:      if(Conn->mDCUserBase)lua_pushstring(L, Conn->mDCUserBase->getEmail().c_str());else lua_pushnil(L); break;
		case ePH_TAG:        if(Conn->mDCUserBase && (Conn->mDCUserBase->getTagNil() & TAGNIL_TAG))lua_pushstring(L, Conn->mDCUserBase->getTag().c_str());else lua_pushnil(L); break;
		case ePH_CONN:       if(Conn->mDCUserBase)lua_pushstring(L, Conn->mDCUserBase->getConnection().c_str());else lua_pushnil(L); break;
		case ePH_BYTE:       if(Conn->mDCUserBase)lua_pushnumber(L, Conn->mDCUserBase->getByte());else lua_pushnil(L); break;
		case ePH_CLIENTN:    if(Conn->mDCUserBase && (Conn->mDCUserBase->getTagNil() & TAGNIL_CLIENT))lua_pushstring(L, Conn->mDCUserBase->getClient().c_str());else lua_pushnil(L); break;
		case ePH_CLIENTV:    if(Conn->mDCUserBase && (Conn->mDCUserBase->getTagNil() & TAGNIL_VERSION))lua_pushstring(L, Conn->mDCUserBase->getVersion().c_str());else lua_pushnil(L); break;
		case ePH_SLOTS:      if(Conn->mDCUserBase && (Conn->mDCUserBase->getTagNil() & TAGNIL_SLOT))lua_pushnumber(L, Conn->mDCUserBase->getSlots());else lua_pushnil(L); break;
		case ePH_USHUBS:     if(Conn->mDCUserBase && (Conn->mDCUserBase->getTagNil() & TAGNIL_UNREG))lua_pushnumber(L, Conn->mDCUserBase->getUnregHubs());else lua_pushnil(L); break;
		case ePH_REGHUBS:    if(Conn->mDCUserBase && (Conn->mDCUserBase->getTagNil() & TAGNIL_REG))lua_pushnumber(L, Conn->mDCUserBase->getRegHubs());else lua_pushnil(L); break;
		case ePH_OPHUBS:     if(Conn->mDCUserBase && (Conn->mDCUserBase->getTagNil() & TAGNIL_OP))lua_pushnumber(L, Conn->mDCUserBase->getOpHubs());else lua_pushnil(L); break;
		case ePH_LIMIT:      if(Conn->mDCUserBase && (Conn->mDCUserBase->getTagNil() & TAGNIL_LIMIT))lua_pushnumber(L, Conn->mDCUserBase->getLimit());else lua_pushnil(L); break;
		case ePH_OPEN:       if(Conn->mDCUserBase && (Conn->mDCUserBase->getTagNil() & TAGNIL_OPEN))lua_pushnumber(L, Conn->mDCUserBase->getOpen());else lua_pushnil(L); break;
		case ePH_BANDWIDTH:  if(Conn->mDCUserBase && (Conn->mDCUserBase->getTagNil() & TAGNIL_BANDWIDTH))lua_pushnumber(L, Conn->mDCUserBase->getBandwidth());else lua_pushnil(L); break;
		case ePH_DOWNLOAD:   if(Conn->mDCUserBase && (Conn->mDCUserBase->getTagNil() & TAGNIL_DOWNLOAD))lua_pushnumber(L, Conn->mDCUserBase->getDownload());else lua_pushnil(L); break;
		case ePH_FRACTION:   if(Conn->mDCUserBase && (Conn->mDCUserBase->getTagNil() & TAGNIL_FRACTION))lua_pushstring(L, Conn->mDCUserBase->getFraction().c_str());else lua_pushnil(L); break;
		case ePH_INOPLIST:   if(Conn->mDCUserBase)lua_pushboolean(L, Conn->mDCUserBase->getInOpList() ? 1 : 0);else lua_pushnil(L); break;
		case ePH_INIPLIST:   if(Conn->mDCUserBase)lua_pushboolean(L, Conn->mDCUserBase->getInIpList() ? 1 : 0);else lua_pushnil(L); break;
		case ePH_INUSERLIST: if(Conn->mDCUserBase)lua_pushboolean(L, Conn->mDCUserBase->getInUserList() ? 1 : 0);else lua_pushnil(L); break;
		case ePH_KICK:       if(Conn->mDCUserBase)lua_pushboolean(L, Conn->mDCUserBase->getKick() ? 1 : 0);else lua_pushnil(L); break;
		case ePH_REDIRECT:   if(Conn->mDCUserBase)lua_pushboolean(L, Conn->mDCUserBase->getForceMove() ? 1 : 0);else lua_pushnil(L); break;
		case ePH_HIDE:       if(Conn->mDCUserBase)lua_pushboolean(L, Conn->mDCUserBase->getHide() ? 1 : 0);else lua_pushnil(L); break;
		case ePH_PORT:       lua_pushnumber(L, Conn->getPort()); break;
		case ePH_PORTCONN:   lua_pushnumber(L, Conn->getPortConn()); break;
		case ePH_IPCONN:     lua_pushstring(L, Conn->getIpConn().c_str()); break;
		case ePH_MACADDRESS: lua_pushstring(L, Conn->getMacAddress().size() ? Conn->getMacAddress().c_str() : "n/a"); break;
		case ePH_SUPPORTS:   lua_pushstring(L, Conn->getSupports().c_str()); break;
		case ePH_VERSION:    lua_pushstring(L, Conn->getVersion().c_str()); break;
		case ePH_DATA:       lua_pushstring(L, Conn->getData().c_str()); break;
		case ePH_ENTERTIME:  lua_pushnumber(L, (lua_Number)Conn->getEnterTime()); break;
		case ePH_UID:        userdata = (void**)lua_newuserdata(L, sizeof(void*)); *userdata = (void*)Conn; luaL_getmetatable(L, MT_USER_CONN); lua_setmetatable(L, -2); break;
		default:             lua_pushnil(L); break;
	}
	return 1;
}


int UserNewindex(lua_State *L) {
	cDCConnBase * Conn = GetDCConnBase(L, 1);
	if(!Conn) ERR_TYPEMETA(1, "UID", "userdata");
	const char * s = lua_tostring(L, 2);
	if(!s) ERR_TYPEMETA(2, "UID", "string");
	switch(GetHash(s)) {
		case ePH_PROFILE:   Conn->setProfile(luaL_checkint(L, 3)); break;
		case ePH_MYINFO:    if(Conn->mDCUserBase){ s = lua_tostring(L, 3); if(!s) ERR_TYPEMETA(3, "UID", "string"); Conn->mDCUserBase->SetMyINFO(s, Conn->mDCUserBase->getNick()); }else lua_pushnil(L); break;
		case ePH_INOPLIST:  if(Conn->mDCUserBase){ if(lua_type(L, 3) != LUA_TBOOLEAN) ERR_TYPEMETA(3, "UID", "boolean"); Conn->mDCUserBase->setInOpList(lua_toboolean(L, 3) != 0); }else lua_pushnil(L); break;
		case ePH_INIPLIST:  if(Conn->mDCUserBase){ if(lua_type(L, 3) != LUA_TBOOLEAN) ERR_TYPEMETA(3, "UID", "boolean"); Conn->mDCUserBase->setInIpList(lua_toboolean(L, 3) != 0); }else lua_pushnil(L); break;
		case ePH_HIDE:      if(Conn->mDCUserBase){ if(lua_type(L, 3) != LUA_TBOOLEAN) ERR_TYPEMETA(3, "UID", "boolean"); Conn->mDCUserBase->setHide(lua_toboolean(L, 3) != 0); }else lua_pushnil(L); break;
		case ePH_KICK:      if(Conn->mDCUserBase){ if(lua_type(L, 3) != LUA_TBOOLEAN) ERR_TYPEMETA(3, "UID", "boolean"); Conn->mDCUserBase->setKick(lua_toboolean(L, 3) != 0); }else lua_pushnil(L); break;
		case ePH_REDIRECT:  if(Conn->mDCUserBase){ if(lua_type(L, 3) != LUA_TBOOLEAN) ERR_TYPEMETA(3, "UID", "boolean"); Conn->mDCUserBase->setForceMove(lua_toboolean(L, 3) != 0); }else lua_pushnil(L); break;
		case ePH_DATA:      s = lua_tostring(L, 3); if(!s) ERR_TYPEMETA(3, "UID", "string"); Conn->setData(s); break;
		default:            break;
	}
	lua_settop(L, 0);
	return 0;
}


}; // nLua
