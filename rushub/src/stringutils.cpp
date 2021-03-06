/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz
 *
 * modified: 27 Aug 2009
 * Copyright (C) 2009-2013 by Setuper
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


static const int cp1251ToUtf8Table[128] = {                    
	0x82D0,0x83D0,0x9A80E2,0x93D1,0x9E80E2,0xA680E2,0xA080E2,0xA180E2,
	0xAC82E2,0xB080E2,0x89D0,0xB980E2,0x8AD0,0x8CD0,0x8BD0,0x8FD0,    
	0x92D1,0x9880E2,0x9980E2,0x9C80E2,0x9D80E2,0xA280E2,0x9380E2,0x9480E2,
	0,0xA284E2,0x99D1,0xBA80E2,0x9AD1,0x9CD1,0x9BD1,0x9FD1,               
	0xA0C2,0x8ED0,0x9ED1,0x88D0,0xA4C2,0x90D2,0xA6C2,0xA7C2,              
	0x81D0,0xA9C2,0x84D0,0xABC2,0xACC2,0xADC2,0xAEC2,0x87D0,              
	0xB0C2,0xB1C2,0x86D0,0x96D1,0x91D2,0xB5C2,0xB6C2,0xB7C2,              
	0x91D1,0x9684E2,0x94D1,0xBBC2,0x98D1,0x85D0,0x95D1,0x97D1,            
	0x90D0,0x91D0,0x92D0,0x93D0,0x94D0,0x95D0,0x96D0,0x97D0,
	0x98D0,0x99D0,0x9AD0,0x9BD0,0x9CD0,0x9DD0,0x9ED0,0x9FD0,
	0xA0D0,0xA1D0,0xA2D0,0xA3D0,0xA4D0,0xA5D0,0xA6D0,0xA7D0,
	0xA8D0,0xA9D0,0xAAD0,0xABD0,0xACD0,0xADD0,0xAED0,0xAFD0,
	0xB0D0,0xB1D0,0xB2D0,0xB3D0,0xB4D0,0xB5D0,0xB6D0,0xB7D0,
	0xB8D0,0xB9D0,0xBAD0,0xBBD0,0xBCD0,0xBDD0,0xBED0,0xBFD0,
	0x80D1,0x81D1,0x82D1,0x83D1,0x84D1,0x85D1,0x86D1,0x87D1,
	0x88D1,0x89D1,0x8AD1,0x8BD1,0x8CD1,0x8DD1,0x8ED1,0x8FD1
};



/** Typecasting int64_t to string */
string int64ToString(int64_t const & value) {
	char buf[32] = { '\0' };
#ifdef _WIN32
	#if defined(_MSC_VER) && (_MSC_VER >= 1400)
		sprintf_s(buf, 32, "%I64d", value);
	#else
		sprintf(buf, "%I64d", value);
	#endif
#else
	sprintf(buf, "%lld", value);
#endif
	return buf;
}



/** Typecasting string to int64_t */
int64_t stringToInt64(const string & str) {
#ifdef _WIN32
	int64_t result = 0;
	#if defined(_MSC_VER) && (_MSC_VER >= 1400)
		sscanf_s(str.c_str(), "%I64d", &result);
	#else
		sscanf(str.c_str(), "%20I64d", &result);
	#endif
	return result;
#else
	return strtoll(str.c_str(), NULL, 10);
#endif
}



void stringSplit(const string & str, const char * delim, vector<string> & res) {
	size_t i, j = 0;
	const size_t len = strlen(delim);
	while ((i = str.find(delim, j)) != string::npos) {
		res.push_back(str.substr(j, i - j));
		j = i + len;
	}
	res.push_back(str.substr(j));
}



string & trim(string & str) {
	str.erase(0, str.find_first_not_of(' '));
	return str.erase(str.find_last_not_of(' ') + 1);
}



string & cp1251ToUtf8(const string & in, string & out, void (*escape)(char, string &) /*= NULL*/) {
	out.clear();
	out.reserve(3 * in.size());
	const char * ch = in.c_str();
	while (*ch) {
		if (*ch & 0x80) {
			int v = cp1251ToUtf8Table[static_cast<int> (0x7f & *ch++)];
			if (v) {
				out += static_cast<char> (v);
				out += static_cast<char> (v >> 8);
				if (v >>= 16) {
					out += static_cast<char> (v);
				}
			}
		} else {
			if (escape != NULL) {
				(*escape)(*ch++, out);
			} else {
				out += *ch++;
			}
		}
	}
	return out;
}


/**	Check if the given string is a valid utf-8 sequence */
bool isUtf8(const char * str, size_t len) {
	if (len > 0) {
		int expect = 0;
		for (size_t pos = 0; pos < len; ++pos) {
			if (expect) {
				if ((str[pos] & 0xC0) == 0x80) {
					--expect;
				} else {
					return false;
				}
			} else {
				if (str[pos] & 0x80) {
					char div = 0;
					for (div = 0x40; div > 0x10; div /= 2) {
						if (str[pos] & div) {
							++expect;
						} else {
							break;
						}
					}
					if ((str[pos] & div) || (pos + expect >= len)) {
						return false;
					}
				}
			}
		}
	}
	return true;
}



bool isBase32(char c) {
	return (c >= 0x41 && c <= 0x5a) || (c >= 0x32 && c <= 0x37);
}



bool isUpperAlpha(char c) {
	return c >= 0x41 && c <= 0x5a;
}



bool isUpperAlphaNum(char c) {
	return (c >= 0x41 && c <= 0x5a) || (c >= 0x30 && c <= 0x39);
}


} // namespace utils

/**
 * $Id$
 * $HeadURL$
 */
