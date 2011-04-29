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

#ifndef DIR_H
#define DIR_H

/**
	Functions for work with dirs like unix systems
*/

#include <string>

using namespace ::std;

#ifdef _WIN32

	#include <errno.h>
	#include <io.h> /* _findfirst and _findnext set errno iff they return -1 */
	#include <stdlib.h>

	#include <windows.h>

	struct dirent {
		char * d_name;
	};

	typedef struct DIR {
		long handle; /* -1 for failed rewind */
		struct _finddata_t  info;
		struct dirent  result; /* d_name null iff first time */
		char * name;  /* null-terminated char string */
	} DIR; // struct DIR

	DIR * opendir(const char *);
	int closedir(DIR *);
	struct dirent * readdir(DIR *);
	void rewinddir(DIR *);

#else

	#include <sys/types.h>
	#include <dirent.h>
	#include <sys/types.h> /** mkdir */
	#include <sys/stat.h> /** mkdir */
	#include <stdlib.h>

	#ifndef MAX_PATH
	#define MAX_PATH 256
	#endif

#endif // _WIN32

namespace utils {

class Dir {

public:

	/** Create dir */
	static int mkDir(const char * path);

	/** Check exists of the path */
	static bool isPathExist(const char * name);

	static bool isFileExist(const char * name);

	static void execPath(string &);

	static string pathForFile(const char * filePath);

	static void checkEndSlash(string &);

	static bool checkPath(string &);

}; // class Dir

}; // namespace utils

#endif // DIR_H

/**
* $Id$
* $HeadURL$
*/
