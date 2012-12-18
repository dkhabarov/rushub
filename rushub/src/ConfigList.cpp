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

#include "ConfigList.h"

namespace configuration {



ConfigListBase::ConfigListBase() : Obj("ConfigListBase") {
	mBase = NULL; /** NULL base pointer */
	mFactory = new ConfigFactory;
}

ConfigListBase::~ConfigListBase() {
	Hash_t hash;
	ConfigItem * configItem = NULL; /** Pointer on object */
	for (tVIt it = mKeyList.begin(); it != mKeyList.end(); ++it) {
		hash = *it; /** Get hash from vector */
		configItem = mList.find(hash); /** Get object by hash */
		mList.remove(hash); /** Del object from list by hash */
		this->mFactory->del(configItem); /** Del object */
	}
	if (mFactory != NULL) {
		delete mFactory;
	}
	mFactory = NULL;
}



/** Adding config */
ConfigItem * ConfigListBase::add(const string & key, ConfigItem * configItem) {
	Hash_t hash = mList.mHash(key);
	if (!mList.add(hash, configItem)) { /** Add */
		if (log(LEVEL_DEBUG)) {
			ConfigItem * other = mList.find(hash);
			LOG_STREAM("Don't add " << key << " because of " << (other ? other->mName.c_str() : "NULL"));
			return NULL;
		}
	}
	mKeyList.push_back(hash); /** Push back of vector */
	configItem->mName = key; /** Record name */
	return configItem;
}



/** Get config by name */
ConfigItem * ConfigListBase::operator[](const char * name) {
	return mList.find(mList.mHash(name));
}



/** Get config by name */
ConfigItem * ConfigListBase::operator[](const string & name) {
	return mList.find(mList.mHash(name));
}



/** Set new address */
void ConfigListBase::setBaseTo(void * new_base) {
	if (mBase) {
		for (tVIt it = mKeyList.begin(); it != mKeyList.end(); ++it) {
			mList.find(*it)->mAddress = reinterpret_cast<void *> (long(mList.find(*it)->mAddress) + (long(new_base) - long(mBase)));
		}
	}
	mBase = new_base;
}


} // namespace configuration

/**
 * $Id$
 * $HeadURL$
 */
