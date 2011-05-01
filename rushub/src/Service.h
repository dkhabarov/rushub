/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * Copyright (C) 2009-2011 by Setuper
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

#ifndef SERVICE_H
#define SERVICE_H

int runHub(int argc, char ** argv, bool bService = false);

#ifdef _WIN32

#include "Server.h"

enum {
	eDaemon = 1,
	eService,
	eConfig,
	eInstall,
	eUninstall,
	eQuit,
	eHelp
};

static const struct {
	const char * val;
	int id;
} arg_list[] = {
	{"-s", eService},
	{"-c", eConfig},
	{"-i", eInstall},
	{"-u", eUninstall},
	{"-q", eQuit},
	{"-h", eHelp},
	{"-service",   eService},
	{"-config",    eConfig},
	{"-install",   eInstall},
	{"-uninstall", eUninstall},
	{"-quit",      eQuit},
	{"-help",      eHelp},
	{"--service",   eService},
	{"--config",    eConfig},
	{"--install",   eInstall},
	{"--uninstall", eUninstall},
	{"--quit",      eQuit},
	{"--help",      eHelp},
	{NULL, 0} // terminator
};

static SERVICE_STATUS_HANDLE ssh;
static SERVICE_STATUS ss;



class Service : public Obj {

public:

	static Service * mCurService;
	static bool isService;

public:

	Service();
	~Service();

	static void WINAPI ctrlHandler(DWORD dwCtrl);
	static void WINAPI serviceMain(DWORD dwArgc, LPTSTR * lpszArgv);

	static int installService(char * name = NULL, const char * configFile = NULL);
	static int uninstallService(char * name = NULL);
	static int serviceStop(char * name = NULL);
	static int serviceStart(char * name = NULL);
	static int cli(int argc, char * argv[], string & configFile);
	static int start();
	static int stop();

}; // Service

#endif // _WIN32

#endif // SERVICE_H

/**
 * $Id$
 * $HeadURL$
 */
