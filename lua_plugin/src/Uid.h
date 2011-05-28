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

#ifndef UID_H
#define UID_H

#include "api.h"

#define MT_USER_CONN "User object"

#define ERR_TYPEMETA(NARG, OBJ, SARG) { \
	return luaL_error(L, "bad argument #%d to " LUA_QS " (%s expected, got %s)", NARG, OBJ, SARG, luaL_typename(L, NARG)); \
}

namespace luaplugin {

enum ParamHash { /** Param's hash */

	PARAM_HASH_IP         = 90709,      //< "sIP"
	PARAM_HASH_UID        = 92355,      //< "UID"
	PARAM_HASH_TAG        = 4321573,    //< "sTag"
	PARAM_HASH_IPCONN     = 73645412,   //< "sIPConn"
	PARAM_HASH_KICK       = 136610353,  //< "bKick"
	PARAM_HASH_NICK       = 136635514,  //< "sNick"
	PARAM_HASH_CONN       = 137104726,  //< "sConn"
	PARAM_HASH_OPEN       = 137319447,  //< "iOpen"
	PARAM_HASH_HIDE       = 141759876,  //< "bHide"
	PARAM_HASH_MODE       = 141762382,  //< "sMode"
	PARAM_HASH_BYTE       = 142044234,  //< "iByte"
	PARAM_HASH_DATA       = 143784544,  //< "sData"
	PARAM_HASH_PORT       = 148797436,  //< "iPort"
	PARAM_HASH_DESC       = 149333075,  //< "sDesc"
	PARAM_HASH_FRACTION   = 227502190,  //< "sFraction"
	PARAM_HASH_REGHUBS    = 376863999,  //< "iRegHubs"
	PARAM_HASH_INUSERLIST = 353254788,  //< "bInUserList"
	PARAM_HASH_REDIRECT   = 355925330,  //< "bRedirect"
	PARAM_HASH_ENTERTIME  = 381948915,  //< "iEnterTime"
	PARAM_HASH_SUPPORTS   = 404020597,  //< "sSupports"
	PARAM_HASH_PORTCONN   = 491785156,  //< "iPortConn"
	PARAM_HASH_INOPLIST   = 549773767,  //< "bInOpList"
	PARAM_HASH_INIPLIST   = 549865865,  //< "bInIpList"
	PARAM_HASH_BANDWIDTH  = 568949422,  //< "iBandwidth"
	PARAM_HASH_SHARE      = 632724080,  //< "iShare"
	PARAM_HASH_CLIENTV    = 885079029,  //< "sClientVersion"
	PARAM_HASH_PROFILE    = 970292741,  //< "iProfile"
	PARAM_HASH_SLOTS      = 1035743627, //< "iSlots"
	PARAM_HASH_VERSION    = 1055480429, //< "sVersion"
	PARAM_HASH_EMAIL      = 1106490266, //< "sEmail"	
	PARAM_HASH_DOWNLOAD   = 1180934250, //< "iDownload"
	PARAM_HASH_LIMIT      = 1231291461, //< "iLimit"
	PARAM_HASH_MACADDRESS = 1643104854, //< "sMacAddress"
	PARAM_HASH_MYINFO     = 1652447804, //< "sMyINFO"
	PARAM_HASH_CLIENTN    = 1719639115, //< "sClientName"
	PARAM_HASH_USHUBS     = 1868311651, //< "iUsHubs"
	PARAM_HASH_OPHUBS     = 1868312482, //< "iOpHubs"

};

class Uid {

public:

	static void createMetaTable(lua_State *);

private:

	static int uidToString(lua_State *);
	static int userIndex(lua_State *);
	static int userNewIndex(lua_State *);

	static unsigned int getHash(const char *);

}; // class Uid

}; // namespace luaplugin

#endif // UID_H

/**
 * $Id$
 * $HeadURL$
 */
