/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * Copyright (C) 2009-2012 by Setuper
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


#include "LuaPlugin.h"
#include "Dir.h"

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif // HAVE_CONFIG_H

#if HAVE_TINYXML_H
  #include <tinyxml.h>
#else
  #include "tinyxml/tinyxml.h"
#endif // HAVE_TINYXML_H

#ifdef _WIN32
	#pragma comment(lib, "tinyxml.lib")
#endif // _WIN32

using namespace ::utils;

DcServerBase * LuaPlugin::mCurServer = NULL;
LuaPlugin * LuaPlugin::mCurLua = NULL;
string LuaPlugin::mLuaCPath;
string LuaPlugin::mLuaPath;
bool LuaPlugin::mSetLuaPath = false;

LuaPlugin::LuaPlugin() : 
	mCurScript(NULL),
	mCurUser(NULL),
	mListFlags(0)
{
	mCurLua = this;
	mName = PLUGIN_NAME;
	mVersion = PLUGIN_VERSION;
}

LuaPlugin::~LuaPlugin() {
	save();
	clear();
	mCurServer = NULL;
	mCurLua = NULL;
}

/** Actions when loading plugin */
void LuaPlugin::onLoad(DcServerBase * dcServerBase) {
	//setlocale(LC_ALL, dcServerBase->getLocale().c_str());
	mCurServer = dcServerBase;

	regAll(dcServerBase); // Registration all events for this plugin

	string mainDir(dcServerBase->getMainDir());

	mScriptsDir = mainDir;
	mScriptsDir.append(STR_LEN("scripts/"));
	Dir::checkPath(mScriptsDir);

	string libs = mainDir;
	libs.append(STR_LEN("libs/"));
	Dir::checkPath(libs);

	#ifdef _WIN32

		// replace slashes
		size_t pos = mainDir.find('/');
		while (pos != mainDir.npos) {
			mainDir.replace(pos, 1, LUA_DIRSEP);
			pos = mainDir.find('/', pos);
		}

		mLuaCPath = mainDir + "libs" LUA_DIRSEP "?.dll;" +
			mainDir + "?.dll;" +
			mainDir + "scripts" LUA_DIRSEP "?.dll;" +
			mainDir + "scripts" LUA_DIRSEP "libs" LUA_DIRSEP "?.dll;";

	#else

		mLuaCPath = mainDir + "libs" LUA_DIRSEP "?.so;" +
			mainDir + "?.so;" +
			mainDir + "scripts" LUA_DIRSEP "?.so;" +
			mainDir + "scripts" LUA_DIRSEP "libs" LUA_DIRSEP "?.so;";

	#endif

	mLuaPath = mainDir + "libs" LUA_DIRSEP "?.lua;" +
		mainDir + "libs" LUA_DIRSEP "?" LUA_DIRSEP "init.lua;" +
		mainDir + "?.lua;" +
		mainDir + "?" LUA_DIRSEP "init.lua;" +
		mainDir + "scripts" LUA_DIRSEP "?.lua;" +
		mainDir + "scripts" LUA_DIRSEP "?" LUA_DIRSEP "init.lua;" +
		mainDir + "scripts" LUA_DIRSEP "libs" LUA_DIRSEP "?.lua;" +
		mainDir + "scripts" LUA_DIRSEP "libs" LUA_DIRSEP "?" LUA_DIRSEP "init.lua;";

	mSetLuaPath = false;

	loadScripts();
	save();
}

