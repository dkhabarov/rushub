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

#ifndef FUNCS_H
#define FUNCS_H

#include "Plugin.h" // ::dcserver
#include "lua/lua.hpp"


using namespace ::dcserver;

namespace luaplugin {


typedef enum {
	USERVALUE_PROFILE = 1,
	USERVALUE_MYINFO,
	USERVALUE_DATA,
	USERVALUE_OPLIST,
	USERVALUE_HIDE,
	USERVALUE_IPLIST
} UserValue; /** UserValue */


typedef struct {
	short isExist;
} Config;


DcConnBase * getDcConnBase(lua_State * L, int indx);


int configTostring(lua_State * L);

int configTable(lua_State * L);

int configIndex(lua_State * L);

int configNewindex(lua_State * L);


void logError(const char * msg = NULL);

void copyValue(lua_State * from, lua_State * to, int pos);


// GetGVal(sScriptName, sParam)
int getGVal(lua_State * L);

// SetGVal(sScriptName, sParam, Value)
int setGVal(lua_State * L);

// SendToUser(UID/sToNick, sData, sNick, sFrom)
int sendToUser(lua_State * L);

// SendToAll(sData, sNick, sFrom)
int sendToAll(lua_State * L);

// SendToAllExceptNicks(tExcept, sData, sNick, sFrom)
int sendToAllExceptNicks(lua_State * L);

// SendToAllExceptIps(tExcept, sData, sNick, sFrom)
int sendToAllExceptIps(lua_State * L);

// SendToProfile(iProfile/tProfiles, sData, sNick, sFrom)
int sendToProfile(lua_State * L);

// SendToIp(sIP, sData, sNick, sFrom, iProfile/tProfiles)
int sendToIp(lua_State * L);

// SendToNicks(tNicks, sData, sNick, sFrom)
int sendToNicks(lua_State * L);

// GetUser(UID/sNick, iByte)
int getUser(lua_State * L);

// SetUser(UID/sNick, iType, Value)
int setUser(lua_State * L);

// GetUsers(sIP, iByte)
int getUsers(lua_State * L);

// GetUsersCount()
int getUsersCount(lua_State * L);

// GetTotalShare()
int getTotalShare(lua_State * L);

// GetUpTime()
int getUpTime(lua_State * L);

// Disconnect(sNick/UID)
int disconnect(lua_State * L);

// DisconnectIP(sIP)
int disconnectIp(lua_State * L);

// RestartScripts(iType)
int restartScripts(lua_State * L);

// RestartScript(sScriptName)
int restartScript(lua_State * L);

// StopScript(sScriptName)
int stopScript(lua_State * L);

// StartScript(sScriptName)
int startScript(lua_State * L);

// GetScripts()
int getScripts(lua_State * L);

// GetScript(sScriptName)
int getScript(lua_State * L);

// MoveUpScript(sScriptName)
int moveUpScript(lua_State * L);

// MoveDownScript(sScriptName)
int moveDownScript(lua_State * L);

// SetCmd(sData)
int setCmd(lua_State * L);

// AddTimer(iId, iInterval, sFunc)
int addTimer(lua_State * L);

// DelTimer(iId)
int delTimer(lua_State * L);

// GetConfig(sName)
int getConfig(lua_State * L);

// SetConfig(sName, sValue)
int setConfig(lua_State * L);

// GetLang(sName)
int getLang(lua_State * L);

// SetLang(sName, sValue)
int setLang(lua_State * L);

// Call(sScriptName, sFunc, sParam)
int call(lua_State * L);

// RegBot(sNick, bKey, sMyINFO, sIP)
int regBot(lua_State * L);

// UnregBot(sNick)
int unregBot(lua_State * L);

// SetHubState(iNumber)
int setHubState(lua_State * L);

// Redirect(UID/sNick, sAddress, [sReason])
int redirect(lua_State * L);


}; // namespace luaplugin

#endif // FUNCS_H

/**
 * $Id$
 * $HeadURL$
 */
