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

#ifndef CLOADER_H
#define CLOADER_H

#include "cobj.h"

#include <string>
#include <fstream> /** for ifstream is */
#include <sstream> /** for streams */

namespace nConfig {

class cConfigListBase;

class cConfigLoader : public cObj {

public:

	cConfigLoader();
	~cConfigLoader();

	bool Load(cConfigListBase *, const char*);
	bool Save(cConfigListBase *, const char*);

	bool LoadFromFile();
	bool SaveToFile();
	bool SaveToStream(ostream &);

	bool LoadFromXml();
	bool SaveToXml();

private:
	cConfigListBase * mConfigList;
	string msFileName;

}; // cConfigLoader

}; // nConfig

#endif // CLOADER_H
