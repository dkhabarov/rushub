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


#include "LuaPlugin.h"
#include "Dir.h"

#include "tinyxml/tinyxml.h"
#ifdef _WIN32
	#pragma comment(lib, "tinyxml.lib")
#endif

using namespace ::utils;

DcServerBase * LuaPlugin::mCurServer = NULL;
LuaPlugin * LuaPlugin::mCurLua = NULL;
string LuaPlugin::mLuaCPath;
string LuaPlugin::mLuaPath;
bool LuaPlugin::mSetLuaPath = false;

LuaPlugin::LuaPlugin() : 
	mCurScript(NULL),
	mCurDCConn(NULL)
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

	string mainDir(dcServerBase->getMainDir());

	mScriptsDir = mainDir + "scripts/";
	string libs = mainDir + "libs/";
	Dir::checkPath(mScriptsDir);
	Dir::checkPath(libs);

	#ifdef _WIN32

		// replace slashes
		size_t pos = mainDir.find("/");
		while (pos != mainDir.npos) {
			mainDir.replace(pos, 1, LUA_DIRSEP);
			pos = mainDir.find("/", pos);
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
bool LuaPlugin::regAll(PluginListBase * pluginListBase) {
	pluginListBase->regCallList("Timer",       this);
	pluginListBase->regCallList("Conn",        this);
	pluginListBase->regCallList("Disconn",     this);
	pluginListBase->regCallList("Enter",       this);
	pluginListBase->regCallList("Exit",        this);
	pluginListBase->regCallList("Supports",    this);
	pluginListBase->regCallList("Key",         this);
	pluginListBase->regCallList("Validate",    this);
	pluginListBase->regCallList("MyPass",      this);
	pluginListBase->regCallList("Version",     this);
	pluginListBase->regCallList("GetNickList", this);
	pluginListBase->regCallList("MyINFO",      this);
	pluginListBase->regCallList("Chat",        this);
	pluginListBase->regCallList("To",          this);
	pluginListBase->regCallList("CTM",         this);
	pluginListBase->regCallList("RCTM",        this);
	pluginListBase->regCallList("Search",      this);
	pluginListBase->regCallList("SR",          this);
	pluginListBase->regCallList("Kick",        this);
	pluginListBase->regCallList("OpForce",     this);
	pluginListBase->regCallList("GetINFO",     this);
	pluginListBase->regCallList("MCTo",        this);
	pluginListBase->regCallList("Any",         this);
	pluginListBase->regCallList("Unknown",     this);
	pluginListBase->regCallList("Flood",       this);
	pluginListBase->regCallList("WebData",     this);
	return true;
}


////////////////////////////////////////////

LuaInterpreter * LuaPlugin::findScript(const string & scriptName) {
	LuaInterpreterList::iterator it;
	LuaInterpreter * script = NULL;
	for (it = mLua.begin(); it != mLua.end(); ++it) {
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
		mTasksList.addTask(NULL, TASKTYPE_SAVE);
		if (current) {
			script->mEnabled = false;
			mTasksList.addTask((void *) script, TASKTYPE_STOPSCRIPT);
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
				mTasksList.addTask((void *) script, TASKTYPE_RESTARTSCRIPT);
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
	LuaInterpreterList::iterator it, it_prev;
	LuaInterpreter * script = NULL;
	bool first = true;
	for (it = mLua.begin(); it != mLua.end(); ++it) {
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
	TiXmlDocument file((this->getPluginDir() + "scripts.xml").c_str());
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
					file.append(".lua");
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
	for (LuaInterpreterList::iterator it = mLua.begin(); it != mLua.end(); ++it) {
		if (*it != NULL) {
			delete *it;
			*it = NULL;
		}
	}
	mLua.clear();
	return 1;
}

void LuaPlugin::save() {
	TiXmlDocument file((this->getPluginDir() + "scripts.xml").c_str());
	file.InsertEndChild(TiXmlDeclaration("1.0", "windows-1251", "yes"));
	TiXmlElement mainItem("Scripts");
	mainItem.SetAttribute("Version", PLUGIN_VERSION);
	LuaInterpreterList::iterator it;
	LuaInterpreter * script = NULL;
	for (it = mLua.begin(); it != mLua.end(); ++it) {
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
	LuaInterpreterList::iterator it, it_prev = mLua.begin();
	for (it = ++mLua.begin(); it != mLua.end(); ++it) {
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
	LuaInterpreterList::iterator it;
	LuaInterpreter * tmp = NULL;
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


// ToDo: add cmd param
/** 1 - blocked */
int LuaPlugin::callAll(const char * funcName, DcUserBase * dcUserBase, bool param /*= true*/) {

	DcConnBase * dcConnBase = dcUserBase->mDcConnBase; // TODO refactoring
	int ret = 0;
	int block = 0; // On defaults don't block
	mCurDCConn = dcConnBase; // TODO refactoring

	LuaInterpreter * script = NULL;
	for (LuaInterpreterList::iterator it = mLua.begin(); it != mLua.end(); ++it) {

		script = *it;
		if (!script || !script->mL) {
			continue;
		}

		script->newCallParam((void *) dcConnBase, LUA_TLIGHTUSERDATA); // TODO refactoring

		// ToDo
		if (param) {
			script->newCallParam((void *) dcConnBase->getCommand(), LUA_TSTRING); // TODO refactoring
		}

		ret = script->callFunc(funcName);
		if (ret == 1) { // 1 - blocked
			mCurDCConn = NULL; // TODO refactoring
			return 1;
		} else if (ret && (!block || block > ret)) {
			block = ret;
		}
	}
	mCurDCConn = NULL; // TODO refactoring
	return block;
}

// //////////////////////////////////////////Events///////////////////////////

/** Executed on each step of the timer of the server (100 msec) */
int LuaPlugin::onTimer() {
	mTasksList.checkTasks();
	for (LuaInterpreterList::iterator it = mLua.begin(); it != mLua.end(); ++it) {
		if (*it) {
			(*it)->onTimer();
		}
	}
	return 1;
}

/** OnConfigChange event */
int LuaPlugin::onConfigChange(const char * name, const char * value) {
	LuaInterpreter * script = NULL;
	for (LuaInterpreterList::iterator it = mLua.begin(); it != mLua.end(); ++it) {
		script = *it;
		if (!script || !script->mL) {
			continue;
		}
		script->newCallParam((void *) name, LUA_TSTRING);
		script->newCallParam((void *) value, LUA_TSTRING);
		script->callFunc("OnConfigChange");
	}
	return 0;
}

/** onFlood event */
int LuaPlugin::onFlood(DcUserBase * dcUserBase, int type1, int type2) {
	DcConnBase * dcConnBase = dcUserBase->mDcConnBase; // TODO refactoring
	int ret = 0, block = 0; // On defaults don't block
	LuaInterpreter * script = NULL;
	for (LuaInterpreterList::iterator it = mLua.begin(); it != mLua.end(); ++it) {
		script = *it;
		if (!script || !script->mL) {
			continue;
		}
		script->newCallParam((void *) dcConnBase, LUA_TLIGHTUSERDATA); // TODO refactoring
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
	return block;
}

/// onWebData(WebID, sData)
int LuaPlugin::onWebData(WebUserBase * webUserBase) {
	int ret = 0, block = 0; // On defaults don't block
	LuaInterpreter * script = NULL;
	for (LuaInterpreterList::iterator it = mLua.begin(); it != mLua.end(); ++it) {
		script = *it;
		if (!script || !script->mL) {
			continue;
		}
		script->newCallParam((void *) webUserBase, LUA_TLIGHTUSERDATA);
		script->newCallParam((void *) webUserBase->getCommand(), LUA_TSTRING);
		ret = script->callFunc("OnWebData");
		if (ret == 1) {
			return 1; // 1 - blocked
		}
		if (ret && (!block || block > ret)) {
			block = ret;
		}
	}
	return block;
}

int LuaPlugin::onScriptAction(const char * scriptName, const char * action) {
	LuaInterpreterList::iterator it;
	LuaInterpreter * script = NULL;
	for (it = mLua.begin(); it != mLua.end(); ++it) {
		script = *it;
		if (!script || !script->mL) {
			continue;
		} else if (scriptName) {
			script->newCallParam((void *) scriptName, LUA_TSTRING);
		}
		if (script->callFunc(action)) {
			return 0;
		}
	}
	return 1;
}

/** OnScriptError event */
int LuaPlugin::onScriptError(LuaInterpreter * current, const char * scriptName, const char * errMsg, bool stoped) {
	LuaInterpreter * script = NULL;
	for (LuaInterpreterList::iterator it = mLua.begin(); it != mLua.end(); ++it) {
		script = *it;
		if (!script || !script->mL || script == current) {
			continue;
		}
		script->newCallParam((void *)scriptName, LUA_TSTRING);
		script->newCallParam((void *)errMsg, LUA_TSTRING);
		script->newCallParam(lua_Number(stoped), LUA_TBOOLEAN);
		if (script->callFunc("OnScriptError")) {
			return 0;
		}
	}
	return 1;
}

/** onAny event */
int LuaPlugin::onAny(DcUserBase * dcUserBase, int type) {

	DcConnBase * dcConnBase = dcUserBase->mDcConnBase; // TODO refactoring
	int ret = 0, block = 0; // On defaults don't block
	LuaInterpreter * script = NULL;
	for (LuaInterpreterList::iterator it = mLua.begin(); it != mLua.end(); ++it) {
		script = *it;
		if (!script || !script->mL) {
			continue;
		}
		script->newCallParam((void *) dcConnBase, LUA_TLIGHTUSERDATA); // TODO refactoring
		script->newCallParam((void *) dcConnBase->getCommand(), LUA_TSTRING); // TODO refactoring
		script->newCallParam(lua_Number(type), LUA_TNUMBER);
		ret = script->callFunc("OnAny");
		if (ret == 1) {
			return 1; // 1 - blocked
		}
		if (ret && (!block || block > ret)) {
			block = ret;
		}
	}
	return block;
}


// OnUserConnected(tUser)
int LuaPlugin::onUserConnected(DcUserBase * dcUserBase) {
		return callAll("OnUserConnected", dcUserBase, false);
}

// OnUserDisconnected(tUser)
int LuaPlugin::onUserDisconnected(DcUserBase * dcUserBase) {
		return callAll("OnUserDisconnected", dcUserBase, false);
}

// OnUserEnter(tUser)
int LuaPlugin::onUserEnter(DcUserBase * dcUserBase) {
		return callAll("OnUserEnter", dcUserBase, false);
}

// OnUserExit(tUser)
int LuaPlugin::onUserExit(DcUserBase * dcUserBase) {
		return callAll("OnUserExit", dcUserBase, false);
}

// OnSupports(tUser, sData)
int LuaPlugin::onSupports(DcUserBase * dcUserBase) {
	return callAll("OnSupports", dcUserBase, true);
}

// OnKey(tUser, sData)
int LuaPlugin::onKey(DcUserBase * dcUserBase) {
	return callAll("OnKey", dcUserBase, true);
}

// OnUnknown(tUser, sData)
int LuaPlugin::onUnknown(DcUserBase * dcUserBase) {
	return callAll("OnUnknown", dcUserBase, true);
}

// OnValidateNick(tUser, sData)
int LuaPlugin::onValidateNick(DcUserBase * dcUserBase) {
	return callAll("OnValidateNick", dcUserBase, true);
}

// OnMyPass(tUser, sData)
int LuaPlugin::onMyPass(DcUserBase * dcUserBase) {
	return callAll("OnMyPass", dcUserBase, true);
}

// OnVersion(tUser, sData)
int LuaPlugin::onVersion(DcUserBase * dcUserBase) {
	return callAll("OnVersion", dcUserBase, true);
}

// OnGetNickList(tUser, sData)
int LuaPlugin::onGetNickList(DcUserBase * dcUserBase) {
	return callAll("OnGetNickList", dcUserBase, true);
}

// OnMyINFO(tUser, sData)
int LuaPlugin::onMyINFO(DcUserBase * dcUserBase) {
	return callAll("OnMyINFO", dcUserBase, true);
}

// OnChat(tUser, sData)
int LuaPlugin::onChat(DcUserBase * dcUserBase) {
	return callAll("OnChat", dcUserBase, true);
}

// OnTo(tUser, sData)
int LuaPlugin::onTo(DcUserBase * dcUserBase) {
	return callAll("OnTo", dcUserBase, true);
}

// OnConnectToMe(tUser, sData)
int LuaPlugin::onConnectToMe(DcUserBase * dcUserBase) {
	return callAll("OnConnectToMe", dcUserBase, true);
}

// OnRevConnectToMe(tUser, sData)
int LuaPlugin::onRevConnectToMe(DcUserBase * dcUserBase) {
	return callAll("OnRevConnectToMe", dcUserBase, true);
}

// OnSearch(tUser, sData)
int LuaPlugin::onSearch(DcUserBase * dcUserBase) {
	return callAll("OnSearch", dcUserBase, true);
}

// OnSR(tUser, sData)
int LuaPlugin::onSR(DcUserBase * dcUserBase) {
	return callAll("OnSR", dcUserBase, true);
}

// OnKick(tUser, sData)
int LuaPlugin::onKick(DcUserBase * dcUserBase) {
	return callAll("OnKick", dcUserBase, true);
}

// OnOpForceMove(tUser, sData)
int LuaPlugin::onOpForceMove(DcUserBase * dcUserBase) {
	return callAll("OnOpForceMove", dcUserBase, true);
}

// OnGetINFO(tUser, sData)
int LuaPlugin::onGetINFO(DcUserBase * dcUserBase) {
	return callAll("OnGetINFO", dcUserBase, true);
}

// OnMCTo(tUser, sData)
int LuaPlugin::onMCTo(DcUserBase * dcUserBase) {
	return callAll("OnMCTo", dcUserBase, true);
}


REG_PLUGIN(LuaPlugin);


/**
 * $Id$
 * $HeadURL$
 */
