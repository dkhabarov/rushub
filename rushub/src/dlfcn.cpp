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

#include "dlfcn.h"
#include <windows.h>
#include <stdio.h>

static void set_dl_error(void) {
	DWORD err = GetLastError();
	if(FormatMessageA(FORMAT_MESSAGE_IGNORE_INSERTS |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		err,
		MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
		last_dyn_error,
		sizeof(last_dyn_error) - 1,
		NULL) == 0
	)
	#if defined(_MSC_VER) && (_MSC_VER >= 1400)
		sprintf_s(last_dyn_error, sizeof(last_dyn_error) - 1, "unknown error %lu", err);
	#else
		sprintf(last_dyn_error, "unknown error %lu", err);
	#endif
}

char * dlerror(void) {
	if(last_dyn_error[0])
		return last_dyn_error;
	return NULL;
}

void * dlsym(void *handle, const char *symbol) {
	void *ptr;
	ptr = GetProcAddress((HMODULE)handle, symbol);
	if(!ptr) {
		set_dl_error();
		return NULL;
	}
	last_dyn_error[0] = 0;
	return ptr;
}

void * dlopen(const char *path, int mode) {
	HMODULE h;
	int prevmode;
	/* Disable popup error messages when loading DLLs */
	prevmode = SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
	h = LoadLibraryA(path);
	SetErrorMode(prevmode);
	if(!h) {
		set_dl_error();
		return NULL;
	}
	last_dyn_error[0] = 0;
	return (void *)h;
}

int dlclose(void *handle) {
	if(!FreeLibrary((HMODULE)handle)) {
		set_dl_error();
		return 1;
	}
	last_dyn_error[0] = 0;
	return 0;
}

#endif // _WIN32
