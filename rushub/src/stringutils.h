/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz
 *
 * modified: 27 Aug 2009
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

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#ifndef _WIN32
	#define __int64 long long
	#include <stdlib.h> // for strtoll
#endif

using namespace ::std;

namespace utils {

bool loadFileInString(const string &, string &);

string & stringReplace(const string &, const string &, string &, const string &, bool b = false);
string & stringReplace(const string &, const string &, string &, double, bool b = false);
string & stringReplace(const string &, const string &, string &, int, bool b = false);
string & stringReplace(const string &, const string &, string &, long, bool b = false);
string & stringReplace(const string &, const string &, string &, __int64, bool b = false);

string int64ToString(__int64 const &);
__int64 stringToInt64(const string &);

int countLines(const string &);
bool limitLines(const string &, int);

void stringSplit(const string &, const char * sDelim, vector<string> &);

string & trim(string &);

}; // namespace utils

#endif // STRING_UTILS_H

/**
 * $Id$
 * $HeadURL$
 */
