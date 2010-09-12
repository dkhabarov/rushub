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

#ifndef CPLUGINBASE_H
#define CPLUGINBASE_H

#include "cpluginlistbase.h"
#include <string>

#ifndef REG_PLUGIN
  #ifdef _WIN32
    #define BUILDING_DLL 1
    #if BUILDING_DLL
      #define DLLIMPORT __declspec (dllexport)
    #else
      #define DLLIMPORT __declspec (dllimport)
    #endif
    #define REG_PLUGIN(__classname) \
    extern "C" {\
      DLLIMPORT cPluginBase * get_plugin(){return new (__classname);}\
      DLLIMPORT void del_plugin(cPluginBase *Plugin){if(Plugin) delete Plugin;}\
    }
  #else
    #define REG_PLUGIN(__classname) \
    extern "C" {\
      cPluginBase * get_plugin(void){return new (__classname);}\
      void del_plugin(cPluginBase *plugin){if(plugin) delete plugin;}\
    }
  #endif // _WIN32
#endif // REG_PLUGIN

#ifndef INTERNAL_PLUGIN_VERSION
#  define INTERNAL_PLUGIN_VERSION 10005
#endif

using ::std::string;

namespace nPlugin
{

/** Plugin base class */
class cPluginBase
{
private:

  bool mbIsAlive; /** State of plugin (loaded or not loaded) */

public:
  int miVersion; /** Interface version of all plugins */

public:

  cPluginBase() : mbIsAlive(true), miVersion(0), mPluginList(NULL)
  {
    miVersion = INTERNAL_PLUGIN_VERSION;
  }
  virtual ~cPluginBase(){}

  const string &Name(){ return msName; } /** Get name of the plugin */
  const string &Version(){return msVersion;} /** Get version of the plugin */
  void Suicide(){mbIsAlive = true;} /** Destruction plugin */
  bool IsAlive(){return mbIsAlive;} /** Check state */
  void SetPluginList(cPluginListBase * PluginList){mPluginList = PluginList;} /** Set plugin list for plugin */
  virtual const string & GetPluginDir(){ return mPluginList->GetPluginDir(); }; /** Plugin's dir */
  virtual bool RegAll(cPluginListBase*) = 0; /** Reg function in all call lists */

protected:

  string msName; /** Name of the plugin */
  string msVersion; /** Version of the plugin */
  cPluginListBase * mPluginList; /** Pointer to list of all plugins */

}; // cPluginBase

}; // nPlugin

#endif // CPLUGINBASE_H
