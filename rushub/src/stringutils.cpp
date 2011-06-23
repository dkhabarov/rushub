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
			str.append("\r\n", 2);
		} else {
			addLine = true;
		}
		str.append(buf);
	}
	ifs.close();
	return true;
}



/** Searching for in string str substrings %[varname] and change all found substrings on sBy with putting the got string in sDest */
string & stringReplace(const string & str, const string & varname, string & dest, const string & by, bool b, bool first) {
	dest = str;
	if (!b) {
		string search("%[");
		search.append(varname);
		search.append("]", 1);
		size_t pos = dest.find(search);
		if (first == true && pos != 0) {
			return dest;
		}
		while (pos != dest.npos) {
			dest.replace(pos, search.size(), by);
			pos = dest.find(search, pos);
		}
	} else {
		size_t pos = dest.find(varname);
		if (first == true && pos != 0) {
			return dest;
		}
		while (pos != dest.npos) {
			dest.replace(pos, varname.size(), by);
			pos = dest.find(varname, pos);
		}
	}
	
	
	return dest;
}



/** Searching for in string str substrings %[varname] and change all found substrings on sBy with putting the got string in sDest */
string & stringReplace(const string & str, const string & varname, string & dest, int by, bool b, bool first) {
	ostringstream os;
	os << by;
	return stringReplace(str, varname, dest, os.str(), b, first);
}



/** Searching for in string str substrings %[varname] and change all found substrings on sBy with putting the got string in sDest */
string & stringReplace(const string & str, const string & varname, string & dest, double by, bool b, bool first) {
	ostringstream os;
	os << by;
	return stringReplace(str, varname, dest, os.str(), b, first);
}



/** Searching for in string str substrings %[varname] and change all found substrings on sBy with putting the got string in sDest */
string & stringReplace(const string & str, const string & varname, string & dest, long by, bool b, bool first) {
	ostringstream os;
	os << by;
	return stringReplace(str, varname, dest, os.str(), b, first);
}



/** Searching for in string str substrings %[varname] and change all found substrings on sBy with putting the got string in sDest */
string & stringReplace(const string & str, const string & varname, string & dest, __int64 by, bool b, bool first) {
	return stringReplace(str, varname, dest, int64ToString(by), b, first);
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
	str.erase(0, str.find_first_not_of(' '));
	return str.erase(str.find_last_not_of(' ') + 1);
}



}; // namespace utils

/**
 * $Id$
 * $HeadURL$
 */
