/*
 * RusHub - hub server for Direct Connect peer to peer network.

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

#ifdef _WIN32
 
#include "Service.h"
#include "DcServer.h"
#include "StringToArg.h" // for StringToArg.String2Arg
#include "Dir.h" // for checkPath

#define stricmp _stricmp

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
	#pragma warning(disable:4996) // Disable "This function or variable may be unsafe."
#endif

using ::dcserver::DcServer;

Service * Service::mCurService = NULL;
bool Service::IsService = false;

Service::Service() : Obj("Service") {
	mCurService = this;
}

Service::~Service() {
}

/** InstallService */
int Service::InstallService(char * name, const char * confPath) {

	// Open SC Manager
	SC_HANDLE Manager = ::OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (!Manager) {
		cout << "Open SCManager failed (" << GetLastError() << ")" << endl;
		return -1;
	}

	// Service path
	char sBuf[MAX_PATH+1] = { '\0' };
	if (!GetModuleFileName(NULL, sBuf, MAX_PATH)) {
		cout << "Error in GetModuleFileName (" << GetLastError() << ")" << endl;
	}

	// Service name
	if (!name) {
		strcpy(name, "rushub");
	}

	// Service path + name
	string sCmd = "\"" + string(sBuf) + "\" -s \"" + string(name) + "\"";

	// Service config path
	if (confPath) {
		sCmd += " -c \"" + string(confPath) + "\"";
	}

	// Create service
	SC_HANDLE service = ::CreateService(
		Manager,
		name,
		name,
		0,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_AUTO_START,
		SERVICE_ERROR_NORMAL,
		sCmd.c_str(),
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	);

	if (!service) {
		cout << "Create service failed (" << GetLastError() << ")" << endl;
		CloseServiceHandle(Manager);
		return -2;
	}

	cout << "Service '" << name << "' installed successfully" << endl;

	CloseServiceHandle(service);
	CloseServiceHandle(Manager);
	return 0;
}

/** UninstallService */
int Service::UninstallService(char * name) {

	// Open SC Manager
	SC_HANDLE Manager = ::OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);

	if (!Manager) {
		cout << "Open SCManager failed (" << GetLastError() << ")" << endl;
		return -1;
	}

	// Service name
	if (!name) {
		strcpy(name, "rushub");
	}

	// Open service
	SC_HANDLE service = ::OpenService(Manager, name, SERVICE_QUERY_STATUS | SERVICE_STOP | DELETE);

	if (!service) {
		cout << "Open service failed (" << GetLastError() << ")" << endl;
		::CloseServiceHandle(Manager);
		return -2;
	}

	SERVICE_STATUS_PROCESS ssp;
	DWORD dwBytesNeeded;

	// Stop service, if it was started
	if (::QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded) != 0) {
		if (ssp.dwCurrentState != SERVICE_STOPPED && ssp.dwCurrentState != SERVICE_STOP_PENDING) {
			::ControlService(service, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)&ssp);
		}
	}

	// Delete service
	if (::DeleteService(service) == 0) {
		cout << "Delete service failed (" << GetLastError() << ")" << endl;
		::CloseServiceHandle(service);
		::CloseServiceHandle(Manager);
		return -3;
	}

	cout << "Service '" << name << "' was deleted successfully" << endl;

	::CloseServiceHandle(service);
	::CloseServiceHandle(Manager);
	return 0;
}

int Service::ServiceStart(char * name) {

	SC_HANDLE Manager = ::OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (!Manager) {
		cout << "Open SCManager failed (" << GetLastError() << ")" << endl;
		return -1;
	}

	// Service name
	if (!name) {
		strcpy(name, "rushub");
	}

	SC_HANDLE service = OpenService(Manager, name, SERVICE_START);
	if (!service) {
		cout << "Open service failed (" << GetLastError() << ")" << endl;
		::CloseServiceHandle(Manager);
		return -2;
	}

	if (!StartService(service, 0, NULL)) {
		cout << "Cannot start service (" << GetLastError() << ")" << endl;
	} else {
		cout << "Service '" << name << "' was started successfully" << endl;
	}

	::CloseServiceHandle(service);
	::CloseServiceHandle(Manager);
	return 0;
}

