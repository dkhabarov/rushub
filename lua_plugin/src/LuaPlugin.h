/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * Copyright (C) 2009-2011 by Setuper
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

#ifndef LUA_PLUGIN_H
#define LUA_PLUGIN_H

#include "Plugin.h" // ::dcserver
#include "LuaInterpreter.h"
#include "TaskList.h"
#include "api.h"
#include "HubConfig.h"

#include <vector>
#include <list>
#include <math.h> // abs
#include <string>

#if defined(_DEBUG) && defined(VLD)
	#include <vld.h>
#endif

#define PLUGIN_NAME "LuaPlugin"
#define PLUGIN_VERSION "2.2"

namespace dcserver {

	class DcServerBase;

}; // namespace dcserver


using namespace ::std;
using namespace ::plugin;
using namespace ::dcserver;
using namespace ::luaplugin;


class LuaPlugin : public Plugin {

public:

	string mScriptsDir; /** Scripts path (directory) */

	static DcServerBase * mCurServer; /** Current server */
	static LuaPlugin * mCurLua; /** Current plugin */

	static string mLuaPath; /** lua package.path */
	static string mLuaCPath; /** lua package.cpath */
	static bool mSetLuaPath; /** is set package.path and package.cpath */

	typedef list<LuaInterpreter *> LuaInterpreterList;
	LuaInterpreterList mLua; /** Script-list */

	LuaInterpreter * mCurScript; /** Current script. When script is working only! */
	string mLastError; /** Last error in scripts */

	HubConfig mHubConfig;
	TasksList mTasksList;
	TimerList * mTimerList;

	DcConnBase * mCurDCConn; /** Current connection (only for events) */

public:

	LuaPlugin();
	virtual ~LuaPlugin();

	virtual const string & getScriptsDir() {
		return mScriptsDir;
	}

	virtual void onLoad(DcServerBase *); /** Actions when loading plugin */
	virtual bool regAll(PluginListBase *); /** Registration all events for this plugin */

	// events
	virtual int onUserConnected(DcUserBase *);
	virtual int onUserDisconnected(DcUserBase *);
	virtual int onUserEnter(DcUserBase *);
	virtual int onUserExit(DcUserBase *);
	virtual int onSupports(DcUserBase *);
	virtual int onKey(DcUserBase *);
	virtual int onValidateNick(DcUserBase *);
	virtual int onMyPass(DcUserBase *);
	virtual int onVersion(DcUserBase *);
	virtual int onGetNickList(DcUserBase *);
	virtual int onMyINFO(DcUserBase *);
	virtual int onChat(DcUserBase *);
	virtual int onTo(DcUserBase *);
	virtual int onConnectToMe(DcUserBase *);
	virtual int onRevConnectToMe(DcUserBase *);
	virtual int onSearch(DcUserBase *);
	virtual int onSR(DcUserBase *);
	virtual int onKick(DcUserBase *);
	virtual int onOpForceMove(DcUserBase *);
	virtual int onGetINFO(DcUserBase *);
	virtual int onMCTo(DcUserBase *);
	virtual int onTimer();
	virtual int onAny(DcUserBase *, int);
	virtual int onUnknown(DcUserBase *);
	virtual int onFlood(DcUserBase *, int, int);
	virtual int onWebData(WebUserBase *, WebParserBase *);

	virtual int onScriptError(LuaInterpreter *, const char * scriptName, const char * errMsg, bool stoped = true);
	virtual int onScriptAction(const char * scriptName, const char * action);

	int onConfigChange(const char * name, const char * value);

	int callAll(const char * fancName, DcUserBase * dcUserBase, bool param = true); /** Calling event for all scripts */

	LuaInterpreter * findScript(const string & scriptName);
	LuaInterpreter * addScript(const string & scriptName, bool onlyNew = false);

	void checkNewScripts();

	int loadScripts(); /** Loading all scripts */
	int clear(); /** Stoping and removing all scripts */

	/** Restarting all scripts 
		@param iType: 0 - simple restarting, 1 - restarting except loaded script, 2 - restarting except current script
	*/
	int restartScripts(LuaInterpreter * curScript = NULL, int type = 0);

	int startScript(const string & scriptName);
	int startScript(LuaInterpreter *);
	int stopScript(LuaInterpreter *, bool current = false);
	int restartScript(LuaInterpreter *, bool current = false);

	int checkExists(LuaInterpreter *);

	void save(); /** Saving all scripts */

	int moveUp(LuaInterpreter *);
	int moveDown(LuaInterpreter *);

}; // class LuaPlugin

#endif // LUA_PLUGIN_H

/**
 * $Id$
 * $HeadURL$
 */
