/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * Copyright (C) 2009-2010 by Setuper
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

#include "api.h"
#include "clua.h"

#include <fstream>
#include <vector>
#ifndef _WIN32
	#include <string.h> // strlen
#endif
using namespace std;

enum {
	eHS_STOP,
	eHS_RESTART
};


#define MAX_TIMERS 100 // max count timers per script
#define ERR_COUNT(SARG) { \
	lua_Debug ar; \
	if(!lua_getstack(L, 0, &ar)) return luaL_error(L, "bad argument count (%s expected, got %d)", SARG, lua_gettop(L)); \
	lua_getinfo(L, "n", &ar); \
	if(ar.name == NULL) ar.name = "?"; \
	return luaL_error(L, "bad argument count to " LUA_QS " (%s expected, got %d)", ar.name, SARG, lua_gettop(L)); \
}
#define CHECK_COUNT(NARG) \
if(lua_gettop(L) != NARG) { \
	char sBuf[32]; \
	sprintf(sBuf, "%d", NARG); \
	ERR_COUNT(sBuf); \
}

#define ERR_TYPEMETA(NARG, OBJ, SARG) { \
	return luaL_error(L, "bad argument #%d to " LUA_QS " (%s expected, got %s)", NARG, OBJ, SARG, luaL_typename(L, NARG)); \
}

#define REDIRECT_REASON_MAX_LEN 1024
#define REDIRECT_ADDRESS_MAX_LEN 128
#define MSGLEN(LEN) if(LEN < 1 || LEN > 128000) { lua_settop(L, 0); lua_pushnil(L); lua_pushliteral(L, "very long string."); return 2; }
#define NICKLEN(LEN) if(LEN < 1 || LEN > 64) { lua_settop(L, 0); lua_pushnil(L); lua_pushliteral(L, "very long nick."); return 2; }
#define FILELEN(LEN) if(LEN < 1 || LEN > 256) { lua_settop(L, 0); lua_pushnil(L); lua_pushliteral(L, "very long file name."); return 2; }
#define ERR(MSG) { lua_settop(L, 0); lua_pushnil(L); lua_pushstring(L, MSG); return 2; }
#define CHECKSCRIPTNAME() size_t iFileSize = sScriptName.size(); FILELEN(iFileSize); if(iFileSize <= 4 || (0 != sScriptName.compare(iFileSize - 4, 4, ".lua"))) sScriptName.append(".lua");
#define FINDINTERPRETER() \
cLua::tvLuaInterpreter::iterator it; \
for(it = cLua::mCurLua->mLua.begin(); it != cLua::mCurLua->mLua.end(); ++it) { \
	if((*it)->msName == sScriptName) { \
		LIP = *it; break; \
	} \
} \
if(!LIP || !LIP->mL) ERR("script was not found");

// lua 5.1
#define luaL_typeerror luaL_typerror

