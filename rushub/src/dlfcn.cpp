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

static void set_dl_error() {
	DWORD err = GetLastError();
	if (FormatMessageA(FORMAT_MESSAGE_IGNORE_INSERTS |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		err,
		MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
		last_dyn_error,
		sizeof(last_dyn_error) - 1,
		NULL) == 0
	) {
		#if defined(_MSC_VER) && (_MSC_VER >= 1400)
			sprintf_s(last_dyn_error, sizeof(last_dyn_error) - 1, "unknown error %lu", err);
		#else
			sprintf(last_dyn_error, "unknown error %lu", err);
		#endif
	}
}

char * dlerror() {
	if (last_dyn_error[0]) {
		return last_dyn_error;
	}
	return NULL;
}

void * dlsym(void * handle, const char * symbol) {
	void * ptr = GetProcAddress((HMODULE)handle, symbol);
	if (!ptr) {
		set_dl_error();
		return NULL;
	}
	last_dyn_error[0] = 0;
	return ptr;
}

void * dlopen(const char * path, int) {
	HMODULE h;
	int prevmode;
	/* Disable popup error messages when loading DLLs */
	prevmode = SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
	h = LoadLibraryA(path);
	SetErrorMode(prevmode);
	if (!h) {
		set_dl_error();
		return NULL;
	}
	last_dyn_error[0] = 0;
	return (void *)h;
}

int dlclose(void * handle) {
	if (!FreeLibrary((HMODULE)handle)) {
		set_dl_error();
		return 1;
	}
	last_dyn_error[0] = 0;
	return 0;
}

#elif defined(__APPLE__)

#include <mach-o/dyld.h>

static void set_dl_error() {
	NSLinkEditErrors errs;
	int num;
	const char * file;
	const char * err;
	NSLinkEditError(&errs, &num, &file, &err);
	strcpy(last_dyn_error, err);
}

char * dlerror() {
	if (last_dyn_error[0]) {
		return last_dyn_error;
	}
	return NULL;
}

void * dlsym(void * handle, const char * symbol) {
	NSSymbol nsSymbol = NSLookupSymbolInModule((NMModule) handle, symbol);
	if (nsSymbol == NULL) {
		sprintf(last_dyn_error, "symbol %s not found", symbol);
		return NULL;
	}
	return (void *) NSAddressOfSymbol(nsSymbol);
}

void * dlopen(const char * path, int) {
	if (!_dyld_present()) {
		strcpy(last_dyn_error, "dyld not present");
		return NULL;
	}

	NSObjectFileImage file;
	NSObjectFileImageReturnCode code = NSCreateObjectFileImageFromFile(path, &file);
	if (code != NSObjectFileImageSuccess) {
		switch (code) {
			case NSObjectFileImageInappropriateFile :
				strcpy(last_dyn_error, "file is not a bundle");

			case NSObjectFileImage :
				strcpy(last_dyn_error, "library is for wrong CPU type");

			case NSObjectFileImage :
				strcpy(last_dyn_error, "bad format");

			case NSObjectFileImage :
				strcpy(last_dyn_error, "cannot access file");

			default:
				strcpy(last_dyn_error, "unknown error");

		}
		return NULL;
	}

	NSModule module = NSLinkModule(file, path,
		NSLINKMODULE_OPTION_PRIVATE | NSLINKMODULE_OPTION_RETURN_ON_ERROR);
	NSDestroyObjectFileImage(file);

	if (module == NULL) {
		set_dl_error();
		return NULL;
	}

	last_dyn_error[0] = 0;
	return module;
}

int dlclose(void * handle) {
	NSUnLinkModule((NSModule) handle,
		NSUNLINKMODULE_OPTION_RESET_LAZY_REFERENCES);
	return 0;
}

#endif // _WIN32
