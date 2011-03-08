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

#ifndef PLUGIN_LOADER_H
#define PLUGIN_LOADER_H

#include "Obj.h"
#include "dlfcn.h"

#include <string>

using ::std::string;

namespace plugin {


class Plugin;



/** Loader of plugin */
class PluginLoader : public Obj {

public:

	/** Pointer on plugin */
	Plugin * mPlugin;

public:

	PluginLoader(const string & filePath);
	~PluginLoader();

	/** Is error? */
	bool isError();

	/** Get error msg */
	inline const string & getError() {
		return mError;
	}

	/** Get lib name of plugin */
	inline const string & getFileName() {
		return mFile;
	}

	/** Open lib dll(so) */
	bool open();

	/** Close lib dll(so) */
	bool close();

	/** loadSym */
	bool loadSym();

	/** loadSym from dll(so) */
	void * loadSym(const char *);

	/** Log */
	bool strLog();

protected:

	typedef Plugin *(*tGetPluginFunc) ();

	typedef void (*tDelPluginFunc) (Plugin *);

	/** Handle of lib */
	void * mHandle;

	/** Error msg */
	string mError;

	string mFile;

	/** Pointer on create-function from lib */
	tGetPluginFunc mGetPluginFunc;

	/** Pointer on remove-function from lib */
	tDelPluginFunc mDelPluginFunc;

}; // PluginLoader

}; // namespace plugin

#endif // PLUGIN_LOADER_H
