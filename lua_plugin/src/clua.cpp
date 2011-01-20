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

#include "clua.h"
#include "cdir.h"

#include "tinyxml/tinyxml.h"
#ifdef _WIN32
	#pragma comment(lib, "tinyxml.lib")
#endif

cDCServerBase * cLua::mCurServer = NULL;
cLua * cLua::mCurLua = NULL;
string cLua::msLuaCPath;
string cLua::msLuaPath;
bool cLua::mbSetLuaPath = false;

cLua::cLua() : 
	mCurScript(NULL),
	mCurDCParser(NULL),
	mCurDCConn(NULL)
{
	mCurLua = this;
	msName = PLUGIN_NAME;
	msVersion = PLUGIN_VERSION;
}

cLua::~cLua() {
	Save();
	this->Clear();
	mCurServer = NULL;
	mCurLua = NULL;
}

/** Actions when loading plugin */
void cLua::onLoad(cDCServerBase *DCServer) {
	//setlocale(LC_ALL, DCServer->GetLocale().c_str());
	mCurServer = DCServer;
	cPlugin::onLoad(DCServer);

	string sMainDir(DCServer->GetMainDir());

	msScriptsDir = sMainDir + "scripts/";
	string sLibs = sMainDir + "libs/";
	if(!DirExists(msScriptsDir.c_str())) mkDir(msScriptsDir.c_str());
	if(!DirExists(sLibs.c_str())) mkDir(sLibs.c_str());

	#ifdef _WIN32

		// replace slashes
		size_t iPos = sMainDir.find("/");
		while(iPos != sMainDir.npos) {
			sMainDir.replace(iPos, 1, LUA_DIRSEP);
			iPos = sMainDir.find("/", iPos);
		}

		msLuaCPath = sMainDir + "libs" LUA_DIRSEP "?.dll;" +
			sMainDir + "?.dll;" +
			sMainDir + "scripts" LUA_DIRSEP "?.dll;" +
			sMainDir + "plugins" LUA_DIRSEP "?.dll;" +
			sMainDir + "scripts" LUA_DIRSEP "libs" LUA_DIRSEP "?.dll;";

	#else

		msLuaCPath = sMainDir + "libs" LUA_DIRSEP "?.so;" +
			sMainDir + "?.so;" +
			sMainDir + "scripts" LUA_DIRSEP "?.so;" +
			sMainDir + "plugins" LUA_DIRSEP "?.so;" +
			sMainDir + "scripts" LUA_DIRSEP "libs" LUA_DIRSEP "?.so;";

	#endif

	msLuaPath = sMainDir + "libs" LUA_DIRSEP "?.lua;" +
		sMainDir + "libs" LUA_DIRSEP "?" LUA_DIRSEP "init.lua;" +
		sMainDir + "?.lua;" +
		sMainDir + "?" LUA_DIRSEP "init.lua;" +
		sMainDir + "scripts" LUA_DIRSEP "?.lua;" +
		sMainDir + "scripts" LUA_DIRSEP "?" LUA_DIRSEP "init.lua;" +
		sMainDir + "plugins" LUA_DIRSEP "?.lua;" +
		sMainDir + "plugins" LUA_DIRSEP "?" LUA_DIRSEP "init.lua;" +
		sMainDir + "scripts" LUA_DIRSEP "libs" LUA_DIRSEP "?.lua;" +
		sMainDir + "scripts" LUA_DIRSEP "libs" LUA_DIRSEP "?" LUA_DIRSEP "init.lua;";

	mbSetLuaPath = false;

	LoadScripts();
	Save();
}

/** Registration all events */
bool cLua::RegAll(cPluginListBase* PluginList) {
	PluginList->RegCallList("Timer", this);
	PluginList->RegCallList("Conn", this);
	PluginList->RegCallList("Disconn", this);
	PluginList->RegCallList("Enter", this);
	PluginList->RegCallList("Exit", this);
	PluginList->RegCallList("Supports", this);
	PluginList->RegCallList("Key", this);
	PluginList->RegCallList("Validate", this);
	PluginList->RegCallList("MyPass", this);
	PluginList->RegCallList("Version", this);
	PluginList->RegCallList("GetNickList", this);
	PluginList->RegCallList("MyINFO", this);
	PluginList->RegCallList("Chat", this);
	PluginList->RegCallList("To", this);
	PluginList->RegCallList("CTM", this);
	PluginList->RegCallList("RCTM", this);
	PluginList->RegCallList("Search", this);
	PluginList->RegCallList("SR", this);
	PluginList->RegCallList("Kick", this);
	PluginList->RegCallList("OpForce", this);
	PluginList->RegCallList("GetINFO", this);
	PluginList->RegCallList("MCTo", this);
	PluginList->RegCallList("Any", this);
	PluginList->RegCallList("Unknown", this);
	PluginList->RegCallList("Flood", this);
	PluginList->RegCallList("WebData", this);

	return true;
}


