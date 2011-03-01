/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz

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

#include "Config.h"
#include "stringutils.h" // for StringToInt64

#ifndef _WIN32
	#include <memory.h>
#else
	#if defined(_MSC_VER) && (_MSC_VER >= 1400)
		#pragma warning(disable:4996) // Disable "This function or variable may be unsafe."
	#endif
#endif


using namespace ::utils; // for StringToInt64


namespace configuration {



Config::Config(void * address) : 
	mAddress(address),
	mEmpty(true)
{
}



Config::~Config() {
}



istream & operator >> (istream & is, Config & config) {
	return config.readFromStream(is);
}



ostream & operator << (ostream & os, Config & config) {
	return config.writeToStream(os);
}



/** Convert from string */
#define CCONVERTFROMCHAR() \
void ConfigChar::convertFrom(const string & str) { \
	*this = *str.c_str(); \
}



#define CCONVERTFROMPCHAR() \
void ConfigPChar::convertFrom(const string & str) { \
	char * data = this->data(); \
	if (data) { \
		delete data; \
	} \
	data = new char[str.size() + 1]; \
	memcpy(data, str.data(), str.size() + 1); \
	*this = data; \
}



#define CCONVERTFROM(TYPE, SUFFIX, SUB) \
void Config##SUFFIX::convertFrom(const string & str) { \
	*this = SUB; \
} \
void ConfigP##SUFFIX::convertFrom(const string & str) { \
	TYPE * data = this->data(); \
	if (data) { \
		delete data; \
	} \
	data = new TYPE(SUB); \
	*this = data; \
}



CCONVERTFROMCHAR();
CCONVERTFROMPCHAR();
CCONVERTFROM(bool, Bool, ((str == "true") ? true : (0 != atoi(str.c_str()))));
CCONVERTFROM(double, Double, atof(str.c_str()));
CCONVERTFROM(int, Int, atoi(str.c_str()));
CCONVERTFROM(long, Long, atol(str.c_str()));
CCONVERTFROM(unsigned int, UInt, atol(str.c_str()));
CCONVERTFROM(unsigned long, ULong, strtoul(str.c_str(), NULL, 10));
CCONVERTFROM(__int64, Int64, StringToInt64(str));
CCONVERTFROM(string, String, str);


/** Convert to string */
#define CCONVERTTO(SUFFIX, SUB) \
void Config##SUFFIX::convertTo(string & str) { \
	SUB; \
}



CCONVERTTO(Bool,    str = ((this->data()) ? "1" : "0"))
CCONVERTTO(String,  str = this->data())
CCONVERTTO(PChar,   str = this->data())
CCONVERTTO(PBool,   str = (*(this->data()) ? "1" : "0"))
CCONVERTTO(PString, str = *(this->data()))
CCONVERTTO(Char,    sprintf(mBuffer, "%c",    this->data());  str = mBuffer)
CCONVERTTO(Double,  sprintf(mBuffer, "%f",    this->data());  str = mBuffer)
CCONVERTTO(Int,     sprintf(mBuffer, "%d",    this->data());  str = mBuffer)
CCONVERTTO(Long,    sprintf(mBuffer, "%ld",   this->data());  str = mBuffer)
CCONVERTTO(UInt,    sprintf(mBuffer, "%u",    this->data());  str = mBuffer)
CCONVERTTO(ULong,   sprintf(mBuffer, "%lu",   this->data());  str = mBuffer)
CCONVERTTO(Int64,   sprintf(mBuffer, "%lld",  this->data());  str = mBuffer)
CCONVERTTO(PDouble, sprintf(mBuffer, "%f",  *(this->data())); str = mBuffer)
CCONVERTTO(PInt,    sprintf(mBuffer, "%d",  *(this->data())); str = mBuffer)
CCONVERTTO(PLong,   sprintf(mBuffer, "%ld", *(this->data())); str = mBuffer)
CCONVERTTO(PUInt,   sprintf(mBuffer, "%u",  *(this->data())); str = mBuffer)
CCONVERTTO(PULong,  sprintf(mBuffer, "%lu", *(this->data())); str = mBuffer)
CCONVERTTO(PInt64,  sprintf(mBuffer, "%lld",*(this->data())); str = mBuffer)



/** Read from stream */
#define READFROMSTREAM(SUFFIX) \
istream & Config##SUFFIX::readFromStream(istream & is) { \
	string str; \
	is >> str; \
	this->convertFrom(str); \
	return is; \
}



#define READFROMSTREAMTOSTR(SUFFIX) \
istream & Config##SUFFIX::readFromStream(istream & is) { \
	string str; \
	stringstream ss; \
	getline(is, str); \
	ss << str; str.empty(); \
	while (is.good()) { \
		str.empty(); \
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
READFROMSTREAM(Long);
READFROMSTREAM(UInt);
READFROMSTREAM(ULong);
READFROMSTREAM(Int64);
READFROMSTREAM(Char);
READFROMSTREAM(PBool);
READFROMSTREAM(PDouble);
READFROMSTREAM(PInt);
READFROMSTREAM(PLong);
READFROMSTREAM(PUInt);
READFROMSTREAM(PULong);
READFROMSTREAM(PInt64);



/** Write in stream */
#define WRITETOSTREAM(SUFFIX) \
ostream & Config##SUFFIX::writeToStream(ostream & os) { \
	string str; \
	this->convertTo(str); \
	os << str; \
	return os; \
}



WRITETOSTREAM(Bool);
WRITETOSTREAM(Double);
WRITETOSTREAM(Int);
WRITETOSTREAM(Long);
WRITETOSTREAM(UInt);
WRITETOSTREAM(ULong);
WRITETOSTREAM(Int64);
WRITETOSTREAM(Char);
WRITETOSTREAM(String);
WRITETOSTREAM(PChar);
WRITETOSTREAM(PBool);
WRITETOSTREAM(PDouble);
WRITETOSTREAM(PInt);
WRITETOSTREAM(PLong);
WRITETOSTREAM(PUInt);
WRITETOSTREAM(PULong);
WRITETOSTREAM(PInt64);
WRITETOSTREAM(PString);

/** NULL values */
#define ISNULL(SUFFIX)  bool Config##SUFFIX::isNull()  { return !this->data(); } // 0, false, \0
#define ISNULLPCHAR()   bool ConfigPChar ::isNull()    { return !this->data() || !*(this->data()); } // 0 or \0
#define ISNULLDOUBLE()  bool ConfigDouble::isNull()    { return this->data() == 0.; } // 0.
#define ISNULLSTRING()  bool ConfigString::isNull()    { return !this->data().size(); } // ""
#define ISNULLP(SUFFIX) bool ConfigP##SUFFIX::isNull() { return !this->data() || !*(this->data()); }
#define ISNULLPDOUBLE() bool ConfigPDouble::isNull()   { return !this->data() || *(this->data()) == 0.; }
#define ISNULLPSTRING() bool ConfigPString::isNull()   { return !this->data() || !((*(this->data())).size()); }

ISNULL(Bool);
ISNULL(Int);
ISNULL(UInt);
ISNULL(Long);
ISNULL(Int64);
ISNULL(ULong);
ISNULL(Char);
ISNULLPCHAR();
ISNULLDOUBLE();
ISNULLSTRING();
ISNULLP(Bool);
ISNULLP(Int);
ISNULLP(UInt);
ISNULLP(Long);
ISNULLP(Int64);
ISNULLP(ULong);
ISNULLPDOUBLE();
ISNULLPSTRING();

}; // namespace configuration
