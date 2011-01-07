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

#ifndef CPLUGINLOADER_H
#define CPLUGINLOADER_H

#include "cobj.h"
#include "dlfcn.h"
#include <string>

using std::string;

namespace nPlugin {

class cPlugin;

/** Loader of plugin */
class cPluginLoader : public cObj {

public:

	cPlugin * mPlugin; /** Pointer on plugin */

public:

	cPluginLoader(const string &sPathFile);
	~cPluginLoader();

	bool IsError(); /** Is error? */
	const string & Error(){return msError;} /** Get error msg */
	const string & GetFileName(){ return msFile;} /** Get lib name of plugin */
	bool Open(); /** Open lib dll(so) */
	bool Close(); /** Close lib dll(so) */
	bool LoadSym(); /** LoadSym */
	void * LoadSym(const char *); /** LoadSym from dll(so) */
	int StrLog(ostream & os, int iLevel, int iMaxLevel, bool bIsError = false); /** Log */

protected:

	typedef cPlugin *(*tGetPluginFunc)();
	typedef void (*tDelPluginFunc)(cPlugin *);

	void *mHandle; /** Handle of lib */
	string msError; /** Error msg */
	string msFile;

	tGetPluginFunc mGetPluginFunc; /** Pointer on create-function from lib */
	tDelPluginFunc mDelPluginFunc; /** Pointer on remove-function from lib */

}; // cPluginLoader

}; // nPlugin

#endif // CPLUGINLOADER_H