////////////////////////////////////////////

cLuaInterpreter * cLua::FindScript(const string & sScriptName) {
	tvLuaInterpreter::iterator it;
	cLuaInterpreter * Script;
	for(it = mLua.begin(); it != mLua.end(); ++it) {
		Script = *it;
		if(Script->msName == sScriptName)
			return Script;
	}
	return NULL;
}

int cLua::CheckExists(cLuaInterpreter * Script) {
	if(!Script) return 0;
	if(!FileExists((msScriptsDir + Script->msName).c_str())) {
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
cLuaInterpreter * cLua::AddScript(const string & sScriptName, bool bOnlyNew) {
	cLuaInterpreter * Script = FindScript(sScriptName);
	if(!Script && FileExists((msScriptsDir + sScriptName).c_str())) {
		Script = new cLuaInterpreter(sScriptName, msScriptsDir);
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
int cLua::StartScript(const string & sScriptName) {
	return StartScript(AddScript(sScriptName));
}

/** Attempt to start script
	@return -1 - already, was loaded earlier
	@return 0 - loaded
	@return LUA_ERRFILE (6) - script was not found
	@return LUA_ERRSYNTAX (3) - error in the script
	@return LUA_ERRMEM (4) - memory error
*/
int cLua::StartScript(cLuaInterpreter * Script) {
	if(!Script) return LUA_ERRFILE;
	mTasksList.AddTask(NULL, eT_Save);
	int iRet = Script->Start();
	return iRet;
}

/** Stoping script
	@return -1 - already, was stoped earlier
	@return 0 - stoped,
	@return LUA_ERRFILE - script was not found
*/
int cLua::StopScript(cLuaInterpreter * Script, bool bCurrent) {
	int iRet = LUA_ERRFILE;
	if(Script) {
		mTasksList.AddTask(NULL, eT_Save);
		if(bCurrent) {
			Script->mbEnabled = false;
			mTasksList.AddTask((void*)Script, eT_StopScript);
			if(!CheckExists(Script)) { // Removing from list in the case of absence
				mLua.remove(Script);
				iRet = LUA_ERRFILE;
			} else
				iRet = 0;
		} else {
			iRet = -1;
			if(Script->Stop()) {
				Script->DelTmr();
				iRet = 0;
			}
			if(!CheckExists(Script)) {
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
int cLua::RestartScript(cLuaInterpreter * Script, bool bCurrent) {
	if(Script) {
		mTasksList.AddTask(NULL, eT_Save);
		if(bCurrent) {
			int st = luaL_loadfile(Script->mL, (msScriptsDir + Script->msName).c_str());
			if(!st)
				mTasksList.AddTask((void*)Script, eT_RestartScript);
			else
				StopScript(Script, true);
			return st;
		} else {
			int st = StopScript(Script);
			if(st == 0 || st == -1)
				return StartScript(Script);
		}
	}
	return LUA_ERRFILE;
}

/** Restarting all scripts */
int cLua::RestartScripts(cLuaInterpreter * CurScript, int iType) {
	tvLuaInterpreter::iterator it, it_prev;
	cLuaInterpreter * Script;
	bool bFirst = true;
	for(it = mLua.begin(); it != mLua.end(); ++it) {
		Script = *it;
		if(((iType == 0 || (iType == 2 && Script != CurScript)) && Script->mbEnabled && RestartScript(Script, Script == CurScript) == LUA_ERRFILE) ||
			(!Script->mbEnabled && StopScript(Script, Script == CurScript) == LUA_ERRFILE)) {
			if(bFirst) {
				bFirst = false;
				it = mLua.begin();
			}
			else it = it_prev;
		}
		it_prev = it;
	}
	mTasksList.AddTask(NULL, eT_Save);
	CheckNewScripts();
	return 1;
}

/** Loading all scripts */
int cLua::LoadScripts() {
	TiXmlDocument file((this->GetPluginDir() + "scripts.xml").c_str());
	if(file.LoadFile()) {
		TiXmlHandle MainHandle(&file);
		TiXmlElement *MainItem = MainHandle.FirstChild("Scripts").Element();
		if(MainItem != NULL) {
			char *sName, *sEnabled;
			TiXmlNode *Value = NULL;
			while((Value = MainItem->IterateChildren(Value)) != NULL) {
				if(Value->ToElement() == NULL || 
					(sName = (char *)Value->ToElement()->Attribute("Name")) == NULL || 
					(sEnabled = (char *)Value->ToElement()->Attribute("Enabled")) == NULL) {
					continue;
				}
				string sFile(sName);
				if((sFile.size() <= 4) || (0 != sFile.compare(sFile.size() - 4, 4, ".lua")))
					sFile.append(".lua");
				cLuaInterpreter * Script = AddScript(sFile, true);
				if(strcmp(sEnabled, "false") != 0 && strcmp(sEnabled, "0") != 0)
					StartScript(Script);
			}
		}
	}
	CheckNewScripts();
	return 1;
}

void cLua::CheckNewScripts() {
	const char * sScriptsDir = msScriptsDir.c_str();
	if(!DirExists(sScriptsDir)) mkDir(sScriptsDir);
	DIR *dir = opendir(sScriptsDir);
	if(!dir) return;

	struct dirent * entry = NULL;
	string sFile;

	while(NULL != (entry = readdir(dir))) {
		sFile = entry->d_name;
		if((sFile.size() > 4) && (0 == sFile.compare(sFile.size() - 4, 4, ".lua")))
			AddScript(sFile, true);
			//StartScript(AddScript(sFile, true));
	}
	closedir(dir);
}

/** Stoping all scripts */
int cLua::Clear() {
	tvLuaInterpreter::iterator it;
	for(it = mLua.begin(); it != mLua.end(); ++it) {
		if(*it != NULL) delete *it;
		*it = NULL;
	}
	mLua.clear();
	return 1;
}

void cLua::Save() {
	TiXmlDocument file((this->GetPluginDir() + "scripts.xml").c_str());
	file.InsertEndChild(TiXmlDeclaration("1.0", "windows-1251", "yes"));
	TiXmlElement MainItem("Scripts");
	MainItem.SetAttribute("Version", PLUGIN_VERSION);
	tvLuaInterpreter::iterator it;
	cLuaInterpreter * Script;
	for(it = mLua.begin(); it != mLua.end(); ++it) {
		Script = *it;
		TiXmlElement Item("Script");
		Item.SetAttribute("Name", Script->msName.c_str());
		Item.SetAttribute("Enabled", Script->mL == NULL ? 0 : 1);
		MainItem.InsertEndChild(Item);
	}
	file.InsertEndChild(MainItem);
	file.SaveFile();
}

int cLua::MoveUp(cLuaInterpreter * Script) {
	tvLuaInterpreter::iterator it, it_prev = mLua.begin();
	for(it = ++mLua.begin(); it != mLua.end(); ++it) {
		if(*it == Script) {
			mLua.erase(it);
			mLua.insert(it_prev, Script);
			return 1;
		}
		it_prev = it;
	}
	return 0;
}

int cLua::MoveDown(cLuaInterpreter * Script) {
	tvLuaInterpreter::iterator it;
	cLuaInterpreter * tmp;
	for(it = mLua.begin(); it != --mLua.end(); ++it) {
		if(*it == Script) {
			++it;
			tmp = *it;
			mLua.insert(--it, tmp);
			mLua.erase(++it);
			return 1;
		}
	}
	return 0;
}



/** 1 - blocked */
int cLua::CallAll(const char* sFuncName, cDCConnBase * conn, cDCParserBase * DCParser) {
	int iRet = 0;
	int iBlock = 0; // On defaults don't block
	if(DCParser) mCurDCParser = DCParser;
	else mCurDCParser = NULL;
	mCurDCConn = conn;

	cLuaInterpreter * Script;
	for(tvLuaInterpreter::iterator it = mLua.begin(); it != mLua.end(); ++it) {

		Script = *it;
		if(!Script->mL) continue;

		Script->NewCallParam((void *)conn, LUA_TLIGHTUSERDATA);
		if(mCurDCParser) Script->NewCallParam((void *)mCurDCParser->msStr.c_str(), LUA_TSTRING);

		iRet = Script->CallFunc(sFuncName);
		if(iRet == 1) { // 1 - blocked
			mCurDCParser = NULL;
			mCurDCConn = NULL;
			return 1;
		}
		if(iRet && (!iBlock || iBlock > iRet))
			iBlock = iRet;
	}
	mCurDCParser = NULL;
	mCurDCConn = NULL;
	return iBlock;
}

// //////////////////////////////////////////Events///////////////////////////

/** Executed on each step of the timer of the server (100 msec) */
int cLua::onTimer() {
	mTasksList.CheckTasks();
	for(tvLuaInterpreter::iterator it = mLua.begin(); it != mLua.end(); ++it) {
		(*it)->onTimer();
	}
	return 1;
}

/** OnConfigChange event */
int cLua::OnConfigChange(const char * sName, const char * sValue) {
	cLuaInterpreter * Script;
	for(tvLuaInterpreter::iterator it = mLua.begin(); it != mLua.end(); ++it) {
		Script = *it;
		if(!Script->mL) continue;
		Script->NewCallParam((void *)sName, LUA_TSTRING);
		Script->NewCallParam((void *)sValue, LUA_TSTRING);
		Script->CallFunc("OnConfigChange");
	}
	return 0;
}

/** onFlood event */
int cLua::onFlood(cDCConnBase * DCConn, int iType1, int iType2) {
	if(DCConn != NULL) {
		int iRet = 0, iBlock = 0; // On defaults don't block
		cLuaInterpreter * Script;
		for(tvLuaInterpreter::iterator it = mLua.begin(); it != mLua.end(); ++it) {
			Script = *it;
			if(!Script->mL) continue;
			Script->NewCallParam((void *)DCConn, LUA_TLIGHTUSERDATA);
			Script->NewCallParam(lua_Number(iType1), LUA_TNUMBER);
			Script->NewCallParam(lua_Number(iType2), LUA_TNUMBER);
			iRet = Script->CallFunc("OnFlood");
			if(iRet == 1) { // 1 - blocked
				return 1;
			}
			if(iRet && (!iBlock || iBlock > iRet))
			iBlock = iRet;
		}
		return iBlock;
	} else LogError("Error in cLua::onFlood");
	return 1;
}

/// onWebData(WebID, sData)
int cLua::onWebData(cDCConnBase * Conn, cWebParserBase * Parser) {
	if((Conn != NULL) && (Parser != NULL)) {
		int iRet = 0, iBlock = 0; // On defaults don't block
		cLuaInterpreter * Script;
		for(tvLuaInterpreter::iterator it = mLua.begin(); it != mLua.end(); ++it) {
			Script = *it;
			if(!Script->mL) continue;
			Script->NewCallParam((void *)Conn, LUA_TLIGHTUSERDATA);
			Script->NewCallParam((void *)Parser->msStr.c_str(), LUA_TSTRING);
			iRet = Script->CallFunc("OnWebData");
			if(iRet == 1) return 1; // 1 - blocked
			if(iRet && (!iBlock || iBlock > iRet))
			iBlock = iRet;
		}
		return iBlock;
	}
	LogError("Error in cLua::onWebData");
	return 1;
}

int cLua::OnScriptAction(const char * sScriptName, const char * sAction) {
	tvLuaInterpreter::iterator it;
	cLuaInterpreter * Script;
	for(it = mLua.begin(); it != mLua.end(); ++it) {
		Script = *it;
		if(!Script->mL) continue;
		if(sScriptName) Script->NewCallParam((void *)sScriptName, LUA_TSTRING);
		if(Script->CallFunc(sAction))
			return 0;
	}
	return 1;
}

/** OnScriptError event */
int cLua::OnScriptError(cLuaInterpreter * Current, const char* sScriptName, const char* sErrMsg, bool bStoped) {
	tvLuaInterpreter::iterator it;
	cLuaInterpreter * Script;
	for(it = mLua.begin(); it != mLua.end(); ++it) {
		Script = *it;
		if(!Script->mL || Script == Current) continue;
		Script->NewCallParam((void *)sScriptName, LUA_TSTRING);
		Script->NewCallParam((void *)sErrMsg, LUA_TSTRING);
		Script->NewCallParam(lua_Number(bStoped), LUA_TBOOLEAN);
		if(Script->CallFunc("OnScriptError"))
			return 0;
	}
	return 1;
}

/** onAny event */
int cLua::onAny(cDCConnBase * DCConn, cDCParserBase * DCParser) {
	if((DCConn != NULL) && (DCParser != NULL)) {
		int iRet = 0, iBlock = 0; // On defaults don't block
		cLuaInterpreter * Script;
		for(tvLuaInterpreter::iterator it = mLua.begin(); it != mLua.end(); ++it) {
			Script = *it;
			if(!Script->mL) continue;
			Script->NewCallParam((void *)DCConn, LUA_TLIGHTUSERDATA);
			Script->NewCallParam((void *)DCParser->msStr.c_str(), LUA_TSTRING);
			Script->NewCallParam(lua_Number((double)DCParser->GetType()), LUA_TNUMBER);
			iRet = Script->CallFunc("OnAny");
			if(iRet == 1) return 1; // 1 - blocked
			if(iRet && (!iBlock || iBlock > iRet))
			iBlock = iRet;
		}
		return iBlock;
	} else LogError("Error in cLua::onAny");
	return 1;
}

#define DC_ACTION_1(FUNC, NAME) \
int cLua::FUNC(cDCConnBase * Conn) { \
	if(Conn != NULL) \
		return CallAll(NAME, Conn); \
	LogError("Error in cLua::" NAME); \
	return 1; \
}

#define DC_ACTION_2(FUNC, NAME) \
int cLua::FUNC(cDCConnBase * Conn, cDCParserBase * DCParser) { \
	if((Conn != NULL) && (DCParser != NULL)) \
		return CallAll(NAME, Conn, DCParser); \
	LogError("Error in cLua::" NAME); \
	return 1; \
}

#define DC_ACTION_3(FUNC, NAME) \
int cLua::FUNC(cDCConnBase * Conn, cDCParserBase * DCParser) { \
	if((Conn != NULL) && (Conn->mDCUserBase != NULL) && (DCParser != NULL)) \
		return CallAll(NAME, Conn, DCParser); \
	LogError("Error in cLua::" NAME); \
	return 1; \
}

DC_ACTION_1(onUserConnected,    "OnUserConnected"   ); // OnUserConnected(tUser)
DC_ACTION_1(onUserDisconnected, "OnUserDisconnected"); // OnUserDisconnected(tUser)
DC_ACTION_1(onUserEnter,        "OnUserEnter"       ); // OnUserEnter(tUser)
DC_ACTION_1(onUserExit,         "OnUserExit"        ); // OnUserExit(tUser)

DC_ACTION_2(onSupports,         "OnSupports"        ); // OnSupports(tUser, sData)
DC_ACTION_2(onKey,              "OnKey"             ); // OnKey(tUser, sData)
DC_ACTION_2(onUnknown,          "OnUnknown"         ); // OnUnknown(tUser, sData)

DC_ACTION_3(onValidateNick,     "OnValidateNick"    ); // OnValidateNick(tUser, sData)
DC_ACTION_3(onMyPass,           "OnMyPass"          ); // OnMyPass(tUser, sData)
DC_ACTION_3(onVersion,          "OnVersion"         ); // OnVersion(tUser, sData)
DC_ACTION_3(onGetNickList,      "OnGetNickList"     ); // OnGetNickList(tUser, sData)
DC_ACTION_3(onMyINFO,           "OnMyINFO"          ); // OnMyINFO(tUser, sData)
DC_ACTION_3(onChat,             "OnChat"            ); // OnChat(tUser, sData)
DC_ACTION_3(onTo,               "OnTo"              ); // OnTo(tUser, sData)
DC_ACTION_3(onConnectToMe,      "OnConnectToMe"     ); // OnConnectToMe(tUser, sData)
DC_ACTION_3(onRevConnectToMe,   "OnRevConnectToMe"  ); // OnRevConnectToMe(tUser, sData)
DC_ACTION_3(onSearch,           "OnSearch"          ); // OnSearch(tUser, sData)
DC_ACTION_3(onSR,               "OnSR"              ); // OnSR(tUser, sData)
DC_ACTION_3(onKick,             "OnKick"            ); // OnKick(tUser, sData)
DC_ACTION_3(onOpForceMove,      "OnOpForceMove"     ); // OnOpForceMove(tUser, sData)
DC_ACTION_3(onGetINFO,          "OnGetINFO"         ); // OnGetINFO(tUser, sData)
DC_ACTION_3(onMCTo,             "OnMCTo"            ); // OnMCTo(tUser, sData)

REG_PLUGIN(cLua);
