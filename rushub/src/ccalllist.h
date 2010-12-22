/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz

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

#ifndef CCALLLIST_H
#define CCALLLIST_H

#include <string>
#include <iostream>
#include <list>
#include <algorithm>
#include <functional>
#include "cobj.h"

using namespace std;

namespace nPlugin {

class cPlugin;
class cPluginList;
class cPluginListBase;

/** Base class for call list */
class cCallList : public cObj {

private:

	typedef list<cPlugin*> tPlugins;
	tPlugins mPlugins; /** Plugin list of call list */
	cPluginList *mPluginList; /** Pointer on common plugin list */

public:

	/** Unary function of plugin list for current call list */
	struct ufCallOne : public unary_function<void, tPlugins::iterator> {
		cPluginList *mPluginList; /** Pointer on common plugin list */
		cCallList *mCallList; /** Pointer on call list */
		int miCall; /** Returning value from plugin */

		void SetCallList(cCallList *CallList){mCallList = CallList;}
		ufCallOne(cPluginList *PluginList) : 
			mPluginList(PluginList),
			miCall(0)
		{}
		void operator()(cPlugin*);
	};

private:

	ufCallOne mCallOne; /** Unary function */
	string msName; /** Name of the call list */

public:

	cCallList(cPluginList*, string sId);
	virtual ~cCallList() {}

	/** Get name of the call list */
	virtual const string &Name() const { return msName; }

	bool Reg(cPlugin *); /** Registers plugin in call list */
	bool Unreg(cPlugin *); /** Remove registration from call list */
	virtual int CallAll(); /** Call all plugins */
	virtual int CallOne(cPlugin *) = 0; /** Call one plugin */
	virtual void ListRegs(ostream &os, const char *sSep); /** Show plugins list for this call */

}; // cCallList

}; // nPlugin

#endif // CCALLLIST_H
