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

#include "cpluginlist.h"
#include "stringutils.h" /** StrCompare */
#include "cdir.h"
#include "cpluginloader.h"

namespace nDCServer {

namespace nPlugin {

cPluginList::cPluginList(const string sPath) :
	cObj("cPluginList"),
	cPluginListBase(),
	msPluginDir(sPath)
{
	if(!DirExists(msPluginDir.c_str())) mkdir(msPluginDir.c_str(), NULL);
	if(Log(1)) LogStream() << "Using plugins in: " << msPluginDir << endl;
}

void cPluginList::SetServer(cDCServerBase * server) {
	mDCServer = server;
}

/** Loading all plugins from plugins dir */
bool cPluginList::LoadAll() {
	if(Log(3)) LogStream() << "Open plugin dir: " << msPluginDir << endl;
	DIR *dir = opendir(msPluginDir.c_str());
	if(!dir) {
		if(ErrLog(1)) LogStream() << "Open plugin dir error" << endl;
		return false;
	}
	struct dirent * entry = NULL;
	string sFile;
	while(NULL != (entry = readdir(dir))) {
		sFile = entry->d_name;
		#ifdef _WIN32
			if((sFile.size() > 4) && (0 == StrCompare(sFile, sFile.size() - 4, 4, ".dll")))
		#else
			if((sFile.size() > 3) && (0 == StrCompare(sFile, sFile.size() - 3, 3, ".so")))
		#endif
		{
			if(Log(3)) LogStream() << "Plugin file name: " << sFile << endl;
			LoadPlugin(msPluginDir + sFile);
		}
	}
	closedir(dir);
	return true;
}

/** Loadin plugin from file (lib) dll (so) */
bool cPluginList::LoadPlugin(const string &sPathFile) {
	cPluginLoader * PluginLoader = new cPluginLoader(sPathFile);
	msError = "";
	if(Log(3)) LogStream() << "Attempt loading plugin: " << sPathFile << endl;
	try {
		if(
			!PluginLoader->Open() ||
			!PluginLoader->LoadSym() ||
			!mPluginList.Add(mPluginList.mHash(PluginLoader->mPlugin->Name()), PluginLoader)
		) {
			msError = PluginLoader->Error();
			if(Log(2)) LogStream() << "Failure loading plugin: " << sPathFile << (msError == "ok" ? "" : ("(" + msError + ")")) << endl;
			PluginLoader->Close();
			delete PluginLoader;
			return false;
		}
		PluginLoader->mPlugin->SetPluginList(this); /** Set pointer on list for plugin */
		PluginLoader->mPlugin->RegAll(this); /** Reg all call-function for this plugin */
		OnPluginLoad(PluginLoader->mPlugin); /** OnLoad */
	} catch (...) {
		if(ErrLog(1)) LogStream() << "Plugin " << sPathFile << " caused an exception" << endl;
		PluginLoader->Close();
		delete PluginLoader;
		return false;
	}
	if(Log(3)) LogStream() << "Success loading plugin: " << sPathFile << endl;
	delete PluginLoader;
	return true;
}

/** Unload plugin by name */
bool cPluginList::UnloadPlugin(const string &sName) {
	unsigned long Key = mPluginList.mHash(sName);
	cPluginLoader *PluginLoader = mPluginList.Find(Key);
	if(!PluginLoader || !mPluginList.Remove(Key)) {
		if(ErrLog(2)) LogStream() << "Can't unload plugin name: '" << sName << "'" << endl;
		return false;
	}
	tAllCallLists::iterator it;
	for(it = mAllCallLists.begin(); it != mAllCallLists.end(); ++it)
		(*it)->Unreg(PluginLoader->mPlugin);
	delete PluginLoader;
	return true;
}

/** Reload plugin by name */
bool cPluginList::ReloadPlugin(const string &sName) {
	cPluginLoader *PluginLoader = mPluginList.Find(mPluginList.mHash(sName));
	if(!PluginLoader) return false;
	string sPathFile = PluginLoader->GetFileName();
	if(!UnloadPlugin(sName)) return false;
	if(!LoadPlugin(sPathFile)) return false;
	return true;
}

/** Set call list */
bool cPluginList::SetCallList(string sId, cCallList *CallList) {
	if(!CallList) return false;
	if(sId.size() == 0) return false;
	return mAllCallLists.Add(mAllCallLists.mHash(sId), CallList);
}

/** Reg plugin in call list */
bool cPluginList::RegCallList(const char * sId, cPluginBase *Plugin) {
	if(!Plugin) return false;
	cCallList * CallList = mAllCallLists.Find(mAllCallLists.mHash(sId));
	if(!CallList) return false;
	return CallList->Reg(Plugin);
}

/** Unreg plugin in call list */
bool cPluginList::UnregCallList(const char * sId, cPluginBase *Plugin) {
	if(!Plugin) return false;
	cCallList *CallList = mAllCallLists.Find(mAllCallLists.mHash(sId));
	if(!CallList) return false;
	return CallList->Unreg(Plugin);
}

/** Show all loading plugins and versions */
void cPluginList::List(ostream &os) {
	tPluginList::iterator it;
	for(it = mPluginList.begin(); it != mPluginList.end(); ++it) {
		os << (*it)->mPlugin->Name() << " " << (*it)->mPlugin->Version() << "\n";
	}
}

/** Show all plugins for all calls */
void cPluginList::ListAll(ostream &os) {
	tAllCallLists::iterator it;
	for(it = mAllCallLists.begin(); it != mAllCallLists.end(); ++it) {
		os << "CallList: " << (*it)->Name() << "\n";
		(*it)->ListRegs(os, "   ");
	}
}

/** Get plugin by name */
cPluginBase * cPluginList::GetPlugin(const string &sName) {
	cPluginLoader *pi;
	pi = mPluginList.Find(mPluginList.mHash(sName));
	if(pi) return pi->mPlugin;
	else return NULL;
}

/** Get plugin by lib */
cPluginBase * cPluginList::GetPluginByLib(const string &sLib) {
	tPluginList::iterator it;
	for(it = mPluginList.begin(); it != mPluginList.end(); ++it)
		if((*it)->GetFileName() == sLib) return (*it)->mPlugin;
	return NULL;
}

/** OnPluginLoad */
void cPluginList::OnPluginLoad(cPluginBase *Plugin) {
	if(Log(1)) LogStream() << "OnPluginLoad: " << Plugin->Name() << endl;
	((cPlugin *)Plugin)->OnLoad(mDCServer);
}

}; // nPlugin

}; // nDCServer
