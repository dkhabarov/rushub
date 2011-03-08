/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz

 * modified: 27 Aug 2009
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

/** Function of the comparison of the substring from string str1 with string str2
  (0 - equal, 1 - not equal, -1 - not is faithfully given size of the substring str1) */
int StrCompare(const string &str1, size_t start, size_t count, const string &str2);

/** Removing the spare reserved place in internal buffer of the string */
void ShrinkStringToFit(string &str);

/** Removing symbols on the left */
void StrCutLeft(string &, size_t);

/** Removing symbols on the left and record result in other string */
void StrCutLeft(const string &, string &, size_t);

bool LoadFileInString(const string &, string &);

string & StringReplace(const string &, const string &, string &, const string &, bool b = false);
string & StringReplace(const string &, const string &, string &, double, bool b = false);
string & StringReplace(const string &, const string &, string &, int, bool b = false);
string & StringReplace(const string &, const string &, string &, long, bool b = false);
string & StringReplace(const string &, const string &, string &, __int64, bool b = false);

string ReplaceSp(const string &, bool bTo = false);

string Int64ToString(__int64 const &);
__int64 StringToInt64(const string &);

int CountLines(const string &);
bool LimitLines(const string &, int);

void StringSplit(const string &, char sDelim, vector<string> &);

string & trim(string &);

}; // namespace utils

#endif // STRING_UTILS_H