/** Registration all events */
bool LuaPlugin::regAll(DcServerBase * dcServerBase) {
	dcServerBase->regCallList("Timer",       this);
	dcServerBase->regCallList("Conn",        this);
	dcServerBase->regCallList("Disconn",     this);
	dcServerBase->regCallList("Enter",       this);
	dcServerBase->regCallList("Exit",        this);
	dcServerBase->regCallList("Supports",    this);
	dcServerBase->regCallList("Key",         this);
	dcServerBase->regCallList("Validate",    this);
	dcServerBase->regCallList("MyPass",      this);
	dcServerBase->regCallList("Version",     this);
	dcServerBase->regCallList("GetNickList", this);
	dcServerBase->regCallList("MyINFO",      this);
	dcServerBase->regCallList("Chat",        this);
	dcServerBase->regCallList("To",          this);
	dcServerBase->regCallList("CTM",         this);
	dcServerBase->regCallList("RCTM",        this);
	dcServerBase->regCallList("Search",      this);
	dcServerBase->regCallList("SR",          this);
	dcServerBase->regCallList("Kick",        this);
	dcServerBase->regCallList("OpForce",     this);
	dcServerBase->regCallList("GetINFO",     this);
	dcServerBase->regCallList("MCTo",        this);
	dcServerBase->regCallList("Any",         this);
	dcServerBase->regCallList("Unknown",     this);
	dcServerBase->regCallList("Flood",       this);
	dcServerBase->regCallList("WebData",     this);
	return true;
}


////////////////////////////////////////////

LuaInterpreter * LuaPlugin::findScript(const string & scriptName) {
	listLuaInterpreter::iterator it, it_e = mLua.end();
	LuaInterpreter * script = NULL;
	for (it = mLua.begin(); it != it_e; ++it) {
		script = *it;
		if (script && script->mName == scriptName) {
			return script;
		}
	}
	return NULL;
}

int LuaPlugin::checkExists(LuaInterpreter * script) {
	if (!script) {
		return 0;
	} else if (!Dir::isFileExist((mScriptsDir + script->mName).c_str())) {
		script->mEnabled = false;
		return 0;
	}
	return 1;
}

/** Creation and addition script in list 
	@param sScriptName - script name
	@param bOnlyNew - return new scripts only
	@return NULL - not exist or (bOnlyNew && old script)
*/
LuaInterpreter * LuaPlugin::addScript(const string & scriptName, bool onlyNew) {
	LuaInterpreter * script = findScript(scriptName);
	if (!script && Dir::isFileExist((mScriptsDir + scriptName).c_str())) {
		script = new LuaInterpreter(scriptName, mScriptsDir);
		mLua.push_back(script);
		return script;
	}
	return (onlyNew == true) ? NULL : script;
}

/** Starting script by name
	@return -1 - already, was loaded earlier
	@return 0 - loaded
	@return LUA_ERRFILE (6) - script was not found
	@return LUA_ERRSYNTAX (3) - error in the script
	@return LUA_ERRMEM (4) - memory error
*/
int LuaPlugin::startScript(const string & scriptName) {
	return startScript(addScript(scriptName));
}

/** Attempt to start script
	@return -1 - already, was loaded earlier
	@return 0 - loaded
	@return LUA_ERRFILE (6) - script was not found
	@return LUA_ERRSYNTAX (3) - error in the script
	@return LUA_ERRMEM (4) - memory error
*/
int LuaPlugin::startScript(LuaInterpreter * script) {
	if (!script) {
		return LUA_ERRFILE;
	}
	clearListFlags();
	mTasksList.addTask(NULL, TASKTYPE_SAVE);
	return script->start();
}

/** Stoping script
	@return -1 - already, was stoped earlier
	@return 0 - stoped,
	@return LUA_ERRFILE - script was not found
*/
int LuaPlugin::stopScript(LuaInterpreter * script, bool current) {
	int ret = LUA_ERRFILE;
	if (script) {
		clearListFlags();
		mTasksList.addTask(NULL, TASKTYPE_SAVE);
		if (current) {
			script->mEnabled = false;
			mTasksList.addTask(static_cast<void *> (script), TASKTYPE_STOPSCRIPT);
			if (!checkExists(script)) { // Removing from list in the case of absence
				mLua.remove(script);
				ret = LUA_ERRFILE;
			} else {
				ret = 0;
			}
		} else {
			ret = -1;
			if (script->stop()) {
				ret = 0;
			}
			if (!checkExists(script)) {
				mLua.remove(script);
				delete script;
				script = NULL;
				ret = LUA_ERRFILE;
			}
		}
	}
	return ret;
}

