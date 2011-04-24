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

#ifndef CONFIG_LIST_H
#define CONFIG_LIST_H

#include "Obj.h"
#include "HashMap.h"
#include "Config.h"

#include <vector>

using ::std::vector;
using namespace ::utils;

namespace configuration {

#define NEWITEM(TYPE, SUFFIX) \
virtual Config##SUFFIX * add(TYPE & v) { return new Config##SUFFIX(v); } \
virtual ConfigP##SUFFIX * add(TYPE * & v) { return new ConfigP##SUFFIX(v); }

/** The Class of the making the object for any type. 
Contains the functions of the creation of all available types. */
class ConfigFactory {

public:

  ConfigFactory() {
	}
	NEWITEM(bool, Bool);            /** virtual ConfigBool    * add(bool & var) { return new ConfigBool(var) } */
	NEWITEM(double, Double);        /** virtual ConfigDouble  * add(double & var) { return new ConfigDouble(var) } */
	NEWITEM(int, Int);              /** virtual ConfigInt     * add(int & var) { return new ConfigInt(var) } */
	NEWITEM(long, Long);            /** virtual ConfigLong    * add(long & var) { return new ConfigLong(var) } */
	NEWITEM(unsigned int, UInt);    /** virtual ConfigUInt    * add(unsigned int & var) { return new ConfigUInt(var) } */
	NEWITEM(unsigned long, ULong);  /** virtual ConfigULong   * add(unsigned long & var) { return new ConfigULong(var) } */
	NEWITEM(__int64, Int64);        /** virtual ConfigInt64   * add(__int64 & var) { return new ConfigInt64(var) } */
	NEWITEM(char, Char);            /** virtual ConfigChar    * add(char & var) { return new ConfigChar(var) } */
	NEWITEM(string, String);        /** virtual ConfigString  * add(string & var) { return new ConfigString(var) } */
	virtual void del(Config * config) { /** Removing config */
		delete config;
	}
}; // ConfigFactory

/** Base config class (Config Base Base) */
class ConfigListBase : public Obj {

public:

	typedef unsigned Hash_t;
	typedef vector<Hash_t> tVector; /** Vector */
	typedef tVector::iterator tVIt; /** Iterator of vector */
	typedef HashMap<Config *, Hash_t> tHashMap;
	typedef tHashMap::iterator tHLMIt; /** Iterator of list */

	tHashMap mList; /** Main list of data */
	tVector mKeyList; /** Main vector of all hash values */

	ConfigFactory * mFactory; /** Config factory */
	void * mBase; /** Main pointer */

	Hash<Hash_t> mHash; /** Hash function */

	/** Iterator for config class */
	struct iterator {
		ConfigListBase * mConfigListBase;
		tVIt mIterator;

		iterator() {
		}

		iterator(ConfigListBase * configListBase, const tVIt & it) : 
			mConfigListBase(configListBase),
			mIterator(it)
		{
		}

		Config * operator * () {
			return mConfigListBase->mList.find(*mIterator);
		}

		iterator & operator ++ () {
			++mIterator;
			return *this;
		}

		inline bool operator != (iterator & it) {
			return mIterator != it.mIterator;
		}

		iterator(iterator & it) {
			operator = (it);
		}

		iterator & set(ConfigListBase * configListBase, const tVIt & it) {
			mConfigListBase = configListBase;
			mIterator = it;
			return *this;
		}

		iterator & operator = (iterator & it) {
			mIterator = it.mIterator;
			mConfigListBase = it.mConfigListBase;
			return *this;
		}
	};
	iterator mBegin; /** Begin iterator */
	iterator mEnd; /** End iterator */

public:
	ConfigListBase();
	virtual ~ConfigListBase();

	Config * operator[] (const char *); /** Get config by name */
	Config * operator[] (const string &); /** Get config by name */
	Config * add(const string &, Config *); /** Add config object in list */
	void setBaseTo(void * newBase); /** Set base pointer in address */

	virtual int load() = 0; /** Loading from store */
	virtual int save() = 0; /** Saving to store */

	/** Set begin iterator */
	iterator & begin() {
		return mBegin.set(this, mKeyList.begin());
	}

	/** Set end iterator */
	iterator & end() {
		return mEnd.set(this, mKeyList.end());
	}

}; // class ConfigListBase

/** Adding */
#define ADDMTDS(TYPE) \
Config * add(const string & name, TYPE & var); \
Config * add(const string & name, TYPE & var, TYPE const & def);

#define ADDPMTDS(TYPE) \
Config * add(const string & name, TYPE * & var); \
Config * add(const string & name, TYPE * & var, TYPE const & def);

#define ADDMETHODS(TYPE) \
ADDMTDS(TYPE); \
ADDPMTDS(TYPE);


/**
	This class is a main class for each config with like structure.
	You may link real given with their names. You may get these data or change them.
	Convert from/in string of the type std::string, read/write from/in stream. Very useful class.
*/
class ConfigList : public ConfigListBase {
public:
	ConfigList() {
	}
	virtual ~ConfigList() {
	}

	ADDMTDS(char);
	ADDMTDS(char *);
	ADDMETHODS(bool);
	ADDMETHODS(int);
	ADDMETHODS(long);
	ADDMETHODS(double);
	ADDMETHODS(unsigned int);
	ADDMETHODS(unsigned long);
	ADDMETHODS(__int64);
	ADDMETHODS(string);

}; // class ConfigList

}; // namespace configuration

#endif // CONFIG_LIST_H
