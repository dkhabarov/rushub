/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz
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

#include "stringutils.h"
#include <string.h> // strlen

namespace utils {



/** Function of the comparison of the substring from string str1 with string str2
  (0 - equal, 1 - not equal, -1 - not is faithfully given size of the substring str1) */
int strCompare(const string & str1, size_t start, size_t count, const string & str2) {
	return str1.compare(start, count, str2);
}



/** Removing the spare reserved place in internal buffer of the string */
void shrinkStringToFit(string & str) {
	string(str.data(), str.size()).swap(str);
}



/** Removing symbols on the left */
void strCutLeft(string & str, size_t iCut) {
	if (iCut > str.length()) {
		iCut = str.length();
	}
	string(str, iCut, str.size() - iCut).swap(str);
}



/** Removing symbols on the left and record result in other string */
void strCutLeft(const string & str1, string & str2, size_t cut) {
	if (cut > str1.size()) {
		cut = str1.size();
	}
	string(str1, cut, str1.size() - cut).swap(str2);
}



/** Record from the file to the string */
bool loadFileInString(const string & fileName, string & str) {
	string buf;
	bool addLine = false;
	ifstream ifs(fileName.c_str());

	if (!ifs.is_open()) {
		return false;
	}

	while (!ifs.eof()) {
		getline(ifs, buf);
		if (addLine) {
			str += "\r\n";
		} else {
			addLine = true;
		}
		str += buf;
	}
	ifs.close();
	return true;
}



/** Searching for in string str substrings %[varname] and change all found substrings on sBy with putting the got string in sDest */
string & stringReplace(const string & str, const string & varname, string & dest, const string & by, bool b) {
	string search;
	if (!b) {
		search = "%[";
		search += varname;
		search += "]";
	} else {
		search = varname;
	}
	dest = str;
	size_t pos = dest.find(search);
	while (pos != dest.npos) {
		dest.replace(pos, search.size(), by);
		pos = dest.find(search, pos);
	}
	return dest;
}



/** Searching for in string str substrings %[varname] and change all found substrings on sBy with putting the got string in sDest */
string & stringReplace(const string & str, const string & varname, string & dest, int by, bool b) {
	ostringstream os;
	os << by;
	return stringReplace(str, varname, dest, os.str(), b);
}



/** Searching for in string str substrings %[varname] and change all found substrings on sBy with putting the got string in sDest */
string & stringReplace(const string & str, const string & varname, string & dest, double by, bool b) {
	ostringstream os;
	os << by;
	return stringReplace(str, varname, dest, os.str(), b);
}



/** Searching for in string str substrings %[varname] and change all found substrings on sBy with putting the got string in sDest */
string & stringReplace(const string & str, const string & varname, string & dest, long by, bool b) {
	ostringstream os;
	os << by;
	return stringReplace(str, varname, dest, os.str(), b);
}



/** Searching for in string str substrings %[varname] and change all found substrings on sBy with putting the got string in sDest */
string & stringReplace(const string & str, const string & varname, string & dest, __int64 by, bool b) {
	return stringReplace(str, varname, dest, int64ToString(by), b);
}



string replaceSp(const string & str, bool to) {
	string dest(str), search, rep;
	if (to) {
		search = "\\n";
		rep = "\n";
	} else {
		search = "\n";
		rep = "\\n";
	}
	size_t pos = dest.find(search);
	while (pos != dest.npos) {
		dest.replace(pos, search.size(), rep);
		pos = dest.find(search, pos);
	}
	if (to) {
		search = "\\t";
		rep = "\t";
	} else {
		search = "\t";
		rep = "\\t";
	}
	pos = dest.find(search);
	while (pos != dest.npos) {
		dest.replace(pos, search.size(), rep);
		pos = dest.find(search, pos);
	}
	return dest;
}



/** Typecasting __int64 to string */
string int64ToString(__int64 const & ll) {
	char sBuf[32] = { '\0' };
#ifdef _WIN32
	#if defined(_MSC_VER) && (_MSC_VER >= 1400)
		sprintf_s(sBuf, 32, "%I64d", ll);
	#else
		sprintf(sBuf, "%I64d", ll);
	#endif
#else
	sprintf(sBuf, "%lld", ll);
#endif
	return sBuf;
}



/** Typecasting string to __int64 */
__int64 stringToInt64(const string & str) {
#ifdef _WIN32
	__int64 result = 0;
	#if defined(_MSC_VER) && (_MSC_VER >= 1400)
		sscanf_s(str.c_str(), "%I64d", &result);
	#else
		sscanf(str.c_str(), "%I64d", &result);
	#endif
	return result;
#else
	return strtoll(str.c_str(), NULL, 10);
#endif
}



int countLines(const string & str) {
	int lines = 1;
	size_t pos = 0;
	while (str.npos != (pos = str.find_first_of("\n", pos ? pos + 1 : 0))) {
		++lines;
	}
	return lines;
}



/** Function will return true, if number of the strings less than max */
bool limitLines(const string & str, int max) {
	int lines = 1;
	size_t pos = 0;
	while (str.npos != (pos = str.find_first_of("\n", pos ? pos + 1 : 0))) {
		if (++lines > max) {
			return false;
		}
	}
	return true;
}



void stringSplit(const string & str, const char * sDelim, vector<string> & vRes) {
	size_t i, j = 0;
	while ((i = str.find(sDelim, j)) != str.npos) {
		vRes.push_back(str.substr(j, i - j));
		j = i + strlen(sDelim);
	}
	vRes.push_back(str.substr(j));
}



string & trim(string & str) {
	int begin = 0, iLen = str.size();
	while (iLen && str[iLen - 1] == ' ') {
		--iLen;
	}
	while (str[begin] == ' ') {
		++begin;
	}
	return str.assign(str, begin, iLen - begin);
}



}; // namespace utils

/**
 * $Id$
 * $HeadURL$
 */