/** Restarting script
	@return 0 - restarted
	@return LUA_ERRFILE - script was not found
	@return LUA_ERRSYNTAX - error in the script
*/
int LuaPlugin::restartScript(LuaInterpreter * script, bool current) {
	if (script) {
		mTasksList.addTask(NULL, TASKTYPE_SAVE);
		if (current) {
			int state = luaL_loadfile(script->mL, (mScriptsDir + script->mName).c_str());
			if (!state) {
				mTasksList.addTask(static_cast<void *> (script), TASKTYPE_RESTARTSCRIPT);
			} else {
				stopScript(script, true);
			}
			return state;
		} else {
			int state = stopScript(script);
			if (state == 0 || state == -1) {
				return startScript(script);
			}
		}
	}
	return LUA_ERRFILE;
}

/** Restarting all scripts */
int LuaPlugin::restartScripts(LuaInterpreter * curScript, int type) {
	listLuaInterpreter::iterator it, it_prev, it_e = mLua.end();
	LuaInterpreter * script = NULL;
	bool first = true;
	for (it = mLua.begin(); it != it_e; ++it) {
		script = *it;
		if (script) {
			if (((type == 0 || (type == 2 && script != curScript)) && 
				script->mEnabled && restartScript(script, script == curScript) == LUA_ERRFILE) ||
				(!script->mEnabled && stopScript(script, script == curScript) == LUA_ERRFILE))
			{
				if (first) {
					first = false;
					it = mLua.begin();
				} else {
					it = it_prev;
				}
			}
			it_prev = it;
		}
	}
	mTasksList.addTask(NULL, TASKTYPE_SAVE);
	checkNewScripts();
	return 1;
}

/** Loading all scripts */
int LuaPlugin::loadScripts() {
	TiXmlDocument file((mCurServer->getPluginDir() + "scripts.xml").c_str());
	if (file.LoadFile()) {
		TiXmlHandle mainHandle(&file);
		TiXmlElement * mainItem = mainHandle.FirstChild("Scripts").Element();
		if (mainItem != NULL) {
			const char * name = NULL, *enabled = NULL;
			TiXmlNode * value = NULL;
			while ((value = mainItem->IterateChildren(value)) != NULL) {
				if (value->ToElement() == NULL || 
					(name = value->ToElement()->Attribute("Name")) == NULL || 
					(enabled = value->ToElement()->Attribute("Enabled")) == NULL
				) {
					continue;
				}
				string file(name);
				if ((file.size() <= 4) || (0 != file.compare(file.size() - 4, 4, ".lua"))) {
					file.append(STR_LEN(".lua"));
				}
				LuaInterpreter * script = addScript(file, true);
				if (strcmp(enabled, "false") != 0 && strcmp(enabled, "0") != 0) {
					startScript(script);
				}
			}
		}
	}
	checkNewScripts();
	return 1;
}

void LuaPlugin::checkNewScripts() {

	Dir::checkPath(mScriptsDir);

	DIR * dir = opendir(mScriptsDir.c_str());
	if (!dir) {
		return;
	}

	struct dirent * entry = NULL;
	string file;

	while (NULL != (entry = readdir(dir))) {
		file = entry->d_name;
		if ((file.size() > 4) && (0 == file.compare(file.size() - 4, 4, ".lua"))) {
			addScript(file, true);
		}
	}

	closedir(dir);
}

/** Stoping all scripts */
int LuaPlugin::clear() {
	clearListFlags();
	listLuaInterpreter::iterator it, it_e = mLua.end();
	for (it = mLua.begin(); it != it_e; ++it) {
		if (*it != NULL) {
			delete *it;
			*it = NULL;
		}
	}
	mLua.clear();
	return 1;
}

