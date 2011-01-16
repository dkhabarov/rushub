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

#ifndef UID_H
#define UID_H

#include "api.h"

#define MT_USER_CONN "User object"

#define ERR_TYPEMETA(NARG, OBJ, SARG) { \
	return luaL_error(L, "bad argument #%d to " LUA_QS " (%s expected, got %s)", NARG, OBJ, SARG, luaL_typename(L, NARG)); \
}

namespace nLua {

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

int UidTostring(lua_State *L);
int UserIndex(lua_State *L);
int UserNewindex(lua_State *L);

}; // nLua

#endif // UID_H
