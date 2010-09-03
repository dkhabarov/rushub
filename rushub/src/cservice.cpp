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

#ifdef _WIN32
 
#include "cservice.h"
#include "cdir.h" // for DirExists
#include "cdcserver.h"

using nDCServer::cDCServer;

cService * cService::mCurService = NULL;
SERVICE_STATUS_HANDLE cService::ssh;
SERVICE_STATUS cService::ss;

cService::cService() : cObj("cService")
{
  mCurService = this;
}

cService::~cService()
{
}

int cService::InstallService(char * sName, const char * sPath) {
  SC_HANDLE Manager = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if(Manager == NULL) {
    printf("Open SCManager failed (%d)", GetLastError());
    return -1;
  }
  char sBuf[MAX_PATH+1];
  ::GetModuleFileName(NULL, sBuf, MAX_PATH);

  if(!sName) sName = "rushub";
  string sCmd = "\"" + string(sBuf) + "\" -s " + string(sName);

  if(sPath) sCmd += " -c " + string(sPath);

  SC_HANDLE Service = ::CreateService(Manager, sName, sName, 0, SERVICE_WIN32_OWN_PROCESS,
    SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, sCmd.c_str(),
    NULL, NULL, NULL, NULL, NULL);

  if(Service == NULL) {
    printf("Create service failed (%d)", GetLastError());
    CloseServiceHandle(Manager);
    return -2;
  }
  printf("Service '%s' installed successfully.", sName);
  CloseServiceHandle(Service);
  CloseServiceHandle(Manager);
  return 0;
}

int cService::UninstallService(char * sName) {
  SC_HANDLE Manager = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if(Manager == NULL) {
    printf("Open SCManager failed (%d)", GetLastError());
    return -1;
  }
  if(!sName) sName = "rushub";
  SC_HANDLE Service = ::OpenService(Manager, sName, SERVICE_QUERY_STATUS | SERVICE_STOP | DELETE);
  if(Service == NULL) {
    printf("Open service failed (%d)", GetLastError());
    ::CloseServiceHandle(Manager);
    return -2;
  }
  SERVICE_STATUS_PROCESS ssp;
  DWORD dwBytesNeeded;
  if(::QueryServiceStatusEx(Service, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded) != 0)
    if(ssp.dwCurrentState != SERVICE_STOPPED && ssp.dwCurrentState != SERVICE_STOP_PENDING)
      ::ControlService(Service, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)&ssp);

  if(::DeleteService(Service) == false) {
    printf("Delete service failed (%d)", GetLastError());
    ::CloseServiceHandle(Service);
    ::CloseServiceHandle(Manager);
    return -3;
  }
  printf("Service '%s' deleted successfully.", sName);
  ::CloseServiceHandle(Service);
  ::CloseServiceHandle(Manager);
  return 0;
}

void WINAPI cService::CtrlHandler(DWORD dwCtrl) {
  switch(dwCtrl) {
    case SERVICE_CONTROL_SHUTDOWN:
    case SERVICE_CONTROL_STOP:
      ss.dwCurrentState = SERVICE_STOP_PENDING;
      // Service stop.
    case SERVICE_CONTROL_INTERROGATE: 
      // Fall through to send current status.
      break;
    default:
      break;
  }
  if(SetServiceStatus(ssh, &ss) == false)
    if(mCurService->ErrLog(0))
      mCurService->LogStream() << "Set Service status failed (" << (unsigned)GetLastError() << ")" << endl;
}