int Service::ServiceStop(char * name) {

	SC_HANDLE Manager = ::OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (!Manager) {
		cout << "Open SCManager failed (" << GetLastError() << ")" << endl;
		return -1;
	}

	// Service name
	if (!name) {
		strcpy(name, "rushub");
	}

	SC_HANDLE service = OpenService(Manager, name, SERVICE_STOP);
	if (!service) {
		cout << "Open service failed (" << GetLastError() << ")" << endl;
		::CloseServiceHandle(Manager);
		return -2;
	}

	SERVICE_STATUS ss;
	if (!ControlService(service, SERVICE_CONTROL_STOP, &ss)) {
		cout << "Cannot stop service (" << GetLastError() << ")" << endl;
	} else {
		cout << "Service '" << name << "' was stoped successfully" << endl;
	}

	::CloseServiceHandle(service);
	::CloseServiceHandle(Manager);
	return 0;
}

int Service::Start() {
	ss.dwCurrentState = SERVICE_RUNNING;
	if (::SetServiceStatus(ssh, &ss) == false) {
		if (Service::mCurService->ErrLog(0)) {
			Service::mCurService->LogStream() << "Set Service Status failed (" << (unsigned long)GetLastError() << ")" << endl;
		}
		return -1;
	}
	return 0;
}

int Service::Stop() {
	ss.dwCurrentState = SERVICE_STOPPED;
	SetServiceStatus(ssh, &ss);
	return 0;
}


int Service::cli(int argc, char * argv[], string & sConfPath, const string &) {

	// Simple start
	if (argc < 2) {
		return 1;
	}

	char *sStartName = NULL, *sConfigDir = NULL, *sInstallName = NULL;
	string sPath;

	for (int i = 1; i < argc; ++i) {

		int j = -1, id = 0;

		while (arg_list[++j].val) {
			if (stricmp(argv[i], arg_list[j].val) == 0) {
				id = arg_list[j].id;
				break;
			}
		}

		switch (id) {

			case eService :
				if (++i >= argc) {
					if (mCurService->ErrLog(0)) {
						mCurService->LogStream() << "Please, set service name." << endl;
					}
					return -1;
				}
				sStartName = argv[i];
				break;

			case eConfig :
				if (++i >= argc) {
					cout << "Please, set directory." << endl;
					return -2;
				}
				sConfigDir = argv[i];
				break;

			case eInstall :
				if (++i >= argc) {
					cout << "Please, set service name." << endl;
					return -3;
				}
				sInstallName = argv[i];
				break;

			case eUninstall :
				if (++i >= argc) {
					cout << "Please, set service name." << endl;
					return -4;
				}
				UninstallService(argv[i]);
				return 0;

			case eQuit :
				if (++i >= argc) {
					cout << "Please, set service name." << endl;
					return -5;
				}
				ServiceStop(argv[i]);
				return 0;

			case eHelp :
				cout << INTERNALNAME " " INTERNALVERSION " build on "__DATE__" "__TIME__ << endl << endl <<
					"Usage: rushub [OPTIONS] ..." << endl << endl <<
					"Options:" << endl <<
					"  -s,\t--service <name>\tstart hub as service" << endl <<
					"  -c,\t--config <dir>\t\tset main config directory for hub" << endl <<
					"  -i,\t--install <name>\tinstall service" << endl <<
					"  -u,\t--uninstall <name>\tuninstall service, then exit" << endl <<
					"  -q,\t--quit <name>\tstop service, then exit" << endl <<
					"  -h,\t--help\t\t\tprint this help, then exit" << endl;
				return 0;

			default :
				break;

		}
	}

	if (sConfigDir) {

		size_t iLen = strlen(sConfigDir);
		if (!iLen) {
			cout << "Please, set directory." << endl;
			return -5;
		}

		if (sConfigDir[0] != '\\' && sConfigDir[0] != '/' && 
			(iLen < 4 || (sConfigDir[1] != ':' || (sConfigDir[2] != '\\' && sConfigDir[2] != '/')))
		) {
			cout << "Directory must be absolute path." << endl;
			return -6;
		}

		string path(sConfigDir);
		if (!Dir::checkPath(path)) {
			cout << "Directory not exist and can't be created." << endl;
			return -7;
		}

		sConfPath = path;
	}

	if (sInstallName) {
		InstallService(sInstallName, sConfigDir);
	}

	if (IsService) { // Service!
		return 1;
	}

	if (sStartName) {
		SERVICE_TABLE_ENTRY DispatchTable[] = {
			{ sStartName, (LPSERVICE_MAIN_FUNCTION)Service::ServiceMain },
			{ NULL, NULL }
		};
		if (::StartServiceCtrlDispatcher(DispatchTable) == 0) {
			if (GetLastError() == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT) {
				// attempt to start service from console
				mCurService->ServiceStart(sStartName);
			}
			return -2;
		}
		return 0;
	}

	return 0;
}

