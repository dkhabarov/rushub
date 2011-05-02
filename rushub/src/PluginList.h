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

#ifndef PLUGIN_LIST_H
#define PLUGIN_LIST_H

#include "HashMap.h"
#include "Obj.h"
#include "CallList.h"

#include <string>
#include <iostream>

using ::std::string;
using ::std::ostream;

using namespace ::utils;



namespace dcserver {

	class DcServer;

}; // namespace dcserver



namespace plugin {

using namespace ::dcserver;

class PluginLoader;



class PluginList : public Obj, public PluginListBase {

	/** For LoadAll */
	friend class ::dcserver::DcServer;

	/** For SetCallList */
	friend class CallList;

public:

	typedef HashMap<PluginLoader *> PluginLoaderMap;

public:

	PluginList(const string & path);

	virtual ~PluginList();

	virtual PluginList & operator = (const PluginList &) {
		return *this;
	}

	void setServer(DcServerBase *);

	virtual const string & getPluginDir() const {
		return mPluginDir;
	}

	/** Load all plugins */
	bool loadAll();

	/** Load plugin by lib */
	bool loadPlugin(const string & filePath);

	/** Unload plugin by name */
	bool unloadPlugin(const string & name);

	/** Reload plugin by name */
	bool reloadPlugin(const string & name);

	/** Set call list */
	bool setCallList(string id, CallList *);


	virtual bool regCallList(const char * id, Plugin *);

	virtual bool unregCallList(const char * id, Plugin *);


	Plugin * getPlugin(const string & name);

	Plugin * getPluginByLib(const string & lib);


	/** onPluginLoad event */
	void onPluginLoad(Plugin *);

protected:

	typedef unsigned long Hash_t;
	typedef HashMap<CallList *> CallListMap;

	vector<Hash_t> mKeyList;

	/** Plugin dir */
	const string & mPluginDir;

	/** List with loaders of plugins */
	PluginLoaderMap mPluginLoaders;
	vector<Hash_t> mPluginLoaderHash;
	CallListMap mCallLists;

private:

	DcServerBase * mDcServerBase;

}; // class PluginList


}; // namespace plugin

#endif // PLUGIN_LIST_H

/**
 * $Id$
 * $HeadURL$
 */
