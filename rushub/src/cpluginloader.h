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

#ifndef CPLUGINLOADER_H
#define CPLUGINLOADER_H

#include "cobj.h"
#include "dlfcn.h"
#include <string>

using std::string;

namespace nPlugin {

class cPluginBase;

/** Loader of plugin */
class cPluginLoader : public cObj {

public:

	cPluginBase *mPlugin; /** Pointer on plugin */

public:

	cPluginLoader(const string &sPathFile);
	~cPluginLoader();

	bool IsError(){return (msError = dlerror()) != NULL;} /** Is error? */
	string Error(){return string((msError != NULL) ? msError : "ok");} /** Get error msg */
	string GetFileName(){ return msFile;} /** Get lib name of plugin */
	bool Open(); /** Open lib dll(so) */
	bool Close(); /** Close lib dll(so) */
	bool LoadSym(); /** LoadSym */
	void * LoadSym(const char *); /** LoadSym from dll(so) */
	int StrLog(ostream & os, int iLevel, int iMaxLevel); /** Log */

protected:

	typedef cPluginBase *(*tGetPluginFunc)();
	typedef void (*tDelPluginFunc)(cPluginBase *);

	string msFile;
	void *mHandle; /** Handle of lib */
	const char * msError; /** Error msg */

	tGetPluginFunc mGetPluginFunc; /** Pointer on create-function from lib */
	tDelPluginFunc mDelPluginFunc; /** Pointer on remove-function from lib */

}; // cPluginLoader

}; // nPlugin

#endif // CPLUGINLOADER_H