namespace nLua {

int Tostring(lua_State *L) {
	char buf[9];
	sprintf(buf, "%p", lua_touserdata(L, 1));
	lua_pushfstring(L, "%s (%s)", lua_tostring(L, lua_upvalueindex(1)), buf);
	return 1;
}

int ConfigTable(lua_State *L) {
	static const vector<string> & vConf = cLua::mCurServer->GetConfig();
	int indx = 1;
	lua_newtable(L);
	for(vector<string>::const_iterator it = vConf.begin(); it != vConf.end(); ++it) {
		lua_pushnumber(L, indx++);
		lua_pushstring(L, (*it).c_str());
		lua_rawset(L, -3);
	}
	return 1;
}

unsigned int GetHash(const char * s) {
	size_t i, l = strlen(s);
	unsigned h = l;
	for(i = l; i > 0;)
		h ^= ((h << 5) + (h >> 2) + (unsigned char)(s[--i]));
	return h / 2;
}

typedef enum { /** Param's hash */
	ePH_IP         = 90709,      //< "sIP"
	ePH_UID        = 92355,      //< "UID"
	ePH_TAG        = 4321573,    //< "sTag"
	ePH_IPCONN     = 73645412,   //< "sIPConn"
	ePH_KICK       = 136610353,  //< "bKick"
	ePH_NICK       = 136635514,  //< "sNick"
	ePH_CONN       = 137104726,  //< "sConn"
	ePH_OPEN       = 137319447,  //< "iOpen"
	ePH_HIDE       = 141759876,  //< "bHide"
	ePH_MODE       = 141762382,  //< "sMode"
	ePH_BYTE       = 142044234,  //< "iByte"
	ePH_DATA       = 143784544,  //< "sData"
	ePH_PORT       = 148797436,  //< "iPort"
	ePH_DESC       = 149333075,  //< "sDesc"
	ePH_FRACTION   = 227502190,  //< "sFraction"
	ePH_REGHUBS    = 376863999,  //< "iRegHubs"
	ePH_INUSERLIST = 353254788,  //< "bInUserList"
	ePH_REDIRECT   = 355925330,  //< "bRedirect"
	ePH_ENTERTIME  = 381948915,  //< "iEnterTime"
	ePH_SUPPORTS   = 404020597,  //< "sSupports"
	ePH_PORTCONN   = 491785156,  //< "iPortConn"
	ePH_INOPLIST   = 549773767, //< "bInOpList"
	ePH_INIPLIST   = 549865865, //< "bInIpList"
	ePH_BANDWIDTH  = 568949422, //< "iBandwidth"
	ePH_SHARE      = 632724080, //< "iShare"
	ePH_CLIENTV    = 885079029, //< "sClientVersion"
	ePH_PROFILE    = 970292741, //< "iProfile"
	ePH_SLOTS      = 1035743627, //< "iSlots"
	ePH_VERSION    = 1055480429, //< "sVersion"
	ePH_EMAIL      = 1106490266, //< "sEmail"	
	ePH_DOWNLOAD   = 1180934250, //< "iDownload"
	ePH_LIMIT      = 1231291461, //< "iLimit"
	ePH_MACADDRESS = 1643104854, //< "sMacAddress"
	ePH_MYINFO     = 1652447804, //< "sMyINFO"
	ePH_CLIENTN    = 1719639115, //< "sClientName"
	ePH_USHUBS     = 1868311651, //< "iUsHubs"
	ePH_OPHUBS     = 1868312482, //< "iOpHubs"
} tParamHash;

int UserIndex(lua_State *L) {
	cDCConnBase * Conn = (cDCConnBase *)lua_touserdata(L, 1);
	const char * s = lua_tostring(L, 2);
	if(!s) ERR_TYPEMETA(2, "UID", "string");
	switch(GetHash(s)) {
		case ePH_NICK:       if(Conn->mDCUserBase)lua_pushstring(L, Conn->mDCUserBase->GetNick().c_str());else lua_pushnil(L); break;
		case ePH_IP:         lua_pushstring (L, Conn->GetIp().c_str()); break;
		case ePH_PROFILE:    lua_pushnumber (L, Conn->GetProfile()); break;
		case ePH_MYINFO:     if(Conn->mDCUserBase)lua_pushstring(L, Conn->mDCUserBase->GetMyINFO().c_str());else lua_pushnil(L); break;
		case ePH_SHARE:      if(Conn->mDCUserBase)lua_pushnumber(L, (double)Conn->mDCUserBase->GetShare());else lua_pushnil(L); break;
		case ePH_MODE:       if(Conn->mDCUserBase && (Conn->mDCUserBase->mNil & eMYINFO_MODE))lua_pushstring(L, Conn->mDCUserBase->GetMode().c_str());else lua_pushnil(L); break;
		case ePH_DESC:       if(Conn->mDCUserBase)lua_pushstring(L, Conn->mDCUserBase->GetDesc().c_str());else lua_pushnil(L); break;
		case ePH_EMAIL:      if(Conn->mDCUserBase)lua_pushstring(L, Conn->mDCUserBase->GetEmail().c_str());else lua_pushnil(L); break;
		case ePH_TAG:        if(Conn->mDCUserBase && (Conn->mDCUserBase->mNil & eMYINFO_TAG))lua_pushstring(L, Conn->mDCUserBase->GetTag().c_str());else lua_pushnil(L); break;
		case ePH_CONN:       if(Conn->mDCUserBase)lua_pushstring(L, Conn->mDCUserBase->GetConnection().c_str());else lua_pushnil(L); break;
		case ePH_BYTE:       if(Conn->mDCUserBase)lua_pushnumber(L, Conn->mDCUserBase->GetByte());else lua_pushnil(L); break;
		case ePH_CLIENTN:    if(Conn->mDCUserBase && (Conn->mDCUserBase->mNil & eMYINFO_CLIENT))lua_pushstring(L, Conn->mDCUserBase->GetClient().c_str());else lua_pushnil(L); break;
		case ePH_CLIENTV:    if(Conn->mDCUserBase && (Conn->mDCUserBase->mNil & eMYINFO_VERSION))lua_pushstring(L, Conn->mDCUserBase->GetVersion().c_str());else lua_pushnil(L); break;
		case ePH_SLOTS:      if(Conn->mDCUserBase && (Conn->mDCUserBase->mNil & eMYINFO_SLOT))lua_pushnumber(L, Conn->mDCUserBase->GetSlots());else lua_pushnil(L); break;
		case ePH_USHUBS:     if(Conn->mDCUserBase && (Conn->mDCUserBase->mNil & eMYINFO_UNREG))lua_pushnumber(L, Conn->mDCUserBase->GetUnRegHubs());else lua_pushnil(L); break;
		case ePH_REGHUBS:    if(Conn->mDCUserBase && (Conn->mDCUserBase->mNil & eMYINFO_REG))lua_pushnumber(L, Conn->mDCUserBase->GetRegHubs());else lua_pushnil(L); break;
		case ePH_OPHUBS:     if(Conn->mDCUserBase && (Conn->mDCUserBase->mNil & eMYINFO_OP))lua_pushnumber(L, Conn->mDCUserBase->GetOpHubs());else lua_pushnil(L); break;
		case ePH_LIMIT:      if(Conn->mDCUserBase && (Conn->mDCUserBase->mNil & eMYINFO_LIMIT))lua_pushnumber(L, Conn->mDCUserBase->GetLimit());else lua_pushnil(L); break;
		case ePH_OPEN:       if(Conn->mDCUserBase && (Conn->mDCUserBase->mNil & eMYINFO_OPEN))lua_pushnumber(L, Conn->mDCUserBase->GetOpen());else lua_pushnil(L); break;
		case ePH_BANDWIDTH:  if(Conn->mDCUserBase && (Conn->mDCUserBase->mNil & eMYINFO_BANDWIDTH))lua_pushnumber(L, Conn->mDCUserBase->GetBandwidth());else lua_pushnil(L); break;
		case ePH_DOWNLOAD:   if(Conn->mDCUserBase && (Conn->mDCUserBase->mNil & eMYINFO_DOWNLOAD))lua_pushnumber(L, Conn->mDCUserBase->GetDownload());else lua_pushnil(L); break;
		case ePH_FRACTION:   if(Conn->mDCUserBase && (Conn->mDCUserBase->mNil & eMYINFO_FRACTION))lua_pushstring(L, Conn->mDCUserBase->GetFraction().c_str());else lua_pushnil(L); break;
		case ePH_INOPLIST:   if(Conn->mDCUserBase)lua_pushboolean(L, Conn->mDCUserBase->IsInOpList() ? 1 : 0);else lua_pushnil(L); break;
		case ePH_INIPLIST:   if(Conn->mDCUserBase)lua_pushboolean(L, Conn->mDCUserBase->IsInIpList() ? 1 : 0);else lua_pushnil(L); break;
		case ePH_INUSERLIST: if(Conn->mDCUserBase)lua_pushboolean(L, Conn->mDCUserBase->IsInUserList() ? 1 : 0);else lua_pushnil(L); break;
		case ePH_KICK:       if(Conn->mDCUserBase)lua_pushboolean(L, Conn->mDCUserBase->IsKick() ? 1 : 0);else lua_pushnil(L); break;
		case ePH_REDIRECT:   if(Conn->mDCUserBase)lua_pushboolean(L, Conn->mDCUserBase->IsForceMove() ? 1 : 0);else lua_pushnil(L); break;
		case ePH_HIDE:       if(Conn->mDCUserBase)lua_pushboolean(L, Conn->mDCUserBase->IsHide() ? 1 : 0);else lua_pushnil(L); break;
		case ePH_PORT:       lua_pushnumber(L, Conn->GetPort()); break;
		case ePH_PORTCONN:   lua_pushnumber(L, Conn->GetPortConn()); break;
		case ePH_IPCONN:     lua_pushstring(L, Conn->GetIpConn().c_str()); break;
		case ePH_MACADDRESS: lua_pushstring(L, Conn->GetMacAddr().size() ? Conn->GetMacAddr().c_str() : "n/a"); break;
		case ePH_SUPPORTS:   lua_pushstring(L, Conn->GetSupports().c_str()); break;
		case ePH_VERSION:    lua_pushstring(L, Conn->GetVersion().c_str()); break;
		case ePH_DATA:       lua_pushstring(L, Conn->GetData().c_str()); break;
		case ePH_ENTERTIME:  lua_pushnumber(L, (lua_Number)Conn->GetEnterTime()); break;
		case ePH_UID:        lua_pushlightuserdata(L, (void*)Conn); break; // for compatibility
		default:             lua_pushnil(L); break;
	}
	return 1;
}

int UserNewindex(lua_State *L) {
	cDCConnBase * Conn = (cDCConnBase *)lua_touserdata(L, 1);
	const char * s = lua_tostring(L, 2);
	if(!s) ERR_TYPEMETA(2, "UID", "string");
	switch(GetHash(s)) {
		case ePH_PROFILE:   Conn->SetProfile(luaL_checkint(L, 3)); break;
		case ePH_MYINFO:    if(Conn->mDCUserBase){ s = lua_tostring(L, 3); if(!s) ERR_TYPEMETA(3, "UID", "string"); Conn->mDCUserBase->SetMyINFO(s, Conn->mDCUserBase->GetNick()); }else lua_pushnil(L); break;
		case ePH_INOPLIST:  if(Conn->mDCUserBase){ if(lua_type(L, 3) != LUA_TBOOLEAN) ERR_TYPEMETA(3, "UID", "boolean"); Conn->mDCUserBase->SetOpList(lua_toboolean(L, 3) != 0); }else lua_pushnil(L); break;
		case ePH_INIPLIST:  if(Conn->mDCUserBase){ if(lua_type(L, 3) != LUA_TBOOLEAN) ERR_TYPEMETA(3, "UID", "boolean"); Conn->mDCUserBase->SetIpList(lua_toboolean(L, 3) != 0); }else lua_pushnil(L); break;
		case ePH_HIDE:      if(Conn->mDCUserBase){ if(lua_type(L, 3) != LUA_TBOOLEAN) ERR_TYPEMETA(3, "UID", "boolean"); Conn->mDCUserBase->SetHide(lua_toboolean(L, 3) != 0); }else lua_pushnil(L); break;
		case ePH_KICK:      if(Conn->mDCUserBase){ if(lua_type(L, 3) != LUA_TBOOLEAN) ERR_TYPEMETA(3, "UID", "boolean"); Conn->mDCUserBase->SetKick(lua_toboolean(L, 3) != 0); }else lua_pushnil(L); break;
		case ePH_REDIRECT:  if(Conn->mDCUserBase){ if(lua_type(L, 3) != LUA_TBOOLEAN) ERR_TYPEMETA(3, "UID", "boolean"); Conn->mDCUserBase->SetForceMove(lua_toboolean(L, 3) != 0); }else lua_pushnil(L); break;
		case ePH_DATA:      s = lua_tostring(L, 3); if(!s) ERR_TYPEMETA(3, "UID", "string"); Conn->SetData(s); break;
		default:            break;
	}
	lua_settop(L, 0);
	return 0;
}

int ConfigIndex(lua_State *L) {
	cConfig * Config = (cConfig *)lua_touserdata(L, 1);
	if(Config->isExist != 1) {
		lua_settop(L, 0);
		lua_pushnil(L);
		return 1;
	}
	const char * sName = lua_tostring(L, 2);
	if(!sName) ERR_TYPEMETA(2, "Config", "string");
	if(!strcmp(sName, "table")) {
		lua_settop(L, 0);
		lua_pushcfunction(L, &ConfigTable);
		return 1;
	}
	const char * sConfig = cLua::mCurServer->GetConfig(sName);
	lua_settop(L, 0);
	if(!sConfig) lua_pushnil(L);
	else lua_pushstring(L, sConfig);
	return 1;
}

int ConfigNewindex(lua_State *L) {
	cConfig * Config = (cConfig *)lua_touserdata(L, 1);
	if(Config->isExist != 1) {
		lua_settop(L, 0);
		return 0;
	}
	const char * sName = lua_tostring(L, 2);
	if(!sName) ERR_TYPEMETA(2, "Config", "string");
	char * sValue = (char *)lua_tostring(L, 3);
	if(!sValue) sValue = (char *)lua_toboolean(L, 3);
	if(!sValue) ERR_TYPEMETA(3, "Config", "string or boolean");
	cLua::mCurServer->SetConfig(sName, sValue);
	cLua::mCurLua->OnConfigChange(sName, sValue);
	lua_settop(L, 0);
	return 0;
}

void LogError(const char *sMsg) {
	static string s(cLua::mCurServer->GetMainDir() + "luaerr.log");
	ofstream Ofs(s.c_str(), ios_base::app);
	if(sMsg){
		Ofs << "[" << cLua::mCurServer->GetTime() << "] " << string(sMsg) << endl;
	}else{
		Ofs << "[" << cLua::mCurServer->GetTime() << "] unknown LUA error" << endl;}
	Ofs.flush();
	Ofs.close();
}

static void SetTbl(lua_State *L1, lua_State *L2, int idx) {
	lua_newtable(L2);
	lua_pushnil(L1);
	int iTop = lua_gettop(L2);
	while(lua_next(L1, idx > 0 ? idx : idx - 1) != 0) {
		CopyValue(L1, L2, -2); // key
		CopyValue(L1, L2, -1); // value
		lua_rawset(L2, iTop);
		lua_pop(L1, 1);
	}
}

void CopyValue(lua_State *From, lua_State *To, int idx) {
	switch(lua_type(From, idx)) {
		case LUA_TSTRING:
			lua_pushstring(To, lua_tostring(From, idx));
			break;
		case LUA_TNUMBER:
			lua_pushnumber(To, lua_tonumber(From, idx));
			break;
		case LUA_TBOOLEAN:
			lua_pushboolean(To, lua_toboolean(From, idx));
			break;
		case LUA_TTABLE:
			SetTbl(From, To, idx);
			break;
		case LUA_TLIGHTUSERDATA:
			lua_pushlightuserdata(To, lua_touserdata(From, idx));
			break;
		default:
			lua_pushnil(To);
			break;
	}
}

/// GetGVal(sScriptName, sParam)
int GetGVal(lua_State *L) {
	CHECK_COUNT(2);
	size_t iLen;
	string sScriptName(luaL_checklstring(L, 1, &iLen));
	FILELEN(iLen);
	cLuaInterpreter * LIP = NULL;
	CHECKSCRIPTNAME();
	FINDINTERPRETER();

	lua_State *LL = lua_newthread(LIP->mL);
	lua_getglobal(LL, luaL_checkstring(L, 2));

	//lua_settop(LIP->mL, 0);
	lua_pop(LIP->mL, 1);
	CopyValue(LL, L, -1);

	return 1;
}

/// SetGVal(sScriptName, sParam, Value)
int SetGVal(lua_State *L) {
	CHECK_COUNT(3);
	size_t iLen;
	string sScriptName(luaL_checklstring(L, 1, &iLen));
	FILELEN(iLen);
	cLuaInterpreter * LIP = NULL;
	CHECKSCRIPTNAME();
	FINDINTERPRETER();

	lua_State *LL = lua_newthread(LIP->mL);
	const char * sParam = luaL_checkstring(L, 2);
	lua_getglobal(LL, sParam);
	CopyValue(L, LL, 3);
	lua_setglobal(LL, sParam);
	//lua_xmove(LL, LIP->mL, 1);

	lua_pop(LIP->mL, 1);
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// SendToUser(UID/sToNick, sData, sNick, sFrom)
int SendToUser(lua_State *L) {
	size_t iLen;
	int iType;
	const char *sNick = NULL, *sFrom = NULL;
	bool bByUID = true;

	switch(lua_gettop(L)) {
		case 4:
			iType = lua_type(L, 4);
			if(iType != LUA_TNIL) {
				sFrom = luaL_checklstring(L, 4, &iLen);
				NICKLEN(iLen);
			}
		case 3:
			iType = lua_type(L, 3);
			if(iType != LUA_TNIL) {
				sNick = luaL_checklstring(L, 3, &iLen);
				NICKLEN(iLen);
			}
		case 2:
			iType = lua_type(L, 1);
			if(iType != LUA_TLIGHTUSERDATA) {
				if(iType != LUA_TSTRING) return luaL_typeerror(L, 1, "lightuserdata or string");
				bByUID = false;
			}
			break;
		default:
			ERR_COUNT("2, 3 or 4");
	}

	const char * sData = luaL_checklstring(L, 2, &iLen);

	if(bByUID) {
		cDCConnBase * Conn = (cDCConnBase *)lua_touserdata(L, 1);
		if(!Conn || (Conn->mType != eT_DC_CLIENT && Conn->mType != eT_WEB_CLIENT)) ERR("user was not found");
		if(Conn->mType == eT_WEB_CLIENT) {
			if(!Conn->Send(sData)) // not newPolitic fot timers only
				ERR("data was not sent");
		} else {
			MSGLEN(iLen);
			if(!cLua::mCurServer->SendToUser(Conn, sData, sNick, sFrom))
				ERR("user was not found");
		}
	} else {
		const char *sTo = lua_tolstring(L, 1, &iLen);
		NICKLEN(iLen);
		if(!cLua::mCurServer->SendToNick(sTo, sData, sNick, sFrom))
			ERR("user was not found");
	}

	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// SendToAll(sData, sNick, sFrom)
int SendToAll(lua_State *L) {
	size_t iLen;
	const char *sData, *sNick = NULL, *sFrom = NULL;
	switch(lua_gettop(L)) {
		case 3:
			if(lua_type(L, 3) != LUA_TNIL) {
				sFrom = luaL_checklstring(L, 3, &iLen);
				NICKLEN(iLen);
			}
		case 2:
			if(lua_type(L, 2) != LUA_TNIL) {
				sNick = luaL_checklstring(L, 2, &iLen);
				NICKLEN(iLen);
			}
		case 1:
			sData = luaL_checklstring(L, 1, &iLen);
			MSGLEN(iLen);
			break;
		default:
			ERR_COUNT("1, 2 or 3");
	}
	if(!cLua::mCurServer->SendToAll(sData, sNick, sFrom))
		ERR("data was not found");
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// SendToProfile(iProfile/tProfiles, sData, sNick, sFrom)
int SendToProfile(lua_State *L) {
	size_t iLen;
	unsigned long iProfile = 0, iPrf;
	int iType, iProf;
	const char *sData, *sNick = NULL, *sFrom = NULL;
	switch(lua_gettop(L)) {
		case 4:
			if(lua_type(L, 4) != LUA_TNIL) {
				sFrom = luaL_checklstring(L, 4, &iLen);
				NICKLEN(iLen);
			}
		case 3:
			if(lua_type(L, 3) != LUA_TNIL) {
				sNick = luaL_checklstring(L, 3, &iLen);
				NICKLEN(iLen);
			}
		case 2:
			sData = luaL_checklstring(L, 2, &iLen);
			MSGLEN(iLen);

			iType = lua_type(L, 1);
			if(iType == LUA_TTABLE) {
				lua_pushnil(L);
				while(lua_next(L, 1) != 0) {
					if((iProf = luaL_checkint(L, -1) + 1) < 0) iProf = -iProf;
					iPrf = 1 << (iProf % 32);
					if(!(iProfile & iPrf)) iProfile = iProfile | iPrf;
					lua_pop(L, 1);
				}
				if(!iProfile) ERR("list turned out to be empty");
			} else if(iType == LUA_TNUMBER) {
				if((iProf = luaL_checkint(L, 1) + 1) < 0) iProf = -iProf;
				iProfile = 1 << (iProf % 32);
			} else return luaL_typeerror(L, 1, "number or table");
			break;
		default:
			ERR_COUNT("2, 3 or 4");
	}
	if(!cLua::mCurServer->SendToProfiles(iProfile, sData, sNick, sFrom))
		ERR("data was not found");
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// SendToIP(sIP, sData, sNick, sFrom, iProfile/tProfiles)
int SendToIP(lua_State *L) {
	size_t iLen;
	unsigned long iProfile = 0, iPrf;
	int iType, iProf;
	const char *sIP, *sData, *sNick = NULL, *sFrom = NULL;

	switch(lua_gettop(L)) {
		case 5:
			iType = lua_type(L, 1);
			if(iType == LUA_TTABLE) {
				lua_pushnil(L);
				while(lua_next(L, 1) != 0) {
					if((iProf = luaL_checkint(L, -1) + 1) < 0) iProf = -iProf;
					iPrf = 1 << (iProf % 32);
					if(!(iProfile & iPrf)) iProfile = iProfile | iPrf;
					lua_pop(L, 1);
				}
				if(!iProfile) ERR("list turned out to be empty");
			} else if(iType == LUA_TNUMBER) {
				if((iProf = luaL_checkint(L, 1) + 1) < 0) iProf = -iProf;
				iProfile = 1 << (iProf % 32);
			} else if(iType != LUA_TNIL) return luaL_typeerror(L, 1, "number or table");
		case 4:
			if(lua_type(L, 4) != LUA_TNIL) {
				sFrom = luaL_checklstring(L, 4, &iLen);
				NICKLEN(iLen);
			}
		case 3:
			if(lua_type(L, 3) != LUA_TNIL) {
				sNick = luaL_checklstring(L, 3, &iLen);
				NICKLEN(iLen);
			}
		case 2:
			sData = luaL_checklstring(L, 2, &iLen);
			MSGLEN(iLen);
			break;
		default:
			ERR_COUNT("2, 3, 4 or 5");
	}
	sIP = luaL_checklstring(L, 1, &iLen);
	if(sIP && !cLua::mCurServer->SendToIP(sIP, sData, iProfile, sNick, sFrom))
		ERR("wrong ip format");
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// SendToAllExceptNicks(tExcept, sData, sNick, sFrom)
int SendToAllExceptNicks(lua_State *L) {
	size_t iLen;
	vector<string> NickList;
	const char *sData, *sNick = NULL, *sFrom = NULL, *sN;
	switch(lua_gettop(L)) {
		case 4:
			if(lua_type(L, 4) != LUA_TNIL) {
				sFrom = luaL_checklstring(L, 4, &iLen);
				NICKLEN(iLen);
			}
		case 3:
			if(lua_type(L, 3) != LUA_TNIL) {
				sNick = luaL_checklstring(L, 3, &iLen);
				NICKLEN(iLen);
			}
		case 2:
			luaL_checktype(L, 1, LUA_TTABLE);
			sData = luaL_checklstring(L, 2, &iLen);
			MSGLEN(iLen);

			lua_pushnil(L);
			while(lua_next(L, 1) != 0) {
				sN = luaL_checklstring(L, -1, &iLen);
				NICKLEN(iLen);
				NickList.push_back(sN);
				lua_pop(L, 1);
			}
			if(!NickList.size()) ERR("list turned out to be empty");
			break;
		default:
			ERR_COUNT("2, 3 or 4");
	}
	if(!cLua::mCurServer->SendToAllExceptNicks(NickList, sData, sNick, sFrom))
		ERR("data was not found");
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// SendToAllExceptIps(tExcept, sData, sNick, sFrom)
int SendToAllExceptIps(lua_State *L) {
	size_t iLen;
	vector<string> IPList;
	const char *sData, *sNick = NULL, *sFrom = NULL, *sN;
	switch(lua_gettop(L)) {
		case 4:
			if(lua_type(L, 4) != LUA_TNIL) {
				sFrom = luaL_checklstring(L, 4, &iLen);
				NICKLEN(iLen);
			}
		case 3:
			if(lua_type(L, 3) != LUA_TNIL) {
				sNick = luaL_checklstring(L, 3, &iLen);
				NICKLEN(iLen);
			}
		case 2:
			luaL_checktype(L, 1, LUA_TTABLE);
			sData = luaL_checklstring(L, 2, &iLen);
			MSGLEN(iLen);

			lua_pushnil(L);
			while(lua_next(L, 1) != 0) {
				sN = luaL_checklstring(L, -1, &iLen);
				NICKLEN(iLen);
				IPList.push_back(sN);
				lua_pop(L, 1);
			}
			if(!IPList.size()) ERR("list turned out to be empty");
			break;
		default:
			ERR_COUNT("2, 3 or 4");
	}
	if(!cLua::mCurServer->SendToAllExceptIps(IPList, sData, sNick, sFrom))
		ERR("wrong ip format");
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// SendToNicks(tNicks, sData, sNick, sFrom)
int SendToNicks(lua_State *L) {
	size_t iLen;
	const char *sData, *sNick = NULL, *sFrom = NULL, *sN;
	switch(lua_gettop(L)) {
		case 4:
			if(lua_type(L, 4) != LUA_TNIL) {
				sFrom = luaL_checklstring(L, 4, &iLen);
				NICKLEN(iLen);
			}
		case 3:
			if(lua_type(L, 3) != LUA_TNIL) {
				sNick = luaL_checklstring(L, 3, &iLen);
				NICKLEN(iLen);
			}
		case 2:
			luaL_checktype(L, 1, LUA_TTABLE);
			sData = luaL_checklstring(L, 2, &iLen);
			MSGLEN(iLen);

			lua_pushnil(L);
			while(lua_next(L, 1) != 0) {
				sN = luaL_checklstring(L, -1, &iLen);
				NICKLEN(iLen);
				cLua::mCurServer->SendToNick(sN, sData, sNick, sFrom);
				lua_pop(L, 1);
			}
			break;
		default:
			ERR_COUNT("2, 3 or 4");
	}
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// GetUser(UID/sNick, iByte)
int GetUser(lua_State *L) {
	if(lua_type(L, 1) == LUA_TLIGHTUSERDATA)
		lua_pushlightuserdata(L, lua_touserdata(L, 1));
	else if(lua_isstring(L, 1)) {
		size_t iLen;
		const char * sNick = lua_tolstring(L, 1, &iLen);
		NICKLEN(iLen);
		cDCUserBase * User = cLua::mCurServer->GetDCUserBase(sNick);
		if(!User || !User->mDCConnBase) ERR("user was not found");
		lua_pushlightuserdata(L, (void*)User->mDCConnBase);
		luaL_getmetatable(L, MT_USER_CONN);
		lua_setmetatable(L, -2);
	} else return luaL_typeerror(L, 1, "lightuserdata or string");
	return 1;
}

/// SetUser(UID/sNick, iType, Value)
int SetUser(lua_State *L) {
	CHECK_COUNT(3);
	cDCConnBase * Conn;
	if(lua_type(L, 1) == LUA_TLIGHTUSERDATA) {
		Conn = (cDCConnBase *)lua_touserdata(L, 1);
		if(!Conn || Conn->mType != eT_DC_CLIENT) ERR("user was not found");
	} else if(lua_isstring(L, 1)) {
		size_t iLen;
		const char * sNick = lua_tolstring(L, 1, &iLen);
		NICKLEN(iLen);
		cDCUserBase * User = cLua::mCurServer->GetDCUserBase(sNick);
		if(!User || !User->mDCConnBase) ERR("user was not found");
		Conn = User->mDCConnBase;
	} else return luaL_typeerror(L, 1, "lightuserdata or string");

	unsigned iNum = (unsigned)luaL_checkinteger(L, 2);
	if(iNum == eUV_iProfile) {
		Conn->SetProfile(luaL_checkint(L, 3));
	} else if(iNum == eUV_sMyINFO) {
		if(!Conn->mDCUserBase) ERR("user was not found");
		string sMyINFO(luaL_checkstring(L, 3));
		if(!Conn->mDCUserBase->SetMyINFO(sMyINFO, Conn->mDCUserBase->GetNick())) ERR("wrong syntax");
	} else if(iNum == eUV_sData) {
		Conn->SetData(luaL_checkstring(L, 3));
	} else if(iNum == eUV_bOpList) {
		luaL_checktype(L, 3, LUA_TBOOLEAN);
		if(!Conn->mDCUserBase) ERR("user was not found");
		Conn->mDCUserBase->SetOpList(lua_toboolean(L, 3) != 0);
	} else if(iNum == eUV_bHide) {
		luaL_checktype(L, 3, LUA_TBOOLEAN);
		if(!Conn->mDCUserBase) ERR("user was not found");
		Conn->mDCUserBase->SetHide(lua_toboolean(L, 3) != 0);
	} else if(iNum == eUV_bIpList) {
		luaL_checktype(L, 3, LUA_TBOOLEAN);
		if(!Conn->mDCUserBase) ERR("user was not found");
		Conn->mDCUserBase->SetIpList(lua_toboolean(L, 3) != 0);
	}
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// GetUsers(sIP, iByte)
int GetUsers(lua_State *L) {
	lua_newtable(L);
	int iTopTab = lua_gettop(L), i = 1;

	if(lua_type(L, 1) == LUA_TSTRING) {
		const vector<cDCConnBase *> & v = cLua::mCurServer->GetDCConnBase(lua_tostring(L, 1));
		for(vector<cDCConnBase *>::const_iterator it = v.begin(); it != v.end(); ++it) {
			lua_pushnumber(L, i);
			lua_pushlightuserdata(L, (void*)(*it));
			luaL_getmetatable(L, MT_USER_CONN);
			lua_setmetatable(L, -2);
			lua_rawset(L, iTopTab);
			++i;
		}
	} else {
		cDCConnListIterator * it = cLua::mCurServer->GetDCConnListIterator();
		cDCConnBase * Conn;
		while((Conn = it->operator ()()) != NULL) {
			if(Conn->mType != eT_DC_CLIENT) continue;
			lua_pushnumber(L, i);
			lua_pushlightuserdata(L, (void*)Conn);
			luaL_getmetatable(L, MT_USER_CONN);
			lua_setmetatable(L, -2);
			lua_rawset(L, iTopTab);
			++i;
		}
		delete it;
	}
	return 1;
}

/// GetUsersCount()
int GetUsersCount(lua_State *L) {
	lua_pushnumber(L, cLua::mCurServer->GetUsersCount());
	return 1;
}

/// GetTotalShare()
int GetTotalShare(lua_State *L) {
	lua_pushnumber(L, (lua_Number)cLua::mCurServer->GetTotalShare());
	return 1;
}

/// GetUpTime()
int GetUpTime(lua_State *L) {
	lua_pushnumber(L, (lua_Number)cLua::mCurServer->GetUpTime());
	return 1;
}

/// Disconnect(sNick/UID)
int Disconnect(lua_State *L) {
	CHECK_COUNT(1);
	int iType = lua_type(L, 1);
	if(iType == LUA_TLIGHTUSERDATA) {
		cDCConnBase * Conn = (cDCConnBase *)lua_touserdata(L, 1);
		if(!Conn)
			ERR("user was not found");
		Conn->Disconnect();
	} else if(iType == LUA_TSTRING) {
		size_t iLen;
		const char * sNick = lua_tolstring(L, 1, &iLen);
		NICKLEN(iLen);
		cDCUserBase * User = cLua::mCurServer->GetDCUserBase(sNick);
		if(!User || !User->mDCConnBase) ERR("user was not found");
		User->mDCConnBase->Disconnect();
	} else return luaL_typeerror(L, 1, "lightuserdata or string");

	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// DisconnectIP(sIP, iProfile/tProfiles)
int DisconnectIP(lua_State *L) {
	int iTop = lua_gettop(L);
	if(iTop < 1 || 2 < iTop) ERR_COUNT("1 or 2");

	size_t iLen;
	const char * sIP = luaL_checklstring(L, 1, &iLen);
	if(iLen < 7 || iLen > 15) {
		luaL_argerror(L, 1, "ip has wrong format");
		return 0;
	}

	unsigned long iProfile = 0;
	if(iTop == 2) {
		int iProf, iType = lua_type(L, 2);
		unsigned long iPrf;
		if(iType == LUA_TTABLE) {
			lua_pushnil(L);
			while(lua_next(L, 1) != 0) {
				if((iProf = luaL_checkint(L, -1) + 1) < 0) iProf = -iProf;
				iPrf = 1 << (iProf % 32);
				if(!(iProfile & iPrf)) iProfile = iProfile | iPrf;
				lua_pop(L, 1);
			}
			if(!iProfile) ERR("list turned out to be empty");
		} else if(iType == LUA_TNUMBER) {
			if((iProf = luaL_checkint(L, 1) + 1) < 0) iProf = -iProf;
			iProfile = 1 << (iProf % 32);
		} else if(iType != LUA_TNIL) return luaL_typeerror(L, 1, "number or table");
	}

	const vector<cDCConnBase *> & v = cLua::mCurServer->GetDCConnBase(sIP);
	vector<cDCConnBase *>::const_iterator it;
	if(!iProfile)
		for(it = v.begin(); it != v.end(); ++it)
			(*it)->Disconnect();
	else {
		int iPrf;
		for(it = v.begin(); it != v.end(); ++it) {
			iPrf = (*it)->GetProfile() + 1;
			if(iPrf < 0) iPrf = -iPrf;
			if(iPrf > 31) iPrf = (iPrf % 32) - 1;
			if(iProfile & (1 << iPrf))
				(*it)->Disconnect();
		}
	}
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// RestartScripts(iType)
int RestartScripts(lua_State *L) {
	int iType = 0;
	if(!lua_isnoneornil(L, 1)) {
		iType = luaL_checkint(L, 1);
		if(iType < 0 || iType > 2) iType = 0;
	}
	cLua::mCurLua->RestartScripts(cLua::mCurLua->mCurScript, iType);
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// RestartScript(sScriptName)
int RestartScript(lua_State *L) {
	int st;
	if(!lua_isnoneornil(L, 1)) {
		string sScriptName(luaL_checkstring(L, 1));
		CHECKSCRIPTNAME();
		cLuaInterpreter * Script = cLua::mCurLua->FindScript(sScriptName);
		if(Script)
			st = cLua::mCurLua->RestartScript(Script, Script->mL == L);
		else
			st = LUA_ERRFILE;
	} else
		st = cLua::mCurLua->RestartScript(cLua::mCurLua->mCurScript, true);

	if(st == 0) { lua_settop(L, 0); lua_pushboolean(L, 1); return 1; } // script was restarted
	if(st == LUA_ERRFILE) { lua_settop(L, 0); lua_pushnil(L); return 1; } // script was not found
	if(st == -1) ERR("script was started already");
	if(st == LUA_ERRMEM) ERR("memory error");
	if(st == LUA_ERRSYNTAX || st == LUA_YIELD || st == LUA_ERRRUN || st == LUA_ERRERR)
		ERR(cLua::mCurLua->msLastError.c_str());
	ERR("unknown error");
}

/// StopScript(sScriptName)
int StopScript(lua_State *L) {
	int st;
	cLuaInterpreter * Script = cLua::mCurLua->mCurScript;
	if(!lua_isnoneornil(L, 1)) {
		string sScriptName(luaL_checkstring(L, 1));
		CHECKSCRIPTNAME();
		Script = cLua::mCurLua->FindScript(sScriptName);
		if(Script)
			st = cLua::mCurLua->StopScript(Script, Script->mL == L);
		else
			st = LUA_ERRFILE;
	} else
		st = cLua::mCurLua->StopScript(Script, true);

	if(st == 0) { // script was stopped
		cLua::mCurLua->OnScriptAction(Script->msName.c_str(), "OnScriptStop");
		lua_settop(L, 0); lua_pushboolean(L, 1); return 1;
	}
	if(st == LUA_ERRFILE) { lua_settop(L, 0); lua_pushnil(L); return 1; } // script was not found
	if(st == -1) ERR("script was stoped already");
	ERR("unknown error");
}

/// StartScript(sScriptName)
int StartScript(lua_State *L) {
	string sScriptName(luaL_checkstring(L, 1));
	CHECKSCRIPTNAME();
	int st = cLua::mCurLua->StartScript(sScriptName);

	if(st == 0) { // script was started
		cLua::mCurLua->OnScriptAction(sScriptName.c_str(), "OnScriptStart");
		lua_settop(L, 0); lua_pushboolean(L, 1); return 1;
	}
	if(st == LUA_ERRFILE) { lua_settop(L, 0); lua_pushnil(L); return 1; } // script was not found
	if(st == -1) ERR("script was started already");
	if(st == LUA_ERRMEM) ERR("memory error");
	if(st == LUA_ERRSYNTAX || st == LUA_YIELD || st == LUA_ERRRUN || st == LUA_ERRERR)
		ERR(cLua::mCurLua->msLastError.c_str());
	ERR("unknown error");
}

/// GetScripts()
int GetScripts(lua_State *L) {
	lua_newtable(L);
	int i = 1, iTop = lua_gettop(L);
	cLua::tvLuaInterpreter::iterator it;
	for(it = cLua::mCurLua->mLua.begin(); it != cLua::mCurLua->mLua.end(); ++it) {
		lua_pushnumber(L, i);
		lua_newtable(L);
		lua_pushliteral(L, "sName");
		lua_pushstring(L, (*it)->msName.c_str());
		lua_rawset(L, iTop + 2);
		lua_pushliteral(L, "bEnabled");
		lua_pushboolean(L, ((*it)->mbEnabled == false) ? 0 : 1);
		lua_rawset(L, iTop + 2);
		lua_pushliteral(L, "iMemUsage");
		if((*it)->mbEnabled) lua_pushnumber(L, lua_gc((*it)->mL, LUA_GCCOUNT, 0));
		else lua_pushnumber(L, 0);
		lua_rawset(L, iTop + 2);
		lua_rawset(L, iTop);
		++i;
	}
	return 1;
}

/// GetScript(sScriptName)
int GetScript(lua_State *L) {
	int iTop = lua_gettop(L);
	if(iTop > 1) ERR_COUNT("0 or 1");
	cLuaInterpreter * Script;
	if(!lua_isnoneornil(L, 1)) {
		string sScriptName(luaL_checkstring(L, 1));
		CHECKSCRIPTNAME();
		Script = cLua::mCurLua->FindScript(sScriptName);
		if(!Script) ERR("script was not found");
	} else {
		Script = cLua::mCurLua->mCurScript;
	}
	lua_newtable(L);
	++iTop;
	lua_pushliteral(L, "sName");
	lua_pushstring(L, Script->msName.c_str());
	lua_rawset(L, iTop);
	lua_pushliteral(L, "bEnabled");
	lua_pushboolean(L, (Script->mbEnabled == false) ? 0 : 1);
	lua_rawset(L, iTop);
	lua_pushliteral(L, "iMemUsage");
	if(Script->mbEnabled) lua_pushnumber(L, lua_gc(Script->mL, LUA_GCCOUNT, 0));
	else lua_pushnumber(L, 0);
	lua_rawset(L, iTop);
	return 1;
}

/// MoveUpScript(sScriptName)
int MoveUpScript(lua_State *L) {
	if(!lua_isnoneornil(L, 1)) {
		string sScriptName(luaL_checkstring(L, 1));
		CHECKSCRIPTNAME();
		cLuaInterpreter * Script = cLua::mCurLua->FindScript(sScriptName);
		if(Script) {
			cLua::mCurLua->mTasksList.AddTask((void*)Script, eT_MoveUp);
		} else ERR("script was not found");
	} else
		cLua::mCurLua->mTasksList.AddTask((void*)cLua::mCurLua->mCurScript, eT_MoveUp);

	cLua::mCurLua->mTasksList.AddTask(NULL, eT_Save);
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// MoveDownScript(sScriptName)
int MoveDownScript(lua_State *L) {
	if(!lua_isnoneornil(L, 1)) {
		string sScriptName(luaL_checkstring(L, 1));
		CHECKSCRIPTNAME();
		cLuaInterpreter * Script = cLua::mCurLua->FindScript(sScriptName);
		if(Script) {
			cLua::mCurLua->mTasksList.AddTask((void*)Script, eT_MoveDown);
		} else ERR("script was not found");
	} else
		cLua::mCurLua->mTasksList.AddTask((void*)cLua::mCurLua->mCurScript, eT_MoveDown);

	cLua::mCurLua->mTasksList.AddTask(NULL, eT_Save);
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// SetCmd(sData)
int SetCmd(lua_State *L) {
	CHECK_COUNT(1);
	const char * sData = luaL_checkstring(L, 1);
	if(cLua::mCurLua->mCurDCParser) {
		int iType = cLua::mCurServer->CheckCmd(sData);
		if(iType != cLua::mCurLua->mCurDCParser->GetType() ||
			(
				iType == eDC_MYNIFO && 
				cLua::mCurLua->mCurDCConn && 
				cLua::mCurLua->mCurDCConn->mDCUserBase->GetNick() != cLua::mCurLua->mCurDCParser->ChunkString(eCH_MI_NICK)
			)
		) {
			luaL_argerror(L, 1, "wrong syntax");
			return 0;
		}
	}
	cLua::mCurLua->mCurDCParser->msStr = sData;
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// AddTimer(iId, iInterval, sFunc)
int AddTimer(lua_State *L) {
	int iTop = lua_gettop(L);
	if(iTop < 2 || iTop > 3) ERR_COUNT("2 or 3");
	string sFunc("OnTimer");
	int iId(luaL_checkint(L, 1)), iInterval(luaL_checkint(L, 2));
	if(iTop == 3)
		sFunc = (char*)luaL_checkstring(L, 3);

	lua_getglobal(L, sFunc.c_str());
	if(lua_isnil(L, lua_gettop(L)) || lua_type(L, -1) != LUA_TFUNCTION) ERR("timer function was not found");

	if(cLua::mCurLua->mCurScript->Size() > MAX_TIMERS)
		return luaL_error(L, "bad count timers for this script (max %d)", MAX_TIMERS);
	cTimer * timer = new cTimer(iId, iInterval, sFunc.c_str(), cLua::mCurLua->mCurScript);
	lua_settop(L, 0);
	lua_pushinteger(L, cLua::mCurLua->mCurScript->AddTmr(timer));
	return 1;
}

/// DelTimer(iId)
int DelTimer(lua_State *L) {
	CHECK_COUNT(1);
	int iNum = luaL_checkint(L, 1);
	lua_settop(L, 0);
	lua_pushinteger(L, cLua::mCurLua->mCurScript->DelTmr(iNum));
	return 1;
}

/// GetConfig(sName)
int GetConfig(lua_State *L) {
	CHECK_COUNT(1);
	const char * sConfig = cLua::mCurServer->GetConfig(luaL_checkstring(L, 1));
	if(!sConfig) ERR("config was not found");
	lua_settop(L, 0);
	lua_pushstring(L, sConfig);
	return 1;
}

/// SetConfig(sName, sValue)
int SetConfig(lua_State *L) {
	CHECK_COUNT(2);
	char * sVal = (char *)lua_tostring(L, 2);
	if(!sVal) sVal = (char *)lua_toboolean(L, 2);
	bool bRes = cLua::mCurServer->SetConfig(luaL_checkstring(L, 1), sVal);
	if(!bRes) ERR("config was not found");
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// GetLang(sName)
int GetLang(lua_State *L) {
	CHECK_COUNT(1);
	const char * sConfig = cLua::mCurServer->GetLang(luaL_checkstring(L, 1));
	if(!sConfig) ERR("config was not found");
	lua_settop(L, 0);
	lua_pushstring(L, sConfig);
	return 1;
}

/// SetLang(sName, sValue)
int SetLang(lua_State *L) {
	CHECK_COUNT(2);
	char * sVal = (char *)lua_tostring(L, 2);
	if(!sVal) sVal = (char *)lua_toboolean(L, 2);
	bool bRes = cLua::mCurServer->SetLang(luaL_checkstring(L, 1), sVal);
	if(!bRes) ERR("config was not found");
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}


/// Call(sScriptName, sFunc, ...)
int Call(lua_State *L) {
	int iTop = lua_gettop(L);
	if(iTop < 2) ERR_COUNT("more 1");
	string sScriptName(luaL_checkstring(L, 1));
	cLuaInterpreter * LIP = NULL;
	CHECKSCRIPTNAME();
	FINDINTERPRETER();

	lua_State *LL = lua_newthread(LIP->mL);
	lua_settop(LL, 0);

	int iBase = lua_gettop(LL);
	lua_pushliteral(LL, "_TRACEBACK");
	lua_rawget(LL, LUA_GLOBALSINDEX); // lua 5.1
	//lua_rawget(LL, LUA_ENVIRONINDEX); // lua 5.2
	lua_insert(LL, iBase);

	lua_getglobal(LL, luaL_checkstring(L, 2));
	if(lua_type(LL, -1) != LUA_TFUNCTION) {
		lua_remove(LL, iBase); // remove _TRACEBACK
		ERR("function was not found");
	}

	int iPos = 2;
	while(++iPos <= iTop)
		CopyValue(L, LL, iPos);

	if(lua_pcall(LL, iTop - 2, LUA_MULTRET, iBase)) {
		const char * sErrMsg = lua_tostring(LL, -1);
		lua_remove(LL, iBase); // remove _TRACEBACK
		return luaL_error(L, sErrMsg);
	}

	iPos = 0;
	iTop = lua_gettop(LL);
	lua_settop(L, 0);
	while(++iPos <= iTop)
		CopyValue(LL, L, iPos);

	lua_remove(LL, iBase); // remove _TRACEBACK
	return iTop;
}

/// RegBot(sNick, bKey, sMyINFO, sIP)
int RegBot(lua_State *L) {
	size_t iLen;
	int iType;
	string sNick, sMyINFO, sIP;
	bool bKey = true;

	switch(lua_gettop(L)) {
		case 4:
			iType = lua_type(L, 4);
			if(iType != LUA_TNIL)
				sIP = luaL_checklstring(L, 4, &iLen);
		case 3:
			iType = lua_type(L, 3);
			if(iType != LUA_TNIL)
				sMyINFO = luaL_checklstring(L, 3, &iLen);
		case 2:
			bKey = lua_toboolean(L, 2);
		case 1:
			iType = lua_type(L, 1);
			if(iType != LUA_TNIL) {
				sNick = luaL_checklstring(L, 1, &iLen);
				NICKLEN(iLen);
			}
			break;
		default:
			ERR_COUNT("1, 2, 3 or 4");
	}

	int iRes = cLua::mCurServer->RegBot(sNick, sMyINFO, sIP, bKey);
	if(iRes < 0) {
		if(iRes == -1) ERR("bad nick");
		if(iRes == -2) ERR("bad MyINFO");
		if(iRes == -3) ERR("bad nick (used)");
		ERR("unknown error");
	}

	cLua::mCurLua->mCurScript->mBotList.push_back(sNick);
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// UnregBot(sNick)
int UnregBot(lua_State *L) {
	CHECK_COUNT(1);
	size_t iLen;
	const char * sNick = luaL_checklstring(L, 1, &iLen);
	NICKLEN(iLen);
	if(cLua::mCurServer->UnregBot(sNick) == -1) ERR("bot was not found");
	cLua::mCurLua->mCurScript->mBotList.remove(sNick);
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// Redirect(UID/sNick, sAddress, [sReason])
int Redirect(lua_State *L) {
	size_t iLen;
	int iType;
	const char *sAddress = NULL, *sReason = NULL;

	switch(lua_gettop(L)) {
		case 3:
			iType = lua_type(L, 3);
			if(iType != LUA_TNIL) {
				sReason = luaL_checklstring(L, 3, &iLen);
				if(iLen > REDIRECT_REASON_MAX_LEN) { lua_settop(L, 0); lua_pushnil(L); lua_pushliteral(L, "very long reason."); return 2; }
			}
		case 2:
			sAddress = luaL_checklstring(L, 2, &iLen);
			if(iLen > REDIRECT_ADDRESS_MAX_LEN) { lua_settop(L, 0); lua_pushnil(L); lua_pushliteral(L, "very long address."); return 2; }
			break;
		default:
			ERR_COUNT("2 or 3");
	}

	iType = lua_type(L, 1);
	cDCConnBase * Conn = NULL;
	if(iType != LUA_TLIGHTUSERDATA) {
		if(iType != LUA_TSTRING) return luaL_typeerror(L, 1, "lightuserdata or string");
		cDCUserBase * User = cLua::mCurServer->GetDCUserBase(lua_tostring(L, 1));
		if(!User || !User->mDCConnBase) ERR("user was not found");
		Conn = User->mDCConnBase;
	} else {
		Conn = (cDCConnBase *)lua_touserdata(L, 1);
	}

	if(!Conn || (Conn->mType != eT_DC_CLIENT)) ERR("user was not found");

	cLua::mCurServer->ForceMove(Conn, sAddress, sReason);
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// SetHubState(iNumber) | iNumber = 0 or nil - stop, iNumber = 1 - restart
int SetHubState(lua_State *L) {
	int iTop = lua_gettop(L);
	if(iTop > 1) ERR_COUNT("0 or 1");

	if(iTop == 0 || lua_isnil(L, 1))
		cLua::mCurServer->StopHub(); // stoping
	else {
		int iNum(luaL_checkint(L, 1));
		if(iNum == eHS_STOP)
			cLua::mCurServer->StopHub(); // stoping
		else if(iNum == eHS_RESTART)
			cLua::mCurServer->RestartHub(); // restarting
	}
	return 0;
}

}; // nLua
