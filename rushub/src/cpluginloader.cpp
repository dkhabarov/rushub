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

#include "cpluginloader.h"
#include "cpluginbase.h"

namespace nPlugin
{

cPluginLoader::cPluginLoader(const string &sPathFile) :
  cObj("cPluginLoader"),
  mPlugin(NULL),
  msFile(sPathFile),
  mHandle(NULL),
  msError(NULL),
  mGetPluginFunc(NULL),
  mDelPluginFunc(NULL)
{}

cPluginLoader::~cPluginLoader()
{
}

/** Open lib dll(so) */
bool cPluginLoader::Open()
{
  mHandle = dlopen(msFile.c_str(), RTLD_NOW);
  if(!mHandle || IsError()) {
    if(!mHandle) IsError();
    if(ErrLog(1)) LogStream() << "Can't open file '" << msFile << "' because:" << Error() << " handle(" << mHandle << ")" << endl;
    return false;
  }
  return true;
}

/** Close lib dll(so) */
bool cPluginLoader::Close()
{
  if(mHandle) {
    if(mPlugin && mDelPluginFunc && mPlugin->miVersion >= INTERNAL_PLUGIN_VERSION && mPlugin->miVersion < 20000) mDelPluginFunc(mPlugin);
    mPlugin = NULL;
    dlclose(mHandle);
    if(IsError()) {
      if(ErrLog(1)) LogStream() << "Can't close :" << Error() << endl;
      return false;
    }
    mHandle = NULL;
  }
  return true;
}

/** LoadSym */
bool cPluginLoader::LoadSym()
{
  if(!mGetPluginFunc) mGetPluginFunc = tGetPluginFunc(LoadSym("get_plugin"));
  if(!mDelPluginFunc) mDelPluginFunc = tDelPluginFunc(LoadSym("del_plugin"));
  if(!mGetPluginFunc || !mDelPluginFunc) return false;
  return ((mPlugin = mGetPluginFunc()) != NULL) && (mPlugin->miVersion >= INTERNAL_PLUGIN_VERSION && mPlugin->miVersion < 20000);
}

/** LoadSym from dll(so) */
void * cPluginLoader::LoadSym(const char *sName)
{
  void *func = dlsym(mHandle, sName);
  if(IsError()) {
    if(ErrLog(1)) LogStream() << "Can't load " << sName <<" exported interface :" << Error() << endl;
    return NULL;
  }
  return func;
}

/** Log */
int cPluginLoader::StrLog(ostream & os, int iLevel, int iMaxLevel)
{
  if(cObj::StrLog(os, iLevel, iMaxLevel)) {
    LogStream() << "(" << msFile << ") ";
    return 1;
  }
  return 0;
}

}; // nPlugin
