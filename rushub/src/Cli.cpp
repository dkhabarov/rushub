/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * Copyright (C) 2010 by Nikolay Bogdanov
 * E-Mail: admin at klan-hub dot ru (admin@klan-hub.ru)
 * Copyright (C) 2009-2013 by Setuper
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

#ifndef _WIN32

#include "Cli.h"
#include "Server.h"
#include "Logger.h"



Cli::Cli() : mDaemon(false), mSyslog(false) {
}



Cli::~Cli() {
}



const string & Cli::getConfigFile() const {
	return mConfigFile;
}



/** Code to parce cli arguments */
void Cli::detectArgs(int argc, char ** argv) {

	int next_option;

	/** Define arguments */
	const char * short_options = "hc:sc:c:vc:dc";
	struct option long_options[] = {
		{ "help",			0, NULL, 'h' },
		{ "daemon",		0, NULL, 'd' },
		{ "config",		1, NULL, 'c' },
		{ "syslog",		0, NULL, 's' },
		{ "version",	0, NULL, 'v' },
		{ NULL,				0, NULL, 0   }
	};

	/** Detect arguments */
	do {
		next_option = getopt_long(argc, argv, short_options, long_options, NULL);
		switch (next_option) {
			case 'h' :
				printUsage(stdout, EXIT_SUCCESS);
				break;

			case 'c' :
				mConfigFile = optarg;
				break;

			case 's' :
				Logger::getInstance()->mSysLogOn = true;
				break;

			case '?' :
				printUsage(stdout, EXIT_SUCCESS);
				break;

			case -1 :
				break;

			case 'd' :
				mDaemon = true;
				break;

			case 'v' :
				printVersion(stdout, EXIT_SUCCESS);
				break;

			default :
				exit(EXIT_SUCCESS);

		}
	} while (next_option != -1);
}



/** Code for demonization */
pid_t Cli::demonizeServer() {

	/** Create new process */
	pid_t pid = 0;

	if (mDaemon) {
		pid = fork();

		/** Checking pid creation */
		if (pid < 0) {
			fprintf(stderr, "Cannot create process %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}

		/** Detaching from terminal */
		setsid();

		/** Closing parent process */
		if (pid > 0) {
			fprintf(stdout, "Process created with PID: %d\n", pid);
			exit(EXIT_SUCCESS);
		}

		/** Reopen standart streams */
		if (freopen("/dev/null", "r", stdin) == NULL || freopen("/dev/null", "w", stdout) == NULL || freopen("/dev/null", "w", stderr) == NULL) {
			fprintf(stderr, "Unable to detach from terminal\n");
			exit(EXIT_FAILURE);
		}
	}
	return pid;
}



void Cli::printUsage(FILE * stream, int exitStatus) {
	fprintf(stream, "This is help for Rushub.\n\t-h,\t-help\t Show this text\n\t-v,\t-version Show application version\n\t-d,\t-daemon\t Run application in daemon-mode\n\t-c,\t-config\t Setup config file\n\t-s,\t-syslog\t Use syslog facility for logging\n");
	exit(exitStatus);
}



void Cli::printVersion(FILE * stream, int exitStatus) {
	fprintf(stream, "You running  " INTERNALNAME " " INTERNALVERSION ".\nType -help (-h) to see more arguments.\n");
	exit(exitStatus);
}


#endif // _WIN32

/**
 * $Id$
 * $HeadURL$
 */
