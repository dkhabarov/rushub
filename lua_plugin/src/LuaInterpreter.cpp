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

#include "LuaInterpreter.h"
#include "LuaPlugin.h" // for dirs
#include "Uid.h"

#include <string.h>

#define HAVE_LUA_5_1

using namespace ::std;

namespace luaplugin {

LuaInterpreter::LuaInterpreter(const string & sName, string & sPath) : 
	mName(sName),
	msPath(sPath),
	mL(NULL),
	mbEnabled(false)
{
}

LuaInterpreter::~LuaInterpreter() {
	Stop();
}

/** On start script (-1 - run already)*/
int LuaInterpreter::Start() {
	if (mL) {
		return -1;
	}
	mbEnabled = true;
	mL = luaL_newstate();

#ifdef HAVE_LUA_5_1
	luaL_openlibs(mL);
#else
	luaopen_base(mL);
	luaopen_table(mL);
	luaopen_io(mL);
	luaopen_string(mL);
	luaopen_math(mL);
	luaopen_debug(mL);
	luaopen_loadlib(mL);
#endif

	// set new package.path and package.cpath
	lua_pushliteral(mL, "package");
	lua_gettable(mL, LUA_GLOBALSINDEX);

	if (!LuaPlugin::mCurLua->mbSetLuaPath) {
		lua_pushliteral(mL, "path");
		lua_rawget(mL, -2);
		LuaPlugin::mCurLua->msLuaPath.append(lua_tostring(mL, -1));
		lua_pop(mL, 1);

		lua_pushliteral(mL, "cpath");
		lua_rawget(mL, -2);
		LuaPlugin::mCurLua->msLuaCPath.append(lua_tostring(mL, -1));
		lua_pop(mL, 1);

		LuaPlugin::mCurLua->mbSetLuaPath = true;
	}

	lua_pushliteral(mL, "path");
	lua_pushstring(mL, LuaPlugin::mCurLua->msLuaPath.c_str());
	lua_settable(mL, -3);

	lua_pushliteral(mL, "cpath");
	lua_pushstring(mL, LuaPlugin::mCurLua->msLuaCPath.c_str());
	lua_settable(mL, -3);

	lua_settop(mL, 0);


	lua_newtable(mL);
	RegFunc("GetGVal",              &GetGVal);
	RegFunc("SetGVal",              &SetGVal);
	RegFunc("SendToUser",           &sendToUser);
	RegFunc("SendToAll",            &sendToAll);
	RegFunc("SendToAllExceptNicks", &sendToAllExceptNicks);
	RegFunc("SendToAllExceptIPs",   &sendToAllExceptIps);
	RegFunc("SendToProfile",        &SendToProfile);
	RegFunc("SendToIP",             &sendToIp);
	RegFunc("SendToNicks",          &SendToNicks);
	RegFunc("GetUser",              &GetUser);
	RegFunc("SetUser",              &SetUser);
	RegFunc("GetUsersCount",        &getUsersCount);
	RegFunc("GetTotalShare",        &getTotalShare);
	RegFunc("GetUpTime",            &getUpTime);
	RegFunc("Disconnect",           &disconnect);
	RegFunc("DisconnectIP",         &DisconnectIP);
	RegFunc("RestartScripts",       &RestartScripts);
	RegFunc("RestartScript",        &RestartScript);
	RegFunc("StopScript",           &StopScript);
	RegFunc("StartScript",          &StartScript);
	RegFunc("GetScripts",           &GetScripts);
	RegFunc("GetScript",            &GetScript);
	RegFunc("MoveUpScript",         &MoveUpScript);
	RegFunc("MoveDownScript",       &MoveDownScript);
	RegFunc("SetCmd",               &SetCmd);
	RegFunc("GetUsers",             &GetUsers);
	RegFunc("AddTimer",             &AddTimer);
	RegFunc("DelTimer",             &DelTimer);
	RegFunc("GetConfig",            &getConfig);
	RegFunc("SetConfig",            &setConfig);
	RegFunc("GetLang",              &getLang);
	RegFunc("SetLang",              &setLang);
	RegFunc("Call",                 &Call);
	RegFunc("RegBot",               &regBot);
	RegFunc("UnregBot",             &unregBot);
	RegFunc("SetHubState",          &SetHubState);
	RegFunc("Redirect",             &redirect);

	RegStrField("sLuaPluginVersion", PLUGIN_NAME" "PLUGIN_VERSION);
	RegStrField("sHubVersion", LuaPlugin::mCurServer->getHubInfo().c_str());
	RegStrField("sMainDir", LuaPlugin::mCurServer->getMainDir().c_str());
	RegStrField("sScriptsDir", LuaPlugin::mCurLua->GetScriptsDir().c_str());
	RegStrField("sSystem", LuaPlugin::mCurServer->getSystemVersion().c_str());

	lua_setglobal(mL, "Core");

	CreateConfigMT();
	Config * config = (Config*)lua_newuserdata(mL, sizeof(Config));
	config->isExist = 1;
	luaL_getmetatable(mL, MT_CONFIG);
	lua_setmetatable(mL, -2);
	lua_setglobal(mL, "Config");

	LuaInterpreter * Old = LuaPlugin::mCurLua->mCurScript;
	LuaPlugin::mCurLua->mCurScript = this;
	#ifdef HAVE_LUA_5_1
		int iStatus = luaL_dofile(mL, (char *)(msPath + mName).c_str());
	#else
		int iStatus = lua_dofile(mL, (char *)(msPath + mName).c_str());
	#endif
	if (iStatus) {
		OnError("OnError", lua_tostring(mL, -1), true);
		LuaPlugin::mCurLua->mCurScript = Old;
		return iStatus;
	}
	CreateUserMT();
	CallFunc("OnStartup");
	LuaPlugin::mCurLua->mCurScript = Old;
	return 0;
}


/** On stop script */
int LuaInterpreter::Stop() {
	if (mL) {
		DelTmr();

		for (tBotList::iterator it = mBotList.begin(); it != mBotList.end(); ++it) {
			LuaPlugin::mCurServer->unregBot(*it);
		}
		mBotList.clear();

		CallFunc("OnExit");
		lua_close(mL);
		mbEnabled = false;
		mL = NULL;
		return 1;
	}
	mbEnabled = false;
	return 0;
}


void LuaInterpreter::RegFunc(const char * sFuncName, int (*fncptr)(lua_State *)) {
	lua_pushstring(mL, sFuncName);
	lua_pushcfunction(mL, fncptr);
	lua_rawset(mL, -3);
}

void LuaInterpreter::RegStrField(const char * sName, const char * sVal) {
	lua_pushstring(mL, sName);
	lua_pushstring(mL, sVal);
	lua_rawset(mL, -3);
}


/** 1 - lock! */
int LuaInterpreter::CallFunc(const char * sFunc) {
	tvCallParams::iterator it;
	lua_settop(mL, 0);
	int iBase = lua_gettop(mL);

	lua_pushliteral(mL, "_TRACEBACK");
	lua_rawget(mL, LUA_GLOBALSINDEX); // lua 5.1
	if (lua_isfunction(mL, -1)) {
		iBase = lua_gettop(mL);
	} else {
		lua_pop(mL, 1);
	}
	//lua_rawget(mL, LUA_ENVIRONINDEX); // lua 5.2
	//lua_insert(mL, iBase);

	lua_getglobal(mL, sFunc);
	if (lua_isnil(mL, -1)) { // function not exists
		for (it = mCallParams.begin(); it != mCallParams.end(); ++it) {
			delete (*it);
		}
		mCallParams.clear();
		lua_pop(mL, -1); // remove nil value
		lua_remove(mL, iBase); // remove _TRACEBACK
		return 0;
	}

	void ** userdata;
	for (it = mCallParams.begin(); it != mCallParams.end(); ++it) {
		switch ((*it)->type) {
			case LUA_TLIGHTUSERDATA :
				userdata = (void **) lua_newuserdata(mL, sizeof(void *));
				*userdata = (*it)->data;
				luaL_getmetatable(mL, MT_USER_CONN);
				lua_setmetatable(mL, -2);
				break;

			case LUA_TSTRING :
				lua_pushstring(mL, (char*)(*it)->data);
				break;

			case LUA_TBOOLEAN :
				lua_pushboolean(mL, int((*it)->num));
				break;

			case LUA_TNUMBER :
				lua_pushnumber(mL, (*it)->num);
				break;

			default :
				lua_pushnil(mL);
				break;

		}
		delete (*it);
	}
	const int iLen = mCallParams.size();
	mCallParams.clear();

	LuaInterpreter * Old = LuaPlugin::mCurLua->mCurScript;
	LuaPlugin::mCurLua->mCurScript = this;

	if (lua_pcall(mL, iLen, 1, iBase)) {
		if (!OnError(sFunc, lua_tostring(mL, -1))) {
			lua_pop(mL, 1);
			lua_remove(mL, iBase); // remove _TRACEBACK
		}
		LuaPlugin::mCurLua->mCurScript = Old;
		return 0;
	}

	int iVal = 0;
	if (lua_isboolean(mL, -1)) {
		iVal = ((lua_toboolean(mL, -1) == 0) ? 0 : 1);
	} else if(lua_isnumber(mL, -1)) {
		iVal = (int)lua_tonumber(mL, -1);
	}

	lua_pop(mL, 1);
	lua_remove(mL, iBase); // remove _TRACEBACK

	LuaPlugin::mCurLua->mCurScript = Old;
	return iVal;
}

bool LuaInterpreter::OnError(const char * sFunc, const char * sErrMsg, bool bStop) {
	bool bStoped = true;
	LuaPlugin::mCurLua->msLastError = sErrMsg;
	LogError(sErrMsg);
	if (strcmp(sFunc, "OnError")) {
		NewCallParam((void *) sErrMsg, LUA_TSTRING);
		bStoped = !CallFunc("OnError");
	}
	bStoped = bStoped || bStop;
	LuaPlugin::mCurLua->OnScriptError(this, mName.c_str(), sErrMsg, bStoped);
	if (bStoped) {
		return !LuaPlugin::mCurLua->StopScript(this, true);
	}
	return false;
}

void LuaInterpreter::Timer(int iId, const char * sFunc) {
	lua_getglobal(mL, sFunc);
	lua_pushnumber(mL, iId);
	LuaInterpreter * Old = LuaPlugin::mCurLua->mCurScript;
	LuaPlugin::mCurLua->mCurScript = this;
	if (lua_pcall(mL, 1, 0, 0)) {
		if (!OnError("OnTimer", lua_tostring(mL, -1), true)) {
			lua_pop(mL, 1);
		}
	}
	LuaPlugin::mCurLua->mCurScript = Old;
}

void LuaInterpreter::NewCallParam(void * Data, int iType) {
	sParam * Param = new sParam(Data, iType);
	mCallParams.push_back(Param);
}

void LuaInterpreter::NewCallParam(lua_Number Data, int iType) {
	sParam * Param = new sParam(Data, iType);
	mCallParams.push_back(Param);
}

void LuaInterpreter::CreateUserMT() {
	if (!luaL_newmetatable(mL, MT_USER_CONN)) {
		return;
	}

	lua_pushliteral(mL, "__index");
	lua_pushstring(mL, "userIndex");
	lua_pushcclosure(mL, Uid::userIndex, 1);
	lua_settable(mL, -3);

	lua_pushliteral(mL, "__newindex");
	lua_pushstring(mL, "userNewIndex");
	lua_pushcclosure(mL, Uid::userNewIndex, 1);
	lua_settable(mL, -3);

	lua_pushliteral(mL, "__tostring");
	lua_pushstring(mL, MT_USER_CONN);
	lua_pushcclosure(mL, Uid::uidToString, 1);
	lua_settable(mL, -3);

	lua_pushliteral(mL, "__metatable");
	lua_pushliteral(mL, "You're not allowed to get this metatable");
	lua_settable(mL, -3);

	lua_settop(mL, 0);
}

void LuaInterpreter::CreateConfigMT() {
	if (!luaL_newmetatable(mL, MT_CONFIG)) {
		return;
	}

	lua_pushliteral(mL, "__index");
	lua_pushstring(mL, "ConfigIndex");
	lua_pushcclosure(mL, ConfigIndex, 1);
	lua_settable(mL, -3);

	lua_pushliteral(mL, "__newindex");
	lua_pushstring(mL, "ConfigNewindex");
	lua_pushcclosure(mL, ConfigNewindex, 1);
	lua_settable(mL, -3);

	lua_pushliteral(mL, "__tostring");
	lua_pushstring(mL, MT_CONFIG);
	lua_pushcclosure(mL, ConfigTostring, 1);
	lua_settable(mL, -3);

	lua_pushliteral(mL, "__metatable");
	lua_pushliteral(mL, "You're not allowed to get this metatable");
	lua_settable(mL, -3);

	lua_settop(mL, 0);
}

}; // namespace luaplugin
