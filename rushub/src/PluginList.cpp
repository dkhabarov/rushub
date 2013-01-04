/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz
 *
 * modified: 27 Aug 2009
 * Copyright (C) 2009-2013 by Setuper
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
	LOG(LEVEL_INFO, "Using plugins in: " << mPluginDir);
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
	LOG(LEVEL_DEBUG, "Open plugin dir: " << mPluginDir);

	DIR * dir = opendir(mPluginDir.c_str());
	if (!dir) {
		LOG(LEVEL_ERROR, "Open plugin dir error");
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
			LOG(LEVEL_DEBUG, "Plugin file name: " << file);
			loadPlugin(mPluginDir + file);
		}
	}

	closedir(dir);
	return true;
}



/** Loadin plugin from file (lib) dll (so) */
bool PluginList::loadPlugin(const string & filePath) {

	PluginLoader * pluginLoader = new PluginLoader(filePath);
	LOG(LEVEL_DEBUG, "Attempt loading plugin: " << filePath);

	try {

		if (
			!pluginLoader->open() ||
			!pluginLoader->loadSym() ||
			!mPluginLoaders.add(mPluginLoaders.mHash(pluginLoader->mPlugin->getName()), pluginLoader)
		) {

			const string & error = pluginLoader->getError();
			LOG(LEVEL_WARN, "Failure loading plugin: " << filePath << 
				(error.empty() ? "" : (" (" + error + ")")));

			pluginLoader->close();
			delete pluginLoader;
			return false;
		}
		mPluginLoaderHash.push_back(mPluginLoaders.mHash(pluginLoader->mPlugin->getName()));

		onPluginLoad(pluginLoader->mPlugin); /** OnLoad */

	} catch (...) {
		LOG(LEVEL_ERROR, "Plugin " << filePath << " caused an exception");
		pluginLoader->close();
		delete pluginLoader;
		return false;
	}

	LOG(LEVEL_DEBUG, "Success loading plugin: " << filePath);
	return true;
}



/** Unload plugin by name */
bool PluginList::unloadPlugin(const string & name) {

	unsigned long key = mPluginLoaders.mHash(name);
	PluginLoader * pluginLoader = mPluginLoaders.find(key);

	if (!pluginLoader || !mPluginLoaders.remove(key)) {
		LOG(LEVEL_ERROR, "Can't unload plugin name: '" << name << "'");
		return false;
	}

	CallList * callList = NULL;
	for (CallListMap::iterator it = mCallLists.begin(); it != mCallLists.end(); ++it) {
		callList = (*it);
		if (callList->mName.size()) {
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
	if (!callList || id.empty()) {
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
	const string & name = plugin->getName();
	const string & version = plugin->getVersion();

	LOG(LEVEL_INFO, "Plugin detected: " << (name != "" ? name : "n/a") <<
		" v " << (version != "" ? version : "n/a"));

	plugin->onLoad(mDcServerBase);
}


} // namespace plugin

/**
 * $Id$
 * $HeadURL$
 */
