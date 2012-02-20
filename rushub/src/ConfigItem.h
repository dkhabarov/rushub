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

#ifndef CONFIG_H
#define CONFIG_H

#include "stdinc.h"

#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h> // atoi unix

using namespace ::std;


namespace configuration {



typedef enum {
	ITEM_TYPE_VOID,     // void
	ITEM_TYPE_BOOL,     // bool
	ITEM_TYPE_DOUBLE,   // double
	ITEM_TYPE_INT,      // int
	ITEM_TYPE_LONG,     // long
	ITEM_TYPE_UINT,     // unsigned int
	ITEM_TYPE_ULONG,    // unsigned long
	ITEM_TYPE_LLONG,    // int64_t
	ITEM_TYPE_CHAR,     // char
	ITEM_TYPE_STRING,   // string
	ITEM_TYPE_PCHAR,    // char*
	ITEM_TYPE_PBOOL,    // bool*
	ITEM_TYPE_PDOUBLE,  // double*
	ITEM_TYPE_PINT,     // int*
	ITEM_TYPE_PLONG,    // long*
	ITEM_TYPE_PUINT,    // unsigned int*
	ITEM_TYPE_PULONG,   // unsigned long*
	ITEM_TYPE_PLLONG,   // int64_t*
	ITEM_TYPE_PSTRING,  // string*
} ItemType;



/** Abstract class of the element configuration */
class ConfigItem {

	friend class ConfigListBase; /** for mAddress */

public:

	/** Var name */
	string mName;

public:

	ConfigItem(void * address = NULL);

	virtual ~ConfigItem();

	/** Operator of the apropriation data */
	template <class T> ConfigItem & operator = (const T & i) {
		*(T *)address() = i;
		return *this;
	}

	/** Operator of the transformation type */
	template <class T> operator T() {
		return *(T *)mAddress;
	}

	friend istream & operator >> (istream &, ConfigItem &);

	friend ostream & operator << (ostream &, ConfigItem &);

	/** Read from stream */
	virtual istream & readFromStream(istream &) = 0;

	/** Write to stream */
	virtual ostream & writeToStream(ostream &) = 0;

	/** Function of the return the identifier */
	virtual ItemType getTypeId() = 0;

	/** Function of the absence data */
	virtual bool isNull() = 0;

	/** Converts from line */
	virtual void convertFrom(const string &) = 0;

	/** Converts to line */
	virtual void convertTo(string &) = 0;

protected:

	/** Address of var */
	void * mAddress;

	bool mEmpty;

	/** Buffer for conversion in string */
	char mBuffer[32];

protected:

	/** Function returns address */
	virtual void * address() {
		return mAddress;
	}

}; // class ConfigItem



/** Class of the type data (ConfigItemTYPE) */
#define CONFIGBASEITEM(TYPE, TYPE_ID, SUFFIX) \
class ConfigItem##SUFFIX : public ConfigItem { \
public: \
	ConfigItem##SUFFIX(TYPE & var) : ConfigItem((void *) & var) {} \
	ConfigItem##SUFFIX(TYPE & var, string const & name) : ConfigItem((void *) & var) { this->mName = name; } \
	virtual ~ConfigItem##SUFFIX() {} \
	virtual ConfigItem##SUFFIX & operator = (TYPE const & i) { *(TYPE *)address() = i; return *this; } /** Operator of the apropriation */ \
	virtual TYPE & data() { return *(TYPE *)mAddress; } /** Returns data */ \
	virtual istream & readFromStream(istream & is); /** Read from stream */ \
	virtual ostream & writeToStream(ostream & os); /** Write to stream */ \
	virtual ItemType getTypeId() { return TYPE_ID; } /** Function of the return the identifier */ \
	virtual void convertFrom(const string & str); /** Converts from line */ \
	virtual void convertTo(string & str); /** Converts to line */ \
	virtual bool isNull(); /** Returns true if and when variable mAddress empty - a data are absent */ \
}; // ConfigItem##SUFFIX

/** Announcement of the classes of the main types */
CONFIGBASEITEM(bool, ITEM_TYPE_BOOL, Bool);               /** ConfigItemBool */
CONFIGBASEITEM(double, ITEM_TYPE_DOUBLE, Double);         /** ConfigItemDouble */
CONFIGBASEITEM(int, ITEM_TYPE_INT, Int);                  /** ConfigItemInt */
CONFIGBASEITEM(long, ITEM_TYPE_LONG, Long);               /** ConfigItemLong */
CONFIGBASEITEM(unsigned, ITEM_TYPE_UINT, UInt);           /** ConfigItemUInt */
CONFIGBASEITEM(unsigned long, ITEM_TYPE_ULONG, ULong);    /** ConfigItemULong */
CONFIGBASEITEM(int64_t, ITEM_TYPE_LLONG, Int64);          /** ConfigItemInt64 */
CONFIGBASEITEM(char, ITEM_TYPE_CHAR, Char);               /** ConfigItemChar */
CONFIGBASEITEM(string, ITEM_TYPE_STRING, String);         /** ConfigItemString */
CONFIGBASEITEM(char *, ITEM_TYPE_PCHAR, PChar);           /** ConfigItemPChar */
CONFIGBASEITEM(bool *, ITEM_TYPE_PBOOL, PBool);           /** ConfigItemPBool */
CONFIGBASEITEM(double *, ITEM_TYPE_PDOUBLE, PDouble);     /** ConfigItemPDouble */
CONFIGBASEITEM(int *, ITEM_TYPE_PINT, PInt);              /** ConfigItemPInt */
CONFIGBASEITEM(long *, ITEM_TYPE_PLONG, PLong);           /** ConfigItemPLong */
CONFIGBASEITEM(unsigned int *, ITEM_TYPE_PUINT, PUInt);   /** ConfigItemPUInt */
CONFIGBASEITEM(unsigned long *, ITEM_TYPE_PULONG, PULong);/** ConfigItemPULong */
CONFIGBASEITEM(int64_t *, ITEM_TYPE_PLLONG, PInt64);      /** ConfigItemPLLong */
CONFIGBASEITEM(string *, ITEM_TYPE_PSTRING, PString);     /** ConfigItemPString */

}; // namespace configuration

#endif // CONFIG_H

/**
 * $Id$
 * $HeadURL$
 */