void LuaPlugin::save() {
	TiXmlDocument file((mCurServer->getPluginDir() + "scripts.xml").c_str());
	file.InsertEndChild(TiXmlDeclaration("1.0", "windows-1251", "yes"));
	TiXmlElement mainItem("Scripts");
	mainItem.SetAttribute("Version", PLUGIN_VERSION);
	listLuaInterpreter::iterator it, it_e = mLua.end();
	LuaInterpreter * script = NULL;
	for (it = mLua.begin(); it != it_e; ++it) {
		script = *it;
		if (script) {
			TiXmlElement item("Script");
			item.SetAttribute("Name", script->mName.c_str());
			item.SetAttribute("Enabled", script->mL == NULL ? 0 : 1);
			mainItem.InsertEndChild(item);
		}
	}
	file.InsertEndChild(mainItem);
	file.SaveFile();
}

int LuaPlugin::moveUp(LuaInterpreter * script) {
	listLuaInterpreter::iterator it, it_prev = mLua.begin(), it_e = mLua.end();
	clearListFlags();
	for (it = ++mLua.begin(); it != it_e; ++it) {
		if (*it) {
			if (*it == script) {
				mLua.erase(it);
				mLua.insert(it_prev, script);
				return 1;
			}
			it_prev = it;
		}
	}
	return 0;
}

int LuaPlugin::moveDown(LuaInterpreter * script) {
	listLuaInterpreter::iterator it;
	LuaInterpreter * tmp = NULL;
	clearListFlags();
	for (it = mLua.begin(); it != --mLua.end(); ++it) {
		if ((*it) && *it == script) {
			++it;
			tmp = *it;
			mLua.insert(--it, tmp);
			mLua.erase(++it);
			return 1;
		}
	}
	return 0;
}



// TODO add cmd param
/** 1 - blocked */
int LuaPlugin::callAll(const char * funcName, unsigned int listFlag, vectorLuaInterpreter & vli, DcUserBase * dcUserBase, bool param /*= true*/) {

	int ret = 0;
	int block = 0; // On defaults don't block
	mCurUser = dcUserBase;
	LuaInterpreter * script = NULL;

	if (!getListFlag(listFlag)) {
		vli.clear();
		listLuaInterpreter::iterator it, it_e = mLua.end();
		for (it = mLua.begin(); it != it_e; ++it) {
			script = *it;
			if (script && script->mL && script->testFunc(funcName)) {
				vli.push_back(script); // adding in cache
			}
		}
		addListFlag(listFlag);
	}

	// loop cache
	for (size_t i = 0; i < vli.size(); ++i) {
		script = vli[i];
		if (script && script->mL) {
			script->newCallParam(static_cast<void *> (dcUserBase), LUA_TLIGHTUSERDATA);
			if (param) {
				script->newCallParam(static_cast<void *> (const_cast<char *> (dcUserBase->getCommand())), LUA_TSTRING);
			}

			ret = script->callFunc(funcName);
			if (ret == 1) { // 1 - blocked
				mCurUser = NULL;
				return 1;
			} else if (ret && (!block || block > ret)) {
				block = ret;
			}
		}
	}

	mCurUser = NULL;
	return block;
}



// //////////////////////////////////////////Events///////////////////////////


/** Executed on each step of the timer of the server (100 msec) */
int LuaPlugin::onTimer() {
	mTasksList.checkTasks();

	LuaInterpreter * script = NULL;
	if (!getListFlag(LIST_TIMER)) {
		mTimer.clear();
		listLuaInterpreter::iterator it, it_e = mLua.end();
		for (it = mLua.begin(); it != it_e; ++it) {
			script = *it;
			if (script && script->mL && script->size()) {
				mTimer.push_back(script); // adding in cache
			}
		}
		addListFlag(LIST_TIMER);
	}

	// loop cache
	for (size_t i = 0; i < mTimer.size(); ++i) {
		script = mTimer[i];
		if (script && script->mL) {
			script->onTimer();
		}
	}
	return 1;
}



