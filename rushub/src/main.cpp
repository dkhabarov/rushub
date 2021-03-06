/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz
 *
 * modified: 27 Aug 2009
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

#include "DcServer.h" // first (def winsock2.h)
#include "Service.h"
#include "Dir.h"

#include <signal.h>

// Visual Leak Detector
#ifdef VLD
	#include <vld.h>
#endif


#ifdef _WIN32
	#define SIGQUIT 1000
	#define SIGHUP 1001
	#define SIGTSTP 1002
	#include "Exception.h"
#else
	#include "Cli.h"
#endif

using ::dcserver::DcServer;
using namespace ::server;

static void sigHandler(int sig) {

	switch (sig) {

		case SIGINT :
			// Fallthrough

		case SIGTERM :
			// Fallthrough

		case SIGQUIT :
			// Fallthrough

		case SIGTSTP :
			// Fallthrough

		case SIGHUP :
			LOG_CLASS(DcServer::currentDcServer, LEVEL_INFO, "Received a " << sig << " signal, quiting");
			cout << "Received a " << sig << " signal, quiting" << endl;
			DcServer::currentDcServer->Server::stop(0);
			signal(sig, sigHandler);
			break;

		default :
			LOG_CLASS(DcServer::currentDcServer, LEVEL_INFO, "Received a " << sig << " signal, ignoring it");
			signal(sig, sigHandler);
			break;
	}
}

int runHub(int argc, char ** argv, bool isService /*= false*/) {

	int result = 1;

	#ifndef _WIN32
		Cli cli;
		cli.detectArgs(argc, argv);
		isService = false; // for used
		umask(077); // Setting umask for daemon
	#endif

	while (result == 1) {

		string configFile;
		#ifndef _WIN32
			if (cli.getConfigFile().empty()) {
				Dir::pathForFile(argv[0], configFile);
				configFile.append(STR_LEN("RusHub.xml"));
			} else {
				configFile = cli.getConfigFile();
			}
			if (cli.getDaemon()) {
				cli.demonizeServer();
			}
		#else
			Service service;
			Dir::execPath(configFile);
			configFile.append(STR_LEN("RusHub.xml"));
			if (service.cli(argc, argv, configFile) <= 0) {
				return -1;
			}
		#endif

		/** Creating the server */
		DcServer server(configFile, argv[0]);

		/** Listening all ports */
		if (server.listening() != 0) {
			LOG_CLASS(&server, LEVEL_FATAL, "Listening failed");
			return -2;
		}

		#ifdef _WIN32
			if (isService && service.start() < 0) {
				return -3;
			}
		#endif

		result = server.run(); // 1 = restart

		#ifdef _WIN32
			if (isService && service.stop() < 0) {
				return -4;
			}
		#endif

	}
	return result;
}

int main(int argc, char ** argv) {

	signal(SIGINT, sigHandler);
	signal(SIGTERM, sigHandler);
	#ifndef _WIN32
		signal(SIGIO, sigHandler);
		signal(SIGHUP, sigHandler);
		signal(SIGQUIT, sigHandler);
		signal(SIGPIPE, sigHandler);
		signal(SIGTSTP, sigHandler);
	#else
		SetUnhandledExceptionFilter(&Exception::exceptionFilter);
	#endif

	return runHub(argc, argv);
}

/**
 * $Id$
 * $HeadURL$
 */
