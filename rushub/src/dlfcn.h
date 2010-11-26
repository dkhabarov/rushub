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

#ifndef DLFCN
#define DLFCN

#ifdef _WIN32

#define RTLD_LAZY 0x001 /** Lazy function call binding. */
#define RTLD_NOW 0x002 /** Immediate function call binding. */
#define RTLD_BINDING_MASK 0x3 /** Mask of binding time value. */

char *dlerror(void);
void *dlsym(void *handle, const char *symbol);
void *dlopen(const char *path, int mode);
int dlclose(void *handle);

static char last_dyn_error[512];

#else
#	include <dlfcn.h>
#endif // _WIN32

#endif // DLFCN
