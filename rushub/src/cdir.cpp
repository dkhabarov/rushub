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

#include "cdir.h"
#include <string.h>

#ifdef _WIN32

struct DIR {
	long                handle; /* -1 for failed rewind */
	struct _finddata_t  info;
	struct dirent       result; /* d_name null iff first time */
	char                *name;  /* null-terminated char string */
};

DIR *opendir(const char *name) {
	DIR *dir = 0;
	if(name && name[0]) {
		size_t base_length = strlen(name);
		const char *all = /* search pattern must end with suitable wildcard */
		strchr("/\\", name[base_length - 1]) ? "*" : "/*";
		if((dir = (DIR *) malloc(sizeof *dir)) != 0 &&
			(dir->name = (char *) malloc(base_length + strlen(all) + 1)) != 0)
		{
			strcat(strcpy(dir->name, name), all);
			if((dir->handle = (long) _findfirst(dir->name, &dir->info)) != -1) {
				dir->result.d_name = 0;
			} else { /* rollback */
				free(dir->name);
				free(dir);
				dir = 0;
			}
		} else { /* rollback */
			free(dir);
			dir = 0;
			errno = ENOMEM;
		}
	}	else	{
		errno = EINVAL;
	}
	return dir;
}

int closedir(DIR *dir) {
	int result = -1;
	if(dir)	{
		if(dir->handle != -1)	{
			result = _findclose(dir->handle);
		}
		free(dir->name);
		free(dir);
	}
	if(result == -1) { /* map all errors to EBADF */
		errno = EBADF;
	}
	return result;
}

struct dirent *readdir(DIR *dir) {
	struct dirent *result = 0;
	if(dir && dir->handle != -1) {
		if(!dir->result.d_name || _findnext(dir->handle, &dir->info) != -1) {
			result = &dir->result;
			result->d_name = dir->info.name;
		}
	} else {
		errno = EBADF;
	}
	return result;
}

void rewinddir(DIR *dir) {
	if(dir && dir->handle != -1) {
		_findclose(dir->handle);
		dir->handle = (long) _findfirst(dir->name, &dir->info);
		dir->result.d_name = 0;
	} else {
		errno = EBADF;
	}
}

int mkdir(const char * path, int mode) {
	if(CreateDirectoryA(path, NULL))
		return 0;
	return -1;
}

#endif // _WIN32


/** Check exists of the dir */
bool DirExists(const char* sName)	{
	DIR *dir;
	dir = opendir(sName);
	if(dir == NULL) return false;
	closedir(dir);
	return true;
}

bool FileExists(const char * sName) {
#ifdef _WIN32
	DWORD code = GetFileAttributesA(sName);
	if(code != INVALID_FILE_ATTRIBUTES && code != FILE_ATTRIBUTE_DIRECTORY) return true;
#else
	struct stat st;
	if(stat(sName, &st) == 0 && S_ISDIR(st.st_mode) == 0) return true;
#endif
	return false;
}

void ExecPath(string & sPath) {
	#ifdef _WIN32
		char * sExPath = NULL;
		char sBuf[MAX_PATH+1];
		::GetModuleFileName(NULL, sBuf, MAX_PATH);
		sExPath = sBuf;
		char * sSlash = strrchr(sExPath, '\\');
		if(sSlash) sPath = string(sExPath, sSlash - sExPath);
		else sPath = sExPath;
		size_t iPos = sPath.find("\\");
		while(iPos != sPath.npos) {
			sPath.replace(iPos, 1, "/");
			iPos = sPath.find("\\", iPos);
		}
		sPath.append("/");
	#else
		char * sHomeDir = getenv("HOME");
		if(sHomeDir) {
			sPath = sHomeDir;
			sPath.append("/rushub");
		} else {
			sPath = "./.rushub";
		}
	#endif

	/* A check to existing path */
	const char * sDir = sPath.c_str();
	if(!DirExists(sDir))
		mkdir(sDir, NULL);
}

void CheckEndSlash(string & sPath) {
	size_t iPos = sPath.find("\\");
	while(iPos != sPath.npos) {
		sPath.replace(iPos, 1, "/");
		iPos = sPath.find("\\", iPos);
	}
	if(sPath.substr(sPath.size()-1, 1) != "/")
		sPath.append("/");
}

void CheckPath(string & sPath) {
	// Check MAX_PATH size
	if(sPath.size() >= MAX_PATH) {
		sPath = sPath.substr(0, MAX_PATH - 1);
		CheckEndSlash(sPath);
	}
	const char * sDir = sPath.c_str();
	if(!DirExists(sDir)) mkdir(sDir, NULL);
}