int LuaPlugin::onScriptAction(const char * scriptName, const char * action) {
	// no cache
	LuaInterpreter * script = NULL;
	listLuaInterpreter::iterator it, it_e = mLua.end();
	for (it = mLua.begin(); it != it_e; ++it) {
		script = *it;
		if (!script || !script->mL) {
			continue;
		} else if (scriptName) {
			script->newCallParam(static_cast<void *> (const_cast<char *> (scriptName)), LUA_TSTRING);
		}
		if (script->callFunc(action)) {
			return 0;
		}
	}
	return 1;
}



/** OnConfigChange event */
int LuaPlugin::onConfigChange(const char * name, const char * value) {
	LuaInterpreter * script = NULL;
	if (!getListFlag(LIST_CONFIG_CHANGE)) {
		mConfigChange.clear();
		listLuaInterpreter::iterator it, it_e = mLua.end();
		for (it = mLua.begin(); it != it_e; ++it) {
			script = *it;
			if (script && script->mL && script->testFunc("OnConfigChange")) {
				mConfigChange.push_back(script); // adding in cache
			}
		}
		addListFlag(LIST_CONFIG_CHANGE);
	}

	// loop cache
	for (size_t i = 0; i < mConfigChange.size(); ++i) {
		script = mConfigChange[i];
		if (script && script->mL) {
			script->newCallParam(static_cast<void *> (const_cast<char *> (name)), LUA_TSTRING);
			script->newCallParam(static_cast<void *> (const_cast<char *> (value)), LUA_TSTRING);
			script->callFunc("OnConfigChange");
		}
	}
	return 0;
}



/** onFlood event */
int LuaPlugin::onFlood(DcUserBase * dcUserBase, int type1, int type2) {
	int ret = 0, block = 0; // On defaults don't block
	LuaInterpreter * script = NULL;
	if (!getListFlag(LIST_FLOOD)) {
		mFlood.clear();
		listLuaInterpreter::iterator it, it_e = mLua.end();
		for (it = mLua.begin(); it != it_e; ++it) {
			script = *it;
			if (script && script->mL && script->testFunc("OnFlood")) {
				mFlood.push_back(script); // adding in cache
			}
		}
		addListFlag(LIST_FLOOD);
	}

	// loop cache
	for (size_t i = 0; i < mFlood.size(); ++i) {
		script = mFlood[i];
		if (script && script->mL) {
			script->newCallParam(static_cast<void *> (dcUserBase), LUA_TLIGHTUSERDATA);
			script->newCallParam(lua_Number(type1), LUA_TNUMBER);
			script->newCallParam(lua_Number(type2), LUA_TNUMBER);
			ret = script->callFunc("OnFlood");
			if (ret == 1) { // 1 - blocked
				return 1;
			}
			if (ret && (!block || block > ret)) {
				block = ret;
			}
		}
	}
	return block;
}



/// onWebData(WebID, sData)
int LuaPlugin::onWebData(WebUserBase * webUserBase) {
	int ret = 0, block = 0; // On defaults don't block
	LuaInterpreter * script = NULL;

	if (!getListFlag(LIST_WEB_DATA)) {
		mWebData.clear();
		listLuaInterpreter::iterator it, it_e = mLua.end();
		for (it = mLua.begin(); it != it_e; ++it) {
			script = *it;
			if (script && script->mL && script->testFunc("OnWebData")) {
				mWebData.push_back(script); // adding in cache
			}
		}
		addListFlag(LIST_WEB_DATA);
	}

	// loop cache
	for (size_t i = 0; i < mWebData.size(); ++i) {
		script = mWebData[i];
		if (script && script->mL) {
			script->newCallParam(static_cast<void *> (webUserBase), LUA_TLIGHTUSERDATA);
			script->newCallParam(static_cast<void *> (const_cast<char *> (webUserBase->getCommand())), LUA_TSTRING);
			ret = script->callFunc("OnWebData");
			if (ret == 1) {
				return 1; // 1 - blocked
			}
			if (ret && (!block || block > ret)) {
				block = ret;
			}
		}
	}
	return block;
}



