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

#include "ConfigList.h"

namespace configuration {



ConfigListBase::ConfigListBase() : Obj("ConfigListBase") {
	mBase = NULL; /** NULL base pointer */
	mFactory = new ConfigFactory;
}

ConfigListBase::~ConfigListBase() {
	Hash_t hash;
	Config * config = NULL; /** Pointer on object */
	for (tVIt it = mKeyList.begin(); it != mKeyList.end(); ++it) {
		hash = *it; /** Get hash from vector */
		config = mList.Find(hash); /** Get object by hash */
		mList.Remove(hash); /** Del object from list by hash */
		this->mFactory->Delete(config); /** Del object */
	}
	if (mFactory != NULL) {
		delete mFactory;
	}
	mFactory = NULL;
}



/** Adding config */
Config * ConfigListBase::Add(const string & key, Config * config) {
	Hash_t hash = mList.mHash(key);
	if (!mList.Add(hash, config)) { /** Add */
		if (Log(1)) {
			Config * other = mList.Find(hash);
			LogStream() << "Don't add " << key << " because of " << (other ? other->mName.c_str() : "NULL") << endl;
			return NULL;
		}
	}
	mKeyList.push_back(hash); /** Push back of vector */
	config->mName = key; /** Record name */
	return config;
}



/** Get config by name */
Config * ConfigListBase::operator[](const char * name) {
	return mList.Find(mList.mHash(name));
}



/** Get config by name */
Config * ConfigListBase::operator[](const string & name) {
	return mList.Find(mList.mHash(name));
}



/** Set new address */
void ConfigListBase::SetBaseTo(void * new_base) {
	if (mBase) {
		for (tVIt it = mKeyList.begin(); it != mKeyList.end(); ++it) {
			mList.Find(*it)->mAddress = (void *)(long(mList.Find(*it)->mAddress) + (long(new_base) - long(mBase)));
		}
	}
	mBase = new_base;
}



/** Creating and adding configs */
#define ADDVAL(TYPE) \
Config * ConfigList::Add(const string & name, TYPE & var) { \
	Config * config = this->mFactory->Add(var); \
	return this->ConfigListBase::Add(name, config); \
} \
Config * ConfigList::Add(const string & name, TYPE & var, TYPE const & def) { \
	Config * config = this->Add(name, var); \
	*config = def; \
	return config; \
}



#define ADDPVAL(TYPE) \
Config * ConfigList::Add(const string & name, TYPE* & var) { \
	Config * config = this->mFactory->Add(var); \
	return this->ConfigListBase::Add(name, config); \
} \
Config * ConfigList::Add(const string & name, TYPE* & var, TYPE const & def) { \
	Config * config = this->Add(name, var); \
	*config = &def; \
	return config; \
}



#define ADDVALUES(TYPE) \
ADDVAL(TYPE); \
ADDPVAL(TYPE);

ADDVAL(char);
ADDVAL(char *);
ADDVALUES(bool);
ADDVALUES(int);
ADDVALUES(double);
ADDVALUES(long);
ADDVALUES(unsigned);
ADDVALUES(unsigned long);
ADDVALUES(__int64);
ADDVALUES(string);

}; // namespace configuration
