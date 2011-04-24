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


#include "LuaPlugin.h"
#include "Dir.h"

#include "tinyxml/tinyxml.h"
#ifdef _WIN32
	#pragma comment(lib, "tinyxml.lib")
#endif

using namespace ::utils;

DcServerBase * LuaPlugin::mCurServer = NULL;
LuaPlugin * LuaPlugin::mCurLua = NULL;
string LuaPlugin::msLuaCPath;
string LuaPlugin::msLuaPath;
bool LuaPlugin::mbSetLuaPath = false;

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
	this->clear();
	mCurServer = NULL;
	mCurLua = NULL;
}

/** Actions when loading plugin */
void LuaPlugin::onLoad(DcServerBase * dcServerBase) {
	//setlocale(LC_ALL, dcServerBase->getLocale().c_str());
	mCurServer = dcServerBase;

	string mainDir(dcServerBase->getMainDir());

	msScriptsDir = mainDir + "scripts/";
	string libs = mainDir + "libs/";
	Dir::checkPath(msScriptsDir);
	Dir::checkPath(libs);

	#ifdef _WIN32

		// replace slashes
		size_t pos = mainDir.find("/");
		while (pos != mainDir.npos) {
			mainDir.replace(pos, 1, LUA_DIRSEP);
			pos = mainDir.find("/", pos);
		}

		msLuaCPath = mainDir + "libs" LUA_DIRSEP "?.dll;" +
			mainDir + "?.dll;" +
			mainDir + "scripts" LUA_DIRSEP "?.dll;" +
			mainDir + "scripts" LUA_DIRSEP "libs" LUA_DIRSEP "?.dll;";

	#else

		msLuaCPath = mainDir + "libs" LUA_DIRSEP "?.so;" +
			mainDir + "?.so;" +
			mainDir + "scripts" LUA_DIRSEP "?.so;" +
			mainDir + "scripts" LUA_DIRSEP "libs" LUA_DIRSEP "?.so;";

	#endif

	msLuaPath = mainDir + "libs" LUA_DIRSEP "?.lua;" +
		mainDir + "libs" LUA_DIRSEP "?" LUA_DIRSEP "init.lua;" +
		mainDir + "?.lua;" +
		mainDir + "?" LUA_DIRSEP "init.lua;" +
		mainDir + "scripts" LUA_DIRSEP "?.lua;" +
		mainDir + "scripts" LUA_DIRSEP "?" LUA_DIRSEP "init.lua;" +
		mainDir + "scripts" LUA_DIRSEP "libs" LUA_DIRSEP "?.lua;" +
		mainDir + "scripts" LUA_DIRSEP "libs" LUA_DIRSEP "?" LUA_DIRSEP "init.lua;";

	mbSetLuaPath = false;

	LoadScripts();
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

LuaInterpreter * LuaPlugin::FindScript(const string & sScriptName) {
	tvLuaInterpreter::iterator it;
	LuaInterpreter * Script;
	for (it = mLua.begin(); it != mLua.end(); ++it) {
		Script = *it;
		if (Script->mName == sScriptName) {
			return Script;
		}
	}
	return NULL;
}

int LuaPlugin::CheckExists(LuaInterpreter * Script) {
	if (!Script) {
		return 0;
	} else if (!Dir::isFileExist((msScriptsDir + Script->mName).c_str())) {
		Script->mbEnabled = false;
		return 0;
	}
	return 1;
}

/** Creation and addition script in list 
	@param sScriptName - script name
	@param bOnlyNew - return new scripts only
	@return NULL - not exist or (bOnlyNew && old script)
*/
LuaInterpreter * LuaPlugin::AddScript(const string & sScriptName, bool bOnlyNew) {
	LuaInterpreter * Script = FindScript(sScriptName);
	if (!Script && Dir::isFileExist((msScriptsDir + sScriptName).c_str())) {
		Script = new LuaInterpreter(sScriptName, msScriptsDir);
		mLua.push_back(Script);
		return Script;
	}
	return (bOnlyNew == true) ? NULL : Script;
}

/** Starting script by name
	@return -1 - already, was loaded earlier
	@return 0 - loaded
	@return LUA_ERRFILE (6) - script was not found
	@return LUA_ERRSYNTAX (3) - error in the script
	@return LUA_ERRMEM (4) - memory error
*/
int LuaPlugin::StartScript(const string & sScriptName) {
	return StartScript(AddScript(sScriptName));
}

/** Attempt to start script
	@return -1 - already, was loaded earlier
	@return 0 - loaded
	@return LUA_ERRFILE (6) - script was not found
	@return LUA_ERRSYNTAX (3) - error in the script
	@return LUA_ERRMEM (4) - memory error
*/
int LuaPlugin::StartScript(LuaInterpreter * Script) {
	if (!Script) {
		return LUA_ERRFILE;
	}
	mTasksList.AddTask(NULL, eT_Save);
	int iRet = Script->start();
	return iRet;
}

/** Stoping script
	@return -1 - already, was stoped earlier
	@return 0 - stoped,
	@return LUA_ERRFILE - script was not found
*/
int LuaPlugin::StopScript(LuaInterpreter * Script, bool bCurrent) {
	int iRet = LUA_ERRFILE;
	if (Script) {
		mTasksList.AddTask(NULL, eT_Save);
		if (bCurrent) {
			Script->mbEnabled = false;
			mTasksList.AddTask((void *) Script, eT_StopScript);
			if (!CheckExists(Script)) { // Removing from list in the case of absence
				mLua.remove(Script);
				iRet = LUA_ERRFILE;
			} else {
				iRet = 0;
			}
		} else {
			iRet = -1;
			if (Script->Stop()) {
				Script->DelTmr();
				iRet = 0;
			}
			if (!CheckExists(Script)) {
				mLua.remove(Script);
				delete Script;
				Script = NULL;
				iRet = LUA_ERRFILE;
			}
		}
	}
	return iRet;
}

/** Restarting script
	@return 0 - restarted
	@return LUA_ERRFILE - script was not found
	@return LUA_ERRSYNTAX - error in the script
*/
int LuaPlugin::RestartScript(LuaInterpreter * Script, bool bCurrent) {
	if (Script) {
		mTasksList.AddTask(NULL, eT_Save);
		if (bCurrent) {
			int st = luaL_loadfile(Script->mL, (msScriptsDir + Script->mName).c_str());
			if (!st) {
				mTasksList.AddTask((void *) Script, eT_RestartScript);
			} else {
				StopScript(Script, true);
			}
			return st;
		} else {
			int st = StopScript(Script);
			if (st == 0 || st == -1) {
				return StartScript(Script);
			}
		}
	}
	return LUA_ERRFILE;
}

/** Restarting all scripts */
int LuaPlugin::RestartScripts(LuaInterpreter * CurScript, int iType) {
	tvLuaInterpreter::iterator it, it_prev;
	LuaInterpreter * Script;
	bool bFirst = true;
	for (it = mLua.begin(); it != mLua.end(); ++it) {
		Script = *it;
		if (((iType == 0 || (iType == 2 && Script != CurScript)) && 
			Script->mbEnabled && RestartScript(Script, Script == CurScript) == LUA_ERRFILE) ||
			(!Script->mbEnabled && StopScript(Script, Script == CurScript) == LUA_ERRFILE))
		{
			if (bFirst) {
				bFirst = false;
				it = mLua.begin();
			} else {
				it = it_prev;
			}
		}
		it_prev = it;
	}
	mTasksList.AddTask(NULL, eT_Save);
	CheckNewScripts();
	return 1;
}

/** Loading all scripts */
int LuaPlugin::LoadScripts() {
	TiXmlDocument file((this->getPluginDir() + "scripts.xml").c_str());
	if (file.LoadFile()) {
		TiXmlHandle MainHandle(&file);
		TiXmlElement *MainItem = MainHandle.FirstChild("Scripts").Element();
		if (MainItem != NULL) {
			char *sName, *sEnabled;
			TiXmlNode *Value = NULL;
			while ((Value = MainItem->IterateChildren(Value)) != NULL) {
				if (Value->ToElement() == NULL || 
					(sName = (char *)Value->ToElement()->Attribute("Name")) == NULL || 
					(sEnabled = (char *)Value->ToElement()->Attribute("Enabled")) == NULL
				) {
					continue;
				}
				string sFile(sName);
				if ((sFile.size() <= 4) || (0 != sFile.compare(sFile.size() - 4, 4, ".lua"))) {
					sFile.append(".lua");
				}
				LuaInterpreter * Script = AddScript(sFile, true);
				if (strcmp(sEnabled, "false") != 0 && strcmp(sEnabled, "0") != 0) {
					StartScript(Script);
				}
			}
		}
	}
	CheckNewScripts();
	return 1;
}

void LuaPlugin::CheckNewScripts() {

	Dir::checkPath(msScriptsDir);

	DIR * dir = opendir(msScriptsDir.c_str());
	if (!dir) {
		return;
	}

	struct dirent * entry = NULL;
	string file;

	while (NULL != (entry = readdir(dir))) {
		file = entry->d_name;
		if ((file.size() > 4) && (0 == file.compare(file.size() - 4, 4, ".lua"))) {
			AddScript(file, true);
		}
	}

	closedir(dir);
}

/** Stoping all scripts */
int LuaPlugin::clear() {
	for (tvLuaInterpreter::iterator it = mLua.begin(); it != mLua.end(); ++it) {
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
	TiXmlElement MainItem("Scripts");
	MainItem.SetAttribute("Version", PLUGIN_VERSION);
	tvLuaInterpreter::iterator it;
	LuaInterpreter * Script;
	for (it = mLua.begin(); it != mLua.end(); ++it) {
		Script = *it;
		TiXmlElement Item("Script");
		Item.SetAttribute("Name", Script->mName.c_str());
		Item.SetAttribute("Enabled", Script->mL == NULL ? 0 : 1);
		MainItem.InsertEndChild(Item);
	}
	file.InsertEndChild(MainItem);
	file.SaveFile();
}

int LuaPlugin::MoveUp(LuaInterpreter * Script) {
	tvLuaInterpreter::iterator it, it_prev = mLua.begin();
	for (it = ++mLua.begin(); it != mLua.end(); ++it) {
		if (*it == Script) {
			mLua.erase(it);
			mLua.insert(it_prev, Script);
			return 1;
		}
		it_prev = it;
	}
	return 0;
}

int LuaPlugin::MoveDown(LuaInterpreter * Script) {
	tvLuaInterpreter::iterator it;
	LuaInterpreter * tmp;
	for (it = mLua.begin(); it != --mLua.end(); ++it) {
		if (*it == Script) {
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
int LuaPlugin::callAll(const char* sFuncName, DcConnBase * dcConnBase, bool param /*= true*/) {
	int iRet = 0;
	int iBlock = 0; // On defaults don't block
	mCurDCConn = dcConnBase;

	LuaInterpreter * Script = NULL;
	for (tvLuaInterpreter::iterator it = mLua.begin(); it != mLua.end(); ++it) {

		Script = *it;
		if (!Script->mL) {
			continue;
		}

		Script->NewCallParam((void *)dcConnBase, LUA_TLIGHTUSERDATA);

		// ToDo
		if (param) {
			Script->NewCallParam((void *)dcConnBase->getCommand(), LUA_TSTRING);
		}

		iRet = Script->CallFunc(sFuncName);
		if (iRet == 1) { // 1 - blocked
			mCurDCConn = NULL;
			return 1;
		} else if (iRet && (!iBlock || iBlock > iRet)) {
			iBlock = iRet;
		}
	}
	mCurDCConn = NULL;
	return iBlock;
}

// //////////////////////////////////////////Events///////////////////////////

/** Executed on each step of the timer of the server (100 msec) */
int LuaPlugin::onTimer() {
	mTasksList.CheckTasks();
	for (tvLuaInterpreter::iterator it = mLua.begin(); it != mLua.end(); ++it) {
		(*it)->onTimer();
	}
	return 1;
}

/** OnConfigChange event */
int LuaPlugin::OnConfigChange(const char * sName, const char * sValue) {
	LuaInterpreter * Script;
	for (tvLuaInterpreter::iterator it = mLua.begin(); it != mLua.end(); ++it) {
		Script = *it;
		if (!Script->mL) {
			continue;
		}
		Script->NewCallParam((void *)sName, LUA_TSTRING);
		Script->NewCallParam((void *)sValue, LUA_TSTRING);
		Script->CallFunc("OnConfigChange");
	}
	return 0;
}

/** onFlood event */
int LuaPlugin::onFlood(DcConnBase * dcConnBase, int iType1, int iType2) {
	if (dcConnBase != NULL) {
		int iRet = 0, iBlock = 0; // On defaults don't block
		LuaInterpreter * Script;
		for (tvLuaInterpreter::iterator it = mLua.begin(); it != mLua.end(); ++it) {
			Script = *it;
			if (!Script->mL) {
				continue;
			}
			Script->NewCallParam((void *)dcConnBase, LUA_TLIGHTUSERDATA);
			Script->NewCallParam(lua_Number(iType1), LUA_TNUMBER);
			Script->NewCallParam(lua_Number(iType2), LUA_TNUMBER);
			iRet = Script->CallFunc("OnFlood");
			if (iRet == 1) { // 1 - blocked
				return 1;
			}
			if (iRet && (!iBlock || iBlock > iRet)) {
				iBlock = iRet;
			}
		}
		return iBlock;
	}
	LogError("Error in LuaPlugin::onFlood");
	return 1;
}

/// onWebData(WebID, sData)
int LuaPlugin::onWebData(DcConnBase * dcConnBase, WebParserBase * webParserBase) {
	int iRet = 0, iBlock = 0; // On defaults don't block
	LuaInterpreter * Script;
	for (tvLuaInterpreter::iterator it = mLua.begin(); it != mLua.end(); ++it) {
		Script = *it;
		if (!Script->mL) {
			continue;
		}
		Script->NewCallParam((void *)dcConnBase, LUA_TLIGHTUSERDATA);
		Script->NewCallParam((void *)webParserBase->mParseString.c_str(), LUA_TSTRING);
		iRet = Script->CallFunc("OnWebData");
		if (iRet == 1) {
			return 1; // 1 - blocked
		}
		if (iRet && (!iBlock || iBlock > iRet)) {
			iBlock = iRet;
		}
	}
	return iBlock;
}

int LuaPlugin::OnScriptAction(const char * sScriptName, const char * sAction) {
	tvLuaInterpreter::iterator it;
	LuaInterpreter * Script;
	for (it = mLua.begin(); it != mLua.end(); ++it) {
		Script = *it;
		if (!Script->mL) {
			continue;
		} else if (sScriptName) {
			Script->NewCallParam((void *)sScriptName, LUA_TSTRING);
		}
		if (Script->CallFunc(sAction)) {
			return 0;
		}
	}
	return 1;
}

/** OnScriptError event */
int LuaPlugin::OnScriptError(LuaInterpreter * Current, const char * sScriptName, const char * sErrMsg, bool bStoped) {
	LuaInterpreter * Script = NULL;
	for (tvLuaInterpreter::iterator it = mLua.begin(); it != mLua.end(); ++it) {
		Script = *it;
		if (!Script->mL || Script == Current) {
			continue;
		}
		Script->NewCallParam((void *)sScriptName, LUA_TSTRING);
		Script->NewCallParam((void *)sErrMsg, LUA_TSTRING);
		Script->NewCallParam(lua_Number(bStoped), LUA_TBOOLEAN);
		if (Script->CallFunc("OnScriptError")) {
			return 0;
		}
	}
	return 1;
}

/** onAny event */
int LuaPlugin::onAny(DcConnBase * dcConnBase, int type) {

	int iRet = 0, iBlock = 0; // On defaults don't block
	LuaInterpreter * Script = NULL;
	for (tvLuaInterpreter::iterator it = mLua.begin(); it != mLua.end(); ++it) {
		Script = *it;
		if (!Script->mL) {
			continue;
		}
		Script->NewCallParam((void *)dcConnBase, LUA_TLIGHTUSERDATA);
		Script->NewCallParam((void *)dcConnBase->getCommand(), LUA_TSTRING);
		Script->NewCallParam(lua_Number(type), LUA_TNUMBER);
		iRet = Script->CallFunc("OnAny");
		if (iRet == 1) {
			return 1; // 1 - blocked
		}
		if (iRet && (!iBlock || iBlock > iRet)) {
			iBlock = iRet;
		}
	}
	return iBlock;
}


// OnUserConnected(tUser)
int LuaPlugin::onUserConnected(DcConnBase * dcConnBase) {
		return callAll("OnUserConnected", dcConnBase, false);
}

// OnUserDisconnected(tUser)
int LuaPlugin::onUserDisconnected(DcConnBase * dcConnBase) {
		return callAll("OnUserDisconnected", dcConnBase, false);
}

// OnUserEnter(tUser)
int LuaPlugin::onUserEnter(DcConnBase * dcConnBase) {
		return callAll("OnUserEnter", dcConnBase, false);
}

// OnUserExit(tUser)
int LuaPlugin::onUserExit(DcConnBase * dcConnBase) {
		return callAll("OnUserExit", dcConnBase, false);
}

// OnSupports(tUser, sData)
int LuaPlugin::onSupports(DcConnBase * dcConnBase) {
	return callAll("OnSupports", dcConnBase, true);
}

// OnKey(tUser, sData)
int LuaPlugin::onKey(DcConnBase * dcConnBase) {
	return callAll("OnKey", dcConnBase, true);
}

// OnUnknown(tUser, sData)
int LuaPlugin::onUnknown(DcConnBase * dcConnBase) {
	return callAll("OnUnknown", dcConnBase, true);
}

// OnValidateNick(tUser, sData)
int LuaPlugin::onValidateNick(DcConnBase * dcConnBase) {
	return callAll("OnValidateNick", dcConnBase, true);
}

// OnMyPass(tUser, sData)
int LuaPlugin::onMyPass(DcConnBase * dcConnBase) {
	return callAll("OnMyPass", dcConnBase, true);
}

// OnVersion(tUser, sData)
int LuaPlugin::onVersion(DcConnBase * dcConnBase) {
	return callAll("OnVersion", dcConnBase, true);
}

// OnGetNickList(tUser, sData)
int LuaPlugin::onGetNickList(DcConnBase * dcConnBase) {
	return callAll("OnGetNickList", dcConnBase, true);
}

// OnMyINFO(tUser, sData)
int LuaPlugin::onMyINFO(DcConnBase * dcConnBase) {
	return callAll("OnMyINFO", dcConnBase, true);
}

// OnChat(tUser, sData)
int LuaPlugin::onChat(DcConnBase * dcConnBase) {
	return callAll("OnChat", dcConnBase, true);
}

// OnTo(tUser, sData)
int LuaPlugin::onTo(DcConnBase * dcConnBase) {
	return callAll("OnTo", dcConnBase, true);
}

// OnConnectToMe(tUser, sData)
int LuaPlugin::onConnectToMe(DcConnBase * dcConnBase) {
	return callAll("OnConnectToMe", dcConnBase, true);
}

// OnRevConnectToMe(tUser, sData)
int LuaPlugin::onRevConnectToMe(DcConnBase * dcConnBase) {
	return callAll("OnRevConnectToMe", dcConnBase, true);
}

// OnSearch(tUser, sData)
int LuaPlugin::onSearch(DcConnBase * dcConnBase) {
	return callAll("OnSearch", dcConnBase, true);
}

// OnSR(tUser, sData)
int LuaPlugin::onSR(DcConnBase * dcConnBase) {
	return callAll("OnSR", dcConnBase, true);
}

// OnKick(tUser, sData)
int LuaPlugin::onKick(DcConnBase * dcConnBase) {
	return callAll("OnKick", dcConnBase, true);
}

// OnOpForceMove(tUser, sData)
int LuaPlugin::onOpForceMove(DcConnBase * dcConnBase) {
	return callAll("OnOpForceMove", dcConnBase, true);
}

// OnGetINFO(tUser, sData)
int LuaPlugin::onGetINFO(DcConnBase * dcConnBase) {
	return callAll("OnGetINFO", dcConnBase, true);
}

// OnMCTo(tUser, sData)
int LuaPlugin::onMCTo(DcConnBase * dcConnBase) {
	return callAll("OnMCTo", dcConnBase, true);
}


REG_PLUGIN(LuaPlugin);
