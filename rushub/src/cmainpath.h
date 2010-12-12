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

#ifndef CMAINPATH_H
#define CMAINPATH_H

#include "cobj.h"
#include "cdcconn.h"

namespace nDCServer {

class cMainPath : public cObj {

	string msConfPath;
	string msExPath;
	string msLogPath;
	string msPluginPath;

public:

	cMainPath(const string & sConfPath, const string & sExPath);
	~cMainPath();

	const string & GetConfPath() const { return msConfPath; }
	const string & GetExPath() const { return msExPath; }
	const string & GetLogPath() const { return msLogPath; }
	const string & GetPluginPath() const { return msPluginPath; }

	void SetConfPath(const string & sNewConfPath) { msConfPath = sNewConfPath; }
	void SetExPath(const string & sNewExPath) { msExPath = sNewExPath; }
	void SetLogsPath(const string & sNewLogPath) { msLogPath = sNewLogPath; }
	void SetPluginsPath(const string & sNewPluginPath) { msPluginPath = sNewPluginPath; }
	
}; // cMainPath

}; // nDCServer

#endif // CMAINPATH_H