void WINAPI cService::ServiceMain(DWORD dwArgc, LPSTR *lpszArgv) {
  ssh = ::RegisterServiceCtrlHandler(lpszArgv[0], CtrlHandler);
  if(!ssh) {
    if(mCurService->ErrLog(0))
      mCurService->LogStream() << "Register service ctrl handler failed (" << (unsigned)GetLastError() << ")" << endl;
    cout << "Register service ctrl handler failed (" << (unsigned)GetLastError() << ")" << endl;
    return;
  }
  ss.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  ss.dwCurrentState = SERVICE_START_PENDING;
  ss.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP;
  ss.dwWin32ExitCode = NO_ERROR;
  ss.dwCheckPoint = 0;
  ss.dwWaitHint = 10 * 1000;
  if(::SetServiceStatus(ssh, &ss) == false) {
    if(mCurService->ErrLog(0))
      mCurService->LogStream() << "Set service status failed (" << (unsigned)GetLastError() << ")" << endl;
    cout << "Set service status failed (" << (unsigned)GetLastError() << ")" << endl;
    return;
  }

  /** Creating the server */
  cDCServer Server(mCurService->msMainPath, mCurService->msMainFile);

  /** Listening all ports */
  if(Server.Listening(0) != 0) {
    if(mCurService->ErrLog(0))
      mCurService->LogStream() << "Listening failed" << endl;
    cout << "Listening failed" << endl;
    return;
  }

  ss.dwCurrentState = SERVICE_RUNNING;
  if(::SetServiceStatus(ssh, &ss) == false) {
    if(mCurService->ErrLog(0))
      mCurService->LogStream() << "Set Service Status failed (" << (unsigned)GetLastError() << ")" << endl;
    cout << "Set Service Status failed (" << (unsigned)GetLastError() << ")" << endl;
    return;
  }
  Server.Run();
}

int cService::SetService(int argc, char* argv[], const string & sMainPath, const char * sMainFile) {
  if(argc < 2) return 1;
  char *sStartName = NULL, *sConfigDir = NULL, *sInstallName = NULL;
  string sPath;
  msMainPath = sMainPath;
  msMainFile = (char*)sMainFile;

  for(int i = 1; i < argc; ++i) {
    int j = 0, id = 0;
    while(arg_list[j].val) {
      if(stricmp(argv[i], arg_list[j].val) == 0) {
        id = arg_list[j].id;
        break;
      }
      ++j;
    }

    switch(id) {
      case eService:
        if(++i > argc) {
          if(ErrLog(0)) LogStream() << "Please, set service name." << endl;
          return -1;
        }
        sStartName = argv[i];
        break;
      case eConfig:
        if(++i > argc) {
          printf("Please, set directory.");
          return -2;
        }
        sConfigDir = argv[i];
        break;
      case eInstall:
        if(++i > argc) {
          printf("Please, set service name.");
          return -3;
        }
        sInstallName = argv[i];
        break;
      case eUninstall:
        if(++i > argc) {
          printf("Please, set service name.");
          return -4;
        }
        UninstallService(argv[i]);
        return 0;
      case eHelp:
        printf(INTERNALNAME" "INTERNALVERSION" build on "__DATE__" "__TIME__"\n\n"
          "Usage: rushub [OPTIONS] ...\n\n"
          "Options:\n"
          "  -s,\t--service <name>\tstart hub as service\n"
          "  -c,\t--config <dir>\t\tset main config directory for hub\n"
          "  -i,\t--install <name>\tinstall service\n"
          "  -u,\t--uninstall <name>\tuninstall service, then exit\n"
          "  -h,\t--help\t\t\tprint this help, then exit\n"
        );
        return 0;
      default:
        break;
    }
  }


  if(sConfigDir) {
    size_t iLen = strlen(sConfigDir);
    if(!iLen) {
      printf("Please, set directory.");
      return -5;
    }
    if(sConfigDir[0] != '\\' && sConfigDir[0] != '/' && 
      (iLen < 4 || (sConfigDir[1] != ':' || (sConfigDir[2] != '\\' && sConfigDir[2] != '/'))))
    {
      printf("Directory must be absolute path.");
      return -6;
    }

    if(DirExists(sConfigDir) == false)
      if(CreateDirectory(sConfigDir, NULL) == 0) 
        printf("Directory not exist and can't be created.");
    msMainPath = sConfigDir;
  }

  string sFile(msMainPath + string("system.log"));
  if(!cObj::mOfs.is_open()) cObj::mOfs.open(sFile.c_str());
  //if(!cObj::mErrOfs.is_open()) cObj::mErrOfs.open("errors.log");


  if(sInstallName) {
    InstallService(sInstallName, sConfigDir);
  }
  if(sStartName) {
    SERVICE_TABLE_ENTRY DispatchTable[] = {
      { sStartName, ServiceMain }, 
      { NULL, NULL }
    };
    if(::StartServiceCtrlDispatcher(DispatchTable) == false) {
      if(mCurService->ErrLog(0))
        mCurService->LogStream() << "Service start failed" << endl;
      cout << "Service start failed" << endl;
	    return -7;
    }
  }

  return 0;
}

#endif // _WIN32
