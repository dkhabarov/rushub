/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * Copyright (C) 2009-2010 by Setuper
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

#include "cmainpath.h"
#include "cdir.h"

namespace nDCServer {

cMainPath::cMainPath(const string & sConfPath, const string & sExPath) : 
	cObj("cMainPath"),
	msConfPath(sConfPath),
	msExPath(sExPath)
{
	// Config Path
	CheckEndSlash(msConfPath);
	CheckPath(msConfPath);

	// Logs Path
	msLogPath = msConfPath + "logs/";
	CheckPath(msLogPath);
	msPath = &msLogPath; // Set log point path

	// Plugins Path
	msPluginPath = msConfPath + "plugins/";
	CheckPath(msPluginPath);

	if(mOfs.is_open()) mOfs.close();
}

cMainPath::~cMainPath() {
	msPath = NULL;
}

} // nDCServer
