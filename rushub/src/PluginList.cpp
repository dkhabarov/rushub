/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz
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

#include "PluginList.h"
#include "PluginLoader.h"
#include "Dir.h"


namespace plugin {



PluginList::PluginList(const string & path) :
	Obj("PluginList"),
	mPluginDir(path),
	mDcServerBase(NULL)
{

	if (Log(1)) {
		LogStream() << "Using plugins in: " << mPluginDir << endl;
	}

}



PluginList::~PluginList() {
	unloadAll();
}



bool PluginList::unloadAll() {
	// Removing plugins
	Hash_t hash(0);
	PluginLoader * pluginLoader = NULL;
	for (vector<Hash_t>::iterator it = mPluginLoaderHash.begin(); it != mPluginLoaderHash.end(); ++it) {
		hash = (*it);
		pluginLoader = mPluginLoaders.find(hash);
		if (pluginLoader) {
			unloadPlugin(pluginLoader->mPlugin->getName());
		}
	}

	// Removing call lists
	for (vector<Hash_t>::iterator it = mKeyList.begin(); it != mKeyList.end(); ++it) {
		mCallLists.remove(*it);
	}

	mKeyList.clear();
	mPluginLoaderHash.clear();
	return true;
}



void PluginList::setServer(DcServerBase * dcServerBase) {
	mDcServerBase = dcServerBase;
}



/** Loading all plugins from plugins dir */
bool PluginList::loadAll() {
	if (Log(3)) {
		LogStream() << "Open plugin dir: " << mPluginDir << endl;
	}

	DIR * dir = opendir(mPluginDir.c_str());
	if (!dir) {
		if (ErrLog(1)) {
			LogStream() << "Open plugin dir error" << endl;
		}
		return false;
	}

	struct dirent * entry = NULL;
	string file;
	while (NULL != (entry = readdir(dir))) {
		file = entry->d_name;
		#ifdef _WIN32
			if ((file.size() > 4) && (0 == file.compare(file.size() - 4, 4, ".dll")))
		#else
			if ((file.size() > 3) && (0 == file.compare(file.size() - 3, 3, ".so")))
		#endif
		{
			if (Log(3)) {
				LogStream() << "Plugin file name: " << file << endl;
			}
			loadPlugin(mPluginDir + file);
		}
	}

	closedir(dir);
	return true;
}



/** Loadin plugin from file (lib) dll (so) */
bool PluginList::loadPlugin(const string & filePath) {

	PluginLoader * pluginLoader = new PluginLoader(filePath);
	if (Log(3)) {
		LogStream() << "Attempt loading plugin: " << filePath << endl;
	}

	try {

		if (
			!pluginLoader->open() ||
			!pluginLoader->loadSym() ||
			!mPluginLoaders.add(mPluginLoaders.mHash(pluginLoader->mPlugin->getName()), pluginLoader)
		) {

			const string & error = pluginLoader->getError();
			if (Log(0)) {
				LogStream() << "Failure loading plugin: " << filePath << 
					(error.empty() ? "" : (" (" + error + ")")) << endl;
			}

			pluginLoader->close();
			delete pluginLoader;
			return false;
		}
		mPluginLoaderHash.push_back(mPluginLoaders.mHash(pluginLoader->mPlugin->getName()));

		pluginLoader->mPlugin->setPluginList(this); /** Set pointer on list for plugin */
		pluginLoader->mPlugin->regAll(this); /** Reg all call-function for this plugin */
		onPluginLoad(pluginLoader->mPlugin); /** OnLoad */

	} catch (...) {

		if (ErrLog(1)) {
			LogStream() << "Plugin " << filePath << 
				" caused an exception" << endl;
		}

		pluginLoader->close();
		delete pluginLoader;
		return false;
	}

	if (Log(3)) {
		LogStream() << "Success loading plugin: " << filePath << endl;
	}

	return true;
}



/** Unload plugin by name */
bool PluginList::unloadPlugin(const string & name) {

	unsigned long key = mPluginLoaders.mHash(name);
	PluginLoader * pluginLoader = mPluginLoaders.find(key);

	if (!pluginLoader || !mPluginLoaders.remove(key)) {
		if (ErrLog(2)) {
			LogStream() << "Can't unload plugin name: '" << name << "'" << endl;
		}
		return false;
	}

	CallList * callList = NULL;
	for (CallListMap::iterator it = mCallLists.begin(); it != mCallLists.end(); ++it) {
		callList = (*it);
		if (callList->mName != "") {
			callList->unreg(pluginLoader->mPlugin);
		}
	}

	pluginLoader->close();
	delete pluginLoader;
	return true;
}



/** Reload plugin by name */
bool PluginList::reloadPlugin(const string & name) {
	PluginLoader * pluginLoader = mPluginLoaders.find(mPluginLoaders.mHash(name));
	if (
		!pluginLoader || 
		!unloadPlugin(name)
	) {
		return false;
	}
	return loadPlugin(pluginLoader->getFileName());
}



/** Set call list */
bool PluginList::setCallList(const string & id, CallList * callList) {
	if (!callList || id.size() == 0) {
		return false;
	}

	Hash_t hash = mCallLists.mHash(id);
	if (mCallLists.add(hash, callList)) {
		mKeyList.push_back(hash);
	}
	return false;
}



/** Reg plugin in call list */
bool PluginList::regCallList(const char * id, Plugin * plugin) {

	if (!plugin) {
		return false;
	}

	CallList * callList = mCallLists.find(mCallLists.mHash(id));
	return callList != NULL ? 
		callList->reg(plugin) : 
		false;
}



/** Unreg plugin in call list */
bool PluginList::unregCallList(const char * id, Plugin * plugin) {

	if (!plugin) {
		return false;
	}

	CallList * callList = mCallLists.find(mCallLists.mHash(id));
	return callList != NULL ? 
		callList->unreg(plugin) : 
		false;
}



/** Get plugin by name */
Plugin * PluginList::getPlugin(const string & name) {
	PluginLoader * pluginLoader = mPluginLoaders.find(mPluginLoaders.mHash(name));
	return pluginLoader != NULL ? 
		pluginLoader->mPlugin : 
		NULL;
}



/** Get plugin by lib */
Plugin * PluginList::getPluginByLib(const string & lib) {
	for (PluginLoaderMap::iterator it = mPluginLoaders.begin(); it != mPluginLoaders.end(); ++it) {
		if ((*it)->getFileName() == lib) {
			return (*it)->mPlugin;
		}
	}
	return NULL;
}



/** onPluginLoad */
void PluginList::onPluginLoad(Plugin * plugin) {

	if (Log(1)) {
		const string & name = plugin->getName();
		const string & version = plugin->getVersion();
		LogStream() << "Plugin detected: " << (name != "" ? name : "n/a") <<
			" v " << (version != "" ? version : "n/a") << endl;
	}

	plugin->onLoad(mDcServerBase);

}


}; // namespace plugin

/**
 * $Id$
 * $HeadURL$
 */
