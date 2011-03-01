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

#ifndef DC_CONFIG_LOADER_H
#define DC_CONFIG_LOADER_H

#include "Obj.h"
#include "ConfigLoader.h"

#include <string>
#include <fstream> // for ifstream
#include <sstream> // for streams

using namespace ::configuration;
using namespace ::std;


namespace dcserver {



class DcConfigLoader : public Obj, public ConfigLoader {

public:

	DcConfigLoader();
	~DcConfigLoader();

	int load(ConfigListBase *, const ConfigStore &);
	int save(ConfigListBase *, const ConfigStore &);

	int loadFromFile(ConfigListBase *, const char * fileName);
	int saveToFile(ConfigListBase *, const char * fileName);
	int saveToStream(ConfigListBase *, ostream &);

	int loadFromXml(ConfigListBase *, const char * fileName);
	int saveToXml(ConfigListBase *, const char * fileName);

}; // class DcConfigLoader


}; // namespace dcserver

#endif // DC_CONFIG_LOADER_H
