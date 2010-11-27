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

#ifndef CLUA_H
#define CLUA_H

#include "cplugin.h" /** nDCServer */
#include "cluainterpreter.h"
#include "api.h"
#include "ctaskslist.h"

#include <vector>
#include <list>
#include <math.h> /** abs */

#define PLUGIN_NAME "LuaScripts"
#define PLUGIN_VERSION "1.27"

namespace nDCServer{class cDCServerBase;}

using namespace std;
using namespace ::nPlugin;
using namespace ::nDCServer;
using namespace ::nDCServer::nPlugin;
using namespace ::nDCServer::nProtoEnums;
using namespace ::nLua;


class cLua : public cPlugin
{

public:

	string msScriptsDir; /** Scripts path (directory) */

	static cDCServerBase * mCurServer; /** Current server */
	static cLua * mCurLua; /** Current plugin */

	typedef list<cLuaInterpreter *> tvLuaInterpreter;
	tvLuaInterpreter mLua; /** Script-list */

	cLuaInterpreter * mCurScript; /** Current script. When script is working only! */
	string msLastError; /** Last error in scripts */

	cTasksList mTasksList;
	cTimerList * mTimerList;

	cDCParserBase * mCurDCParser; /** Current cmd-parser (only for events) */
	cDCConnBase * mCurDCConn; /** Current connection (only for events) */

public:

	cLua();
	virtual ~cLua();

	virtual const string & GetScriptsDir(){ return msScriptsDir; }

	virtual void OnLoad(cDCServerBase *); /** Actions when loading plugin */
	virtual bool RegAll(cPluginListBase* PluginList); /** Registration all events for this plugin */

// events
	virtual int OnUserConnected(cDCConnBase *);
	virtual int OnUserDisconnected(cDCConnBase *);
	virtual int OnUserEnter(cDCConnBase *);
	virtual int OnUserExit(cDCConnBase *);
	virtual int OnSupports(cDCConnBase *, cDCParserBase *);
	virtual int OnKey(cDCConnBase *, cDCParserBase *);
	virtual int OnValidateNick(cDCConnBase *, cDCParserBase *);
	virtual int OnMyPass(cDCConnBase *, cDCParserBase *);
	virtual int OnVersion(cDCConnBase *, cDCParserBase *);
	virtual int OnGetNickList(cDCConnBase *, cDCParserBase *);
	virtual int OnMyINFO(cDCConnBase *, cDCParserBase *);
	virtual int OnChat(cDCConnBase *, cDCParserBase *);
	virtual int OnTo(cDCConnBase *, cDCParserBase *);
	virtual int OnConnectToMe(cDCConnBase *, cDCParserBase *);
	virtual int OnRevConnectToMe(cDCConnBase *, cDCParserBase *);
	virtual int OnSearch(cDCConnBase *, cDCParserBase *);
	virtual int OnSR(cDCConnBase *, cDCParserBase *);
	virtual int OnKick(cDCConnBase *, cDCParserBase *);
	virtual int OnOpForceMove(cDCConnBase *, cDCParserBase *);
	virtual int OnGetINFO(cDCConnBase *, cDCParserBase *);
	virtual int OnMCTo(cDCConnBase *, cDCParserBase *);
	virtual int OnTimer();
	virtual int OnAny(cDCConnBase *, cDCParserBase *);
	virtual int OnUnknown(cDCConnBase *, cDCParserBase *);
	virtual int OnFlood(cDCConnBase *, int, int);
	virtual int OnWebData(cDCConnBase *, cWebParserBase *);

	virtual int OnScriptError(cLuaInterpreter * Current, const char* sScriptName, const char* sErrMsg, bool bStoped = true);
	virtual int OnScriptAction(const char * sScriptName, const char * sAction);

	int OnConfigChange(const char *, const char *);

	int CallAll(const char *, cDCConnBase * conn = NULL, cDCParserBase * DCParser = NULL); /** Calling event for all scripts */

	cLuaInterpreter * FindScript(const string & sScriptName);
	cLuaInterpreter * AddScript(const string & sScriptName, bool bOnlyNew = false);

	void CheckNewScripts();

	int LoadScripts(); /** Loading all scripts */
	int Clear(); /** Stoping and removing all scripts */

	/** Restarting all scripts 
		@param iType: 0 - simple restarting, 1 - restarting except loaded script, 2 - restarting except current script
	*/
	int RestartScripts(cLuaInterpreter * CurScript = NULL, int iType = 0);

	int StartScript(const string & sScriptName);
	int StartScript(cLuaInterpreter * Script);
	int StopScript(cLuaInterpreter * Script, bool bCurrent = false);
	int RestartScript(cLuaInterpreter * Script, bool bCurrent = false);

	int CheckExists(cLuaInterpreter * Script);

	void Save(); /** Saving all scripts */

	int MoveUp(cLuaInterpreter * Script);
	int MoveDown(cLuaInterpreter * Script);

}; // cLua

#endif // CLUA_H
