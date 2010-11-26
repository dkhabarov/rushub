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

#ifndef CCONFIG_H
#define CCONFIG_H

#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h> // atoi unix

using namespace std;

#if (!defined _WIN32) && (!defined __int64)
	#define __int64 long long
#endif

namespace nConfig {

typedef enum {
	eIT_VOID,     // void
	eIT_BOOL,     // bool
	eIT_DOUBLE,   // double
	eIT_INT,      // int
	eIT_LONG,     // long
	eIT_UINT,     // unsigned int
	eIT_ULONG,    // unsigned long
	eIT_LLONG,    // __int64
	eIT_CHAR,     // char
	eIT_STRING,   // string
	eIT_PCHAR,    // char*
	eIT_PBOOL,    // bool*
	eIT_PDOUBLE,  // double*
	eIT_PINT,     // int*
	eIT_PLONG,    // long*
	eIT_PUINT,    // unsigned int*
	eIT_PULONG,   // unsigned long*
	eIT_PLLONG,   // __int64*
	eIT_PSTRING,  // string*
} tItemType;

/** Abstract class of the element configuration (Config) */
class cConfig {

	friend class cConfigListBase; /** for mAddr */

public:
	string msName; /** Var name */
	char msBuf[32]; /** Buffer for conversion in string */

protected:
	bool mbEmpty;
	void *mAddr; /** Address of var */

public:
	cConfig(void *addr = 0) : mbEmpty(true), mAddr(addr) {};
	virtual ~cConfig() {};

	template <class TYPE> cConfig &operator = (const TYPE &i) { *(TYPE*)Address() = i; return *this; } /** Operator of the apropriation data */
	template <class TYPE> operator TYPE() { return *(TYPE*)mAddr; } /** Operator of the transformation of the type */

	virtual istream &ReadFromStream(istream &) = 0; /** Read from stream */
	virtual ostream &WriteToStream(ostream &) = 0; /** Write to stream */
	friend istream &operator >> (istream &is, cConfig &ci) { return ci.ReadFromStream(is); }
	friend ostream &operator << (ostream &os, cConfig &ci) { return ci.WriteToStream(os); }

	virtual tItemType GetTypeID() = 0; /** Function of the return the identifier */
	virtual bool IsNULL() = 0; /** Function of the absence data */
	virtual void ConvertFrom(const string &) = 0; /** Converts from line */
	virtual void ConvertTo(string &) = 0; /** Converts to line */

protected:
	virtual void *Address() { return mAddr; } /** Function returns address */

}; // cConfig

/** Class of the type data (ConfigTYPE) */
#define CONFIGBASEITEM(TYPE, TYPEID, SUFFIX) \
class cConfig##SUFFIX : public cConfig { \
public: \
	cConfig##SUFFIX(TYPE &var):cConfig((void*)&var) {} \
	cConfig##SUFFIX(TYPE &var, string const &sName) : cConfig((void*)&var) { this->msName = sName; } \
	virtual ~cConfig##SUFFIX() {} \
	virtual cConfig##SUFFIX &operator = (TYPE const &i) { *(TYPE*)Address() = i; return *this; } /** Operator of the apropriation */ \
	virtual TYPE & Data() { return *(TYPE*)mAddr; } /** Returns data */ \
	virtual istream &ReadFromStream(istream& is); /** Read from stream */ \
	virtual ostream &WriteToStream(ostream& os); /** Write to stream */ \
	virtual tItemType GetTypeID() { return TYPEID; } /** Function of the return the identifier */ \
	virtual void ConvertFrom(const string &str); /** Converts from line */ \
	virtual void ConvertTo(string &str); /** Converts to line */ \
	virtual bool IsNULL(); /** Returns true if and when variable mAddr empty - a data are absent */ \
}; // cConfig##SUFFIX

/** Announcement of the classes of the main types */
CONFIGBASEITEM(bool, eIT_BOOL, Bool);               /** cConfigBool */
CONFIGBASEITEM(double, eIT_DOUBLE, Double);         /** cConfigDouble */
CONFIGBASEITEM(int, eIT_INT, Int);                  /** cConfigInt */
CONFIGBASEITEM(long, eIT_LONG, Long);               /** cConfigLong */
CONFIGBASEITEM(unsigned, eIT_UINT, UInt);           /** cConfigUInt */
CONFIGBASEITEM(unsigned long, eIT_ULONG, ULong);    /** cConfigULong */
CONFIGBASEITEM(__int64, eIT_LLONG, Int64);          /** cConfigInt64 */
CONFIGBASEITEM(char, eIT_CHAR, Char);               /** cConfigChar */
CONFIGBASEITEM(string, eIT_STRING, String);         /** cConfigString */
CONFIGBASEITEM(char *, eIT_PCHAR, PChar);           /** cConfigPChar */
CONFIGBASEITEM(bool *, eIT_PBOOL, PBool);           /** cConfigPBool */
CONFIGBASEITEM(double *, eIT_PDOUBLE, PDouble);     /** cConfigPDouble */
CONFIGBASEITEM(int *, eIT_PINT, PInt);              /** cConfigPInt */
CONFIGBASEITEM(long *, eIT_PLONG, PLong);           /** cConfigPLong */
CONFIGBASEITEM(unsigned int *, eIT_PUINT, PUInt);   /** cConfigPUInt */
CONFIGBASEITEM(unsigned long *, eIT_PULONG, PULong);/** cConfigPULong */
CONFIGBASEITEM(__int64 *, eIT_PLLONG, PInt64);      /** cConfigPLLong */
CONFIGBASEITEM(string *, eIT_PSTRING, PString);     /** cConfigPString */

}; // nConfig

#endif // CCONFIG_H
