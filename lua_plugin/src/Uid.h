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

	PARAM_HASH_UID     = 92355,      ///< "UID"
	PARAM_HASH_TAG     = 4321573,    ///< "sTag"
	PARAM_HASH_NICK    = 136635514,  ///< "sNick"
	PARAM_HASH_MYINFO  = 1652447804, ///< "sMyINFO"
	PARAM_HASH_PROFILE = 970292741,  ///< "iProfile"

};

class Uid {

public:

	static void createMetaTable(lua_State *);

	static void pushValue(lua_State *, DcUserBase *, const char * name);
	static int setValue(lua_State *, DcUserBase *, const char * name, int type);

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
