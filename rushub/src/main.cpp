/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz

 * modified: 27 Aug 2009
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

#include "cdcserver.h"
#include "cdir.h"
#include "cservice.h"

#include <signal.h>

#ifdef _WIN32
	#define SIGQUIT 1000
	#define SIGHUP 1001
	#ifdef MODULAR
		#pragma comment(lib, "config.lib")
		#pragma comment(lib, "utils.lib")
		#pragma comment(lib, "server.lib")
		#pragma comment(lib, "webserver.lib")
		#pragma comment(lib, "plugin.lib")
		#pragma comment(lib, "tinyxml.lib")
	#endif
	#include "cexception.h"
#else
	#include "ccli.h"
#endif

using nDCServer::cDCServer;
using namespace nServer;

static void SigHandler(int iSig) {
	switch(iSig) {
		case SIGINT:
		case SIGTERM:
		case SIGQUIT:
		case SIGHUP:

			if(cDCServer::sCurrentServer->Log(0))
				cDCServer::sCurrentServer->LogStream() << "Received a " << iSig << " signal, quiting" << endl;

			cout << "Received a " << iSig << " signal, quiting" << endl;

			cDCServer::sCurrentServer->Stop(0);
			break;
		default:

			if(cDCServer::sCurrentServer->Log(0))
				cDCServer::sCurrentServer->LogStream() << "Received a " << iSig << " signal, ignoring it" << endl;

			signal(iSig, SigHandler);
			break;
	}
}

int runHub(int argc, char **argv, bool bService /*= false*/) {

	int iResult = 1;

	#ifndef _WIN32
		cCli Cli;
		Cli.detectArgs(argc, argv);
		bService = false; // for used
	#endif

	while(iResult == 1) {

		string sConfPath, sExPath;
		#ifndef _WIN32
			if(Cli.getMainDir().empty())
				ExecPath(sConfPath);
			else
				sConfPath = Cli.getMainDir();

			sExPath = sConfPath;
			if(Cli.getDaemon()) Cli.demonizeServer(sConfPath);
			if((chdir(sConfPath.c_str())) < 0) {
				fprintf(stderr, "Can not go to the work directory %s. Error: %s\n", sConfPath.c_str(), strerror(errno));
				return -1;
			}
		#else
			ExecPath(sConfPath);
			sExPath = sConfPath;

			cService Service;
			if(Service.Cli(argc, argv, sConfPath, sExPath) <= 0) return -1;
		#endif

		/** Creating the server */
		cDCServer Server(sConfPath, sExPath);

		/** Listening all ports */
		if(Server.Listening(0) != 0) {
			if(Server.ErrLog(0))
				Server.LogStream() << "Listening failed" << endl;
			return -2;
		}

		#ifdef _WIN32
			if(bService && Service.Start() < 0) return -3;
		#endif

		iResult = Server.Run(); // 1 = restart

		#ifdef _WIN32
			if(bService && Service.Stop() < 0) return -4;
		#endif

	}
	return iResult;
}

int main(int argc, char **argv) {

	signal(SIGINT, SigHandler);
	signal(SIGTERM, SigHandler);
	#ifndef _WIN32
		signal(SIGQUIT, SigHandler);
		signal(SIGHUP, SigHandler);
		signal(SIGPIPE, SigHandler);
		signal(SIGIO, SigHandler);
	#else
		SetUnhandledExceptionFilter(&cException::ExceptionFilter);
	#endif

	return runHub(argc, argv);
}
