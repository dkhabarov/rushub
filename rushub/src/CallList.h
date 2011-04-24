/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz

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

#ifndef CALL_LIST_H
#define CALL_LIST_H

#include "Obj.h"

#include <string>
#include <iostream>
#include <list>
#include <algorithm>
#include <functional>


using namespace ::std;

namespace plugin {

class Plugin;
class PluginList;
class PluginListBase;

/** Base class for call list */
class CallList : public Obj {

public:

	/** Unary function of plugin list for current call list */
	struct ufCallOne : public unary_function<void, list<Plugin *>::iterator> {

		/** Pointer on common plugin list */
		PluginList * mPluginList;

		/** Pointer on call list */
		CallList * mCallList;

		/** Returning value from plugin */
		int miCall;

		ufCallOne(PluginList * pluginList);

		void setCallList(CallList * callList);

		void operator() (Plugin *);
	};

public:

	CallList(PluginList *, string id);

	virtual ~CallList();

	/** Get name of the call list */
	virtual const string & getName() const;

	/** Registers plugin in call list */
	bool reg(Plugin *);

	/** Remove registration from call list */
	bool unreg(Plugin *);

	/** Call all plugins */
	virtual int callAll();

	/** Call one plugin */
	virtual int callOne(Plugin *) = 0;

	/** Show plugins list for this call */
	virtual void listRegs(ostream & os, const char * sep);

private:

	typedef list<Plugin *> tPlugins;

	//< Plugin list of call list
	tPlugins mPlugins;

	//< Pointer on common plugin list
	PluginList * mPluginList;

	//< Unary function
	ufCallOne mCallOne;

	//< Name of the call list
	string mName;

	typedef vector<Plugin *> RemovedPlugins_t;

	//< Removed plugins
	RemovedPlugins_t removedPlugins;


}; // class CallList

}; // namespace plugin

#endif // CALL_LIST_H