/** OnScriptError event */
int LuaPlugin::onScriptError(LuaInterpreter * current, const char * scriptName, const char * errMsg, bool stoped) {
	LuaInterpreter * script = NULL;

	if (!getListFlag(LIST_SCRIPT_ERROR)) {
		mScriptError.clear();
		listLuaInterpreter::iterator it, it_e = mLua.end();
		for (it = mLua.begin(); it != it_e; ++it) {
			script = *it;
			if (script && script->mL && script->testFunc("OnScriptError")) {
				mScriptError.push_back(script); // adding in cache
			}
		}
		addListFlag(LIST_SCRIPT_ERROR);
	}

	// loop cache
	for (size_t i = 0; i < mScriptError.size(); ++i) {
		script = mScriptError[i];
		if (script && script->mL && script == current) {
			script->newCallParam(static_cast<void *> (const_cast<char *> (scriptName)), LUA_TSTRING);
			script->newCallParam(static_cast<void *> (const_cast<char *> (errMsg)), LUA_TSTRING);
			script->newCallParam(lua_Number(stoped), LUA_TBOOLEAN);
			if (script->callFunc("OnScriptError")) {
				return 0;
			}
		}
	}
	return 1;
}



/** onAny event */
int LuaPlugin::onAny(DcUserBase * dcUserBase, int type) {

	int ret = 0, block = 0; // On defaults don't block
	LuaInterpreter * script = NULL;
	if (!getListFlag(LIST_ANY)) {
		mAny.clear();
		listLuaInterpreter::iterator it, it_e = mLua.end();
		for (it = mLua.begin(); it != it_e; ++it) {
			script = *it;
			if (script && script->mL && script->testFunc("OnAny")) {
				mAny.push_back(script); // adding in cache
			}
		}
		addListFlag(LIST_ANY);
	}

	// loop cache
	for (size_t i = 0; i < mAny.size(); ++i) {
		script = mAny[i];
		if (script && script->mL) {
			script->newCallParam(static_cast<void *> (dcUserBase), LUA_TLIGHTUSERDATA);
			script->newCallParam(static_cast<void *> (const_cast<char *> (dcUserBase->getCommand())), LUA_TSTRING);
			script->newCallParam(lua_Number(type), LUA_TNUMBER);
			ret = script->callFunc("OnAny");
			if (ret == 1) {
				return 1; // 1 - blocked
			}
			if (ret && (!block || block > ret)) {
				block = ret;
			}
		}
	}
	return block;
}



// OnUserConnected(tUser)
int LuaPlugin::onUserConnected(DcUserBase * dcUserBase) {
		return callAll("OnUserConnected", LIST_USER_CONNECTED, mUserConnected, dcUserBase, false);
}

// OnUserDisconnected(tUser)
int LuaPlugin::onUserDisconnected(DcUserBase * dcUserBase) {
		return callAll("OnUserDisconnected", LIST_USER_DISCONNECTED, mUserDisconnected, dcUserBase, false);
}

// OnUserEnter(tUser)
int LuaPlugin::onUserEnter(DcUserBase * dcUserBase) {
		return callAll("OnUserEnter", LIST_USER_ENTER, mUserEnter, dcUserBase, false);
}

// OnUserExit(tUser)
int LuaPlugin::onUserExit(DcUserBase * dcUserBase) {
		return callAll("OnUserExit", LIST_USER_EXIT, mUserExit, dcUserBase, false);
}

// OnSupports(tUser, sData)
int LuaPlugin::onSupports(DcUserBase * dcUserBase) {
	return callAll("OnSupports", LIST_SUPPORTS, mSupports, dcUserBase, true);
}

