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

#include "CallList.h"
#include "Plugin.h"
#include "PluginList.h"

namespace plugin {



CallList::CallList(PluginList * pluginList, string id) :
	Obj("CallList"),
	mPluginList(pluginList),
	mCallOne(mPluginList),
	mName(id)
{

	mCallOne.SetCallList(this);

	if (mPluginList) {
		mPluginList->SetCallList(id, this);
	}

}



CallList::~CallList() {
}



/** Get name of the call list */
const string & CallList::getName() const {
	return mName;
}



CallList::ufCallOne::ufCallOne(PluginList * pluginList) : 
	mPluginList(pluginList),
	miCall(0)
{
}



void CallList::ufCallOne::SetCallList(CallList * callList) {
	mCallList = callList;
}



void CallList::ufCallOne::operator() (Plugin * plugin) {

	miCall = mCallList->CallOne(plugin);

/*#ifdef _WIN32
	__try {
		miCall = mCallList->CallOne(plugin);
	} __except( EXCEPTION_EXECUTE_HANDLER) {
		if (mPluginList && mPluginList->ErrLog(0)) {
			mPluginList->LogStream() << "error in plugin: " << plugin->getName() << endl;
		}
		plugin->suicide();
	}
#else
	miCall = mCallList->CallOne(plugin);
#endif*/

	if (!plugin->isAlive()) {
		mCallList->removedPlugins.push_back(plugin);
	}

}




/** Registers plugin in call list */
bool CallList::Reg(Plugin * plugin) {

	if (!plugin) {
		return false;
	}

	tPlugins::iterator i = find(mPlugins.begin(), mPlugins.end(), plugin);
	if (i != mPlugins.end()) {
		return false;
	}

	mPlugins.push_back(plugin);
	return true;
}



/** Remove registration from call list */
bool CallList::Unreg(Plugin *plugin) {

	if (!plugin) {
		return false;
	}

	tPlugins::iterator i = find(mPlugins.begin(), mPlugins.end(), plugin);
	if (i == mPlugins.end()) {
		return false;
	}

	mPlugins.erase(i);
	return true;
}



/** Call all plugins */
int CallList::CallAll() {

	/** 0 - default, 1 - lock, 2, 3 */
	mCallOne.miCall = for_each (mPlugins.begin(), mPlugins.end(), mCallOne).miCall;

	// Check removed
	if (removedPlugins.size()) {
		for (
			RemovedPlugins_t::const_iterator 
				it = removedPlugins.begin(),
				itEnd = removedPlugins.end();
			it != itEnd;
			++it
		) {
			mPluginList->UnloadPlugin((*it)->getName());
		}
		removedPlugins.clear();
	}

	return mCallOne.miCall;
}



/** Show plugins list for this call */
void CallList::ListRegs(ostream & os, const char * sep) {

	for (tPlugins::iterator i = mPlugins.begin(); i != mPlugins.end(); ++i) {
		os << sep << (*i)->getName() << "\n";
	}

}


}; // namespace plugin
