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

#include "api.h"
#include "LuaPlugin.h"
#include "Uid.h"
#include "Dir.h"

#include <fstream>
#include <vector>
#ifndef _WIN32
	#include <string.h> // strlen
#else
	#if defined(_MSC_VER) && (_MSC_VER >= 1400)
		#pragma warning(disable:4996) // Disable "This function or variable may be unsafe."
	#endif
#endif

using namespace ::std;
using namespace ::utils;

enum {
	eHS_STOP,
	eHS_RESTART
};

// lua 5.1
#define luaL_typeerror luaL_typerror

static int maxTimers = 100; // max count timers per script
static size_t redirectReasonMaxLen = 1024;
static size_t redirectAddressMaxLen = 128;

static size_t minMsgLen = 1;
static size_t maxMsgLen = 128000;
static size_t minNickLen = 1;
static size_t maxNickLen = 64;
static size_t minFileLen = 1;
static size_t maxFileLen = 255;

static int errCount(lua_State * L, const char * countStr) {
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

static bool checkCount(lua_State * L, int argNumber) {
	if (lua_gettop(L) != argNumber) {
		char sBuf[32] = { '\0' };
		sprintf(sBuf, "%d", argNumber);
		errCount(L, sBuf);
		return false;
	}
	return true;
}

static bool checkMsgLen(lua_State * L, size_t len) {
	if (len < minMsgLen || len > maxMsgLen) {
		lua_settop(L, 0);
		lua_pushnil(L);
		lua_pushliteral(L, "very long string");
		return false;
	}
	return true;
}

static bool checkNickLen(lua_State * L, size_t len) {
	if (len < minNickLen || len > maxNickLen) {
		lua_settop(L, 0);
		lua_pushnil(L);
		lua_pushliteral(L, "very long nick");
		return false;
	}
	return true;
}

static bool checkFileLen(lua_State * L, size_t len) {
	if (len < minFileLen || len > maxFileLen) {
		lua_settop(L, 0);
		lua_pushnil(L);
		lua_pushliteral(L, "very long file name");
		return false;
	}
	return true;
}

static int error(lua_State * L, const char * msg) {
	lua_settop(L, 0);
	lua_pushnil(L);
	lua_pushstring(L, msg);
	return 2;
}

static bool checkScriptName(lua_State * L, string & name) {
	size_t fileSize = name.size();
	if (!checkFileLen(L, fileSize)) {
		return false;
	}
	if (fileSize <= 4 || (0 != name.compare(fileSize - 4, 4, ".lua"))) {
		name.append(".lua");
	}
	return true;
}

static LuaInterpreter * findInterpreter(lua_State * L, const string & name) {
	LuaInterpreter * luaInterpreter = NULL;
	for (LuaPlugin::tvLuaInterpreter::iterator it = LuaPlugin::mCurLua->mLua.begin();
		it != LuaPlugin::mCurLua->mLua.end();
		++it
	) {
		if ((*it)->mName == name) {
			luaInterpreter = *it;
			break;
		}
	}
	if (!luaInterpreter || !luaInterpreter->mL) {
		error(L, "script was not found");
		return NULL;
	}
	return luaInterpreter;
}



namespace luaplugin {

DcConnBase * getDcConnBase(lua_State * L, int indx) {
	void ** userdata = (void **) lua_touserdata(L, indx);
	DcConnBase * dcConnBase = (DcConnBase *) *userdata;
	if (dcConnBase->mType != CLIENT_TYPE_NMDC && dcConnBase->mType != CLIENT_TYPE_WEB) {
		return NULL;
	}
	return dcConnBase;
}

int ConfigTostring(lua_State * L) {
	char buf[9] = { '\0' };
	sprintf(buf, "%p", lua_touserdata(L, 1));
	lua_pushfstring(L, "%s (%s)", lua_tostring(L, lua_upvalueindex(1)), buf);
	return 1;
}

int ConfigTable(lua_State * L) {
	static const vector<string> & vConf = LuaPlugin::mCurServer->getConfig();
	int indx = 1;
	lua_newtable(L);
	for (vector<string>::const_iterator it = vConf.begin(); it != vConf.end(); ++it) {
		lua_pushnumber(L, indx++);
		lua_pushstring(L, (*it).c_str());
		lua_rawset(L, -3);
	}
	return 1;
}

int ConfigIndex(lua_State * L) {
	Config * config = (Config *) lua_touserdata(L, 1);
	if (config->isExist != 1) {
		lua_settop(L, 0);
		lua_pushnil(L);
		return 1;
	}
	const char * sName = lua_tostring(L, 2);
	if (!sName) {
		ERR_TYPEMETA(2, "Config", "string");
	}
	if (!strcmp(sName, "table")) {
		lua_settop(L, 0);
		lua_pushcfunction(L, &ConfigTable);
		return 1;
	}
	const char * sConfig = LuaPlugin::mCurServer->getConfig(sName);
	lua_settop(L, 0);
	if (!sConfig) {
		lua_pushnil(L);
	} else {
		lua_pushstring(L, sConfig);
	}
	return 1;
}

int ConfigNewindex(lua_State * L) {
	Config * config = (Config *) lua_touserdata(L, 1);
	if (config->isExist != 1) {
		lua_settop(L, 0);
		return 0;
	}
	const char * sName = lua_tostring(L, 2);
	if (!sName) {
		ERR_TYPEMETA(2, "Config", "string");
	}
	char * sValue = (char *) lua_tostring(L, 3);
	if (!sValue) {
		sValue = (char *) lua_toboolean(L, 3);
	}
	if (!sValue) {
		ERR_TYPEMETA(3, "Config", "string or boolean");
	}
	LuaPlugin::mCurServer->setConfig(sName, sValue);
	LuaPlugin::mCurLua->OnConfigChange(sName, sValue);
	lua_settop(L, 0);
	return 0;
}

void LogError(const char * sMsg) {
	string sLogs(LuaPlugin::mCurServer->getMainDir() + "logs/");

	Dir::checkPath(sLogs);

	string logFile(sLogs + "lua_errors.log");
	ofstream Ofs(logFile.c_str(), ios_base::app);
	if (sMsg) {
		Ofs << "[" << LuaPlugin::mCurServer->getTime() << "] " << string(sMsg) << endl;
	} else {
		Ofs << "[" << LuaPlugin::mCurServer->getTime() << "] unknown LUA error" << endl;
	}
	Ofs.flush();
	Ofs.close();
}

static void SetTbl(lua_State * L1, lua_State * L2, int idx) {
	lua_newtable(L2);
	lua_pushnil(L1);
	int iTop = lua_gettop(L2);
	while (lua_next(L1, idx > 0 ? idx : idx - 1) != 0) {
		CopyValue(L1, L2, -2); // key
		CopyValue(L1, L2, -1); // value
		lua_rawset(L2, iTop);
		lua_pop(L1, 1);
	}
}

void CopyValue(lua_State * From, lua_State * To, int idx) {
	void ** userdata;
	void * udata;
	switch (lua_type(From, idx)) {
		case LUA_TSTRING :
			lua_pushstring(To, lua_tostring(From, idx));
			break;

		case LUA_TNUMBER :
			lua_pushnumber(To, lua_tonumber(From, idx));
			break;

		case LUA_TBOOLEAN :
			lua_pushboolean(To, lua_toboolean(From, idx));
			break;

		case LUA_TTABLE :
			SetTbl(From, To, idx);
			break;

		case LUA_TLIGHTUSERDATA :
			lua_pushlightuserdata(To, lua_touserdata(From, idx));
			break;

		case LUA_TUSERDATA :
			udata = (void *) getDcConnBase(From, idx);
			if (udata) {
				userdata = (void**) lua_newuserdata(To, sizeof(void *));
				*userdata = udata;
				luaL_getmetatable(To, MT_USER_CONN);
				lua_setmetatable(To, -2);
			} else {
				lua_pushnil(To);
			}
			break;

		default :
			lua_pushnil(To);
			break;

	}
}

/// GetGVal(sScriptName, sParam)
int GetGVal(lua_State * L) {
	if (!checkCount(L, 2)) {
		return 0;
	}
	size_t iLen;
	string sScriptName(luaL_checklstring(L, 1, &iLen));
	if (!checkScriptName(L, sScriptName)) {
		return 2;
	}
	LuaInterpreter * LIP = findInterpreter(L, sScriptName);
	if (LIP == NULL) {
		return 2;
	}

	lua_State * LL = lua_newthread(LIP->mL);
	lua_getglobal(LL, luaL_checkstring(L, 2));

	//lua_settop(LIP->mL, 0);
	lua_pop(LIP->mL, 1);
	CopyValue(LL, L, -1);

	return 1;
}

/// SetGVal(sScriptName, sParam, Value)
int SetGVal(lua_State * L) {
	if (!checkCount(L, 3)) {
		return 0;
	}
	size_t iLen;
	string sScriptName(luaL_checklstring(L, 1, &iLen));
	if (!checkScriptName(L, sScriptName)) {
		return 2;
	}
	LuaInterpreter * LIP = findInterpreter(L, sScriptName);
	if (LIP == NULL) {
		return 2;
	}

	lua_State * LL = lua_newthread(LIP->mL);
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

/// SendToUser(UID/sToNick/tNicks, sData, sNick, sFrom)
int sendToUser(lua_State * L) {
	size_t iLen;
	int iType;
	const char *sNick = NULL, *sFrom = NULL, *sN;

	switch (lua_gettop(L)) {
		case 4 :
			iType = lua_type(L, 4);
			if (iType != LUA_TNIL) {
				sFrom = luaL_checklstring(L, 4, &iLen);
				if (!checkNickLen(L, iLen)) {
					return 2;
				}
			}

		case 3 :
			iType = lua_type(L, 3);
			if (iType != LUA_TNIL) {
				sNick = luaL_checklstring(L, 3, &iLen);
				if (!checkNickLen(L, iLen)) {
					return 2;
				}
			}

		case 2 :
			iType = lua_type(L, 1);
			break;

		default :
			return errCount(L, "2, 3 or 4");

	}

	const char * sData = luaL_checklstring(L, 2, &iLen);
	if (!checkMsgLen(L, iLen)) {
		return 2;
	}

	if (iType == LUA_TUSERDATA) {
		DcConnBase * dcConnBase = getDcConnBase(L, 1);
		if (!dcConnBase) {
			return error(L, "user was not found");
		}
		if (dcConnBase->mType == CLIENT_TYPE_WEB) {
			if (!dcConnBase->send(sData)) { // not newPolitic fot timers only
				return error(L, "data was not sent");
			}
		} else if (!LuaPlugin::mCurServer->sendToUser(dcConnBase, sData, sNick, sFrom)) {
			return error(L, "user was not found");
		}
	} else if (iType == LUA_TSTRING) {
		const char * sTo = lua_tolstring(L, 1, &iLen);
		if (!checkNickLen(L, iLen)) {
			return 2;
		}
		if (!LuaPlugin::mCurServer->sendToNick(sTo, sData, sNick, sFrom)) {
			return error(L, "user was not found");
		}
	} else if (iType == LUA_TTABLE) {
		lua_pushnil(L);
		while (lua_next(L, 1) != 0) {
			sN = luaL_checklstring(L, -1, &iLen);
			if (!checkNickLen(L, iLen)) {
				return 2;
			}
			LuaPlugin::mCurServer->sendToNick(sN, sData, sNick, sFrom);
			lua_pop(L, 1);
		}
	} else {
		return luaL_typeerror(L, 1, "userdata, string or table");
	}
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

//! deprecated
/// SendToNicks(tNicks, sData, sNick, sFrom)
int SendToNicks(lua_State * L) {
	size_t iLen;
	const char *sData, *sNick = NULL, *sFrom = NULL, *sN;
	switch (lua_gettop(L)) {
		case 4 :
			if (lua_type(L, 4) != LUA_TNIL) {
				sFrom = luaL_checklstring(L, 4, &iLen);
				if (!checkNickLen(L, iLen)) {
					return 2;
				}
			}

		case 3 :
			if (lua_type(L, 3) != LUA_TNIL) {
				sNick = luaL_checklstring(L, 3, &iLen);
				if (!checkNickLen(L, iLen)) {
					return 2;
				}
			}

		case 2 :
			luaL_checktype(L, 1, LUA_TTABLE);
			sData = luaL_checklstring(L, 2, &iLen);
			if (!checkMsgLen(L, iLen)) {
				return 2;
			}

			lua_pushnil(L);
			while (lua_next(L, 1) != 0) {
				sN = luaL_checklstring(L, -1, &iLen);
				if (!checkNickLen(L, iLen)) {
					return 2;
				}
				LuaPlugin::mCurServer->sendToNick(sN, sData, sNick, sFrom);
				lua_pop(L, 1);
			}
			break;

		default :
			return errCount(L, "2, 3 or 4");

	}
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}


/// SendToAll(sData, sNick, sFrom)
int sendToAll(lua_State * L) {
	size_t iLen;
	const char *sData, *sNick = NULL, *sFrom = NULL;
	switch (lua_gettop(L)) {
		case 3 :
			if (lua_type(L, 3) != LUA_TNIL) {
				sFrom = luaL_checklstring(L, 3, &iLen);
				if (!checkNickLen(L, iLen)) {
					return 2;
				}
			}

		case 2 :
			if (lua_type(L, 2) != LUA_TNIL) {
				sNick = luaL_checklstring(L, 2, &iLen);
				if (!checkNickLen(L, iLen)) {
					return 2;
				}
			}

		case 1 :
			sData = luaL_checklstring(L, 1, &iLen);
			if (!checkMsgLen(L, iLen)) {
				return 2;
			}
			break;

		default :
			return errCount(L, "1, 2 or 3");

	}
	if (!LuaPlugin::mCurServer->sendToAll(sData, sNick, sFrom)) {
		return error(L, "data was not found");
	}
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// SendToProfile(iProfile/tProfiles, sData, sNick, sFrom)
int SendToProfile(lua_State * L) {
	size_t iLen;
	unsigned long iProfile = 0, iPrf;
	int iType, iProf;
	const char *sData, *sNick = NULL, *sFrom = NULL;
	switch (lua_gettop(L)) {
		case 4 :
			if (lua_type(L, 4) != LUA_TNIL) {
				sFrom = luaL_checklstring(L, 4, &iLen);
				if (!checkNickLen(L, iLen)) {
					return 2;
				}
			}

		case 3 :
			if (lua_type(L, 3) != LUA_TNIL) {
				sNick = luaL_checklstring(L, 3, &iLen);
				if (!checkNickLen(L, iLen)) {
					return 2;
				}
			}

		case 2 :
			sData = luaL_checklstring(L, 2, &iLen);
			if (!checkMsgLen(L, iLen)) {
				return 2;
			}
			iType = lua_type(L, 1);
			if (iType == LUA_TTABLE) {
				lua_pushnil(L);
				while (lua_next(L, 1) != 0) {
					if ((iProf = luaL_checkint(L, -1) + 1) < 0) {
						iProf = -iProf;
					}
					iPrf = 1 << (iProf % 32);
					if (!(iProfile & iPrf)) {
						iProfile = iProfile | iPrf;
					}
					lua_pop(L, 1);
				}
				if (!iProfile) {
					return error(L, "list turned out to be empty");
				}
			} else if (iType == LUA_TNUMBER) {
				if ((iProf = luaL_checkint(L, 1) + 1) < 0) {
					iProf = -iProf;
				}
				iProfile = 1 << (iProf % 32);
			} else {
				return luaL_typeerror(L, 1, "number or table");
			}
			break;

		default :
			return errCount(L, "2, 3 or 4");

	}
	if (!LuaPlugin::mCurServer->sendToProfiles(iProfile, sData, sNick, sFrom)) {
		return error(L, "data was not found");
	}
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// SendToIP(sIP, sData, sNick, sFrom, iProfile/tProfiles)
int sendToIp(lua_State * L) {
	size_t iLen;
	unsigned long iProfile = 0, iPrf;
	int iType, iProf;
	const char *sIP, *sData, *sNick = NULL, *sFrom = NULL;

	switch (lua_gettop(L)) {
		case 5 :
			iType = lua_type(L, 1);
			if (iType == LUA_TTABLE) {
				lua_pushnil(L);
				while (lua_next(L, 1) != 0) {
					if ((iProf = luaL_checkint(L, -1) + 1) < 0) {
						iProf = -iProf;
					}
					iPrf = 1 << (iProf % 32);
					if (!(iProfile & iPrf)) {
						iProfile = iProfile | iPrf;
					}
					lua_pop(L, 1);
				}
				if (!iProfile) {
					return error(L, "list turned out to be empty");
				}
			} else if (iType == LUA_TNUMBER) {
				if ((iProf = luaL_checkint(L, 1) + 1) < 0) {
					iProf = -iProf;
				}
				iProfile = 1 << (iProf % 32);
			} else if (iType != LUA_TNIL) {
				return luaL_typeerror(L, 1, "number or table");
			}

		case 4 :
			if (lua_type(L, 4) != LUA_TNIL) {
				sFrom = luaL_checklstring(L, 4, &iLen);
				if (!checkNickLen(L, iLen)) {
					return 2;
				}
			}

		case 3 :
			if (lua_type(L, 3) != LUA_TNIL) {
				sNick = luaL_checklstring(L, 3, &iLen);
				if (!checkNickLen(L, iLen)) {
					return 2;
				}
			}

		case 2 :
			sData = luaL_checklstring(L, 2, &iLen);
			if (!checkMsgLen(L, iLen)) {
				return 2;
			}
			break;

		default :
			return errCount(L, "2, 3, 4 or 5");

	}
	sIP = luaL_checklstring(L, 1, &iLen);
	if (sIP && !LuaPlugin::mCurServer->sendToIp(sIP, sData, iProfile, sNick, sFrom)) {
		return error(L, "wrong ip format");
	}
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// SendToAllExceptNicks(tExcept, sData, sNick, sFrom)
int sendToAllExceptNicks(lua_State * L) {
	size_t iLen;
	vector<string> NickList;
	const char *sData, *sNick = NULL, *sFrom = NULL, *sN;
	switch (lua_gettop(L)) {
		case 4 :
			if (lua_type(L, 4) != LUA_TNIL) {
				sFrom = luaL_checklstring(L, 4, &iLen);
				if (!checkNickLen(L, iLen)) {
					return 2;
				}
			}

		case 3 :
			if (lua_type(L, 3) != LUA_TNIL) {
				sNick = luaL_checklstring(L, 3, &iLen);
				if (!checkNickLen(L, iLen)) {
					return 2;
				}
			}

		case 2 :
			luaL_checktype(L, 1, LUA_TTABLE);
			sData = luaL_checklstring(L, 2, &iLen);
			if (!checkMsgLen(L, iLen)) {
				return 2;
			}
			lua_pushnil(L);
			while (lua_next(L, 1) != 0) {
				sN = luaL_checklstring(L, -1, &iLen);
				if (!checkNickLen(L, iLen)) {
					return 2;
				}
				NickList.push_back(sN);
				lua_pop(L, 1);
			}
			if (!NickList.size()) {
				return error(L, "list turned out to be empty");
			}
			break;

		default :
			return errCount(L, "2, 3 or 4");

	}
	if (!LuaPlugin::mCurServer->sendToAllExceptNicks(NickList, sData, sNick, sFrom)) {
		return error(L, "data was not found");
	}
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// SendToAllExceptIPs(tExcept, sData, sNick, sFrom)
int sendToAllExceptIps(lua_State * L) {
	size_t iLen;
	vector<string> IPList;
	const char *sData, *sNick = NULL, *sFrom = NULL, *sN;
	switch (lua_gettop(L)) {
		case 4 :
			if (lua_type(L, 4) != LUA_TNIL) {
				sFrom = luaL_checklstring(L, 4, &iLen);
				if (!checkNickLen(L, iLen)) {
					return 2;
				}
			}

		case 3 :
			if (lua_type(L, 3) != LUA_TNIL) {
				sNick = luaL_checklstring(L, 3, &iLen);
				if (!checkNickLen(L, iLen)) {
					return 2;
				}
			}

		case 2 :
			luaL_checktype(L, 1, LUA_TTABLE);
			sData = luaL_checklstring(L, 2, &iLen);
			if (!checkMsgLen(L, iLen)) {
				return 2;
			}
			lua_pushnil(L);
			while (lua_next(L, 1) != 0) {
				sN = luaL_checklstring(L, -1, &iLen);
				if (!checkNickLen(L, iLen)) {
					return 2;
				}
				IPList.push_back(sN);
				lua_pop(L, 1);
			}
			if (!IPList.size()) {
				return error(L, "list turned out to be empty");
			}
			break;

		default :
			return errCount(L, "2, 3 or 4");

	}
	if (!LuaPlugin::mCurServer->sendToAllExceptIps(IPList, sData, sNick, sFrom)) {
		return error(L, "wrong ip format");
	}
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// GetUser(UID/sNick, iByte)
int GetUser(lua_State * L) {
	if (lua_type(L, 1) == LUA_TUSERDATA) {
		lua_pushvalue(L, 1);
		luaL_getmetatable(L, MT_USER_CONN);
		lua_setmetatable(L, -2);
	} else if (lua_isstring(L, 1)) {
		size_t iLen;
		const char * sNick = lua_tolstring(L, 1, &iLen);
		if (!checkNickLen(L, iLen)) {
			return 2;
		}
		DcUserBase * User = LuaPlugin::mCurServer->getDcUserBase(sNick);
		if (!User || !User->mDcConnBase) {
			return error(L, "user was not found");
		}
		void ** userdata = (void **) lua_newuserdata(L, sizeof(void *));
		*userdata = (void *)User->mDcConnBase;
		luaL_getmetatable(L, MT_USER_CONN);
		lua_setmetatable(L, -2);
	} else {
		return luaL_typeerror(L, 1, "userdata or string");
	}
	return 1;
}

//! deprecated
/// SetUser(UID/sNick, iType, Value)
int SetUser(lua_State * L) {
	if (!checkCount(L, 3)) {
		return 0;
	}
	DcConnBase * dcConnBase = NULL;
	if (lua_type(L, 1) == LUA_TUSERDATA) {
		dcConnBase = getDcConnBase(L, 1);
		if (!dcConnBase || dcConnBase->mType != CLIENT_TYPE_NMDC) {
			return error(L, "user was not found");
		}
	} else if (lua_isstring(L, 1)) {
		size_t iLen;
		const char * sNick = lua_tolstring(L, 1, &iLen);
		if (!checkNickLen(L, iLen)) {
			return 2;
		}
		DcUserBase * User = LuaPlugin::mCurServer->getDcUserBase(sNick);
		if (!User || !User->mDcConnBase) {
			return error(L, "user was not found");
		}
		dcConnBase = User->mDcConnBase;
	} else {
		return luaL_typeerror(L, 1, "userdata or string");
	}

	unsigned iNum = (unsigned)luaL_checkinteger(L, 2);
	if (iNum == eUV_iProfile) {
		dcConnBase->setProfile(luaL_checkint(L, 3));
	} else if (iNum == eUV_sMyINFO) {
		if (!dcConnBase->mDcUserBase) {
			return error(L, "user was not found");
		}
		string sMyINFO(luaL_checkstring(L, 3));
		if (!dcConnBase->mDcUserBase->setMyINFO(sMyINFO)) {
			return error(L, "wrong syntax");
		}
	} else if (iNum == eUV_sData) {
		dcConnBase->setData(luaL_checkstring(L, 3));
	} else if (iNum == eUV_bOpList) {
		luaL_checktype(L, 3, LUA_TBOOLEAN);
		if (!dcConnBase->mDcUserBase) {
			return error(L, "user was not found");
		}
		dcConnBase->mDcUserBase->setInOpList(lua_toboolean(L, 3) != 0);
	} else if (iNum == eUV_bHide) {
		luaL_checktype(L, 3, LUA_TBOOLEAN);
		if (!dcConnBase->mDcUserBase) {
			return error(L, "user was not found");
		}
		dcConnBase->mDcUserBase->setHide(lua_toboolean(L, 3) != 0);
	} else if (iNum == eUV_bIpList) {
		luaL_checktype(L, 3, LUA_TBOOLEAN);
		if (!dcConnBase->mDcUserBase) {
			return error(L, "user was not found");
		}
		dcConnBase->mDcUserBase->setInIpList(lua_toboolean(L, 3) != 0);
	}
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// GetUsers(sIP, iByte)
int GetUsers(lua_State * L) {
	lua_newtable(L);
	int iTopTab = lua_gettop(L), i = 1;
	if (lua_type(L, 1) == LUA_TSTRING) {
		bool all = false;
		if (lua_isboolean(L, 2) && lua_toboolean(L, 2) == 1) {
			all = true;
		}
		const vector<DcConnBase *> & v = LuaPlugin::mCurServer->getDcConnBase(lua_tostring(L, 1));
		for (vector<DcConnBase *>::const_iterator it = v.begin(); it != v.end(); ++it) {
			if (all || ((*it)->mDcUserBase && (*it)->mDcUserBase->getInUserList())) {
				lua_pushnumber(L, i);
				void ** userdata = (void **) lua_newuserdata(L, sizeof(void *));
				*userdata = (void *) (*it);
				luaL_getmetatable(L, MT_USER_CONN);
				lua_setmetatable(L, -2);
				lua_rawset(L, iTopTab);
				++i;
			}
		}
	} else {
		bool all = false;
		if (lua_isboolean(L, 1) && lua_toboolean(L, 1) == 1) {
			all = true;
		}
		DcConnListIterator * it = LuaPlugin::mCurServer->getDcConnListIterator();
		DcConnBase * dcConnBase = NULL;
		while ((dcConnBase = it->operator ()()) != NULL) {
			if (dcConnBase->mType != CLIENT_TYPE_NMDC) {
				continue;
			}
			if (all || (dcConnBase->mDcUserBase && dcConnBase->mDcUserBase->getInUserList())) {
				lua_pushnumber(L, i);
				void ** userdata = (void **) lua_newuserdata(L, sizeof(void *));
				*userdata = (void *) dcConnBase;
				luaL_getmetatable(L, MT_USER_CONN);
				lua_setmetatable(L, -2);
				lua_rawset(L, iTopTab);
				++i;
			}
		}
		delete it;
	}
	return 1;
}

/// GetUsersCount()
int getUsersCount(lua_State * L) {
	lua_pushnumber(L, LuaPlugin::mCurServer->getUsersCount());
	return 1;
}

/// GetTotalShare()
int getTotalShare(lua_State * L) {
	lua_pushnumber(L, (lua_Number)LuaPlugin::mCurServer->getTotalShare());
	return 1;
}

/// GetUpTime()
int getUpTime(lua_State * L) {
	lua_pushnumber(L, (lua_Number)LuaPlugin::mCurServer->getUpTime());
	return 1;
}

/// Disconnect(sNick/UID)
int disconnect(lua_State * L) {
	if (!checkCount(L, 1)) {
		return 0;
	}
	int iType = lua_type(L, 1);
	if (iType == LUA_TUSERDATA) {
		DcConnBase * dcConnBase = getDcConnBase(L, 1);
		if (!dcConnBase) {
			return error(L, "user was not found");
		}
		dcConnBase->disconnect();
	} else if (iType == LUA_TSTRING) {
		size_t iLen;
		const char * sNick = lua_tolstring(L, 1, &iLen);
		if (!checkNickLen(L, iLen)) {
			return 2;
		}
		DcUserBase * User = LuaPlugin::mCurServer->getDcUserBase(sNick);
		if (!User || !User->mDcConnBase) {
			return error(L, "user was not found");
		}
		User->mDcConnBase->disconnect();
	} else {
		return luaL_typeerror(L, 1, "userdata or string");
	}
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// DisconnectIP(sIP, iProfile/tProfiles)
int DisconnectIP(lua_State * L) {
	int iTop = lua_gettop(L);
	if (iTop < 1 || 2 < iTop) {
		return errCount(L, "1 or 2");
	}

	size_t iLen;
	const char * sIP = luaL_checklstring(L, 1, &iLen);
	if (iLen < 7 || iLen > 15) {
		luaL_argerror(L, 1, "ip has wrong format");
		return 0;
	}

	unsigned long iProfile = 0;
	if (iTop == 2) {
		int iProf, iType = lua_type(L, 2);
		unsigned long iPrf;
		if (iType == LUA_TTABLE) {
			lua_pushnil(L);
			while (lua_next(L, 1) != 0) {
				if ((iProf = luaL_checkint(L, -1) + 1) < 0) {
					iProf = -iProf;
				}
				iPrf = 1 << (iProf % 32);
				if (!(iProfile & iPrf)) {
					iProfile = iProfile | iPrf;
				}
				lua_pop(L, 1);
			}
			if (!iProfile) {
				return error(L, "list turned out to be empty");
			}
		} else if (iType == LUA_TNUMBER) {
			if ((iProf = luaL_checkint(L, 1) + 1) < 0) {
				iProf = -iProf;
			}
			iProfile = 1 << (iProf % 32);
		} else if (iType != LUA_TNIL) {
			return luaL_typeerror(L, 1, "number or table");
		}
	}

	const vector<DcConnBase *> & v = LuaPlugin::mCurServer->getDcConnBase(sIP);
	vector<DcConnBase *>::const_iterator it;
	if (!iProfile) {
		for (it = v.begin(); it != v.end(); ++it) {
			(*it)->disconnect();
		}
	} else {
		int iPrf;
		for (it = v.begin(); it != v.end(); ++it) {
			iPrf = (*it)->getProfile() + 1;
			if (iPrf < 0) {
				iPrf = -iPrf;
			}
			if (iPrf > 31) {
				iPrf = (iPrf % 32) - 1;
			}
			if (iProfile & (1 << iPrf)) {
				(*it)->disconnect();
			}
		}
	}
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// RestartScripts(iType)
int RestartScripts(lua_State *L) {
	int iType = 0;
	if (!lua_isnoneornil(L, 1)) {
		iType = luaL_checkint(L, 1);
		if (iType < 0 || iType > 2) {
			iType = 0;
		}
	}
	LuaPlugin::mCurLua->RestartScripts(LuaPlugin::mCurLua->mCurScript, iType);
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// RestartScript(sScriptName)
int RestartScript(lua_State *L) {
	int st;
	if (!lua_isnoneornil(L, 1)) {
		string sScriptName(luaL_checkstring(L, 1));
		if (!checkScriptName(L, sScriptName)) {
			return 2;
		}
		LuaInterpreter * Script = LuaPlugin::mCurLua->FindScript(sScriptName);
		if (Script) {
			st = LuaPlugin::mCurLua->RestartScript(Script, Script->mL == L);
		} else {
			st = LUA_ERRFILE;
		}
	} else {
		st = LuaPlugin::mCurLua->RestartScript(LuaPlugin::mCurLua->mCurScript, true);
	}

	if (st == 0) { // script was restarted
		lua_settop(L, 0);
		lua_pushboolean(L, 1);
		return 1;
	} else if (st == LUA_ERRFILE) { // script was not found
		lua_settop(L, 0);
		lua_pushnil(L);
		return 1;
	} else if (st == -1) {
		return error(L, "script was started already");
	} else if (st == LUA_ERRMEM) {
		return error(L, "memory error");
	} else if (st == LUA_ERRSYNTAX || st == LUA_YIELD || st == LUA_ERRRUN || st == LUA_ERRERR) {
		return error(L, LuaPlugin::mCurLua->msLastError.c_str());
	}
	return error(L, "unknown error");
}

/// StopScript(sScriptName)
int StopScript(lua_State * L) {
	int st;
	LuaInterpreter * Script = LuaPlugin::mCurLua->mCurScript;
	if (!lua_isnoneornil(L, 1)) {
		string sScriptName(luaL_checkstring(L, 1));
		if (!checkScriptName(L, sScriptName)) {
			return 2;
		}
		Script = LuaPlugin::mCurLua->FindScript(sScriptName);
		if (Script) {
			st = LuaPlugin::mCurLua->StopScript(Script, Script->mL == L);
		} else {
			st = LUA_ERRFILE;
		}
	} else {
		st = LuaPlugin::mCurLua->StopScript(Script, true);
	}

	if (st == 0) { // script was stopped
		LuaPlugin::mCurLua->OnScriptAction(Script->mName.c_str(), "OnScriptStop");
		lua_settop(L, 0);
		lua_pushboolean(L, 1);
		return 1;
	} else if (st == LUA_ERRFILE) { // script was not found
		lua_settop(L, 0);
		lua_pushnil(L);
		return 1;
	} else if (st == -1) {
		return error(L, "script was stoped already");
	}
	return error(L, "unknown error");
}

/// StartScript(sScriptName)
int StartScript(lua_State * L) {
	string sScriptName(luaL_checkstring(L, 1));
	if (!checkScriptName(L, sScriptName)) {
		return 2;
	}
	int st = LuaPlugin::mCurLua->StartScript(sScriptName);

	if (st == 0) { // script was started
		LuaPlugin::mCurLua->OnScriptAction(sScriptName.c_str(), "OnScriptStart");
		lua_settop(L, 0);
		lua_pushboolean(L, 1);
		return 1;
	} else if (st == LUA_ERRFILE) { // script was not found
		lua_settop(L, 0);
		lua_pushnil(L);
		return 1;
	} else if (st == -1) {
		return error(L, "script was started already");
	} else if (st == LUA_ERRMEM) {
		return error(L, "memory error");
	} else if (st == LUA_ERRSYNTAX || st == LUA_YIELD || st == LUA_ERRRUN || st == LUA_ERRERR) {
		return error(L, LuaPlugin::mCurLua->msLastError.c_str());
	}
	return error(L, "unknown error");
}

/// GetScripts()
int GetScripts(lua_State * L) {
	lua_newtable(L);
	int i = 1, iTop = lua_gettop(L);
	LuaPlugin::tvLuaInterpreter::iterator it;
	for (it = LuaPlugin::mCurLua->mLua.begin(); it != LuaPlugin::mCurLua->mLua.end(); ++it) {
		lua_pushnumber(L, i);
		lua_newtable(L);
		lua_pushliteral(L, "sName");
		lua_pushstring(L, (*it)->mName.c_str());
		lua_rawset(L, iTop + 2);
		lua_pushliteral(L, "bEnabled");
		lua_pushboolean(L, ((*it)->mbEnabled == false) ? 0 : 1);
		lua_rawset(L, iTop + 2);
		lua_pushliteral(L, "iMemUsage");
		if ((*it)->mbEnabled) {
			lua_pushnumber(L, lua_gc((*it)->mL, LUA_GCCOUNT, 0));
		} else {
			lua_pushnumber(L, 0);
		}
		lua_rawset(L, iTop + 2);
		lua_rawset(L, iTop);
		++i;
	}
	return 1;
}

/// GetScript(sScriptName)
int GetScript(lua_State * L) {
	int iTop = lua_gettop(L);
	if (iTop > 1) {
		return errCount(L, "0 or 1");
	}
	LuaInterpreter * Script;
	if (!lua_isnoneornil(L, 1)) {
		string sScriptName(luaL_checkstring(L, 1));
		if (!checkScriptName(L, sScriptName)) {
			return 2;
		}
		Script = LuaPlugin::mCurLua->FindScript(sScriptName);
		if (!Script) {
			return error(L, "script was not found");
		}
	} else {
		Script = LuaPlugin::mCurLua->mCurScript;
	}
	lua_newtable(L);
	++iTop;
	lua_pushliteral(L, "sName");
	lua_pushstring(L, Script->mName.c_str());
	lua_rawset(L, iTop);
	lua_pushliteral(L, "bEnabled");
	lua_pushboolean(L, (Script->mbEnabled == false) ? 0 : 1);
	lua_rawset(L, iTop);
	lua_pushliteral(L, "iMemUsage");
	if (Script->mbEnabled) {
		lua_pushnumber(L, lua_gc(Script->mL, LUA_GCCOUNT, 0));
	} else {
		lua_pushnumber(L, 0);
	}
	lua_rawset(L, iTop);
	return 1;
}

/// MoveUpScript(sScriptName)
int MoveUpScript(lua_State *L) {
	if (!lua_isnoneornil(L, 1)) {
		string sScriptName(luaL_checkstring(L, 1));
		if (!checkScriptName(L, sScriptName)) {
			return 2;
		}
		LuaInterpreter * Script = LuaPlugin::mCurLua->FindScript(sScriptName);
		if (Script) {
			LuaPlugin::mCurLua->mTasksList.AddTask((void *)Script, eT_MoveUp);
		} else {
			return error(L, "script was not found");
		}
	} else {
		LuaPlugin::mCurLua->mTasksList.AddTask((void *)LuaPlugin::mCurLua->mCurScript, eT_MoveUp);
	}

	LuaPlugin::mCurLua->mTasksList.AddTask(NULL, eT_Save);
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// MoveDownScript(sScriptName)
int MoveDownScript(lua_State * L) {
	if (!lua_isnoneornil(L, 1)) {
		string sScriptName(luaL_checkstring(L, 1));
		if (!checkScriptName(L, sScriptName)) {
			return 2;
		}
		LuaInterpreter * Script = LuaPlugin::mCurLua->FindScript(sScriptName);
		if (Script) {
			LuaPlugin::mCurLua->mTasksList.AddTask((void*)Script, eT_MoveDown);
		} else {
			return error(L, "script was not found");
		}
	} else {
		LuaPlugin::mCurLua->mTasksList.AddTask((void*)LuaPlugin::mCurLua->mCurScript, eT_MoveDown);
	}

	LuaPlugin::mCurLua->mTasksList.AddTask(NULL, eT_Save);
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// SetCmd(sData)
int SetCmd(lua_State * L) {
	if (!checkCount(L, 1)) {
		return 0;
	}
	const char * sData = luaL_checkstring(L, 1);
	if (!LuaPlugin::mCurLua->mCurDCConn) {
		luaL_argerror(L, 1, "internal fatal error");
		return 0;
	}
	if (!LuaPlugin::mCurLua->mCurDCConn->parseCommand(sData)) {
		luaL_argerror(L, 1, "wrong syntax");
		return 0;
	}
	lua_settop(L, 0);
	lua_pushboolean(L, 0);
	return 1;
}

/// AddTimer(iId, iInterval, sFunc)
int AddTimer(lua_State * L) {
	int iTop = lua_gettop(L);
	if (iTop < 2 || iTop > 3) {
		return errCount(L, "2 or 3");
	}
	string sFunc("OnTimer");
	int iId(luaL_checkint(L, 1)), iInterval(luaL_checkint(L, 2));
	if (iTop == 3) {
		sFunc = (char *)luaL_checkstring(L, 3);
	}

	lua_getglobal(L, sFunc.c_str());
	if (lua_isnil(L, lua_gettop(L)) || lua_type(L, -1) != LUA_TFUNCTION) {
		return error(L, "timer function was not found");
	}

	if (LuaPlugin::mCurLua->mCurScript->Size() > maxTimers) {
		return luaL_error(L, "bad count timers for this script (max %d)", maxTimers);
	}
	cTimer * timer = new cTimer(iId, iInterval, sFunc.c_str(), LuaPlugin::mCurLua->mCurScript);
	lua_settop(L, 0);
	lua_pushinteger(L, LuaPlugin::mCurLua->mCurScript->AddTmr(timer));
	return 1;
}

/// DelTimer(iId)
int DelTimer(lua_State * L) {
	if (!checkCount(L, 1)) {
		return 0;
	}
	int iNum = luaL_checkint(L, 1);
	lua_settop(L, 0);
	lua_pushinteger(L, LuaPlugin::mCurLua->mCurScript->DelTmr(iNum));
	return 1;
}

//! deprecated
/// GetConfig(sName)
int getConfig(lua_State * L) {
	if (!checkCount(L, 1)) {
		return 0;
	}
	const char * sConfig = LuaPlugin::mCurServer->getConfig(luaL_checkstring(L, 1));
	if (!sConfig) {
		return error(L, "config was not found");
	}
	lua_settop(L, 0);
	lua_pushstring(L, sConfig);
	return 1;
}

//! deprecated
/// SetConfig(sName, sValue)
int setConfig(lua_State * L) {
	if (!checkCount(L, 2)) {
		return 0;
	}
	char * sVal = (char *) lua_tostring(L, 2);
	if (!sVal) {
		sVal = (char *) lua_toboolean(L, 2);
	}
	bool bRes = LuaPlugin::mCurServer->setConfig(luaL_checkstring(L, 1), sVal);
	if (!bRes) {
		return error(L, "config was not found");
	}
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// GetLang(sName)
int getLang(lua_State * L) {
	if (!checkCount(L, 1)) {
		return 0;
	}
	const char * sConfig = LuaPlugin::mCurServer->getLang(luaL_checkstring(L, 1));
	if (!sConfig) {
		return error(L, "config was not found");
	}
	lua_settop(L, 0);
	lua_pushstring(L, sConfig);
	return 1;
}

/// SetLang(sName, sValue)
int setLang(lua_State * L) {
	if (!checkCount(L, 2)) {
		return 0;
	}
	char * sVal = (char *) lua_tostring(L, 2);
	if (!sVal) {
		sVal = (char *) lua_toboolean(L, 2);
	}
	bool bRes = LuaPlugin::mCurServer->setLang(luaL_checkstring(L, 1), sVal);
	if (!bRes) {
		return error(L, "config was not found");
	}
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}


/// Call(sScriptName, sFunc, ...)
int Call(lua_State * L) {
	int iTop = lua_gettop(L);
	if (iTop < 2) {
		return errCount(L, "more 1");
	}
	string sScriptName(luaL_checkstring(L, 1));
	if (!checkScriptName(L, sScriptName)) {
		return 2;
	}
	LuaInterpreter * LIP = findInterpreter(L, sScriptName);
	if (LIP == NULL) {
		return 2;
	}

	lua_State * LL = lua_newthread(LIP->mL);
	lua_settop(LL, 0);

	int iBase = lua_gettop(LL);
	lua_pushliteral(LL, "_TRACEBACK");
	lua_rawget(LL, LUA_GLOBALSINDEX); // lua 5.1
	if (lua_isfunction(LL, -1)) {
		iBase = lua_gettop(LL);
	} else {
		lua_pop(LL, 1);
	}
	//lua_rawget(LL, LUA_ENVIRONINDEX); // lua 5.2
	//lua_insert(LL, iBase);

	lua_getglobal(LL, luaL_checkstring(L, 2));
	if (lua_type(LL, -1) != LUA_TFUNCTION) {
		lua_remove(LL, iBase); // remove _TRACEBACK
		return error(L, "function was not found");
	}

	int iPos = 2;
	while (++iPos <= iTop) {
		CopyValue(L, LL, iPos);
	}

	if (lua_pcall(LL, iTop - 2, LUA_MULTRET, iBase)) {
		const char * sErrMsg = lua_tostring(LL, -1);
		lua_remove(LL, iBase); // remove _TRACEBACK
		return luaL_error(L, sErrMsg);
	}

	iPos = 0;
	iTop = lua_gettop(LL);
	lua_settop(L, 0);
	while (++iPos <= iTop) {
		CopyValue(LL, L, iPos);
	}

	lua_remove(LL, iBase); // remove _TRACEBACK
	return iTop;
}

/// RegBot(sNick, bKey, sMyINFO, sIP)
int regBot(lua_State * L) {
	size_t iLen;
	int iType;
	string sNick, sMyINFO, sIP;
	bool bKey = true;

	switch (lua_gettop(L)) {
		case 4 :
			iType = lua_type(L, 4);
			if (iType != LUA_TNIL) {
				sIP = luaL_checklstring(L, 4, &iLen);
			}

		case 3 :
			iType = lua_type(L, 3);
			if (iType != LUA_TNIL) {
				sMyINFO = luaL_checklstring(L, 3, &iLen);
			}

		case 2 :
			bKey = (lua_toboolean(L, 2) != 0);

		case 1 :
			iType = lua_type(L, 1);
			if (iType != LUA_TNIL) {
				sNick = luaL_checklstring(L, 1, &iLen);
				if (!checkNickLen(L, iLen)) {
					return 2;
				}
			}
			break;

		default :
			return errCount(L, "1, 2, 3 or 4");

	}

	int iRes = LuaPlugin::mCurServer->regBot(sNick, sMyINFO, sIP, bKey);
	if (iRes < 0) {
		if (iRes == -1) {
			return error(L, "bad nick");
		} else if (iRes == -2) {
			return error(L, "bad MyINFO");
		} else if (iRes == -3) {
			return error(L, "bad nick (used)");
		}
		return error(L, "unknown error");
	}

	LuaPlugin::mCurLua->mCurScript->mBotList.push_back(sNick);
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// UnregBot(sNick)
int unregBot(lua_State * L) {
	if (!checkCount(L, 1)) {
		return 0;
	}
	size_t iLen;
	const char * sNick = luaL_checklstring(L, 1, &iLen);
	if (!checkNickLen(L, iLen)) {
		return 2;
	}
	if (LuaPlugin::mCurServer->unregBot(sNick) == -1) {
		return error(L, "bot was not found");
	}
	LuaPlugin::mCurLua->mCurScript->mBotList.remove(sNick);
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// Redirect(UID/sNick, sAddress, [sReason])
int redirect(lua_State * L) {
	size_t iLen;
	int iType;
	const char *sAddress = NULL, *sReason = NULL;

	switch (lua_gettop(L)) {
		case 3 :
			iType = lua_type(L, 3);
			if (iType != LUA_TNIL) {
				sReason = luaL_checklstring(L, 3, &iLen);
				if (iLen > redirectReasonMaxLen) {
					lua_settop(L, 0);
					lua_pushnil(L);
					lua_pushliteral(L, "very long reason.");
					return 2;
				}
			}

		case 2 :
			sAddress = luaL_checklstring(L, 2, &iLen);
			if (iLen > redirectAddressMaxLen) {
				lua_settop(L, 0);
				lua_pushnil(L);
				lua_pushliteral(L, "very long address.");
				return 2;
			}
			break;

		default :
			return errCount(L, "2 or 3");

	}

	iType = lua_type(L, 1);
	DcConnBase * dcConnBase = NULL;
	if (iType != LUA_TUSERDATA) {
		if (iType != LUA_TSTRING) {
			return luaL_typeerror(L, 1, "userdata or string");
		}
		DcUserBase * User = LuaPlugin::mCurServer->getDcUserBase(lua_tostring(L, 1));
		if (!User || !User->mDcConnBase) {
			return error(L, "user was not found");
		}
		dcConnBase = User->mDcConnBase;
	} else {
		dcConnBase = getDcConnBase(L, 1);
	}

	if (!dcConnBase || (dcConnBase->mType != CLIENT_TYPE_NMDC)) {
		return error(L, "user was not found");
	}

	LuaPlugin::mCurServer->forceMove(dcConnBase, sAddress, sReason);
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}

/// SetHubState(iNumber) | iNumber = 0 or nil - stop, iNumber = 1 - restart
int SetHubState(lua_State * L) {
	int iTop = lua_gettop(L);
	if (iTop > 1) {
		return errCount(L, "0 or 1");
	}

	if (iTop == 0 || lua_isnil(L, 1)) {
		LuaPlugin::mCurServer->stopHub(); // stoping
	} else {
		int iNum(luaL_checkint(L, 1));
		if (iNum == eHS_STOP) {
			LuaPlugin::mCurServer->stopHub(); // stoping
		} else if(iNum == eHS_RESTART) {
			LuaPlugin::mCurServer->restartHub(); // restarting
		}
	}
	return 0;
}

}; // namespace luaplugin
