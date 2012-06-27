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

#include "api.h"
#include "LuaPlugin.h"
#include "Uid.h"
#include "LuaUtils.h"

#include <vector>
#ifdef _WIN32
	#if defined(_MSC_VER) && (_MSC_VER >= 1400)
		#pragma warning(disable:4996) // Disable "This function or variable may be unsafe."
	#endif
#endif

using namespace ::std;

enum {
	eHS_STOP,
	eHS_RESTART
};

// lua 5.1
#define luaL_typeerror luaL_typerror

static const int maxTimers = 100; // max count timers per script
static const size_t redirectReasonMaxLen = 1024;
static const size_t redirectAddressMaxLen = 128;
static const size_t regExpPatternMaxLen = 1024;
static const size_t regExpSubjectMaxLen = 1024000;


namespace luaplugin {



DcUserBase * getDcUserBase(lua_State * L, int indx) {
	void ** userdata = static_cast<void **> (lua_touserdata(L, indx));
	DcUserBase * dcUserBase = static_cast<DcUserBase *> (*userdata);
	return dcUserBase->mType == CLIENT_TYPE_DC ? dcUserBase : NULL;
}



static void setTbl(lua_State * L1, lua_State * L2, int idx) {
	lua_newtable(L2);
	lua_pushnil(L1);
	int top = lua_gettop(L2);
	int tabIndx = (idx > 0 ? idx : idx - 1);
	while (lua_next(L1, tabIndx) != 0) {
		copyValue(L1, L2, -2); // key
		copyValue(L1, L2, -1); // value
		lua_rawset(L2, top);
		lua_pop(L1, 1);
	}
}



void copyValue(lua_State * from, lua_State * to, int idx) {
	void ** userdata = NULL;
	void * udata = NULL;
	switch (lua_type(from, idx)) {
		case LUA_TSTRING :
			lua_pushstring(to, lua_tostring(from, idx));
			break;

		case LUA_TNUMBER :
			lua_pushnumber(to, lua_tonumber(from, idx));
			break;

		case LUA_TBOOLEAN :
			lua_pushboolean(to, lua_toboolean(from, idx));
			break;

		case LUA_TTABLE :
			setTbl(from, to, idx);
			break;

		case LUA_TLIGHTUSERDATA :
			lua_pushlightuserdata(to, lua_touserdata(from, idx));
			break;

		case LUA_TUSERDATA :
			udata = static_cast<void *> (getDcUserBase(from, idx));
			if (udata) {
				userdata = static_cast<void**> (lua_newuserdata(to, sizeof(void *)));
				*userdata = udata;
				luaL_getmetatable(to, MT_USER_CONN);
				lua_setmetatable(to, -2);
			} else {
				lua_pushnil(to);
			}
			break;

		default :
			lua_pushnil(to);
			break;

	}
}



/// GetGVal(sScriptName, sParam)
int getGVal(lua_State * L) {
	if (!LuaUtils::checkCount(L, 2)) {
		return 0;
	}
	size_t len = 0;
	const char * name = luaL_checklstring(L, 1, &len);
	string scriptName(name, len);
	if (!LuaUtils::checkScriptName(L, scriptName)) {
		return 2;
	}
	LuaInterpreter * LIP = LuaUtils::findInterpreter(L, scriptName);
	if (LIP == NULL) {
		return 2;
	}

	lua_State * LL = lua_newthread(LIP->mL);
	lua_getglobal(LL, luaL_checkstring(L, 2));

	//lua_settop(LIP->mL, 0);
	lua_pop(LIP->mL, 1);
	copyValue(LL, L, -1);

	return 1;
}



/// SetGVal(sScriptName, sParam, Value)
int setGVal(lua_State * L) {
	if (!LuaUtils::checkCount(L, 3)) {
		return 0;
	}
	size_t len = 0;
	const char * name = luaL_checklstring(L, 1, &len);
	string scriptName(name, len);
	if (!LuaUtils::checkScriptName(L, scriptName)) {
		return 2;
	}
	LuaInterpreter * LIP = LuaUtils::findInterpreter(L, scriptName);
	if (LIP == NULL) {
		return 2;
	}

	lua_State * LL = lua_newthread(LIP->mL);
	const char * param = luaL_checkstring(L, 2);
	lua_getglobal(LL, param);
	copyValue(L, LL, 3);
	lua_setglobal(LL, param);
	//lua_xmove(LL, LIP->mL, 1);

	lua_pop(LIP->mL, 1);
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}



/// SendToUser(UID/sToNick/tNicks, sData, sNick, sFrom)
// TODO Web.Send
int sendToUser(lua_State * L) {
	int type;
	size_t fromLen = 0, nickLen = 0;
	const char * from = NULL, *nick = NULL;

	switch (lua_gettop(L)) {
		case 4 :
			type = lua_type(L, 4);
			if (type != LUA_TNIL) {
				from = luaL_checklstring(L, 4, &fromLen);
				if (!LuaUtils::checkNickLen(L, fromLen)) {
					return 2;
				}
			}

		case 3 :
			type = lua_type(L, 3);
			if (type != LUA_TNIL) {
				nick = luaL_checklstring(L, 3, &nickLen);
				if (!LuaUtils::checkNickLen(L, nickLen)) {
					return 2;
				}
			}

		case 2 :
			type = lua_type(L, 1);
			break;

		default :
			return LuaUtils::errCount(L, "2, 3 or 4");

	}

	size_t dataLen = 0;
	const char * data = luaL_checklstring(L, 2, &dataLen);
	if (!LuaUtils::checkMsgLen(L, dataLen)) {
		return 2;
	}

	if (type == LUA_TUSERDATA) {
		DcUserBase * dcUserBase = getDcUserBase(L, 1);
		if (!dcUserBase) {
			return LuaUtils::pushError(L, "user was not found");
		} else if (!LuaPlugin::mCurServer->sendToUser(dcUserBase, string(data, dataLen), nick, from)) {
			return LuaUtils::pushError(L, "user was not found");
		}
	} else if (type == LUA_TSTRING) {
		size_t toLen;
		const char * to = lua_tolstring(L, 1, &toLen);
		if (!LuaUtils::checkNickLen(L, toLen)) {
			return 2;
		}
		if (!LuaPlugin::mCurServer->sendToNick(to, string(data, dataLen), nick, from)) {
			return LuaUtils::pushError(L, "user was not found");
		}
	} else if (type == LUA_TTABLE) {
		lua_pushnil(L);
		const char * to = NULL;
		size_t toLen;
		while (lua_next(L, 1) != 0) {
			to = luaL_checklstring(L, -1, &toLen);
			if (!LuaUtils::checkNickLen(L, toLen)) {
				return 2;
			}
			LuaPlugin::mCurServer->sendToNick(to, string(data, dataLen), nick, from);
			lua_pop(L, 1);
		}
	} else {
		return luaL_typeerror(L, 1, "userdata, string or table");
	}
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}



/// SendToUserRaw(UID/sToNick/tNicks, sData)
// TODO Web.Send
int sendToUserRaw(lua_State * L) {
	if (!LuaUtils::checkCount(L, 2)) {
		return 0;
	}
	int type = lua_type(L, 1);

	size_t dataLen = 0;
	const char * data = luaL_checklstring(L, 2, &dataLen);
	if (!LuaUtils::checkMsgLen(L, dataLen)) {
		return 2;
	}

	if (type == LUA_TUSERDATA) {
		DcUserBase * dcUserBase = getDcUserBase(L, 1);
		if (!dcUserBase) {
			return LuaUtils::pushError(L, "user was not found");
		} else if (!LuaPlugin::mCurServer->sendToUserRaw(dcUserBase, string(data, dataLen))) {
			return LuaUtils::pushError(L, "user was not found");
		}
	} else if (type == LUA_TSTRING) {
		size_t toLen;
		const char * to = lua_tolstring(L, 1, &toLen);
		if (!LuaUtils::checkNickLen(L, toLen)) {
			return 2;
		}
		if (!LuaPlugin::mCurServer->sendToNickRaw(to, string(data, dataLen))) {
			return LuaUtils::pushError(L, "user was not found");
		}
	} else if (type == LUA_TTABLE) {
		lua_pushnil(L);
		const char * to = NULL;
		size_t toLen;
		while (lua_next(L, 1) != 0) {
			to = luaL_checklstring(L, -1, &toLen);
			if (!LuaUtils::checkNickLen(L, toLen)) {
				return 2;
			}
			LuaPlugin::mCurServer->sendToNickRaw(to, string(data, dataLen));
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
int sendToNicks(lua_State * L) {

	LuaUtils::deprecatedFunc(L, "SendToUser");

	size_t fromLen = 0, nickLen = 0, dataLen = 0, toLen = 0;
	const char *from = NULL, *nick = NULL, *data = NULL, *to = NULL;
	switch (lua_gettop(L)) {
		case 4 :
			if (lua_type(L, 4) != LUA_TNIL) {
				from = luaL_checklstring(L, 4, &fromLen);
				if (!LuaUtils::checkNickLen(L, fromLen)) {
					return 2;
				}
			}

		case 3 :
			if (lua_type(L, 3) != LUA_TNIL) {
				nick = luaL_checklstring(L, 3, &nickLen);
				if (!LuaUtils::checkNickLen(L, nickLen)) {
					return 2;
				}
			}

		case 2 :
			luaL_checktype(L, 1, LUA_TTABLE);
			data = luaL_checklstring(L, 2, &dataLen);
			if (!LuaUtils::checkMsgLen(L, dataLen)) {
				return 2;
			}

			lua_pushnil(L);
			while (lua_next(L, 1) != 0) {
				to = luaL_checklstring(L, -1, &toLen);
				if (!LuaUtils::checkNickLen(L, toLen)) {
					return 2;
				}
				LuaPlugin::mCurServer->sendToNick(to, string(data, dataLen), nick, from);
				lua_pop(L, 1);
			}
			break;

		default :
			return LuaUtils::errCount(L, "2, 3 or 4");

	}
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}



/// SendToAll(sData, sNick, sFrom)
int sendToAll(lua_State * L) {
	size_t fromLen = 0, nickLen = 0, dataLen = 0;
	const char *from = NULL, *nick = NULL, *data = NULL;
	switch (lua_gettop(L)) {
		case 3 :
			if (lua_type(L, 3) != LUA_TNIL) {
				from = luaL_checklstring(L, 3, &fromLen);
				if (!LuaUtils::checkNickLen(L, fromLen)) {
					return 2;
				}
			}

		case 2 :
			if (lua_type(L, 2) != LUA_TNIL) {
				nick = luaL_checklstring(L, 2, &nickLen);
				if (!LuaUtils::checkNickLen(L, nickLen)) {
					return 2;
				}
			}

		case 1 :
			data = luaL_checklstring(L, 1, &dataLen);
			if (!LuaUtils::checkMsgLen(L, dataLen)) {
				return 2;
			}
			break;

		default :
			return LuaUtils::errCount(L, "1, 2 or 3");

	}
	if (!LuaPlugin::mCurServer->sendToAll(string(data, dataLen), nick, from)) {
		return LuaUtils::pushError(L, "data was not send");
	}
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}



/// SendToAllRaw(sData)
int sendToAllRaw(lua_State * L) {
	if (!LuaUtils::checkCount(L, 1)) {
		return 0;
	}
	size_t dataLen = 0;
	const char * data = luaL_checklstring(L, 1, &dataLen);
	if (!LuaUtils::checkMsgLen(L, dataLen)) {
		return 2;
	}

	if (!LuaPlugin::mCurServer->sendToAllRaw(string(data, dataLen))) {
		return LuaUtils::pushError(L, "data was not send");
	}
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}



/// SendToProfile(iProfile/tProfiles, sData, sNick, sFrom)
int sendToProfile(lua_State * L) {
	unsigned long profile = 0, prf;
	int type, prof;
	size_t fromLen = 0, nickLen = 0, dataLen = 0;
	const char *from = NULL, *nick = NULL, *data = NULL;
	switch (lua_gettop(L)) {
		case 4 :
			if (lua_type(L, 4) != LUA_TNIL) {
				from = luaL_checklstring(L, 4, &fromLen);
				if (!LuaUtils::checkNickLen(L, fromLen)) {
					return 2;
				}
			}

		case 3 :
			if (lua_type(L, 3) != LUA_TNIL) {
				nick = luaL_checklstring(L, 3, &nickLen);
				if (!LuaUtils::checkNickLen(L, nickLen)) {
					return 2;
				}
			}

		case 2 :
			data = luaL_checklstring(L, 2, &dataLen);
			if (!LuaUtils::checkMsgLen(L, dataLen)) {
				return 2;
			}
			type = lua_type(L, 1);
			if (type == LUA_TTABLE) {
				lua_pushnil(L);
				while (lua_next(L, 1) != 0) {
					if ((prof = luaL_checkint(L, -1) + 1) < 0) {
						prof = -prof;
					}
					prf = static_cast<unsigned long> (1 << (prof % 32));
					if (!(profile & prf)) {
						profile = profile | prf;
					}
					lua_pop(L, 1);
				}
				if (!profile) {
					return LuaUtils::pushError(L, "list turned out to be empty");
				}
			} else if (type == LUA_TNUMBER) {
				if ((prof = luaL_checkint(L, 1) + 1) < 0) {
					prof = -prof;
				}
				profile = static_cast<unsigned long> (1 << (prof % 32));
			} else {
				return luaL_typeerror(L, 1, "number or table");
			}
			break;

		default :
			return LuaUtils::errCount(L, "2, 3 or 4");

	}
	if (!LuaPlugin::mCurServer->sendToProfiles(profile, string(data, dataLen), nick, from)) {
		return LuaUtils::pushError(L, "data was not send");
	}
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}



/// SendToProfileRaw(iProfile/tProfiles, sData)
int sendToProfileRaw(lua_State * L) {
	if (!LuaUtils::checkCount(L, 2)) {
		return 0;
	}
	size_t dataLen = 0;
	const char * data = luaL_checklstring(L, 2, &dataLen);

	if (!LuaUtils::checkMsgLen(L, dataLen)) {
		return 2;
	}
	unsigned long profile = 0;
	int prof;
	int type = lua_type(L, 1);
	if (type == LUA_TTABLE) {
		lua_pushnil(L);
		while (lua_next(L, 1) != 0) {
			if ((prof = luaL_checkint(L, -1) + 1) < 0) {
				prof = -prof;
			}
			unsigned long prf = static_cast<unsigned long> (1 << (prof % 32));
			if (!(profile & prf)) {
				profile = profile | prf;
			}
			lua_pop(L, 1);
		}
		if (!profile) {
			return LuaUtils::pushError(L, "list turned out to be empty");
		}
	} else if (type == LUA_TNUMBER) {
		if ((prof = luaL_checkint(L, 1) + 1) < 0) {
			prof = -prof;
		}
		profile = static_cast<unsigned long> (1 << (prof % 32));
	} else {
		return luaL_typeerror(L, 1, "number or table");
	}

	if (!LuaPlugin::mCurServer->sendToProfilesRaw(profile, string(data, dataLen))) {
		return LuaUtils::pushError(L, "data was not send");
	}
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}



/// SendToIP(sIP, sData, sNick, sFrom, iProfile/tProfiles)
int sendToIp(lua_State * L) {
	unsigned long profile = 0, prf;
	int type, prof;

	size_t fromLen = 0, nickLen = 0, dataLen = 0, ipLen = 0;
	const char *from = NULL, *nick = NULL, *data = NULL, *ip = NULL;

	switch (lua_gettop(L)) {
		case 5 :
			type = lua_type(L, 1);
			if (type == LUA_TTABLE) {
				lua_pushnil(L);
				while (lua_next(L, 1) != 0) {
					if ((prof = luaL_checkint(L, -1) + 1) < 0) {
						prof = -prof;
					}
					prf = static_cast<unsigned long> (1 << (prof % 32));
					if (!(profile & prf)) {
						profile = profile | prf;
					}
					lua_pop(L, 1);
				}
				if (!profile) {
					return LuaUtils::pushError(L, "list turned out to be empty");
				}
			} else if (type == LUA_TNUMBER) {
				if ((prof = luaL_checkint(L, 1) + 1) < 0) {
					prof = -prof;
				}
				profile = static_cast<unsigned long> (1 << (prof % 32));
			} else if (type != LUA_TNIL) {
				return luaL_typeerror(L, 1, "number or table");
			}

		case 4 :
			if (lua_type(L, 4) != LUA_TNIL) {
				from = luaL_checklstring(L, 4, &fromLen);
				if (!LuaUtils::checkNickLen(L, fromLen)) {
					return 2;
				}
			}

		case 3 :
			if (lua_type(L, 3) != LUA_TNIL) {
				nick = luaL_checklstring(L, 3, &nickLen);
				if (!LuaUtils::checkNickLen(L, nickLen)) {
					return 2;
				}
			}

		case 2 :
			data = luaL_checklstring(L, 2, &dataLen);
			if (!LuaUtils::checkMsgLen(L, dataLen)) {
				return 2;
			}
			break;

		default :
			return LuaUtils::errCount(L, "2, 3, 4 or 5");

	}
	ip = luaL_checklstring(L, 1, &ipLen);
	if (ip && !LuaPlugin::mCurServer->sendToIp(string(ip, ipLen), string(data, dataLen), profile, nick, from)) {
		return LuaUtils::pushError(L, "wrong ip format");
	}
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}



/// SendToIPRaw(sIP, sData, iProfile/tProfiles)
int sendToIpRaw(lua_State * L) {
	unsigned long profile = 0, prf;
	int type, prof;

	size_t dataLen = 0, ipLen = 0;
	const char *data = NULL, *ip = NULL;

	switch (lua_gettop(L)) {
		case 3 :
			type = lua_type(L, 1);
			if (type == LUA_TTABLE) {
				lua_pushnil(L);
				while (lua_next(L, 1) != 0) {
					if ((prof = luaL_checkint(L, -1) + 1) < 0) {
						prof = -prof;
					}
					prf = static_cast<unsigned long> (1 << (prof % 32));
					if (!(profile & prf)) {
						profile = profile | prf;
					}
					lua_pop(L, 1);
				}
				if (!profile) {
					return LuaUtils::pushError(L, "list turned out to be empty");
				}
			} else if (type == LUA_TNUMBER) {
				if ((prof = luaL_checkint(L, 1) + 1) < 0) {
					prof = -prof;
				}
				profile = static_cast<unsigned long> (1 << (prof % 32));
			} else if (type != LUA_TNIL) {
				return luaL_typeerror(L, 1, "number or table");
			}

		case 2 :
			data = luaL_checklstring(L, 2, &dataLen);
			if (!LuaUtils::checkMsgLen(L, dataLen)) {
				return 2;
			}
			break;

		default :
			return LuaUtils::errCount(L, "2, or 3");

	}
	ip = luaL_checklstring(L, 1, &ipLen);
	if (ip && !LuaPlugin::mCurServer->sendToIpRaw(string(ip, ipLen), string(data, dataLen), profile)) {
		return LuaUtils::pushError(L, "wrong ip format");
	}
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}



/// SendToAllExceptNicks(tExcept, sData, sNick, sFrom)
int sendToAllExceptNicks(lua_State * L) {
	vector<string> nickList;
	size_t fromLen = 0, nickLen = 0, dataLen = 0, exceptLen = 0;
	const char *from = NULL, *nick = NULL, *data = NULL, *except = NULL;

	switch (lua_gettop(L)) {
		case 4 :
			if (lua_type(L, 4) != LUA_TNIL) {
				from = luaL_checklstring(L, 4, &fromLen);
				if (!LuaUtils::checkNickLen(L, fromLen)) {
					return 2;
				}
			}

		case 3 :
			if (lua_type(L, 3) != LUA_TNIL) {
				nick = luaL_checklstring(L, 3, &nickLen);
				if (!LuaUtils::checkNickLen(L, nickLen)) {
					return 2;
				}
			}

		case 2 :
			luaL_checktype(L, 1, LUA_TTABLE);
			data = luaL_checklstring(L, 2, &dataLen);
			if (!LuaUtils::checkMsgLen(L, dataLen)) {
				return 2;
			}
			lua_pushnil(L);
			while (lua_next(L, 1) != 0) {
				except = luaL_checklstring(L, -1, &exceptLen);
				if (!LuaUtils::checkNickLen(L, exceptLen)) {
					return 2;
				}
				nickList.push_back(string(except, exceptLen));
				lua_pop(L, 1);
			}
			if (!nickList.size()) {
				return LuaUtils::pushError(L, "list turned out to be empty");
			}
			break;

		default :
			return LuaUtils::errCount(L, "2, 3 or 4");

	}
	if (!LuaPlugin::mCurServer->sendToAllExceptNicks(nickList, string(data, dataLen), nick, from)) {
		return LuaUtils::pushError(L, "data was not send");
	}
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}



/// SendToAllExceptNicksRaw(tExcept, sData)
int sendToAllExceptNicksRaw(lua_State * L) {
	if (!LuaUtils::checkCount(L, 2)) {
		return 0;
	}

	luaL_checktype(L, 1, LUA_TTABLE);

	size_t dataLen = 0, exceptLen = 0;
	const char * data = luaL_checklstring(L, 2, &dataLen);
	if (!LuaUtils::checkMsgLen(L, dataLen)) {
		return 2;
	}

	vector<string> nickList;
	lua_pushnil(L);
	while (lua_next(L, 1) != 0) {
		const char * except = luaL_checklstring(L, -1, &exceptLen);
		if (!LuaUtils::checkNickLen(L, exceptLen)) {
			return 2;
		}
		nickList.push_back(string(except, exceptLen));
		lua_pop(L, 1);
	}
	if (!nickList.size()) {
		return LuaUtils::pushError(L, "list turned out to be empty");
	}

	if (!LuaPlugin::mCurServer->sendToAllExceptNicksRaw(nickList, string(data, dataLen))) {
		return LuaUtils::pushError(L, "data was not send");
	}
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}



/// SendToAllExceptIPs(tExcept, sData, sNick, sFrom)
int sendToAllExceptIps(lua_State * L) {
	vector<string> ipList;
	size_t fromLen = 0, nickLen = 0, dataLen = 0, exceptLen = 0;
	const char *from = NULL, *nick = NULL, *data = NULL, *except = NULL;

	switch (lua_gettop(L)) {
		case 4 :
			if (lua_type(L, 4) != LUA_TNIL) {
				from = luaL_checklstring(L, 4, &fromLen);
				if (!LuaUtils::checkNickLen(L, fromLen)) {
					return 2;
				}
			}

		case 3 :
			if (lua_type(L, 3) != LUA_TNIL) {
				nick = luaL_checklstring(L, 3, &nickLen);
				if (!LuaUtils::checkNickLen(L, nickLen)) {
					return 2;
				}
			}

		case 2 :
			luaL_checktype(L, 1, LUA_TTABLE);
			data = luaL_checklstring(L, 2, &dataLen);
			if (!LuaUtils::checkMsgLen(L, dataLen)) {
				return 2;
			}
			lua_pushnil(L);
			while (lua_next(L, 1) != 0) {
				except = luaL_checklstring(L, -1, &exceptLen);
				if (!LuaUtils::checkNickLen(L, exceptLen)) {
					return 2;
				}
				ipList.push_back(string(except, exceptLen));
				lua_pop(L, 1);
			}
			if (!ipList.size()) {
				return LuaUtils::pushError(L, "list turned out to be empty");
			}
			break;

		default :
			return LuaUtils::errCount(L, "2, 3 or 4");

	}
	if (!LuaPlugin::mCurServer->sendToAllExceptIps(ipList, string(data, dataLen), nick, from)) {
		return LuaUtils::pushError(L, "wrong ip format");
	}
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}



/// SendToAllExceptIPsRaw(tExcept, sData)
int sendToAllExceptIpsRaw(lua_State * L) {
	if (!LuaUtils::checkCount(L, 2)) {
		return 0;
	}

	luaL_checktype(L, 1, LUA_TTABLE);

	size_t dataLen = 0, exceptLen = 0;
	const char * data = luaL_checklstring(L, 2, &dataLen);
	if (!LuaUtils::checkMsgLen(L, dataLen)) {
		return 2;
	}

	vector<string> ipList;
	lua_pushnil(L);
	while (lua_next(L, 1) != 0) {
		const char * except = luaL_checklstring(L, -1, &exceptLen);
		if (!LuaUtils::checkNickLen(L, exceptLen)) {
			return 2;
		}
		ipList.push_back(string(except, exceptLen));
		lua_pop(L, 1);
	}
	if (!ipList.size()) {
		return LuaUtils::pushError(L, "list turned out to be empty");
	}

	if (!LuaPlugin::mCurServer->sendToAllExceptIpsRaw(ipList, string(data, dataLen))) {
		return LuaUtils::pushError(L, "wrong ip format");
	}
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}



/// GetUser(UID/sNick, iByte)
int getUser(lua_State * L) {
	if (lua_type(L, 1) == LUA_TUSERDATA) {
		lua_pushvalue(L, 1);
		luaL_getmetatable(L, MT_USER_CONN);
		lua_setmetatable(L, -2);
	} else if (lua_isstring(L, 1)) {
		size_t len;
		const char * nick = lua_tolstring(L, 1, &len);
		if (!LuaUtils::checkNickLen(L, len)) {
			return 2;
		}
		DcUserBase * dcUserBase = LuaPlugin::mCurServer->getDcUserBase(nick);
		if (!dcUserBase) {
			return LuaUtils::pushError(L, "user was not found");
		}
		void ** userdata = static_cast<void **> (lua_newuserdata(L, sizeof(void *)));
		*userdata = static_cast<void *> (dcUserBase);
		luaL_getmetatable(L, MT_USER_CONN);
		lua_setmetatable(L, -2);
	} else {
		return luaL_typeerror(L, 1, "userdata or string");
	}
	return 1;
}



//! deprecated
/// SetUser(UID/sNick, iType, Value)
int setUser(lua_State * L) {

	LuaUtils::deprecatedFunc(L, "UID[sName] = Value");

	if (!LuaUtils::checkCount(L, 3)) {
		return 0;
	}
	DcUserBase * dcUserBase = NULL;
	if (lua_type(L, 1) == LUA_TUSERDATA) {
		dcUserBase = getDcUserBase(L, 1);
	} else if (lua_isstring(L, 1)) {
		size_t len;
		const char * nick = lua_tolstring(L, 1, &len);
		if (!LuaUtils::checkNickLen(L, len)) {
			return 2;
		}
		dcUserBase = LuaPlugin::mCurServer->getDcUserBase(nick);
	} else {
		return luaL_typeerror(L, 1, "userdata or string");
	}

	if (dcUserBase == NULL) {
		return LuaUtils::pushError(L, "user was not found");
	}

	unsigned num = static_cast<unsigned> (luaL_checkinteger(L, 2));
	if (num == USERVALUE_PROFILE) {
		Uid::setValue(L, dcUserBase, USER_PARAM_PROFILE, ParamBase::TYPE_INT);
	} else if (num == USERVALUE_MYINFO) {
		size_t len = 0;
		const char * info = luaL_checklstring(L, 3, &len);
		if (!dcUserBase->setInfo(string(info, len))) {
			return LuaUtils::pushError(L, "wrong syntax");
		}
	} else if (num == USERVALUE_DATA) {
		Uid::setValue(L, dcUserBase, "sData", ParamBase::TYPE_STRING);
	} else if (num == USERVALUE_OPLIST) {
		luaL_checktype(L, 3, LUA_TBOOLEAN);
		Uid::setValue(L, dcUserBase, USER_PARAM_IN_OP_LIST, ParamBase::TYPE_BOOL);
	} else if (num == USERVALUE_HIDE) {
		luaL_checktype(L, 3, LUA_TBOOLEAN);
		Uid::setValue(L, dcUserBase, USER_PARAM_CAN_HIDE, ParamBase::TYPE_BOOL);
	} else if (num == USERVALUE_IPLIST) {
		luaL_checktype(L, 3, LUA_TBOOLEAN);
		Uid::setValue(L, dcUserBase, USER_PARAM_IN_IP_LIST, ParamBase::TYPE_BOOL);
	}
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}



/// GetUsers(sIP, iByte)
int getUsers(lua_State * L) {
	lua_newtable(L);
	int topTab = lua_gettop(L), i = 1;
	if (lua_type(L, 1) == LUA_TSTRING) {
		bool all = false;
		if (lua_isboolean(L, 2) && lua_toboolean(L, 2) == 1) {
			all = true;
		}
		const vector<DcUserBase *> & v = LuaPlugin::mCurServer->getDcUserBaseByIp(lua_tostring(L, 1));
		for (vector<DcUserBase *>::const_iterator it = v.begin(); it != v.end(); ++it) {
			if (all || (*it)->getParam(USER_PARAM_IN_USER_LIST)->getBool()) {
				lua_pushnumber(L, i);
				void ** userdata = static_cast<void **> (lua_newuserdata(L, sizeof(void *)));
				*userdata = static_cast<void *> (*it);
				luaL_getmetatable(L, MT_USER_CONN);
				lua_setmetatable(L, -2);
				lua_rawset(L, topTab);
				++i;
			}
		}
	} else {
		bool all = false;
		if (lua_isboolean(L, 1) && lua_toboolean(L, 1) == 1) {
			all = true;
		}
		DcListIteratorBase * it = LuaPlugin::mCurServer->getDcListIterator();
		DcUserBase * dcUserBase = NULL;
		while ((dcUserBase = it->operator ()()) != NULL) {
			if (dcUserBase->mType == CLIENT_TYPE_DC) {
				if (all || (dcUserBase->getParam(USER_PARAM_IN_USER_LIST)->getBool())) {
					lua_pushnumber(L, i);
					void ** userdata = static_cast<void **> (lua_newuserdata(L, sizeof(void *)));
					*userdata = static_cast<void *> (dcUserBase);
					luaL_getmetatable(L, MT_USER_CONN);
					lua_setmetatable(L, -2);
					lua_rawset(L, topTab);
					++i;
				}
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
	lua_pushnumber(L, static_cast<lua_Number> (LuaPlugin::mCurServer->getTotalShare()));
	return 1;
}



/// GetUpTime()
int getUpTime(lua_State * L) {
	lua_pushnumber(L, static_cast<lua_Number> (LuaPlugin::mCurServer->getUpTime()));
	return 1;
}



/// Disconnect(sNick/UID)
int disconnect(lua_State * L) {
	if (!LuaUtils::checkCount(L, 1)) {
		return 0;
	}
	int type = lua_type(L, 1);
	DcUserBase * dcUserBase = NULL;
	if (type == LUA_TUSERDATA) {
		dcUserBase = getDcUserBase(L, 1);
	} else if (type == LUA_TSTRING) {
		size_t len;
		const char * nick = lua_tolstring(L, 1, &len);
		if (!LuaUtils::checkNickLen(L, len)) {
			return 2;
		}
		dcUserBase = LuaPlugin::mCurServer->getDcUserBase(nick);
	} else {
		return luaL_typeerror(L, 1, "userdata or string");
	}

	if (dcUserBase == NULL) {
		return LuaUtils::pushError(L, "user was not found");
	}

	dcUserBase->disconnect();

	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}



/// DisconnectIP(sIP, iProfile/tProfiles)
int disconnectIp(lua_State * L) {
	int top = lua_gettop(L);
	if (top < 1 || 2 < top) {
		return LuaUtils::errCount(L, "1 or 2");
	}

	size_t len;
	const char * ip = luaL_checklstring(L, 1, &len);
	if (len < 7 || len > 15) {
		luaL_argerror(L, 1, "ip has wrong format");
		return 0;
	}

	unsigned long profile = 0;
	if (top == 2) {
		int prof, type = lua_type(L, 2);
		unsigned long prf;
		if (type == LUA_TTABLE) {
			lua_pushnil(L);
			while (lua_next(L, 1) != 0) {
				if ((prof = luaL_checkint(L, -1) + 1) < 0) {
					prof = -prof;
				}
				prf = static_cast<unsigned long> (1 << (prof % 32));
				if (!(profile & prf)) {
					profile = profile | prf;
				}
				lua_pop(L, 1);
			}
			if (!profile) {
				return LuaUtils::pushError(L, "list turned out to be empty");
			}
		} else if (type == LUA_TNUMBER) {
			if ((prof = luaL_checkint(L, 1) + 1) < 0) {
				prof = -prof;
			}
			profile = static_cast<unsigned long> (1 << (prof % 32));
		} else if (type != LUA_TNIL) {
			return luaL_typeerror(L, 1, "number or table");
		}
	}

	const vector<DcUserBase *> & v = LuaPlugin::mCurServer->getDcUserBaseByIp(ip);
	vector<DcUserBase *>::const_iterator it;
	if (!profile) {
		for (it = v.begin(); it != v.end(); ++it) {
			(*it)->disconnect();
		}
	} else {
		int prf;
		for (it = v.begin(); it != v.end(); ++it) {
			ParamBase * paramBase = (*it)->getParamForce(USER_PARAM_PROFILE);
			if (paramBase != NULL) {
				prf = paramBase->getInt() + 1;
				if (prf < 0) {
					prf = -prf;
				}
				if (prf > 31) {
					prf = (prf % 32) - 1;
				}
				if (profile & static_cast<unsigned long> (1 << prf)) {
					(*it)->disconnect();
				}
			}
		}
	}
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}



/// RestartScripts(iType)
int restartScripts(lua_State *L) {
	int type = 0;
	if (!lua_isnoneornil(L, 1)) {
		type = luaL_checkint(L, 1);
		if (type < 0 || type > 2) {
			type = 0;
		}
	}
	LuaPlugin::mCurLua->restartScripts(LuaPlugin::mCurLua->mCurScript, type);
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}



/// RestartScript(sScriptName)
int restartScript(lua_State *L) {
	int state;
	if (!lua_isnoneornil(L, 1)) {
		size_t len = 0;
		const char * name = luaL_checklstring(L, 1, &len);
		string scriptName(name, len);
		if (!LuaUtils::checkScriptName(L, scriptName)) {
			return 2;
		}
		LuaInterpreter * script = LuaPlugin::mCurLua->findScript(scriptName);
		if (script) {
			state = LuaPlugin::mCurLua->restartScript(script, script->mL == L);
		} else {
			state = LUA_ERRFILE;
		}
	} else {
		state = LuaPlugin::mCurLua->restartScript(LuaPlugin::mCurLua->mCurScript, true);
	}

	if (state == 0) { // script was restarted
		lua_settop(L, 0);
		lua_pushboolean(L, 1);
		return 1;
	} else if (state == LUA_ERRFILE) { // script was not found
		lua_settop(L, 0);
		lua_pushnil(L);
		return 1;
	} else if (state == -1) {
		return LuaUtils::pushError(L, "script was started already");
	} else if (state == LUA_ERRMEM) {
		return LuaUtils::pushError(L, "memory error");
	} else if (state == LUA_ERRSYNTAX || state == LUA_YIELD || state == LUA_ERRRUN || state == LUA_ERRERR) {
		return LuaUtils::pushError(L, LuaPlugin::mCurLua->mLastError.c_str());
	}
	return LuaUtils::pushError(L, "unknown error");
}



/// StopScript(sScriptName)
int stopScript(lua_State * L) {
	int state;
	LuaInterpreter * script = LuaPlugin::mCurLua->mCurScript;
	if (!lua_isnoneornil(L, 1)) {
		size_t len = 0;
		const char * name = luaL_checklstring(L, 1, &len);
		string scriptName(name, len);
		if (!LuaUtils::checkScriptName(L, scriptName)) {
			return 2;
		}
		script = LuaPlugin::mCurLua->findScript(scriptName);
		if (script) {
			state = LuaPlugin::mCurLua->stopScript(script, script->mL == L);
		} else {
			state = LUA_ERRFILE;
		}
	} else {
		state = LuaPlugin::mCurLua->stopScript(script, true);
	}

	if (state == 0) { // script was stopped
		LuaPlugin::mCurLua->onScriptAction(script->mName.c_str(), "OnScriptStop");
		lua_settop(L, 0);
		lua_pushboolean(L, 1);
		return 1;
	} else if (state == LUA_ERRFILE) { // script was not found
		lua_settop(L, 0);
		lua_pushnil(L);
		return 1;
	} else if (state == -1) {
		return LuaUtils::pushError(L, "script was stoped already");
	}
	return LuaUtils::pushError(L, "unknown error");
}



/// StartScript(sScriptName)
int startScript(lua_State * L) {
	size_t len = 0;
	const char * name = luaL_checklstring(L, 1, &len);
	string scriptName(name, len);
	if (!LuaUtils::checkScriptName(L, scriptName)) {
		return 2;
	}
	int state = LuaPlugin::mCurLua->startScript(scriptName);

	if (state == 0) { // script was started
		LuaPlugin::mCurLua->onScriptAction(scriptName.c_str(), "OnScriptStart");
		lua_settop(L, 0);
		lua_pushboolean(L, 1);
		return 1;
	} else if (state == LUA_ERRFILE) { // script was not found
		lua_settop(L, 0);
		lua_pushnil(L);
		return 1;
	} else if (state == -1) {
		return LuaUtils::pushError(L, "script was started already");
	} else if (state == LUA_ERRMEM) {
		return LuaUtils::pushError(L, "memory error");
	} else if (state == LUA_ERRSYNTAX || state == LUA_YIELD || state == LUA_ERRRUN || state == LUA_ERRERR) {
		return LuaUtils::pushError(L, LuaPlugin::mCurLua->mLastError.c_str());
	}
	return LuaUtils::pushError(L, "unknown error");
}



/// GetScripts()
int getScripts(lua_State * L) {
	lua_newtable(L);
	int i = 1, top = lua_gettop(L);
	LuaPlugin::listLuaInterpreter::iterator it;
	for (it = LuaPlugin::mCurLua->mLua.begin(); it != LuaPlugin::mCurLua->mLua.end(); ++it) {
		if (*it) {
			lua_pushnumber(L, i);
			lua_newtable(L);
			lua_pushliteral(L, "sName");
			lua_pushstring(L, (*it)->mName.c_str());
			lua_rawset(L, top + 2);
			lua_pushliteral(L, "bEnabled");
			lua_pushboolean(L, ((*it)->mEnabled == false) ? 0 : 1);
			lua_rawset(L, top + 2);
			lua_pushliteral(L, "iMemUsage");
			if ((*it)->mEnabled) {
				lua_pushnumber(L, lua_gc((*it)->mL, LUA_GCCOUNT, 0));
			} else {
				lua_pushnumber(L, 0);
			}
			lua_rawset(L, top + 2);
			lua_rawset(L, top);
			++i;
		}
	}
	return 1;
}



/// GetScript(sScriptName)
int getScript(lua_State * L) {
	int top = lua_gettop(L);
	if (top > 1) {
		return LuaUtils::errCount(L, "0 or 1");
	}
	LuaInterpreter * script = NULL;
	if (!lua_isnoneornil(L, 1)) {
		size_t len = 0;
		const char * name = luaL_checklstring(L, 1, &len);
		string scriptName(name, len);
		if (!LuaUtils::checkScriptName(L, scriptName)) {
			return 2;
		}
		script = LuaPlugin::mCurLua->findScript(scriptName);
		if (!script) {
			return LuaUtils::pushError(L, "script was not found");
		}
	} else {
		script = LuaPlugin::mCurLua->mCurScript;
	}
	lua_newtable(L);
	++top;
	lua_pushliteral(L, "sName");
	lua_pushstring(L, script->mName.c_str());
	lua_rawset(L, top);
	lua_pushliteral(L, "bEnabled");
	lua_pushboolean(L, (script->mEnabled == false) ? 0 : 1);
	lua_rawset(L, top);
	lua_pushliteral(L, "iMemUsage");
	if (script->mEnabled) {
		lua_pushnumber(L, lua_gc(script->mL, LUA_GCCOUNT, 0));
	} else {
		lua_pushnumber(L, 0);
	}
	lua_rawset(L, top);
	return 1;
}



/// MoveUpScript(sScriptName)
int moveUpScript(lua_State *L) {
	if (!lua_isnoneornil(L, 1)) {
		size_t len = 0;
		const char * name = luaL_checklstring(L, 1, &len);
		string scriptName(name, len);
		if (!LuaUtils::checkScriptName(L, scriptName)) {
			return 2;
		}
		LuaInterpreter * script = LuaPlugin::mCurLua->findScript(scriptName);
		if (script) {
			LuaPlugin::mCurLua->mTasksList.addTask(static_cast<void *> (script), TASKTYPE_MOVEUP);
		} else {
			return LuaUtils::pushError(L, "script was not found");
		}
	} else {
		LuaPlugin::mCurLua->mTasksList.addTask(static_cast<void *> (LuaPlugin::mCurLua->mCurScript), TASKTYPE_MOVEUP);
	}

	LuaPlugin::mCurLua->mTasksList.addTask(NULL, TASKTYPE_SAVE);
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}



/// MoveDownScript(sScriptName)
int moveDownScript(lua_State * L) {
	if (!lua_isnoneornil(L, 1)) {
		size_t len = 0;
		const char * name = luaL_checklstring(L, 1, &len);
		string scriptName(name, len);
		if (!LuaUtils::checkScriptName(L, scriptName)) {
			return 2;
		}
		LuaInterpreter * script = LuaPlugin::mCurLua->findScript(scriptName);
		if (script) {
			LuaPlugin::mCurLua->mTasksList.addTask(static_cast<void *> (script), TASKTYPE_MOVEDOWN);
		} else {
			return LuaUtils::pushError(L, "script was not found");
		}
	} else {
		LuaPlugin::mCurLua->mTasksList.addTask(static_cast<void *> (LuaPlugin::mCurLua->mCurScript), TASKTYPE_MOVEDOWN);
	}

	LuaPlugin::mCurLua->mTasksList.addTask(NULL, TASKTYPE_SAVE);
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}



/// SetCmd(sData)
// TODO deprecated? SetCmd(UID, sData)
int setCmd(lua_State * L) {
	if (!LuaUtils::checkCount(L, 1)) {
		return 0;
	}
	const char * data = luaL_checkstring(L, 1);
	if (!LuaPlugin::mCurLua->mCurUser) {
		luaL_argerror(L, 1, "internal fatal error");
		return 0;
	}
	if (!LuaPlugin::mCurLua->mCurUser->parseCommand(data)) {
		luaL_argerror(L, 1, "wrong syntax");
		return 0;
	}
	lua_settop(L, 0);
	lua_pushboolean(L, 0);
	return 1;
}



/// AddTimer(iId, iInterval, sFunc)
int addTimer(lua_State * L) {
	int top = lua_gettop(L);
	if (top < 2 || top > 3) {
		return LuaUtils::errCount(L, "2 or 3");
	}
	const char * func = "OnTimer";
	int id(luaL_checkint(L, 1)), interval(luaL_checkint(L, 2));
	if (top == 3) {
		func = luaL_checkstring(L, 3);
	}

	lua_getglobal(L, func);
	if (lua_isnil(L, lua_gettop(L)) || lua_type(L, -1) != LUA_TFUNCTION) {
		return LuaUtils::pushError(L, "timer function was not found");
	}

	if (LuaPlugin::mCurLua->mCurScript->size() > maxTimers) {
		return luaL_error(L, "bad count timers for this script (max %d)", maxTimers);
	}
	Timer * timer = new Timer(id, interval, func, LuaPlugin::mCurLua->mCurScript);
	lua_settop(L, 0);
	lua_pushinteger(L, LuaPlugin::mCurLua->mCurScript->addTmr(timer));
	return 1;
}



/// DelTimer(iId)
int delTimer(lua_State * L) {
	if (!LuaUtils::checkCount(L, 1)) {
		return 0;
	}
	int num = luaL_checkint(L, 1);
	lua_settop(L, 0);
	lua_pushinteger(L, LuaPlugin::mCurLua->mCurScript->delTmr(num));
	return 1;
}



//! deprecated
/// GetConfig(sName)
int getConfig(lua_State * L) {

	LuaUtils::deprecatedFunc(L, "Config[sName]");

	if (!LuaUtils::checkCount(L, 1)) {
		return 0;
	}
	const char * config = LuaPlugin::mCurServer->getConfig(luaL_checkstring(L, 1));
	if (!config) {
		return LuaUtils::pushError(L, "config was not found");
	}
	lua_settop(L, 0);
	lua_pushstring(L, config);
	return 1;
}



//! deprecated
/// SetConfig(sName, sValue)
int setConfig(lua_State * L) {

	LuaUtils::deprecatedFunc(L, "Config[sName] = Value");

	if (!LuaUtils::checkCount(L, 2)) {
		return 0;
	}
	const char * value = lua_tostring(L, 2);
	if (!value) {
		value = lua_toboolean(L, 2) == 0 ? "0" : "1";
	}
	bool res = LuaPlugin::mCurServer->setConfig(luaL_checkstring(L, 1), value);
	if (!res) {
		return LuaUtils::pushError(L, "config was not found");
	}
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}



/// GetLang(sName)
int getLang(lua_State * L) {
	if (!LuaUtils::checkCount(L, 1)) {
		return 0;
	}
	const char * config = LuaPlugin::mCurServer->getLang(luaL_checkstring(L, 1));
	if (!config) {
		return LuaUtils::pushError(L, "config was not found");
	}
	lua_settop(L, 0);
	lua_pushstring(L, config);
	return 1;
}



/// SetLang(sName, sValue)
int setLang(lua_State * L) {
	if (!LuaUtils::checkCount(L, 2)) {
		return 0;
	}
	const char * value = lua_tostring(L, 2);
	if (!value) {
		value = lua_toboolean(L, 2) == 0 ? "0" : "1";
	}
	bool res = LuaPlugin::mCurServer->setLang(luaL_checkstring(L, 1), value);
	if (!res) {
		return LuaUtils::pushError(L, "config was not found");
	}
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}



/// Call(sScriptName, sFunc, ...)
int call(lua_State * L) {
	int top = lua_gettop(L);
	if (top < 2) {
		return LuaUtils::errCount(L, "more 1");
	}
	size_t len = 0;
	const char * name = luaL_checklstring(L, 1, &len);
	string scriptName(name, len);
	if (!LuaUtils::checkScriptName(L, scriptName)) {
		return 2;
	}
	LuaInterpreter * LIP = LuaUtils::findInterpreter(L, scriptName);
	if (LIP == NULL) {
		return 2;
	}

	lua_State * LL = lua_newthread(LIP->mL);
	lua_settop(LL, 0); // clear stack


	int base = lua_gettop(LL);
	int traceback = base;
	lua_pushliteral(LL, "_TRACEBACK");
	lua_rawget(LL, LUA_GLOBALSINDEX); // lua 5.1
	//lua_rawget(LL, LUA_ENVIRONINDEX); // lua 5.2
	if (lua_isfunction(LL, -1)) {
		traceback = lua_gettop(LL);
	} else {
		lua_pop(LL, 1); // remove _TRACEBACK
	}

	lua_getglobal(LL, luaL_checkstring(L, 2));
	if (lua_type(LL, -1) != LUA_TFUNCTION) {
		lua_settop(LL, base); // clear stack
		return LuaUtils::pushError(L, "function was not found");
	}

	int pos = 2;
	while (++pos <= top) {
		copyValue(L, LL, pos);
	}

	if (lua_pcall(LL, top - 2, LUA_MULTRET, traceback)) {
		const char * errMsg = lua_tostring(LL, -1);
		lua_settop(LL, base); // clear stack
		return luaL_error(L, errMsg);
	}

	pos = 0;
	top = lua_gettop(LL);
	lua_settop(L, 0);
	while (++pos <= top) {
		copyValue(LL, L, pos);
	}
	return top;
}



/// RegBot(sNick, bKey, sMyINFO, sIP)
int regBot(lua_State * L) {
	int type;
	size_t len = 0;
	const char * tmp = NULL;
	string ip, info, nick;
	bool key = true;

	switch (lua_gettop(L)) {
		case 4 :
			type = lua_type(L, 4);
			if (type != LUA_TNIL) {
				tmp = luaL_checklstring(L, 4, &len);
				ip.assign(tmp, len);
			}

		case 3 :
			type = lua_type(L, 3);
			if (type != LUA_TNIL) {
				tmp = luaL_checklstring(L, 3, &len);
				info.assign(tmp, len);
			}

		case 2 :
			key = (lua_toboolean(L, 2) != 0);

		case 1 :
			type = lua_type(L, 1);
			if (type != LUA_TNIL) {
				tmp = luaL_checklstring(L, 1, &len);
				nick.assign(tmp, len);
				if (!LuaUtils::checkNickLen(L, len)) {
					return 2;
				}
			}
			break;

		default :
			return LuaUtils::errCount(L, "1, 2, 3 or 4");

	}

	int res = LuaPlugin::mCurServer->regBot(nick, info, ip, key);
	if (res < 0) {
		if (res == -1) {
			return LuaUtils::pushError(L, "bad nick");
		} else if (res == -2) {
			return LuaUtils::pushError(L, "bad MyINFO");
		} else if (res == -3) {
			return LuaUtils::pushError(L, "bad nick (used)");
		}
		return LuaUtils::pushError(L, "unknown error");
	}

	LuaPlugin::mCurLua->mCurScript->mBotList.push_back(nick);
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}



/// UnregBot(sNick)
int unregBot(lua_State * L) {
	if (!LuaUtils::checkCount(L, 1)) {
		return 0;
	}
	size_t len;
	const char * nick = luaL_checklstring(L, 1, &len);
	if (!LuaUtils::checkNickLen(L, len)) {
		return 2;
	}
	if (LuaPlugin::mCurServer->unregBot(nick) == -1) {
		return LuaUtils::pushError(L, "bot was not found");
	}
	LuaPlugin::mCurLua->mCurScript->mBotList.remove(nick);
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}



/// Redirect(UID/sNick, sAddress, [sReason])
int redirect(lua_State * L) {
	size_t len;
	int type;
	const char *address = NULL, *reason = NULL;

	switch (lua_gettop(L)) {
		case 3 :
			type = lua_type(L, 3);
			if (type != LUA_TNIL) {
				reason = luaL_checklstring(L, 3, &len);
				if (len > redirectReasonMaxLen) {
					lua_settop(L, 0);
					lua_pushnil(L);
					lua_pushliteral(L, "very long reason.");
					return 2;
				}
			}

		case 2 :
			address = luaL_checklstring(L, 2, &len);
			if (len > redirectAddressMaxLen) {
				lua_settop(L, 0);
				lua_pushnil(L);
				lua_pushliteral(L, "very long address.");
				return 2;
			}
			break;

		default :
			return LuaUtils::errCount(L, "2 or 3");

	}

	type = lua_type(L, 1);
	DcUserBase * dcUserBase = NULL;
	if (type != LUA_TUSERDATA) {
		if (type != LUA_TSTRING) {
			return luaL_typeerror(L, 1, "userdata or string");
		}
		dcUserBase = LuaPlugin::mCurServer->getDcUserBase(lua_tostring(L, 1));
	} else {
		dcUserBase = getDcUserBase(L, 1);
	}

	if (dcUserBase == NULL) {
		return LuaUtils::pushError(L, "user was not found");
	}

	LuaPlugin::mCurServer->forceMove(dcUserBase, address, reason);
	lua_settop(L, 0);
	lua_pushboolean(L, 1);
	return 1;
}



/// SetHubState(iNumber) | iNumber = 0 or nil - stop, iNumber = 1 - restart
int setHubState(lua_State * L) {
	int top = lua_gettop(L);
	if (top > 1) {
		return LuaUtils::errCount(L, "0 or 1");
	}

	if (top == 0 || lua_isnil(L, 1)) {
		LuaPlugin::mCurServer->stopHub(); // stoping
	} else {
		int num(luaL_checkint(L, 1));
		if (num == eHS_STOP) {
			LuaPlugin::mCurServer->stopHub(); // stoping
		} else if(num == eHS_RESTART) {
			LuaPlugin::mCurServer->restartHub(); // restarting
		}
	}
	return 0;
}


} // namespace luaplugin

/**
 * $Id$
 * $HeadURL$
 */
