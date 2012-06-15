/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * Copyright (C) 2010 by Nikolay Bogdanov
 * E-Mail: admin at klan-hub dot ru (admin@klan-hub.ru)
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

#ifndef CLI_H
#define CLI_H

#ifndef _WIN32

#include "DcServer.h"

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


/// Command Line Interface for unix
class Cli {

private:

	bool mDaemon;
	bool mSyslog;
	string mConfigFile;

public:

	Cli();
	~Cli();

	inline bool getDaemon() const {
		return mDaemon;
	}
	
	inline bool getSyslog() const {
		return mSyslog;
	}
	
	const string & getConfigFile() const;

	void detectArgs(int argc, char ** argv);
	pid_t demonizeServer();

	void printUsage(FILE * stream, int exitStatus);
	void printVersion(FILE * stream, int exitStatus);

}; // class Cli

#endif // _WIN32

#endif // CLI_H

/**
 * $Id$
 * $HeadURL$
 */
