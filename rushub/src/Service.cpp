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
bool Service::isService = false;

Service::Service() : Obj("Service", true) {
	mCurService = this;
}

Service::~Service() {
}

/** installService */
int Service::installService(char * name, const char * configFile) {

	// Open SC Manager
	SC_HANDLE manager = ::OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (!manager) {
		cout << "Open SCManager failed (" << GetLastError() << ")" << endl;
		return -1;
	}

	// Service path
	char buf[MAX_PATH+1] = { '\0' };
	if (!::GetModuleFileName(NULL, buf, MAX_PATH)) {
		cout << "Error in GetModuleFileName (" << GetLastError() << ")" << endl;
	}

	// Service name
	if (!name) {
		strcpy(name, "rushub");
	}

	// Service path + name
	string cmd = "\"" + string(buf) + "\" -s \"" + string(name) + "\"";

	// Service config path
	if (configFile) {
		cmd += " -c \"" + string(configFile) + "\"";
	}

	// Create service
	SC_HANDLE service = ::CreateService(
		manager,
		name,
		name,
		SERVICE_CHANGE_CONFIG,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_AUTO_START,
		SERVICE_ERROR_NORMAL,
		cmd.c_str(),
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	);

	if (!service) {
		cout << "Create service failed (" << GetLastError() << ")" << endl;
		::CloseServiceHandle(manager);
		return -2;
	}

	SERVICE_DESCRIPTION serviceDescription;
	serviceDescription.lpDescription = "NMDC Server";
	::ChangeServiceConfig2(service, SERVICE_CONFIG_DESCRIPTION, &serviceDescription);

	cout << "Service '" << name << "' installed successfully" << endl;

	::CloseServiceHandle(service);
	::CloseServiceHandle(manager);
	return 0;
}

/** uninstallService */
int Service::uninstallService(char * name) {

	// Open SC Manager
	SC_HANDLE manager = ::OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);

	if (!manager) {
		cout << "Open SCManager failed (" << GetLastError() << ")" << endl;
		return -1;
	}

	// Service name
	if (!name) {
		strcpy(name, "rushub");
	}

	// Open service
	SC_HANDLE service = ::OpenService(manager, name, SERVICE_QUERY_STATUS | SERVICE_STOP | DELETE);

	if (!service) {
		cout << "Open service failed (" << GetLastError() << ")" << endl;
		::CloseServiceHandle(manager);
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
		::CloseServiceHandle(manager);
		return -3;
	}

	cout << "Service '" << name << "' was deleted successfully" << endl;

	::CloseServiceHandle(service);
	::CloseServiceHandle(manager);
	return 0;
}

int Service::serviceStart(char * name) {

	SC_HANDLE manager = ::OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (!manager) {
		cout << "Open SCManager failed (" << GetLastError() << ")" << endl;
		return -1;
	}

	// Service name
	if (!name) {
		strcpy(name, "rushub");
	}

	SC_HANDLE service = ::OpenService(manager, name, SERVICE_START);
	if (!service) {
		cout << "Open service failed (" << GetLastError() << ")" << endl;
		::CloseServiceHandle(manager);
		return -2;
	}

	if (!::StartService(service, 0, NULL)) {
		cout << "Cannot start service (" << GetLastError() << ")" << endl;
	} else {
		cout << "Service '" << name << "' was started successfully" << endl;
	}

	::CloseServiceHandle(service);
	::CloseServiceHandle(manager);
	return 0;
}

int Service::serviceStop(char * name) {

	SC_HANDLE manager = ::OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (!manager) {
		cout << "Open SCManager failed (" << GetLastError() << ")" << endl;
		return -1;
	}

	// Service name
	if (!name) {
		strcpy(name, "rushub");
	}

	SC_HANDLE service = ::OpenService(manager, name, SERVICE_STOP);
	if (!service) {
		cout << "Open service failed (" << GetLastError() << ")" << endl;
		::CloseServiceHandle(manager);
		return -2;
	}

	SERVICE_STATUS ss;
	if (!::ControlService(service, SERVICE_CONTROL_STOP, &ss)) {
		cout << "Cannot stop service (" << GetLastError() << ")" << endl;
	} else {
		cout << "Service '" << name << "' was stoped successfully" << endl;
	}

	::CloseServiceHandle(service);
	::CloseServiceHandle(manager);
	return 0;
}

