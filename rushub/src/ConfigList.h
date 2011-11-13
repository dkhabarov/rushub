/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz
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

#ifndef CONFIG_LIST_H
#define CONFIG_LIST_H

#include "Obj.h"
#include "HashMap.h"
#include "ConfigItem.h"

#include <vector>

using ::std::vector;
using namespace ::utils;

namespace configuration {

#define NEWITEM(TYPE, SUFFIX) \
virtual ConfigItem##SUFFIX * add(TYPE & v) { return new ConfigItem##SUFFIX(v); } \
virtual ConfigItemP##SUFFIX * add(TYPE * & v) { return new ConfigItemP##SUFFIX(v); }

/** The Class of the making the object for any type. 
Contains the functions of the creation of all available types. */
class ConfigFactory {

public:

	NEWITEM(bool, Bool);            // virtual ConfigItemBool    * add(bool & var) { return new ConfigItemBool(var); }
	NEWITEM(double, Double);        // virtual ConfigItemDouble  * add(double & var) { return new ConfigItemDouble(var); }
	NEWITEM(int, Int);              // virtual ConfigItemInt     * add(int & var) { return new ConfigItemInt(var); }
	NEWITEM(long, Long);            // virtual ConfigItemLong    * add(long & var) { return new ConfigItemLong(var); }
	NEWITEM(unsigned int, UInt);    // virtual ConfigItemUInt    * add(unsigned int & var) { return new ConfigItemUInt(var); }
	NEWITEM(unsigned long, ULong);  // virtual ConfigItemULong   * add(unsigned long & var) { return new ConfigItemULong(var); }
	NEWITEM(__int64, Int64);        // virtual ConfigItemInt64   * add(__int64 & var) { return new ConfigItemInt64(var); }
	NEWITEM(char, Char);            // virtual ConfigItemChar    * add(char & var) { return new ConfigItemChar(var); }
	NEWITEM(string, String);        // virtual ConfigItemString  * add(string & var) { return new ConfigItemString(var); }
	virtual void del(ConfigItem * configItem) { /** Removing config item */
		delete configItem;
	}
}; // ConfigFactory

/** Base configuration class */
class ConfigListBase : public Obj {

public:

	typedef unsigned Hash_t;
	typedef vector<Hash_t> tVector; /** Vector */
	typedef tVector::iterator tVIt; /** Iterator of vector */
	typedef HashMap<ConfigItem *, Hash_t> tHashMap;
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

		ConfigItem * operator * () {
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

	ConfigItem * operator[] (const char *); /** Get config by name */
	ConfigItem * operator[] (const string &); /** Get config by name */
	ConfigItem * add(const string &, ConfigItem *); /** Add config object in list */
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



/**
	This class is a main class for each configuration with like structure.
	You may link real given with their names. You may get these data or change them.
	Convert from/in string of the type std::string, read/write from/in stream. Very useful class.
*/
class ConfigList : public ConfigListBase {
public:
	ConfigList() {
	}
	virtual ~ConfigList() {
	}

	template <typename T>
	ConfigItem * add(const string & name, T & var) {
		ConfigItem * configItem = this->mFactory->add(var);
		return this->ConfigListBase::add(name, configItem);
	}

	template <typename T>
	ConfigItem * add(const string & name, T & var, T const & def) {
		ConfigItem * configItem = this->add(name, var);
		*configItem = def;
		return configItem;
	}

}; // class ConfigList

}; // namespace configuration

#endif // CONFIG_LIST_H

/**
 * $Id$
 * $HeadURL$
 */
