/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
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

#ifdef _WIN32

#include "Exception.h"

#include <fstream>
#include <time.h>
#include <DbgHelp.h>

#pragma comment(lib, "Dbghelp.lib")

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
	#pragma warning(disable:4996) // Disable "This function or variable may be unsafe."
#endif

int Exception::recursion = 0;
bool Exception::first = true;



Exception::Exception() {
}



Exception::~Exception() {
}



long __stdcall Exception::exceptionFilter(LPEXCEPTION_POINTERS e) {

	if (++recursion > MAX_RECURSIONS) {
		exit(-1);
	}

	string path;
	char buf[MAX_PATH+1] = { '\0' };
	::GetModuleFileName(NULL, buf, MAX_PATH);
	char * exPath = buf;
	char * slash = strrchr(exPath, '\\');
	if (slash) {
		path = string(exPath, slash - exPath);
	}

	// Loads dll and pdb
	#ifndef _DEBUG
		init(path.c_str());
	#endif

	if (first) {
		ofstream f;
		f.open(FILE_NAME);
		f.close();
		first = false;
	}

	char buffer[BUFFERSIZE] = { '\0' };
	time_t now;
	time(&now);
	struct tm t;
	localtime_s(&t, &now);
	strftime(buffer, BUFFERSIZE, "%Y-%m-%d %H:%M:%S", &t);

	char code[BUFFERSIZE] = { '\0' };
	
	#ifdef _WIN64
		sprintf(code, "%I64x", e->ExceptionRecord->ExceptionCode);
	#else
		sprintf(code, "%I32x", e->ExceptionRecord->ExceptionCode);
	#endif
	

	ofstream f;
	f.open(FILE_NAME, ios_base::app);
	
	f << "Code: " << code << endl
		<< "Version: " << INTERNALVERSION << endl
		<< "OS: " << DcServer::currentDcServer->mSysVersion << endl
		<< "Time: " << buffer << endl << endl;

	WIN32_FIND_DATA fd;
	if (FindFirstFile(path.append(STR_LEN("\\rushub.pdb")).c_str(), &fd) == INVALID_HANDLE_VALUE) {
		#ifndef _DEBUG
			f << "Debug symbols was not found" << endl;
			f.close();
			exit(1);
		#else
			f.close();
			return EXCEPTION_CONTINUE_SEARCH;
		#endif
	}
	#ifndef _WIN64
		Exception::stackTrace(f, e->ContextRecord->Eip, e->ContextRecord->Esp, e->ContextRecord->Ebp);
	#endif

	f.close();

	#ifndef _DEBUG
		uninit();
		exit(-1);
	#else
		return EXCEPTION_CONTINUE_SEARCH;
	#endif
}

// ".;%_NT_SYMBOL_PATH%;%_NT_ALTERNATE_SYMBOL_PATH%;%SYSTEMROOT%;%SYSTEMROOT%\System32;" + initPath
int Exception::init(const char * path) {
	SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_FAIL_CRITICAL_ERRORS | SYMOPT_LOAD_LINES );

	char buf[BUFFERSIZE] = { '\0' };
	string symbolPath(STR_LEN("."));
	if (GetEnvironmentVariableA("_NT_SYMBOL_PATH", buf, BUFFERSIZE)) {
		symbolPath.append(STR_LEN(";"));
		symbolPath.append(buf);
	}
	if (GetEnvironmentVariableA("_NT_ALTERNATE_SYMBOL_PATH", buf, BUFFERSIZE)) {
		symbolPath.append(STR_LEN(";"));
		symbolPath.append(buf);
	}
	if (GetEnvironmentVariableA("SYSTEMROOT", buf, BUFFERSIZE)) {
		symbolPath.append(STR_LEN(";"));
		symbolPath.append(buf);
		symbolPath.append(STR_LEN(";"));
		symbolPath.append(buf);
		symbolPath.append(STR_LEN("\\System32"));
	}
	if (path != NULL && path[0] != '\0') {
		symbolPath.append(STR_LEN(";"));
		symbolPath.append(path);
	}
	return SymInitialize(GetCurrentProcess(), symbolPath.c_str(), 1);
}

int Exception::uninit() {
	return SymCleanup(GetCurrentProcess());
}

