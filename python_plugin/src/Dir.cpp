/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * Copyright (C) 2009-2012 by Setuper
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

#include "Dir.h"

#include <string.h>

#ifndef STR_LEN
# define STR_LEN(S) S , sizeof(S) / sizeof(S[0]) - 1
#endif

#ifdef _WIN32

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
	#pragma warning(disable:4996) // Disable "This function or variable may be unsafe."
#endif


DIR * opendir(const char * name) {
	DIR * dir = 0;
	if (name && name[0]) {
		size_t base_length = strlen(name);
		const char * all = /* search pattern must end with suitable wildcard */
		strchr("/\\", name[base_length - 1]) ? "*" : "/*";
		if ((dir = (DIR *) malloc(sizeof *dir)) != 0 &&
			(dir->name = (char *) malloc(base_length + strlen(all) + 1)) != 0)
		{
			strcpy(dir->name, name);
			strcat(dir->name, all);
			if ((dir->handle = (long) _findfirst(dir->name, &dir->info)) != -1) {
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
	}	else {
		errno = EINVAL;
	}
	return dir;
}



int closedir(DIR * dir) {
	int result = -1;
	if (dir)	{
		if (dir->handle != -1)	{
			result = _findclose(dir->handle);
		}
		free(dir->name);
		free(dir);
	}
	if (result == -1) { /* map all errors to EBADF */
		errno = EBADF;
	}
	return result;
}



struct dirent * readdir(DIR * dir) {
	struct dirent * result = 0;
	if (dir && dir->handle != -1) {
		if (!dir->result.d_name || _findnext(dir->handle, &dir->info) != -1) {
			result = &dir->result;
			result->d_name = dir->info.name;
		}
	} else {
		errno = EBADF;
	}
	return result;
}



void rewinddir(DIR * dir) {
	if (dir && dir->handle != -1) {
		_findclose(dir->handle);
		dir->handle = (long) _findfirst(dir->name, &dir->info);
		dir->result.d_name = 0;
	} else {
		errno = EBADF;
	}
}

#endif // _WIN32



namespace utils {

int Dir::mkDir(const char * path) {
	#ifdef _WIN32
		if(CreateDirectory(path, NULL)) {
			return 0;
		}
		return -1;
	#else
		return mkdir(path, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP);
	#endif
}



/** Check exists of the dir */
bool Dir::isPathExist(const char * name)	{
	DIR * dir;
	dir = opendir(name);
	if (dir == NULL) {
		return false;
	}
	closedir(dir);
	return true;
}



bool Dir::isFileExist(const char * name) {
	#ifdef _WIN32
		DWORD code = GetFileAttributes(name);
		if (code != INVALID_FILE_ATTRIBUTES && code != FILE_ATTRIBUTE_DIRECTORY) {
			return true;
		}
	#else
		struct stat st;
		if (stat(name, &st) == 0 && S_ISDIR(st.st_mode) == 0) {
			return true;
		}
	#endif
	return false;
}



void Dir::execPath(string & path) {
	#ifdef _WIN32
		char buf[MAX_PATH + 1] = { '\0' };
		::GetModuleFileName(NULL, buf, MAX_PATH);
		const char * exPath = buf;
		const char * slash = strrchr(exPath, '\\');
		path = slash ? string(exPath, slash - exPath + 1) : exPath;
	#else
		path.assign(STR_LEN("./"));
	#endif
	checkPath(path);
}



void Dir::pathForFile(const char * file, string & path) {
	const char * slash = strrchr(file, '/');
	if (!slash) {
		slash = strrchr(file, '\\');
	}
	if (slash) {
		path = string(file, slash - file + 1);
	} else {
		path = '.';
	}
}



void Dir::checkEndSlash(string & path) {

	size_t pos = path.find('\\');
	while (pos != path.npos) {
		path.replace(pos, 1, 1, '/');
		pos = path.find('\\', pos);
	}

	if (path.empty() || path.find('/', path.size() - 1) == path.npos) {
		path += '/';
	}

}



bool Dir::checkPath(string & path) {

	// Check MAX_PATH size
	if (path.size() >= MAX_PATH) {
		path.assign(path, 0, MAX_PATH - 1);
	}

	checkEndSlash(path);

	const char * dir = path.c_str();
	if (!isPathExist(dir)) {
		if (mkDir(dir) < 0) {
			return false;
		}
	}

	return true;
}

}; // namespace utils

/**
 * $Id$
 * $HeadURL$
 */
