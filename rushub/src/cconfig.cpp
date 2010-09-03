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

#include "cconfig.h"
#include "stringutils.h" /** for StringToInt64 */

#ifndef _WIN32
  #include <memory.h>
#endif

using namespace nUtils; /** for StringToInt64 */

namespace nConfig
{

/** Convert from string */
#define CCONVERTFROMCHAR() \
void cConfigChar   ::ConvertFrom(const string &sStr){*this = *sStr.c_str();}
#define CCONVERTFROMPCHAR() \
void cConfigPChar  ::ConvertFrom(const string &sStr) { \
  char *sData = this->Data(); if(sData) delete sData; sData = new char[sStr.size() + 1]; \
  memcpy(sData, sStr.data(), sStr.size() + 1); *this = sData; \
}
#define CCONVERTFROM(TYPE, SUFFIX, SUB) \
void cConfig##SUFFIX ::ConvertFrom(const string &sStr) { *this = SUB; }\
void cConfigP##SUFFIX::ConvertFrom(const string &sStr) { \
  TYPE * sData = this->Data(); if(sData) delete sData; \
  sData = new TYPE(SUB); *this = sData; \
}
CCONVERTFROMCHAR();
CCONVERTFROMPCHAR();
CCONVERTFROM(bool, Bool, (sStr == "true") ? true : (0 != atoi(sStr.c_str())));
CCONVERTFROM(double, Double, atof(sStr.c_str()));
CCONVERTFROM(int, Int, atoi(sStr.c_str()));
CCONVERTFROM(long, Long, atol(sStr.c_str()));
CCONVERTFROM(unsigned int, UInt, atol(sStr.c_str()));
CCONVERTFROM(unsigned long, ULong, strtoul(sStr.c_str(), NULL, 10));
CCONVERTFROM(__int64, Int64, StringToInt64(sStr));
CCONVERTFROM(string, String, sStr);


/** Convert to string */
#define CCONVERTTO(SUFFIX, SUB) \
void cConfig##SUFFIX::ConvertTo(string &sStr){SUB;}
CCONVERTTO(Bool,    sStr = ((this->Data()) ? "1" : "0"))
CCONVERTTO(String,  sStr = this->Data())
CCONVERTTO(PChar,   sStr = this->Data())
CCONVERTTO(PBool,   sStr = (*(this->Data()) ? "1" : "0"))
CCONVERTTO(PString, sStr = *(this->Data()))
CCONVERTTO(Char,    sprintf(msBuf, "%c",    this->Data());  sStr = msBuf)
CCONVERTTO(Double,  sprintf(msBuf, "%f",    this->Data());  sStr = msBuf)
CCONVERTTO(Int,     sprintf(msBuf, "%d",    this->Data());  sStr = msBuf)
CCONVERTTO(Long,    sprintf(msBuf, "%ld",   this->Data());  sStr = msBuf)
CCONVERTTO(UInt,    sprintf(msBuf, "%u",    this->Data());  sStr = msBuf)
CCONVERTTO(ULong,   sprintf(msBuf, "%lu",   this->Data());  sStr = msBuf)
CCONVERTTO(Int64,   sprintf(msBuf, "%lld",  this->Data());  sStr = msBuf)
CCONVERTTO(PDouble, sprintf(msBuf, "%f",  *(this->Data())); sStr = msBuf)
CCONVERTTO(PInt,    sprintf(msBuf, "%d",  *(this->Data())); sStr = msBuf)
CCONVERTTO(PLong,   sprintf(msBuf, "%ld", *(this->Data())); sStr = msBuf)
CCONVERTTO(PUInt,   sprintf(msBuf, "%u",  *(this->Data())); sStr = msBuf)
CCONVERTTO(PULong,  sprintf(msBuf, "%lu", *(this->Data())); sStr = msBuf)
CCONVERTTO(PInt64,  sprintf(msBuf, "%lld",*(this->Data())); sStr = msBuf)



/** Read from stream */
#define READFROMSTREAM(SUFFIX) \
istream &cConfig##SUFFIX::ReadFromStream(istream& is){string sStr; \
  is >> sStr; \
  this->ConvertFrom(sStr); \
  return is; \
}
#define READFROMSTREAMTOSTR(SUFFIX) \
istream &cConfig##SUFFIX::ReadFromStream(istream& is){ \
  string sStr; stringstream ss; \
  getline(is, sStr); ss << sStr; sStr.empty(); \
  while(is.good()) { \
    sStr.empty(); getline(is, sStr); \
    ss << endl << sStr; \
  } this->ConvertFrom(ss.str()); \
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
ostream &cConfig##SUFFIX::WriteToStream(ostream& os){string sStr; \
  this->ConvertTo(sStr); \
  os << sStr; \
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
#define ISNULL(SUFFIX)  bool cConfig##SUFFIX::IsNULL(){ return !this->Data(); } // 0, false, \0
#define ISNULLPCHAR()   bool cConfigPChar ::IsNULL(){ return !this->Data() || !*(this->Data()); } // 0 or \0
#define ISNULLDOUBLE()  bool cConfigDouble::IsNULL(){ return this->Data() == 0.; } // 0.
#define ISNULLSTRING()  bool cConfigString::IsNULL(){ return !this->Data().size(); } // ""
#define ISNULLP(SUFFIX) bool cConfigP##SUFFIX::IsNULL(){ return !this->Data() || !*(this->Data()); }
#define ISNULLPDOUBLE() bool cConfigPDouble::IsNULL(){ return !this->Data() || *(this->Data()) == 0.; }
#define ISNULLPSTRING() bool cConfigPString::IsNULL(){ return !this->Data() || !((*(this->Data())).size()); }
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

}; // nConfig