// Get function prototype and parameter info from ip address and stack address
int Exception::getFunctionInfo(unsigned long functionAddress, unsigned long stackAddress, char * buff) {
	unsigned long size = 1024 * 16;
	PSYMBOL_INFO pSym = (PSYMBOL_INFO) GlobalAlloc(GMEM_FIXED, size);

	if (pSym == NULL) {
		return 0;
	}

	::ZeroMemory(pSym, size * sizeof(_SYMBOL_INFO));
	pSym->SizeOfStruct = size;
	pSym->MaxNameLen = size - sizeof(IMAGEHLP_SYMBOL);

	strcpy(buff, "?");

	unsigned __int64 disp = 0;
	if (SymFromAddr(GetCurrentProcess(), (unsigned long)functionAddress, &disp, pSym)) {

		char buf[BUFFERSIZE] = "?";
		char * pbuf = buf;
		char * pSep = NULL;

		UnDecorateSymbolName(pSym->Name, buf, BUFFERSIZE,
			UNDNAME_COMPLETE |
			UNDNAME_NO_THISTYPE |
			UNDNAME_NO_SPECIAL_SYMS |
			UNDNAME_NO_MEMBER_TYPE |
			UNDNAME_NO_MS_KEYWORDS |
			UNDNAME_NO_ACCESS_SPECIFIERS
		);

		if (strcmp(buf, "_WinMain@16") == 0) {
			strcpy(buf, "WinMain(HINSTANCE,HINSTANCE,LPCTSTR,int)");
		} else if (strcmp(buf, "main") == 0 || strcmp(buf, "_main") == 0 ) {
			strcpy(buf, "main(int,char**)");
		} else if (strcmp(buf, "_mainCRTStartup") == 0) {
			strcpy(buf, "mainCRTStartup()");
		} else if (strcmp(buf, "_wmain") == 0) {
			strcpy(buf, "wmain(int,char**,char**)");
		} else if (strcmp(buf, "_wmainCRTStartup") == 0) {
			strcpy(buf, "wmainCRTStartup()");
		}
		buff[0] = '\0';

		if (strstr(buf, "(void)") == NULL && strstr(buf, "()") == NULL) {
			unsigned long i = 0;
			for (; ; ++i) {
				pSep = strchr(pbuf, ',');
				if (pSep == NULL) {
					break;
				}
				*pSep = '\0';
				strcat(buff, pbuf);
				sprintf(buff + strlen(buff), " = 0x%08lX,", *((unsigned long*)(stackAddress) + 2 + i));
				pbuf = pSep + 1;
			}
			pSep = strchr(pbuf, ')');
			if (pSep != NULL) {
				*pSep = '\0';
				strcat(buff, pbuf);
				sprintf(buff + strlen(buff), " = 0x%08lX)", *((unsigned long*)(stackAddress) + 2 + i));
				pbuf = pSep + 1;
			}
		}
		GlobalFree(pSym);
		strcat(buff, pbuf);
		return 1;
	}
	GlobalFree(pSym);
	return 0;
}

// Get the module name from a given address
int Exception::getModuleName(unsigned address, char * buff) {
	IMAGEHLP_MODULE moduleInfo;
	::ZeroMemory(&moduleInfo, sizeof(moduleInfo));
	moduleInfo.SizeOfStruct = sizeof(moduleInfo);
	if (SymGetModuleInfo(GetCurrentProcess(), (unsigned long)address, &moduleInfo)) {
		strcpy(buff, moduleInfo.ModuleName);
		return 1;
	}
	strcpy(buff, "?");
	return 0;
}

// Get source file name and line number from IP address
// The output format is: "sourcefile(linenumber)" or "modulename|address" or "address"
int Exception::getSourceInfo(unsigned address, char * buff) {
	IMAGEHLP_LINE lineInfo;
	unsigned long disp;
	char fileName[BUFFERSIZE] = "";
	char moduleInfo[BUFFERSIZE] = "";

	::ZeroMemory(&lineInfo, sizeof(lineInfo));
	lineInfo.SizeOfStruct = sizeof(lineInfo);
	strcpy(buff, "?(?)");

	if (SymGetLineFromAddr(GetCurrentProcess(), address, &disp, &lineInfo)) {
		strcpy(fileName, lineInfo.FileName);
		sprintf(buff, "%s(%u)", fileName, lineInfo.LineNumber);
		return 1;
	}

	getModuleName(address, moduleInfo);
	if (moduleInfo[0] == '?' || moduleInfo[0] == '\0') {
		sprintf(buff, "0x%08X", address);
	} else {
		sprintf(buff, "%s|0x%08X", moduleInfo, address);
	}
	return 0;
}

void Exception::stackTrace(std::ostream & f, unsigned long eip, unsigned long esp, unsigned long ebp) {
	stackTrace(GetCurrentThread(), "Stack trace:", f, eip, esp, ebp);
}

void Exception::stackTrace(void * hThread, char * msg, std::ostream & f, unsigned long eip, unsigned long esp, unsigned long ebp) {
	int bResult;
	char symInfo[BUFFERSIZE] = "?";
	char srcInfo[BUFFERSIZE] = "?";
	void * hProcess = GetCurrentProcess();
	STACKFRAME callStack;

	if (hThread != GetCurrentThread() && SuspendThread(hThread) == -1) {
		f << "No call stack" << endl;
		return;
	}

	::ZeroMemory(&callStack, sizeof(callStack));
	callStack.AddrPC.Offset = eip;
	callStack.AddrStack.Offset = esp;
	callStack.AddrFrame.Offset = ebp;
	callStack.AddrPC.Mode = AddrModeFlat;
	callStack.AddrStack.Mode = AddrModeFlat;
	callStack.AddrFrame.Mode = AddrModeFlat;

	f << msg << endl << endl;

	getFunctionInfo(
		static_cast<unsigned long> (callStack.AddrPC.Offset),
		static_cast<unsigned long> (callStack.AddrFrame.Offset),
		symInfo
	);
	getSourceInfo(static_cast<unsigned long> (callStack.AddrPC.Offset), srcInfo);

	f << srcInfo << ": " << symInfo << endl;

	for (unsigned long i = 0; i < 1000; ++i) {
		bResult = StackWalk(
			IMAGE_FILE_MACHINE_I386,
			hProcess,
			hThread,
			&callStack,
			NULL,
			NULL,
			SymFunctionTableAccess,
			SymGetModuleBase,
			NULL);

		if (i == 0) {
			continue;
		}
		if (!bResult || callStack.AddrFrame.Offset == 0) {
			break;
		}

		getFunctionInfo(
			static_cast<unsigned long> (callStack.AddrPC.Offset),
			static_cast<unsigned long> (callStack.AddrFrame.Offset),
			symInfo
		);
		getSourceInfo(static_cast<unsigned long> (callStack.AddrPC.Offset), srcInfo);

		f << srcInfo << ": " << symInfo << endl;
	}
	if (hThread != GetCurrentThread()) {
		ResumeThread(hThread);
	}
}

#endif // _WIN32

/**
 * $Id$
 * $HeadURL$
 */