// OnKey(tUser, sData)
int LuaPlugin::onKey(DcUserBase * dcUserBase) {
	return callAll("OnKey", LIST_KEY, mKey, dcUserBase, true);
}

// OnUnknown(tUser, sData)
int LuaPlugin::onUnknown(DcUserBase * dcUserBase) {
	return callAll("OnUnknown", LIST_UNKNOWN, mUnknown, dcUserBase, true);
}

// OnValidateNick(tUser, sData)
int LuaPlugin::onValidateNick(DcUserBase * dcUserBase) {
	return callAll("OnValidateNick", LIST_VALIDATE_NICK, mValidateNick, dcUserBase, true);
}

// OnMyPass(tUser, sData)
int LuaPlugin::onMyPass(DcUserBase * dcUserBase) {
	return callAll("OnMyPass", LIST_MY_PASS, mMyPass, dcUserBase, true);
}

// OnVersion(tUser, sData)
int LuaPlugin::onVersion(DcUserBase * dcUserBase) {
	return callAll("OnVersion", LIST_VERSION, mVersionList, dcUserBase, true);
}

// OnGetNickList(tUser, sData)
int LuaPlugin::onGetNickList(DcUserBase * dcUserBase) {
	return callAll("OnGetNickList", LIST_GET_NICK_LIST, mGetNickList, dcUserBase, true);
}

// OnMyINFO(tUser, sData)
int LuaPlugin::onMyINFO(DcUserBase * dcUserBase) {
	return callAll("OnMyINFO", LIST_MY_INFO, mMyINFO, dcUserBase, true);
}

// OnChat(tUser, sData)
int LuaPlugin::onChat(DcUserBase * dcUserBase) {
	return callAll("OnChat", LIST_CHAT, mChat, dcUserBase, true);
}

// OnTo(tUser, sData)
int LuaPlugin::onTo(DcUserBase * dcUserBase) {
	return callAll("OnTo", LIST_TO, mTo, dcUserBase, true);
}

// OnConnectToMe(tUser, sData)
int LuaPlugin::onConnectToMe(DcUserBase * dcUserBase) {
	return callAll("OnConnectToMe", LIST_CONNECT_TO_ME, mConnectToMe, dcUserBase, true);
}

// OnRevConnectToMe(tUser, sData)
int LuaPlugin::onRevConnectToMe(DcUserBase * dcUserBase) {
	return callAll("OnRevConnectToMe", LIST_REV_CONNECT_TO_ME, mRevConnectToMe, dcUserBase, true);
}

// OnSearch(tUser, sData)
int LuaPlugin::onSearch(DcUserBase * dcUserBase) {
	return callAll("OnSearch", LIST_SEARCH, mSearch, dcUserBase, true);
}

// OnSR(tUser, sData)
int LuaPlugin::onSR(DcUserBase * dcUserBase) {
	return callAll("OnSR", LIST_SR, mSR, dcUserBase, true);
}

// OnKick(tUser, sData)
int LuaPlugin::onKick(DcUserBase * dcUserBase) {
	return callAll("OnKick", LIST_KICK, mKick, dcUserBase, true);
}

// OnOpForceMove(tUser, sData)
int LuaPlugin::onOpForceMove(DcUserBase * dcUserBase) {
	return callAll("OnOpForceMove", LIST_OP_FORCE_MOVE, mOpForceMove, dcUserBase, true);
}

// OnGetINFO(tUser, sData)
int LuaPlugin::onGetINFO(DcUserBase * dcUserBase) {
	return callAll("OnGetINFO", LIST_GET_INFO, mGetINFO, dcUserBase, true);
}

// OnMCTo(tUser, sData)
int LuaPlugin::onMCTo(DcUserBase * dcUserBase) {
	return callAll("OnMCTo", LIST_MC_TO, mMCTo, dcUserBase, true);
}


REG_PLUGIN(LuaPlugin)


/**
 * $Id$
 * $HeadURL$
 */
