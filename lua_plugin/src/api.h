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

#ifndef FUNCS_H
#define FUNCS_H

#include "Plugin.h" // ::dcserver
#include "lua/lua.hpp"


using namespace ::dcserver;

namespace luaplugin {


typedef enum {
	eUV_iProfile = 1,
	eUV_sMyINFO,
	eUV_sData,
	eUV_bOpList,
	eUV_bHide,
	eUV_bIpList
} tUserValue; /** tUserValue */


typedef struct {
	short isExist;
} Config;


DcConnBase * getDcConnBase(lua_State * L, int indx);


int ConfigTostring(lua_State * L);

int ConfigTable(lua_State * L);

int ConfigIndex(lua_State * L);

int ConfigNewindex(lua_State * L);


void LogError(const char * msg = NULL);

void CopyValue(lua_State * from, lua_State * to, int pos);


// GetGVal(sScriptName, sParam)
int GetGVal(lua_State * L);

// SetGVal(sScriptName, sParam, Value)
int SetGVal(lua_State * L);

// SendToUser(UID/sToNick, sData, sNick, sFrom)
int sendToUser(lua_State * L);

// SendToAll(sData, sNick, sFrom)
int sendToAll(lua_State * L);

// SendToAllExceptNicks(tExcept, sData, sNick, sFrom)
int sendToAllExceptNicks(lua_State * L);

// SendToAllExceptIps(tExcept, sData, sNick, sFrom)
int sendToAllExceptIps(lua_State * L);

// SendToProfile(iProfile/tProfiles, sData, sNick, sFrom)
int SendToProfile(lua_State * L);

// SendToIp(sIP, sData, sNick, sFrom, iProfile/tProfiles)
int sendToIp(lua_State * L);

// SendToNicks(tNicks, sData, sNick, sFrom)
int SendToNicks(lua_State * L);

// GetUser(UID/sNick, iByte)
int GetUser(lua_State * L);

// SetUser(UID/sNick, iType, Value)
int setUser(lua_State * L);

// GetUsers(sIP, iByte)
int GetUsers(lua_State * L);

// GetUsersCount()
int getUsersCount(lua_State * L);

// GetTotalShare()
int getTotalShare(lua_State * L);

// GetUpTime()
int getUpTime(lua_State * L);

// Disconnect(sNick/UID)
int disconnect(lua_State * L);

// DisconnectIP(sIP)
int DisconnectIP(lua_State * L);

// RestartScripts(iType)
int RestartScripts(lua_State * L);

// RestartScript(sScriptName)
int RestartScript(lua_State * L);

// StopScript(sScriptName)
int StopScript(lua_State * L);

// StartScript(sScriptName)
int StartScript(lua_State * L);

// GetScripts()
int GetScripts(lua_State * L);

// GetScript(sScriptName)
int GetScript(lua_State * L);

// MoveUpScript(sScriptName)
int MoveUpScript(lua_State * L);

// MoveDownScript(sScriptName)
int MoveDownScript(lua_State * L);

// SetCmd(sData)
int SetCmd(lua_State * L);

// AddTimer(iId, iInterval, sFunc)
int AddTimer(lua_State * L);

// DelTimer(iId)
int DelTimer(lua_State * L);

// GetConfig(sName)
int getConfig(lua_State * L);

// SetConfig(sName, sValue)
int setConfig(lua_State * L);

// GetLang(sName)
int getLang(lua_State * L);

// SetLang(sName, sValue)
int setLang(lua_State * L);

// Call(sScriptName, sFunc, sParam)
int Call(lua_State * L);

// RegBot(sNick, bKey, sMyINFO, sIP)
int regBot(lua_State * L);

// UnregBot(sNick)
int unregBot(lua_State * L);

// SetHubState(iNumber)
int SetHubState(lua_State * L);

// Redirect(UID/sNick, sAddress, [sReason])
int redirect(lua_State * L);


}; // namespace luaplugin

#endif // FUNCS_H

/**
* $Id$
* $HeadURL$
*/
