/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz
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

#include "ConfigItem.h"
#include "stringutils.h" // for stringToInt64

#include <math.h>

#ifndef _WIN32
	#include <memory.h>
#else
	#if defined(_MSC_VER) && (_MSC_VER >= 1400)
		#pragma warning(disable:4996) // Disable "This function or variable may be unsafe."
	#endif
#endif


using namespace ::utils; // for stringToInt64


/// Main configuration namespace
namespace configuration {



ConfigItem::ConfigItem(void * address) : 
	mAddress(address),
	mEmpty(true)
{
}



ConfigItem::~ConfigItem() {
}



istream & operator >> (istream & is, ConfigItem & configItem) {
	return configItem.readFromStream(is);
}



ostream & operator << (ostream & os, ConfigItem & configItem) {
	return configItem.writeToStream(os);
}



/** Convert from string */
#define CCONVERTFROMCHAR() \
void ConfigItemChar::convertFrom(const string & str) { \
	*this = *str.c_str(); \
}



#define CCONVERTFROMPCHAR() \
void ConfigItemPChar::convertFrom(const string & str) { \
	char * data = this->data(); \
	if (data) { \
		delete data; \
	} \
	data = new char[str.size() + 1]; \
	memcpy(data, str.data(), str.size() + 1); \
	*this = data; \
}



#define CCONVERTFROM(TYPE, SUFFIX, SUB) \
void ConfigItem##SUFFIX::convertFrom(const string & str) { \
	*this = SUB; \
} \
void ConfigItemP##SUFFIX::convertFrom(const string & str) { \
	TYPE * data = this->data(); \
	if (data) { \
		delete data; \
	} \
	data = new TYPE; \
	*data = SUB; \
	*this = data; \
}



CCONVERTFROMCHAR();
CCONVERTFROMPCHAR();
CCONVERTFROM(bool, Bool, ((str == "true") ? true : (0 != atoi(str.c_str()))));
CCONVERTFROM(double, Double, atof(str.c_str()));
CCONVERTFROM(int, Int, atoi(str.c_str()));
CCONVERTFROM(unsigned int, UInt, atol(str.c_str()));
CCONVERTFROM(int64_t, Int64, stringToInt64(str));
CCONVERTFROM(string, String, str);

/** Convert to string */
void ConfigItemDouble::convertTo(string & str) {
	toString(this->data(), str);
	if (str.find('.') == str.npos) {
		str.append(STR_LEN(".0")); // double mark
	}
}

void ConfigItemPDouble::convertTo(string & str) {
	toString(*(this->data()), str);
	if (str.find('.') == str.npos) {
		str.append(STR_LEN(".0")); // double mark
	}
}

#define CCONVERTTO(SUFFIX) \
void ConfigItem##SUFFIX::convertTo(string & str) { \
	toString(this->data(), str); \
}

#define CCONVERTPTO(SUFFIX) \
void ConfigItem##SUFFIX::convertTo(string & str) { \
	toString(*(this->data()), str); \
}

CCONVERTTO(Bool)
CCONVERTTO(String)
CCONVERTTO(Char)
CCONVERTTO(Int)
CCONVERTTO(UInt)
CCONVERTTO(Int64)
CCONVERTPTO(PBool)
CCONVERTPTO(PString)
CCONVERTPTO(PChar)
CCONVERTPTO(PInt)
CCONVERTPTO(PUInt)
CCONVERTPTO(PInt64)



/** Read from stream */
#define READFROMSTREAM(SUFFIX) \
istream & ConfigItem##SUFFIX::readFromStream(istream & is) { \
	string str; \
	is >> str; \
	this->convertFrom(str); \
	return is; \
}



#define READFROMSTREAMTOSTR(SUFFIX) \
istream & ConfigItem##SUFFIX::readFromStream(istream & is) { \
	string str; \
	stringstream ss; \
	getline(is, str); \
	ss << str; \
	while (is.good()) { \
		getline(is, str); \
		ss << endl << str; \
	} \
	this->convertFrom(ss.str()); \
	return is; \
}



READFROMSTREAMTOSTR(String);
READFROMSTREAMTOSTR(PChar);
READFROMSTREAMTOSTR(PString);
READFROMSTREAM(Bool);
READFROMSTREAM(Double);
READFROMSTREAM(Int);
READFROMSTREAM(UInt);
READFROMSTREAM(Int64);
READFROMSTREAM(Char);
READFROMSTREAM(PBool);
READFROMSTREAM(PDouble);
READFROMSTREAM(PInt);
READFROMSTREAM(PUInt);
READFROMSTREAM(PInt64);



/** Write in stream */
#define WRITETOSTREAM(SUFFIX) \
ostream & ConfigItem##SUFFIX::writeToStream(ostream & os) { \
	string str; \
	this->convertTo(str); \
	os << str; \
	return os; \
}



WRITETOSTREAM(Bool);
WRITETOSTREAM(Double);
WRITETOSTREAM(Int);
WRITETOSTREAM(UInt);
WRITETOSTREAM(Int64);
WRITETOSTREAM(Char);
WRITETOSTREAM(String);
WRITETOSTREAM(PChar);
WRITETOSTREAM(PBool);
WRITETOSTREAM(PDouble);
WRITETOSTREAM(PInt);
WRITETOSTREAM(PUInt);
WRITETOSTREAM(PInt64);
WRITETOSTREAM(PString);



/** NULL values */
#define ISNULL(SUFFIX)  bool ConfigItem##SUFFIX::isNull()  { return !this->data(); } // 0, false, \0
#define ISNULLPCHAR()   bool ConfigItemPChar ::isNull()    { return !this->data() || !*(this->data()); } // 0 or \0
#define ISNULLDOUBLE()  bool ConfigItemDouble::isNull()    { return fabs(this->data() - 0.) < 10e-7; } // 0.
#define ISNULLSTRING()  bool ConfigItemString::isNull()    { return !this->data().size(); } // ""
#define ISNULLP(SUFFIX) bool ConfigItemP##SUFFIX::isNull() { return !this->data() || !*(this->data()); }
#define ISNULLPDOUBLE() bool ConfigItemPDouble::isNull()   { return !this->data() || fabs(*(this->data()) - 0.) < 10e-7; }
#define ISNULLPSTRING() bool ConfigItemPString::isNull()   { return !this->data() || !((*(this->data())).size()); }

ISNULL(Bool);
ISNULL(Int);
ISNULL(UInt);
ISNULL(Int64);
ISNULL(Char);
ISNULLPCHAR();
ISNULLDOUBLE();
ISNULLSTRING();
ISNULLP(Bool);
ISNULLP(Int);
ISNULLP(UInt);
ISNULLP(Int64);
ISNULLPDOUBLE();
ISNULLPSTRING();


}; // namespace configuration

/**
 * $Id$
 * $HeadURL$
 */
