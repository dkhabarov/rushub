/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz

 * modified: 10 Dec 2009
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

#include "ccalllist.h"
#include "cplugin.h"
#include "cpluginlist.h"

namespace nPlugin {

cCallList::cCallList(cPluginList *PluginList, string sId) :
	cObj("cCallList"),
	mPluginList(PluginList), mCallOne(mPluginList), msName(sId)
{
	mCallOne.SetCallList(this);
	if(mPluginList) mPluginList->SetCallList(sId, this);
}

void cCallList::ufCallOne::operator()(cPlugin *Plugin) {
	miCall = mCallList->CallOne(Plugin);
	if(!Plugin->IsAlive()) mPluginList->UnloadPlugin(Plugin->Name());
}

/** Registers plugin in call list */
bool cCallList::Reg(cPlugin *Plugin) {
	if(!Plugin) return false;
	tPlugins::iterator i = find(mPlugins.begin(), mPlugins.end(), Plugin);
	if(i != mPlugins.end()) return false;
	mPlugins.push_back(Plugin);
	return true;
}

/** Remove registration from call list */
bool cCallList::Unreg(cPlugin *Plugin) {
	if(!Plugin) return false;
	tPlugins::iterator i = find(mPlugins.begin(), mPlugins.end(), Plugin);
	if(i == mPlugins.end()) return false;
	mPlugins.erase(i);
	return true;
}

/** Call all plugins */
int cCallList::CallAll() {
	/** 0 - default, 1 - lock, 2, 3 */
	mCallOne.miCall = 0;
	return For_each(mPlugins.begin(), mPlugins.end(), mCallOne).miCall;
}

/** Show plugins list for this call */
void cCallList::ListRegs(ostream &os, const char *sSep) {
	tPlugins::iterator i;
	for(i = mPlugins.begin(); i != mPlugins.end(); ++i) {
		os << sSep << (*i)->Name() << "\n";
	}
}

}; // nPlugin