void WINAPI Service::CtrlHandler(DWORD dwCtrl) {

	switch (dwCtrl) {

		case SERVICE_CONTROL_SHUTDOWN :
			// Fallthrough

		case SERVICE_CONTROL_STOP :
			ss.dwCurrentState = SERVICE_STOP_PENDING;
			ss.dwWin32ExitCode = NO_ERROR;
			if (Service::mCurService->Log(0)) {
				Service::mCurService->LogStream() << "Received a " << dwCtrl << " signal, service stop" << endl;
			}
			// Service stop
			DcServer::currentDcServer->Stop(0);
			break;

		case SERVICE_CONTROL_INTERROGATE :
			if (Service::mCurService->Log(0)) {
				Service::mCurService->LogStream() << "Received a " << dwCtrl << " signal, interrogate" << endl;
			}
			break;

		default :
			if (Service::mCurService->Log(0)) {
				Service::mCurService->LogStream() << "Received a " << dwCtrl << " signal, default" << endl;
			}
			break;

	}
	if (SetServiceStatus(ssh, &ss) == false) {
		if (Service::mCurService->ErrLog(0)) {
			Service::mCurService->LogStream() << "Set Service status failed (" << (unsigned long)GetLastError() << ")" << endl;
		}
	}
}

void WINAPI Service::ServiceMain(DWORD, LPTSTR *lpszArgv) {

	SC_HANDLE Manager = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!Manager) {
		if (Service::mCurService->ErrLog(0)) {
			Service::mCurService->LogStream() << "Open SCManager failed (" << GetLastError() << ")" << endl;
		}
		return;
	}

	SC_HANDLE service = OpenService(Manager, lpszArgv[0], SERVICE_ALL_ACCESS);
	if (!service) {
		if (Service::mCurService->ErrLog(0)) {
			Service::mCurService->LogStream() << "Open service failed (" << GetLastError() << ")" << endl;
		}
		::CloseServiceHandle(Manager);
		return;
	}

	string sBinaryPathName;
	LPQUERY_SERVICE_CONFIG lpBuf = (LPQUERY_SERVICE_CONFIG)malloc(4096);
	if (lpBuf != NULL) {
		DWORD dwBytesNeeded;
		if (!::QueryServiceConfig(service, lpBuf, 4096, &dwBytesNeeded)) {
			if (Service::mCurService->ErrLog(0)) {
				Service::mCurService->LogStream() << "QueryServiceConfig failed (" << GetLastError() << ")" << endl;
			}
			::CloseServiceHandle(service);
			::CloseServiceHandle(Manager);
			return;
		}
		sBinaryPathName = lpBuf->lpBinaryPathName;
		free(lpBuf);
	}

	::CloseServiceHandle(service);
	::CloseServiceHandle(Manager);
	

	ssh = ::RegisterServiceCtrlHandler(lpszArgv[0], CtrlHandler);
	if (!ssh) {
		if (Service::mCurService->ErrLog(0)) {
			Service::mCurService->LogStream() << "Register service ctrl handler failed (" << (unsigned long)GetLastError() << ")" << endl;
		}
		return;
	}
	ss.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	ss.dwCurrentState = SERVICE_START_PENDING;
	ss.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP;
	ss.dwWin32ExitCode = NO_ERROR;
	ss.dwCheckPoint = 0;
	ss.dwWaitHint = 10 * 1000;
	if (::SetServiceStatus(ssh, &ss) == false) {
		if (Service::mCurService->ErrLog(0)) {
			Service::mCurService->LogStream() << "Set service status failed (" << (unsigned long)GetLastError() << ")" << endl;
		}
		return;
	}
	int argc;
	char ** argv;
	StringToArg::String2Arg(sBinaryPathName, &argc, &argv);

	IsService = true;
	runHub(argc, argv, true);
}

#endif // _WIN32
