/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz

 * modified: 10 Dec 2009
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

#include "stringutils.h"

namespace nUtils {

/** Function of the comparison of the substring from string str1 with string str2
  (0 - equal, 1 - not equal, -1 - not is faithfully given size of the substring str1) */
int StrCompare(const string &str1, int start, int count, const string &str2) {
	return str1.compare(start, count, str2);
}

/** Removing the spare reserved place in internal buffer of the string */
void ShrinkStringToFit(string &str) {
	string(str.data(), str.size()).swap(str);
}

/** Removing symbols on the left */
void StrCutLeft(string &sStr, size_t iCut) {
	if(iCut > sStr.length()) iCut = sStr.length();
	string(sStr, iCut, sStr.size() - iCut).swap(sStr);
}

/** Removing symbols on the left and record result in other string */
void StrCutLeft(const string &sStr1, string &sStr2, size_t iCut) {
	if(iCut > sStr1.size()) iCut = sStr1.size();
	string(sStr1, iCut, sStr1.size() - iCut).swap(sStr2);
}

/** Record from the file to the string */
bool LoadFileInString(const string &sFileName, string &sStr) {
	string sBuf;
	bool bAddLine = false;
	ifstream ifs(sFileName.c_str());

	if(!ifs.is_open()) return false;
	while(!ifs.eof()) {
		getline(ifs, sBuf);
		if(bAddLine) sStr += "\r\n";
		else bAddLine = true;
		sStr += sBuf;
	}
	ifs.close();
	return true;
}

/** Searching for in string sStr substrings %[varname] and change all found substrings on sBy with putting the got string in sDest */
string & StringReplace(const string &sStr, const string &sVarname, string &sDest, const string &sBy, bool b) {
	string sSearchvar;
	if(!b) {
		sSearchvar = "%[";
		sSearchvar += sVarname;
		sSearchvar += "]";
	} else sSearchvar = sVarname;
	sDest = sStr;
	size_t iPos = sDest.find(sSearchvar);
	while(iPos != sDest.npos) {
		sDest.replace(iPos, sSearchvar.size(), sBy);
		iPos = sDest.find(sSearchvar, iPos);
	}
	return sDest;
}

/** Searching for in string sStr substrings %[varname] and change all found substrings on sBy with putting the got string in sDest */
string & StringReplace(const string &sStr, const string &sVarname, string &sDest, int iBy, bool b) {
	ostringstream os;
	os << iBy;
	return StringReplace(sStr, sVarname, sDest, os.str(), b);
}

/** Searching for in string sStr substrings %[varname] and change all found substrings on sBy with putting the got string in sDest */
string & StringReplace(const string &sStr, const string &sVarname, string &sDest, double iBy, bool b) {
	ostringstream os;
	os << iBy;
	return StringReplace(sStr, sVarname, sDest, os.str(), b);
}

/** Searching for in string sStr substrings %[varname] and change all found substrings on sBy with putting the got string in sDest */
string & StringReplace(const string &sStr, const string &sVarname, string &sDest, long iBy, bool b) {
	ostringstream os;
	os << iBy;
	return StringReplace(sStr, sVarname, sDest, os.str(), b);
}

/** Searching for in string sStr substrings %[varname] and change all found substrings on sBy with putting the got string in sDest */
string & StringReplace(const string &sStr, const string &sVarname, string &sDest, __int64 iBy, bool b) {
	return StringReplace(sStr, sVarname, sDest, Int64ToString(iBy), b);
}

string ReplaceSp(const string &sStr, bool bTo) {
	string sDest(sStr), sSearch, sRep;
	if(bTo) {
		sSearch = "\\n";
		sRep = "\n";
	} else {
		sSearch = "\n";
		sRep = "\\n";
	}
	size_t iPos = sDest.find(sSearch);
	while(iPos != sDest.npos) {
		sDest.replace(iPos, sSearch.size(), sRep);
		iPos = sDest.find(sSearch, iPos);
	}
	if(bTo) {
		sSearch = "\\t";
		sRep = "\t";
	} else {
		sSearch = "\t";
		sRep = "\\t";
	}
	iPos = sDest.find(sSearch);
	while(iPos != sDest.npos) {
		sDest.replace(iPos, sSearch.size(), sRep);
		iPos = sDest.find(sSearch, iPos);
	}
	return sDest;
}

/** Typecasting __int64 to string */
string Int64ToString(__int64 const &ll) {
	char sBuf[32];
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
__int64 StringToInt64(const string &sStr) {
#ifdef _WIN32
	__int64 iResult = 0;
	#if defined(_MSC_VER) && (_MSC_VER >= 1400)
		sscanf_s(sStr.c_str(), "%I64d", &iResult);
	#else
		sscanf(sStr.c_str(), "%I64d", &iResult);
	#endif
	return iResult;
#else
	return strtoll(sStr.c_str(), NULL, 10);
#endif
}

int CountLines(const string &sStr) {
	int iLines = 1;
	size_t iPos = 0;
	while(sStr.npos != (iPos = sStr.find_first_of("\n", iPos ? iPos + 1 : 0)))
		++iLines;
	return iLines;
}

/** Function will return true, if number of the strings less than iMax */
bool LimitLines(const string &sStr, int iMax) {
	int iLines = 1;
	size_t iPos = 0;
	while(sStr.npos != (iPos = sStr.find_first_of("\n", iPos ? iPos + 1 : 0)))
		if(++iLines > iMax) return false;
	return true;
}

void StringSplit(const string & sStr, char sDelim, vector<string> & vRes) {
	unsigned i,j = 0;
	while( (i = sStr.find_first_of(sDelim, j)) != sStr.npos ) {
		vRes.push_back(sStr.substr(j,i - j));
		j = i + 1;
	}
	vRes.push_back(sStr.substr(j));
}

string & trim(string & sStr) {
	int iBeg = 0, iLen = sStr.size();
	while(iLen && sStr[iLen - 1] == ' ') --iLen;
	while(sStr[iBeg] == ' ') ++iBeg;
	return sStr.assign(sStr, iBeg, iLen - iBeg);
}

}; // nUtils
