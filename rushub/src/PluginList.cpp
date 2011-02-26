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

#include "PluginList.h"
#include "PluginLoader.h"
#include "stringutils.h" // StrCompare
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
	// Removing plugins
	for(vector<Hash_t>::iterator it = mPluginLoaderHash.begin(); it != mPluginLoaderHash.end(); ++it) {
		mPluginLoaders.Remove(*it);
	}

	// Removing call lists
	for(vector<Hash_t>::iterator it = mKeyList.begin(); it != mKeyList.end(); ++it) {
		mCallLists.Remove(*it);
	}
}



void PluginList::SetServer(DcServerBase * dcServerBase) {
	mDcServerBase = dcServerBase;
}



/** Loading all plugins from plugins dir */
bool PluginList::LoadAll() {
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
			if ((file.size() > 4) && (0 == StrCompare(file, file.size() - 4, 4, ".dll")))
		#else
			if ((file.size() > 3) && (0 == StrCompare(file, file.size() - 3, 3, ".so")))
		#endif
		{
			if (Log(3)) {
				LogStream() << "Plugin file name: " << file << endl;
			}
			LoadPlugin(mPluginDir + file);
		}
	}

	closedir(dir);
	return true;
}



/** Loadin plugin from file (lib) dll (so) */
bool PluginList::LoadPlugin(const string & filePath) {

	PluginLoader * pluginLoader = new PluginLoader(filePath);
	if (Log(3)) {
		LogStream() << "Attempt loading plugin: " << filePath << endl;
	}

	try {

		if (
			!pluginLoader->open() ||
			!pluginLoader->loadSym() ||
			!mPluginLoaders.Add(mPluginLoaders.mHash(pluginLoader->mPlugin->getName()), pluginLoader)
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
		OnPluginLoad(pluginLoader->mPlugin); /** OnLoad */

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

	delete pluginLoader;
	return true;
}



/** Unload plugin by name */
bool PluginList::UnloadPlugin(const string & name) {

	unsigned long key = mPluginLoaders.mHash(name);
	PluginLoader * pluginLoader = mPluginLoaders.Find(key);

	if (!pluginLoader || !mPluginLoaders.Remove(key)) {
		if (ErrLog(2)) {
			LogStream() << "Can't unload plugin name: '" << name << "'" << endl;
		}
		return false;
	}

	for (CallListMap::iterator it = mCallLists.begin(); it != mCallLists.end(); ++it) {
		(*it)->Unreg(pluginLoader->mPlugin);
	}

	delete pluginLoader;
	return true;
}



/** Reload plugin by name */
bool PluginList::ReloadPlugin(const string & name) {

	PluginLoader * pluginLoader = mPluginLoaders.Find(mPluginLoaders.mHash(name));
	if (!pluginLoader) {
		return false;
	}

	if (!UnloadPlugin(name)) {
		return false;
	}

	const string & filePath = pluginLoader->getFileName();
	if (!LoadPlugin(filePath)) {
		return false;
	}

	return true;
}



/** Set call list */
bool PluginList::SetCallList(string id, CallList * callList) {
	if (!callList) {
		return false;
	}

	if (id.size() == 0) {
		return false;
	}

	Hash_t hash = mCallLists.mHash(id);
	if (mCallLists.Add(hash, callList)) {
		mKeyList.push_back(hash);
	}
	return false;
}



/** Reg plugin in call list */
bool PluginList::regCallList(const char * id, Plugin * plugin) {

	if (!plugin) {
		return false;
	}

	CallList * callList = mCallLists.Find(mCallLists.mHash(id));
	if (!callList) {
		return false;
	}

	return callList->Reg(plugin);
}



/** Unreg plugin in call list */
bool PluginList::unregCallList(const char * id, Plugin * plugin) {

	if (!plugin) {
		return false;
	}

	CallList * callList = mCallLists.Find(mCallLists.mHash(id));
	if (!callList) {
		return false;
	}

	return callList->Unreg(plugin);
}



/** Show all loading plugins and versions */
void PluginList::list(ostream & os) {

	for (PluginLoaderMap::iterator it = mPluginLoaders.begin(); it != mPluginLoaders.end(); ++it) {
		os << (*it)->mPlugin->getName() << " " << 
			(*it)->mPlugin->getVersion() << "\n";
	}

}



/** Show all plugins for all calls */
void PluginList::ListAll(ostream & os) {

	for (CallListMap::iterator it = mCallLists.begin(); it != mCallLists.end(); ++it) {
		os << "callList: " << (*it)->getName() << "\n";
		(*it)->ListRegs(os, "   ");
	}

}



/** Get plugin by name */
Plugin * PluginList::GetPlugin(const string & name) {

	PluginLoader * pluginLoader = mPluginLoaders.Find(mPluginLoaders.mHash(name));
	if (pluginLoader) {
		return pluginLoader->mPlugin;
	}

	return NULL;
}



/** Get plugin by lib */
Plugin * PluginList::GetPluginByLib(const string & lib) {
	
	for (PluginLoaderMap::iterator it = mPluginLoaders.begin(); it != mPluginLoaders.end(); ++it) {
		if ((*it)->getFileName() == lib) {
			return (*it)->mPlugin;
		}
	}

	return NULL;
}



/** OnPluginLoad */
void PluginList::OnPluginLoad(Plugin * plugin) {

	if (Log(1)) {
		const string & name = plugin->getName();
		const string & version = plugin->getVersion();
		LogStream() << "Plugin detected: " << (name != "" ? name : "n/a") <<
			" v " << (version != "" ? version : "n/a") << endl;
	}

	((Plugin *)plugin)->onLoad(mDcServerBase);

}


}; // namespace plugin
