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

#include "PluginLoader.h"
#include "Plugin.h"

#ifdef _WIN32
	#include <windows.h>
#endif


namespace plugin {

#define VERSION_ERROR "hub supported version of plugin %d, this plugin have version %d"

#ifdef _WIN32
	#if defined(_MSC_VER) && (_MSC_VER >= 1400)
		#pragma warning(disable:4996) // Disable "This function or variable may be unsafe."
	#endif
#endif



template <typename T, typename F> T nasty_cast(F f) {
	union {
		F f;
		T t;
	} u;
	u.f = f;
	return u.t;
}



PluginLoader::PluginLoader(const string & filePath) :
	Obj("PluginLoader"),
	mPlugin(NULL),
	mHandle(NULL),
	mFile(filePath),
	mGetPluginFunc(NULL),
	mDelPluginFunc(NULL)
{
}



PluginLoader::~PluginLoader() {
}



/** Is error? */
bool PluginLoader::isError() {
	const char * error = dlerror();
	if (!error) {
		return false;
	}
	mError = error;
	return true;
}



/** Open lib dll(so) */
bool PluginLoader::open() {
	mHandle = dlopen(mFile.c_str(), RTLD_NOW);
	if (!mHandle || isError()) {
		if (!mHandle) {
			isError(); // ???
		}
		LOG(LEVEL_ERROR, "Can't open file '" << mFile << "' because:" << getError() << " handle(" << mHandle << ")");
		return false;
	}
	return true;
}

bool PluginLoader::destructPlugin() {
	#ifdef _WIN32
		__try {
			mDelPluginFunc(mPlugin);
		} __except(EXCEPTION_EXECUTE_HANDLER) {
			return false;
		}
	#else
		mDelPluginFunc(mPlugin);
	#endif
	return true;
}


/** Close lib dll(so) */
bool PluginLoader::close() {
	if (mHandle) {
		if (mPlugin && mDelPluginFunc && mPlugin->mInternalPluginVersion == INTERNAL_PLUGIN_VERSION) {
			if (!destructPlugin()) {
				LOG(LEVEL_FATAL, "error in DelPluginFunc (" << mPlugin->getName() << ")");
			}
		}
		mPlugin = NULL;
		dlclose(mHandle);
		if (isError()) {
			LOG(LEVEL_ERROR, "Can't close :" << getError());
			return false;
		}
		mHandle = NULL;
	}
	return true;
}



/** loadSym */
bool PluginLoader::loadSym() {

	if (!mGetPluginFunc) {
		mGetPluginFunc = nasty_cast<tGetPluginFunc> (loadSym("get_plugin"));
	}

	if (!mDelPluginFunc) {
		mDelPluginFunc = nasty_cast<tDelPluginFunc> (loadSym("del_plugin"));
	}

	if (!mGetPluginFunc || !mDelPluginFunc || ((mPlugin = mGetPluginFunc()) == NULL)) {
		return false;
	}

	if (mPlugin->mInternalPluginVersion != INTERNAL_PLUGIN_VERSION) {
		char buf[128] = { '\0' };
		sprintf(buf, VERSION_ERROR, INTERNAL_PLUGIN_VERSION, mPlugin->mInternalPluginVersion);
		mError = buf;
		return false;
	}

	return true;
}



/** loadSym from dll(so) */
void * PluginLoader::loadSym(const char * name) {
	void * func = dlsym(mHandle, name);
	if (isError()) {
		LOG(LEVEL_ERROR, "Can't load " << name <<" exported interface :" << getError());
		return NULL;
	}
	return func;
}



/** log */
bool PluginLoader::strLog(int level, ostream & os) {
	Obj::strLog(level, os);
	os << "[" << mFile << "] ";
	return true;
}


} // namespace plugin

/**
 * $Id$
 * $HeadURL$
 */
