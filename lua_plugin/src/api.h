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

#ifndef CFUNCS_H
#define CFUNCS_H

#include "lua/lua.hpp"
#include "cplugin.h" /** nDCServer */

using namespace ::nDCServer;

namespace nLua {

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
} cConfig;

cDCConnBase * GetDCConnBase(lua_State *L, int indx);

int ConfigTostring(lua_State *L);
int ConfigTable(lua_State *L);
int ConfigIndex(lua_State *L);
int ConfigNewindex(lua_State *L);

void LogError(const char *sMsg = NULL);
void CopyValue(lua_State *From, lua_State *To, int iPos);

int GetGVal(lua_State *L);              // GetGVal(sScriptName, sParam)
int SetGVal(lua_State *L);              // SetGVal(sScriptName, sParam, Value)
int SendToUser(lua_State *L);           // SendToUser(UID/sToNick, sData, sNick, sFrom)
int SendToAll(lua_State *L);            // SendToAll(sData, sNick, sFrom)
int SendToAllExceptNicks(lua_State *L); // SendToAllExceptNicks(tExcept, sData, sNick, sFrom)
int SendToAllExceptIps(lua_State *L);   // SendToAllExceptIps(tExcept, sData, sNick, sFrom)
int SendToProfile(lua_State *L);        // SendToProfile(iProfile/tProfiles, sData, sNick, sFrom)
int SendToIP(lua_State *L);             // SendToIP(sIP, sData, sNick, sFrom, iProfile/tProfiles)
int SendToNicks(lua_State *L);          // SendToNicks(tNicks, sData, sNick, sFrom)
int GetUser(lua_State *L);              // GetUser(UID/sNick, iByte)
int SetUser(lua_State *L);              // SetUser(UID/sNick, iType, Value)
int GetUsers(lua_State *L);             // GetUsers(sIP, iByte)
int GetUsersCount(lua_State *L);        // GetUsersCount()
int GetTotalShare(lua_State *L);        // GetTotalShare()
int GetUpTime(lua_State *L);            // GetUpTime()
int disconnect(lua_State *L);           // Disconnect(sNick/UID)
int DisconnectIP(lua_State *L);         // DisconnectIP(sIP)
int RestartScripts(lua_State *L);       // RestartScripts(iType)
int RestartScript(lua_State *L);        // RestartScript(sScriptName)
int StopScript(lua_State *L);           // StopScript(sScriptName)
int StartScript(lua_State *L);          // StartScript(sScriptName)
int GetScripts(lua_State *L);           // GetScripts()
int GetScript(lua_State *L);            // GetScript(sScriptName)
int MoveUpScript(lua_State *L);         // MoveUpScript(sScriptName)
int MoveDownScript(lua_State *L);       // MoveDownScript(sScriptName)
int SetCmd(lua_State *L);               // SetCmd(sData)
int AddTimer(lua_State *L);             // AddTimer(iId, iInterval, sFunc)
int DelTimer(lua_State *L);             // DelTimer(iId)
int GetConfig(lua_State *L);            // GetConfig(sName)
int SetConfig(lua_State *L);            // SetConfig(sName, sValue)
int GetLang(lua_State *L);              // GetLang(sName)
int SetLang(lua_State *L);              // SetLang(sName, sValue)
int Call(lua_State *L);                 // Call(sScriptName, sFunc, sParam)
int RegBot(lua_State *L);               // RegBot(sNick, bKey, sMyINFO, sIP)
int UnregBot(lua_State *L);             // UnregBot(sNick)
int SetHubState(lua_State *L);          // SetHubState(iNumber)
int Redirect(lua_State *L);             // Redirect(UID/sNick, sAddress, [sReason])

}; // nLua

#endif // CFUNCS_H
