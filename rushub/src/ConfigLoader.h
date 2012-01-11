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

#ifndef CONFIG_LOADER_H
#define CONFIG_LOADER_H

#include <string>

using ::std::string;

namespace configuration {



/// Class which contain name and path of configuration
class ConfigStore {

public:
	string mName;
	string mPath;

};

class ConfigListBase;



/// Abstract class for load/save configuration
class ConfigLoader {

public:

	virtual int load(ConfigListBase *, const ConfigStore &) = 0;
	virtual int save(ConfigListBase *, const ConfigStore &) = 0;

}; // class ConfigLoader

}; // namespace configuration

#endif // CONFIG_LOADER_H

/**
 * $Id$
 * $HeadURL$
 */
