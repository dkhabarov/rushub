/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * Copyright (C) 2010 by Nikolay Bogdanov
 * E-Mail: admin at klan-hub dot ru (admin@klan-hub.ru)
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

#ifndef CCLI_H
#define CCLI_H

#ifndef _WIN32

#include "cdcserver.h"

#include <getopt.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

class cCli {

private:

	bool bDaemon;
	bool bSyslog;
	string sMainDir;

public:

	cCli();
	~cCli();
	void printUsage(FILE *stream, int exitStatus);
	void printVersion(FILE *stream, int exitStatus);
	bool getDaemon() const { return bDaemon; }
	bool getSyslog() const { return bSyslog; }
	const string & getMainDir() const { return sMainDir; }
	pid_t demonizeServer(string sMainDir);
	void detectArgs(int argc, char **argv);

}; // cCli

#endif // _WIN32

#endif // CCLI_H
