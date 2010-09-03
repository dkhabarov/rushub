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

#ifndef CDIR_H
#define CDIR_H

/**
  Functions for work with dirs like unix systems
*/

#ifdef _WIN32

  #include <errno.h>
  #include <io.h> /* _findfirst and _findnext set errno iff they return -1 */
  #include <stdlib.h>
  #include <string.h>

  #include <windows.h>

  /** Create dir */
  int mkdir(const char * path, int mode);

  typedef struct DIR DIR;

  struct dirent
  {
    char *d_name;
  };

  DIR           *opendir(const char *);
  int           closedir(DIR *);
  struct dirent *readdir(DIR *);
  void          rewinddir(DIR *);

#else

  #include <sys/types.h>
  #include <dirent.h>
  #include <sys/types.h> /** mkdir */
  #include <sys/stat.h> /** mkdir */
  #include <stdlib.h>

#endif // _WIN32

/** Check exists of the dir */
bool DirExists(const char* sName);

bool FileExists(const char * sName);

#endif // CDIR_H
