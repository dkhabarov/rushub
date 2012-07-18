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

#ifndef HUB_CONFIG_H
#define HUB_CONFIG_H

#include "LuaInterpreter.h"

namespace luaplugin {


typedef struct {
	short isExist;
} Config;


class HubConfig {

public:

	void createMetaTable(lua_State *);

private:

	static int configTostring(lua_State *);
	static int configTable(lua_State *);
	static int configIndex(lua_State *);
	static int configNewindex(lua_State *);

}; // class HubConfig

} // namespace luaplugin

#endif // HUB_CONFIG_H

/**
 * $Id$
 * $HeadURL$
 */
