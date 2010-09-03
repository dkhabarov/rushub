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

#include "cdcserver.h"
#include "cdir.h"
#include "cservice.h"
#include <signal.h>

#ifdef _WIN32
  #define SIGQUIT 1000
  #define SIGHUP 1001
  #pragma comment(lib, "config.lib")
  #pragma comment(lib, "utils.lib")
  #pragma comment(lib, "server.lib")
  #pragma comment(lib, "webserver.lib")
  #pragma comment(lib, "plugin.lib")
  #pragma comment(lib, "tinyxml.lib")
#endif

void SigHandler(int iSig) {
  switch(iSig) {
    case SIGINT:
    case SIGTERM:
    case SIGQUIT:
    case SIGHUP:
      cout << "Received a " << iSig << " signal, quiting" << endl;
      cDCServer::sCurrentServer->~cDCServer();
      cDCServer::sCurrentServer = NULL;
      exit(0);
      break;
    default:
      cout << "Received a " << iSig << " signal, ignoring it" << endl;
      signal(iSig, SigHandler);
      break;
  }
}

using nDCServer::cDCServer;

using namespace nServer;

int main(int, char**) {

  string sMainDir; /** Main dir of config files */
  char * sPath = NULL;

  /** Define main dir */
  #ifdef _WIN32
    char sBuf[MAX_PATH+1];
    ::GetModuleFileName(NULL, sBuf, MAX_PATH);
    sPath = sBuf;
    char * sSlash = strrchr(sPath, '\\');
    if(sSlash) sMainDir = string(sPath, sSlash - sPath);
    else sMainDir = sPath;
    size_t iPos = sMainDir.find("\\");
    while(iPos != sMainDir.npos) {
      sMainDir.replace(iPos, 1, "/");
      iPos = sMainDir.find("\\", iPos);
    }
    sMainDir.append("/");
  #else
    char * sHomeDir = getenv("HOME");
    string tmp;
    if(sHomeDir) {
      tmp = sHomeDir;
      tmp += "/rushub";
      sPath = new char[256];
      strcpy(sPath, tmp.c_str());
    } else {
      strcpy(sPath, "./.rushub");
    }
    signal(SIGQUIT, SigHandler);
    signal(SIGHUP, SigHandler);
  #endif
  signal(SIGINT, SigHandler);
  signal(SIGTERM, SigHandler);

  int iResult = 0;
  //cService Service;
  //if(Service.SetService(argc, argv, sMainDir, sPath) > 0) {

    /** Creating the server */
    cDCServer Server(sMainDir, sPath);

    /** Listening all ports */
    if(Server.Listening(0) != 0) return -1;
    iResult = Server.Run();

  //}

  return iResult;
}
