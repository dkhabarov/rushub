/*
 * RusHub - hub server for Direct Connect peer to peer network.

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

#ifndef LUA_PLUGIN_H
#define LUA_PLUGIN_H

#include "Plugin.h" // ::dcserver
#include "LuaInterpreter.h"
#include "TaskList.h"
#include "api.h"

#include <vector>
#include <list>
#include <math.h> // abs
#include <string>

#define PLUGIN_NAME "LuaPlugin"
#define PLUGIN_VERSION "2.0[beta]"

namespace dcserver {

	class DcServerBase;

}; // namespace dcserver


using namespace ::std;
using namespace ::plugin;
using namespace ::dcserver;
using namespace ::dcserver::protoenums;
using namespace ::luaplugin;


class LuaPlugin : public Plugin {

public:

	string msScriptsDir; /** Scripts path (directory) */

	static DcServerBase * mCurServer; /** Current server */
	static LuaPlugin * mCurLua; /** Current plugin */

	static string msLuaPath; /** lua package.path */
	static string msLuaCPath; /** lua package.cpath */
	static bool mbSetLuaPath; /** is set package.path and package.cpath */

	typedef list<LuaInterpreter *> tvLuaInterpreter;
	tvLuaInterpreter mLua; /** Script-list */

	LuaInterpreter * mCurScript; /** Current script. When script is working only! */
	string msLastError; /** Last error in scripts */

	TasksList mTasksList;
	cTimerList * mTimerList;

	DcParserBase * mCurDCParser; /** Current cmd-parser (only for events) */
	DcConnBase * mCurDCConn; /** Current connection (only for events) */

public:

	LuaPlugin();
	virtual ~LuaPlugin();

	virtual const string & GetScriptsDir(){ return msScriptsDir; }

	virtual void onLoad(DcServerBase *); /** Actions when loading plugin */
	virtual bool regAll(PluginListBase * pluginListBase); /** Registration all events for this plugin */

// events
	virtual int onUserConnected(DcConnBase *);
	virtual int onUserDisconnected(DcConnBase *);
	virtual int onUserEnter(DcConnBase *);
	virtual int onUserExit(DcConnBase *);
	virtual int onSupports(DcConnBase *, DcParserBase *);
	virtual int onKey(DcConnBase *, DcParserBase *);
	virtual int onValidateNick(DcConnBase *, DcParserBase *);
	virtual int onMyPass(DcConnBase *, DcParserBase *);
	virtual int onVersion(DcConnBase *, DcParserBase *);
	virtual int onGetNickList(DcConnBase *, DcParserBase *);
	virtual int onMyINFO(DcConnBase *, DcParserBase *);
	virtual int onChat(DcConnBase *, DcParserBase *);
	virtual int onTo(DcConnBase *, DcParserBase *);
	virtual int onConnectToMe(DcConnBase *, DcParserBase *);
	virtual int onRevConnectToMe(DcConnBase *, DcParserBase *);
	virtual int onSearch(DcConnBase *, DcParserBase *);
	virtual int onSR(DcConnBase *, DcParserBase *);
	virtual int onKick(DcConnBase *, DcParserBase *);
	virtual int onOpForceMove(DcConnBase *, DcParserBase *);
	virtual int onGetINFO(DcConnBase *, DcParserBase *);
	virtual int onMCTo(DcConnBase *, DcParserBase *);
	virtual int onTimer();
	virtual int onAny(DcConnBase *, DcParserBase *);
	virtual int onUnknown(DcConnBase *, DcParserBase *);
	virtual int onFlood(DcConnBase *, int, int);
	virtual int onWebData(DcConnBase *, WebParserBase *);

	virtual int OnScriptError(LuaInterpreter * Current, const char* sScriptName, const char* sErrMsg, bool bStoped = true);
	virtual int OnScriptAction(const char * sScriptName, const char * sAction);

	int OnConfigChange(const char *, const char *);

	int CallAll(const char *, DcConnBase * dcConnBase = NULL, DcParserBase * dcParserBase = NULL); /** Calling event for all scripts */

	LuaInterpreter * FindScript(const string & sScriptName);
	LuaInterpreter * AddScript(const string & sScriptName, bool bOnlyNew = false);

	void CheckNewScripts();

	int LoadScripts(); /** Loading all scripts */
	int Clear(); /** Stoping and removing all scripts */

	/** Restarting all scripts 
		@param iType: 0 - simple restarting, 1 - restarting except loaded script, 2 - restarting except current script
	*/
	int RestartScripts(LuaInterpreter * CurScript = NULL, int iType = 0);

	int StartScript(const string & sScriptName);
	int StartScript(LuaInterpreter * Script);
	int StopScript(LuaInterpreter * Script, bool bCurrent = false);
	int RestartScript(LuaInterpreter * Script, bool bCurrent = false);

	int CheckExists(LuaInterpreter * Script);

	void save(); /** Saving all scripts */

	int MoveUp(LuaInterpreter * Script);
	int MoveDown(LuaInterpreter * Script);

}; // class LuaPlugin

#endif // LUA_PLUGIN_H