int Service::start() {
	ss.dwCurrentState = SERVICE_RUNNING;
	if (::SetServiceStatus(ssh, &ss) == false) {
		if (mCurService->errLog(0)) {
			mCurService->logStream() << "Set Service Status failed (" << (unsigned long)GetLastError() << ")" << endl;
		}
		return -1;
	}
	return 0;
}

int Service::stop() {
	ss.dwCurrentState = SERVICE_STOPPED;
	::SetServiceStatus(ssh, &ss);
	return 0;
}


int Service::cli(int argc, char * argv[], string & configFile) {

	// Simple start
	if (argc < 2) {
		return 1;
	}

	char *startName = NULL, *config = NULL, *installName = NULL;
	string path;

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
					if (mCurService->errLog(0)) {
						mCurService->logStream() << "Please, set service name." << endl;
					}
					return -1;
				}
				startName = argv[i];
				break;

			case eConfig :
				if (++i >= argc) {
					cout << "Please, set config name." << endl;
					return -2;
				}
				config = argv[i];
				break;

			case eInstall :
				if (++i >= argc) {
					cout << "Please, set service name." << endl;
					return -3;
				}
				installName = argv[i];
				break;

			case eUninstall :
				if (++i >= argc) {
					cout << "Please, set service name." << endl;
					return -4;
				}
				uninstallService(argv[i]);
				return 0;

			case eQuit :
				if (++i >= argc) {
					cout << "Please, set service name." << endl;
					return -5;
				}
				serviceStop(argv[i]);
				return 0;

			case eHelp :
				cout << INTERNALNAME " " INTERNALVERSION " build on "__DATE__" "__TIME__ << endl << endl <<
					"Usage: rushub [OPTIONS] ..." << endl << endl <<
					"Options:" << endl <<
					"  -s,\t--service <name>\tstart hub as service" << endl <<
					"  -c,\t--config <dir>\t\tset main config file for hub" << endl <<
					"  -i,\t--install <name>\tinstall service" << endl <<
					"  -u,\t--uninstall <name>\tuninstall service, then exit" << endl <<
					"  -q,\t--quit <name>\t\tstop service, then exit" << endl <<
					"  -h,\t--help\t\t\tprint this help, then exit" << endl;
				return 0;

			default :
				break;

		}
	}

	if (config) {

		size_t len = strlen(config);
		if (!len) {
			cout << "Please, set config file." << endl;
			return -5;
		}

		if (len < 4 || (config[1] != ':' || (config[2] != '\\' && config[2] != '/'))) {
			cout << "Cinfig file must have absolute path." << endl;
			return -6;
		}

		string configPath;
		Dir::pathForFile(config, configPath);
		if (!Dir::checkPath(configPath)) {
			cout << "Directory for config file not exist and can't be created." << endl;
			return -7;
		}

		configFile = config;
	}

	if (installName) {
		installService(installName, config);
		if (startName == NULL) {
			return 0;
		}
	}

	if (isService) { // Service!
		return 1;
	}

	if (startName) {
		SERVICE_TABLE_ENTRY dispatchTable[] = {
			{ startName, (LPSERVICE_MAIN_FUNCTION)Service::serviceMain },
			{ NULL, NULL }
		};
		if (::StartServiceCtrlDispatcher(dispatchTable) == 0) {
			if (GetLastError() == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT) {
				// attempt to start service from console
				mCurService->serviceStart(startName);
			}
			return -2;
		}
		return 0;
	}

	if (config) {
		return 2; // Simple start
	} else {
		// Unknown params
		cout << INTERNALNAME " " INTERNALVERSION " build on "__DATE__" "__TIME__ << endl << endl <<
			"Usage: rushub [OPTIONS] ..." << endl << endl <<
			"Options:" << endl <<
			"  -s,\t--service <name>\tstart hub as service" << endl <<
			"  -c,\t--config <dir>\t\tset main config file for hub" << endl <<
			"  -i,\t--install <name>\tinstall service" << endl <<
			"  -u,\t--uninstall <name>\tuninstall service, then exit" << endl <<
			"  -q,\t--quit <name>\t\tstop service, then exit" << endl <<
			"  -h,\t--help\t\t\tprint this help, then exit" << endl;
		return 0;
	}
}

