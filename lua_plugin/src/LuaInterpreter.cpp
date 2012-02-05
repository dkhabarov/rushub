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

#include "LuaInterpreter.h"
#include "LuaPlugin.h" // for dirs
#include "Uid.h"
#include "Dir.h"

#include <string.h>
#include <fstream>

#define HAVE_LUA_5_1

using namespace ::std;
using namespace ::utils;


namespace luaplugin {


LuaInterpreter::LuaInterpreter(const string & name, string & path) : 
	mName(name),
	mPath(path),
	mL(NULL),
	mEnabled(false)
{
}



LuaInterpreter::~LuaInterpreter() {
	stop();
}



/** On start script (-1 - run already)*/
int LuaInterpreter::start() {
	if (mL) {
		return -1;
	}
	mEnabled = true;
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

	if (!LuaPlugin::mCurLua->mSetLuaPath) {
		size_t len = 0;

		lua_pushliteral(mL, "path");
		lua_rawget(mL, -2);

		const char * path = lua_tolstring(mL, -1, &len);
		LuaPlugin::mCurLua->mLuaPath.append(path, len);
		lua_pop(mL, 1);

		lua_pushliteral(mL, "cpath");
		lua_rawget(mL, -2);

		path = lua_tolstring(mL, -1, &len);
		LuaPlugin::mCurLua->mLuaCPath.append(path, len);
		lua_pop(mL, 1);

		LuaPlugin::mCurLua->mSetLuaPath = true;
	}

	lua_pushliteral(mL, "path");
	lua_pushstring(mL, LuaPlugin::mCurLua->mLuaPath.c_str());
	lua_settable(mL, -3);

	lua_pushliteral(mL, "cpath");
	lua_pushstring(mL, LuaPlugin::mCurLua->mLuaCPath.c_str());
	lua_settable(mL, -3);

	lua_settop(mL, 0);


	lua_newtable(mL);
	regFunc("GetGVal",              &getGVal);
	regFunc("SetGVal",              &setGVal);
	regFunc("SendToUser",           &sendToUser);
	regFunc("SendToAll",            &sendToAll);
	regFunc("SendToAllExceptNicks", &sendToAllExceptNicks);
	regFunc("SendToAllExceptIPs",   &sendToAllExceptIps);
	regFunc("SendToProfile",        &sendToProfile);
	regFunc("SendToIP",             &sendToIp);
	regFunc("SendToNicks",          &sendToNicks);
	regFunc("GetUser",              &getUser);
	regFunc("SetUser",              &setUser);
	regFunc("GetUsersCount",        &getUsersCount);
	regFunc("GetTotalShare",        &getTotalShare);
	regFunc("GetUpTime",            &getUpTime);
	regFunc("Disconnect",           &disconnect);
	regFunc("DisconnectIP",         &disconnectIp);
	regFunc("RestartScripts",       &restartScripts);
	regFunc("RestartScript",        &restartScript);
	regFunc("StopScript",           &stopScript);
	regFunc("StartScript",          &startScript);
	regFunc("GetScripts",           &getScripts);
	regFunc("GetScript",            &getScript);
	regFunc("MoveUpScript",         &moveUpScript);
	regFunc("MoveDownScript",       &moveDownScript);
	regFunc("SetCmd",               &setCmd);
	regFunc("GetUsers",             &getUsers);
	regFunc("AddTimer",             &addTimer);
	regFunc("DelTimer",             &delTimer);
	regFunc("GetConfig",            &getConfig);
	regFunc("SetConfig",            &setConfig);
	regFunc("GetLang",              &getLang);
	regFunc("SetLang",              &setLang);
	regFunc("Call",                 &call);
	regFunc("RegBot",               &regBot);
	regFunc("UnregBot",             &unregBot);
	regFunc("SetHubState",          &setHubState);
	regFunc("Redirect",             &redirect);

	regStrField("sLuaPluginVersion", PLUGIN_NAME " " PLUGIN_VERSION);
	regStrField("sHubVersion", LuaPlugin::mCurServer->getHubInfo().c_str());
	regStrField("sMainDir", LuaPlugin::mCurServer->getMainDir().c_str());
	regStrField("sScriptsDir", LuaPlugin::mCurLua->getScriptsDir().c_str());
	regStrField("sSystem", LuaPlugin::mCurServer->getSystemVersion().c_str());

	lua_setglobal(mL, "Core");

	// Creating Config metatable
	LuaPlugin::mCurLua->mHubConfig.createMetaTable(mL);

	// Creating UID metatable
	Uid::createMetaTable(mL);


	LuaInterpreter * oldScript = LuaPlugin::mCurLua->mCurScript;
	LuaPlugin::mCurLua->mCurScript = this;
	#ifdef HAVE_LUA_5_1
		int iStatus = luaL_dofile(mL, (mPath + mName).c_str());
	#else
		int iStatus = lua_dofile(mL, (mPath + mName).c_str());
	#endif
	if (iStatus) {
		onError("OnError", lua_tostring(mL, -1), true);
		LuaPlugin::mCurLua->mCurScript = oldScript;
		return iStatus;
	}
	callFunc("OnStartup");
	LuaPlugin::mCurLua->mCurScript = oldScript;
	return 0;
}



/** On stop script */
int LuaInterpreter::stop() {
	if (mL) {
		callFunc("OnExit");

		delTmr();

		for (BotList::iterator it = mBotList.begin(); it != mBotList.end(); ++it) {
			LuaPlugin::mCurServer->unregBot(*it);
		}
		mBotList.clear();

		lua_close(mL);
		mEnabled = false;
		mL = NULL;
		return 1;
	}
	mEnabled = false;
	return 0;
}


bool LuaInterpreter::testFunc(const char * funcName) {
	lua_getglobal(mL, funcName);
	bool ret = lua_isnil(mL, -1);
	lua_pop(mL, -1);
	return !ret;
}

/** 1 - lock! */
int LuaInterpreter::callFunc(const char * funcName) {
	lua_settop(mL, 0); // clear stack
	int base = lua_gettop(mL);
	int traceback = base;

	lua_pushliteral(mL, "_TRACEBACK");
	lua_rawget(mL, LUA_GLOBALSINDEX); // lua 5.1
	//lua_rawget(mL, LUA_ENVIRONINDEX); // lua 5.2

	if (lua_isfunction(mL, -1)) {
		traceback = lua_gettop(mL);
	} else {
		lua_pop(mL, 1); // remove _TRACEBACK
	}
	

	lua_getglobal(mL, funcName);
	if (lua_isnil(mL, -1)) { // function not exists
		for (CallParams::iterator it = mCallParams.begin(); it != mCallParams.end(); ++it) {
			delete (*it);
		}
		mCallParams.clear();
		lua_settop(mL, base); // clear stack
		return 0;
	}

	void ** userdata = NULL;
	for (CallParams::iterator it = mCallParams.begin(); it != mCallParams.end(); ++it) {
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
	const int len = mCallParams.size();
	mCallParams.clear();

	LuaInterpreter * oldScript = LuaPlugin::mCurLua->mCurScript;
	LuaPlugin::mCurLua->mCurScript = this;

	if (lua_pcall(mL, len, 1, traceback)) {
		const char * error = lua_tostring(mL, -1);
		lua_settop(mL, base); // clear stack
		onError(funcName, error);
		LuaPlugin::mCurLua->mCurScript = oldScript;
		return 0;
	}

	int ret = 0;
	if (lua_isboolean(mL, -1)) {
		ret = ((lua_toboolean(mL, -1) == 0) ? 0 : 1);
	} else if(lua_isnumber(mL, -1)) {
		ret = (int)lua_tonumber(mL, -1);
	}

	lua_settop(mL, base); // clear stack
	LuaPlugin::mCurLua->mCurScript = oldScript;
	return ret;
}



bool LuaInterpreter::onError(const char * funcName, const char * errMsg, bool stop) {
	bool stoped = true;
	if (errMsg == NULL) {
		errMsg = "unknown LUA error";
	}
	LuaPlugin::mCurLua->mLastError = errMsg;
	logError(LuaPlugin::mCurLua->mLastError);
	if (strcmp(funcName, "OnError")) {
		newCallParam((void *) errMsg, LUA_TSTRING);
		stoped = !callFunc("OnError");
	}
	stoped = stoped || stop;
	LuaPlugin::mCurLua->onScriptError(this, mName.c_str(), errMsg, stoped);
	if (stoped) {
		return !LuaPlugin::mCurLua->stopScript(this, true);
	}
	return false;
}



int LuaInterpreter::addTmr(Timer * timer) {
	if (LuaPlugin::mCurLua->getListFlag(LIST_TIMER)) {
		LuaPlugin::mCurLua->setListFlag(LuaPlugin::mCurLua->getListFlags() - LIST_TIMER);
	}
	return mTimerList.addTimer(timer);
}



int LuaInterpreter::delTmr(int tm) {
	if (LuaPlugin::mCurLua->getListFlag(LIST_TIMER)) {
		LuaPlugin::mCurLua->setListFlag(LuaPlugin::mCurLua->getListFlags() - LIST_TIMER);
	}
	return mTimerList.delTimer(tm);
}



void LuaInterpreter::delTmr() {
	if (LuaPlugin::mCurLua->getListFlag(LIST_TIMER)) {
		LuaPlugin::mCurLua->setListFlag(LuaPlugin::mCurLua->getListFlags() - LIST_TIMER);
	}
	mTimerList.delTimer();
}


void LuaInterpreter::timer(int id, const char * funcName) {
	lua_getglobal(mL, funcName);
	lua_pushnumber(mL, id);
	LuaInterpreter * oldScript = LuaPlugin::mCurLua->mCurScript;
	LuaPlugin::mCurLua->mCurScript = this;
	if (lua_pcall(mL, 1, 0, 0)) {
		if (!onError("OnTimer", lua_tostring(mL, -1), true)) {
			lua_pop(mL, 1);
		}
	}
	LuaPlugin::mCurLua->mCurScript = oldScript;
}



void LuaInterpreter::newCallParam(void * data, int type) {
	Param * param = new Param(data, type);
	mCallParams.push_back(param);
}



void LuaInterpreter::newCallParam(lua_Number data, int type) {
	Param * param = new Param(data, type);
	mCallParams.push_back(param);
}



void LuaInterpreter::logError(const string & msg) {
	string log(LuaPlugin::mCurServer->getLogDir());
	Dir::checkPath(log);
	log.append("lua_errors.log", 14);

	ofstream ofs(log.c_str(), ios_base::app);
	ofs << "[" << LuaPlugin::mCurServer->getTime() << "] " << msg << endl;

	ofs.flush();
	ofs.close();
}



void LuaInterpreter::regFunc(const char * funcName, int (*fncptr)(lua_State *)) {
	lua_pushstring(mL, funcName);
	lua_pushcfunction(mL, fncptr);
	lua_rawset(mL, -3);
}



void LuaInterpreter::regStrField(const char * name, const char * value) {
	lua_pushstring(mL, name);
	lua_pushstring(mL, value);
	lua_rawset(mL, -3);
}


}; // namespace luaplugin

/**
 * $Id$
 * $HeadURL$
 */
