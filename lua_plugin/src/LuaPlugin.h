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
#include <string>

// Visual Leak Detector
#ifdef VLD
	#include <vld.h>
#endif

#define PLUGIN_NAME "LuaPlugin"
#define PLUGIN_VERSION "2.5[beta]"

namespace dcserver {

	class DcServerBase;

}; // namespace dcserver


using namespace ::std;
using namespace ::plugin;
using namespace ::dcserver;
using namespace ::luaplugin;

// ATTENTION! 31 max !!!!!!!!
enum {
	LIST_USER_CONNECTED    = 1 << 0,
	LIST_USER_DISCONNECTED = 1 << 1,
	LIST_USER_ENTER        = 1 << 2,
	LIST_USER_EXIT         = 1 << 3,
	LIST_SUPPORTS          = 1 << 4,
	LIST_KEY               = 1 << 5,
	LIST_VALIDATE_NICK     = 1 << 6,
	LIST_MY_PASS           = 1 << 7,
	LIST_VERSION           = 1 << 8,
	LIST_GET_NICK_LIST     = 1 << 9,
	LIST_MY_INFO           = 1 << 10,
	LIST_CHAT              = 1 << 11,
	LIST_TO                = 1 << 12,
	LIST_CONNECT_TO_ME     = 1 << 13,
	LIST_REV_CONNECT_TO_ME = 1 << 14,
	LIST_SEARCH            = 1 << 15,
	LIST_SR                = 1 << 16,
	LIST_KICK              = 1 << 17,
	LIST_OP_FORCE_MOVE     = 1 << 18,
	LIST_GET_INFO          = 1 << 19,
	LIST_MC_TO             = 1 << 20,
	LIST_TIMER             = 1 << 21,
	LIST_FLOOD             = 1 << 22,
	LIST_WEB_DATA          = 1 << 23,
	LIST_SCRIPT_ERROR      = 1 << 24,
	LIST_CONFIG_CHANGE     = 1 << 25,
	LIST_ANY               = 1 << 26,
	LIST_UNKNOWN           = 1 << 27
}; // ATTENTION! 31 max !!!!!!!!


class LuaPlugin : public Plugin {

public:

	string mScriptsDir; /** Scripts path (directory) */

	static DcServerBase * mCurServer; /** Current server */
	static LuaPlugin * mCurLua; /** Current plugin */

	static string mLuaPath; /** lua package.path */
	static string mLuaCPath; /** lua package.cpath */
	static bool mSetLuaPath; /** is set package.path and package.cpath */

	typedef list<LuaInterpreter *> listLuaInterpreter;
	typedef vector<LuaInterpreter *> vectorLuaInterpreter;
	listLuaInterpreter mLua; /** Script-list */

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
	virtual int onWebData(WebUserBase *);

	virtual int onScriptError(LuaInterpreter *, const char * scriptName, const char * errMsg, bool stoped = true);
	virtual int onScriptAction(const char * scriptName, const char * action);

	int onConfigChange(const char * name, const char * value);

	int callAll(const char * funcName, unsigned int listFlag, vectorLuaInterpreter & vli, DcUserBase * dcUserBase, bool param = true); /** Calling event for all scripts */

	LuaInterpreter * findScript(const string & scriptName);
	LuaInterpreter * addScript(const string & scriptName, bool onlyNew = false);

	void checkNewScripts();

	int loadScripts(); /** Loading all scripts */
	int clear(); /** Stoping and removing all scripts */

	/** Restarting all scripts 
		@param type: 0 - simple restarting, 1 - restarting except loaded script, 2 - restarting except current script
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

	unsigned int getListFlags() {
		return mListFlags;
	}

	void clearListFlags() {
		mListFlags = 0;
	}

	unsigned int getListFlag(unsigned int flag) {
		return mListFlags & flag;
	}

	void setListFlag(unsigned int flag) {
		mListFlags |= flag;
	}
private:

	unsigned int mListFlags;

	vectorLuaInterpreter mUserConnected;
	vectorLuaInterpreter mUserDisconnected;
	vectorLuaInterpreter mUserEnter;
	vectorLuaInterpreter mUserExit;
	vectorLuaInterpreter mSupports;
	vectorLuaInterpreter mKey;
	vectorLuaInterpreter mValidateNick;
	vectorLuaInterpreter mMyPass;
	vectorLuaInterpreter mVersionList;
	vectorLuaInterpreter mGetNickList;
	vectorLuaInterpreter mMyINFO;
	vectorLuaInterpreter mChat;
	vectorLuaInterpreter mTo;
	vectorLuaInterpreter mConnectToMe;
	vectorLuaInterpreter mRevConnectToMe;
	vectorLuaInterpreter mSearch;
	vectorLuaInterpreter mSR;
	vectorLuaInterpreter mKick;
	vectorLuaInterpreter mOpForceMove;
	vectorLuaInterpreter mGetINFO;
	vectorLuaInterpreter mMCTo;
	vectorLuaInterpreter mAny;
	vectorLuaInterpreter mUnknown;
	vectorLuaInterpreter mTimer;
	vectorLuaInterpreter mFlood;
	vectorLuaInterpreter mWebData;
	vectorLuaInterpreter mScriptError;
	vectorLuaInterpreter mConfigChange;

}; // class LuaPlugin

#endif // LUA_PLUGIN_H

/**
 * $Id$
 * $HeadURL$
 */