void WINAPI Service::ctrlHandler(DWORD dwCtrl) {

	int i;

	switch (dwCtrl) {

		case SERVICE_CONTROL_SHUTDOWN :
			// Fallthrough

		case SERVICE_CONTROL_STOP :
			if (mCurService->log(0)) {
				mCurService->logStream() << "Received a " << dwCtrl << " signal, service stop" << endl;
			}

			ss.dwCurrentState = SERVICE_STOP_PENDING;
			ss.dwWin32ExitCode = NO_ERROR;
			ss.dwCheckPoint = 0;
			ss.dwWaitHint = 10 * 1000;
			if (SetServiceStatus(ssh, &ss) == false) {
				if (mCurService->errLog(0)) {
					mCurService->logStream() << "Set Service status failed (" << (unsigned long)GetLastError() << ")" << endl;
					return;
				}
			}

			// Service stop
			DcServer::currentDcServer->stop(0);
			i = 0;
			while (isService && i++ < 10) { // Wait 10 times
				Sleep(1000);
			}
			break;

		case SERVICE_CONTROL_INTERROGATE :
			if (mCurService->log(0)) {
				mCurService->logStream() << "Received a " << dwCtrl << " signal, interrogate" << endl;
			}
			break;

		default :
			if (mCurService->log(0)) {
				mCurService->logStream() << "Received a " << dwCtrl << " signal, default" << endl;
			}
			break;

	}
}

void WINAPI Service::serviceMain(DWORD, LPTSTR *lpszArgv) {

	SC_HANDLE manager = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!manager) {
		if (mCurService->errLog(0)) {
			mCurService->logStream() << "Open SCManager failed (" << GetLastError() << ")" << endl;
		}
		return;
	}

	SC_HANDLE service = OpenService(manager, lpszArgv[0], SERVICE_ALL_ACCESS);
	if (!service) {
		if (mCurService->errLog(0)) {
			mCurService->logStream() << "Open service failed (" << GetLastError() << ")" << endl;
		}
		::CloseServiceHandle(manager);
		return;
	}

	string sBinaryPathName;
	LPQUERY_SERVICE_CONFIG lpBuf = (LPQUERY_SERVICE_CONFIG)malloc(4096);
	if (lpBuf != NULL) {
		DWORD dwBytesNeeded;
		if (!::QueryServiceConfig(service, lpBuf, 4096, &dwBytesNeeded)) {
			if (mCurService->errLog(0)) {
				mCurService->logStream() << "QueryServiceConfig failed (" << GetLastError() << ")" << endl;
			}
			::CloseServiceHandle(service);
			::CloseServiceHandle(manager);
			return;
		}
		sBinaryPathName = lpBuf->lpBinaryPathName;
		free(lpBuf);
	}

	::CloseServiceHandle(service);
	::CloseServiceHandle(manager);
	

	ssh = ::RegisterServiceCtrlHandler(lpszArgv[0], ctrlHandler);
	if (!ssh) {
		if (mCurService->errLog(0)) {
			mCurService->logStream() << "Register service ctrl handler failed (" << (unsigned long)GetLastError() << ")" << endl;
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
		if (mCurService->errLog(0)) {
			mCurService->logStream() << "Set service status failed (" << (unsigned long)GetLastError() << ")" << endl;
		}
		return;
	}
	int argc;
	char ** argv;
	StringToArg::String2Arg(sBinaryPathName, &argc, &argv);

	isService = true;
	runHub(argc, argv, true);
	isService = false;
}

#endif // _WIN32

/**
 * $Id$
 * $HeadURL$
 */
