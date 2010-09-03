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

#ifndef CSERVICE_H
#define CSERVICE_H

#ifdef _WIN32

#include "cserver.h"

enum {
  eDaemon = 1,
  eService,
  eConfig,
  eInstall,
  eUninstall,
  eHelp
};

static const struct {
  const char* val;
  int id;
} arg_list[] = {
  {"-s", eService},
  {"-c", eConfig},
  {"-i", eInstall},
  {"-u", eUninstall},
  {"-h", eHelp},
  {"--service",   eService},
  {"--config",    eConfig},
  {"--install",   eInstall},
  {"--uninstall", eUninstall},
  {"--help",      eHelp},
  {NULL, 0} // terminator
};

class cService : public cObj
{
private:
  static SERVICE_STATUS_HANDLE ssh;
  static SERVICE_STATUS ss;
  static cService * mCurService;

  string msMainPath;
  char* msMainFile;

public:
  cService();
  ~cService();

  int InstallService(char * sName = NULL, const char * sPath = NULL);
  int UninstallService(char * sName = NULL);
  static void WINAPI CtrlHandler(DWORD dwCtrl);
  static void WINAPI ServiceMain(DWORD dwArgc, LPSTR *lpszArgv);
  int SetService(int argc, char* argv[], const string & sMainPath, const char * sMainFile);

}; // cService

#endif // _WIN32

#endif // CSERVICE_H
