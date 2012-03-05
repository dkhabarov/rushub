/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz
 *
 * modified: 27 Aug 2009
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

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include "stdinc.h"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#ifndef _WIN32
	#include <stdlib.h> // for strtoll
#endif

#ifndef STR_LEN
# define STR_LEN(S) S , sizeof(S) / sizeof(S[0]) - 1
#endif

using namespace ::std;

/// Utils namespace
namespace utils {

static const string emptyStr("");

string int64ToString(int64_t const &);
int64_t stringToInt64(const string &);


void stringSplit(const string &, const char * sDelim, vector<string> &);

string & trim(string &);

string & cp1251ToUtf8(const string & in, string & out, void (*escape)(char, string &) = NULL);

bool isBase32(char);
bool isUpperAlpha(char);
bool isUpperAlphaNum(char);

template <class T>
string & toString(const T & value, string & str) {
	ostringstream oss;
	oss << value;
	str = oss.str();
	return str;
}

/** Searching for in string str substrings %[varname] and change all found substrings on pattern with putting the got string in dest */
template <class T>
string & stringReplace(const string & str, const string & varname, string & dest, const T & pattern, bool b = false, bool first = false) {
	string by;
	toString(pattern, by);
	dest = str;
	if (!b) {
		string search(STR_LEN("%["));
		search.append(varname);
		search.append(STR_LEN("]"));
		size_t pos = dest.find(search);
		if (first != true || pos == 0) {
			while (pos != dest.npos) {
				dest.replace(pos, search.size(), by);
				pos = dest.find(search, pos + by.size());
			}
		}
	} else {
		size_t pos = dest.find(varname);
		if (first != true || pos == 0) {
			while (pos != dest.npos) {
				dest.replace(pos, varname.size(), by);
				pos = dest.find(varname, pos + by.size());
			}
		}
	}
	return dest;
}


}; // namespace utils

#endif // STRING_UTILS_H

/**
 * $Id$
 * $HeadURL$
 */
