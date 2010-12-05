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

#ifndef CPLUGIN_H
#define CPLUGIN_H

#include <string>
#include <iostream>
#include <vector>

#ifndef _WIN32
	#ifndef __int64
		#define __int64 long long
	#endif
#endif

#ifndef REG_PLUGIN
	#ifdef _WIN32
		#define BUILDING_DLL 1
		#if BUILDING_DLL
			#define DLLIMPORT __declspec (dllexport)
		#else
			#define DLLIMPORT __declspec (dllimport)
		#endif
		#define REG_PLUGIN(__classname) \
		extern "C" { \
			DLLIMPORT cPlugin * get_plugin() { return new (__classname); } \
			DLLIMPORT void del_plugin(cPlugin *Plugin) { if(Plugin) delete Plugin; } \
		}
	#else
		#define REG_PLUGIN(__classname) \
		extern "C" { \
			cPlugin * get_plugin(void) { return new (__classname); } \
			void del_plugin(cPlugin *plugin) { if(plugin) delete plugin; } \
		}
	#endif // _WIN32
#endif // REG_PLUGIN

#ifndef INTERNAL_PLUGIN_VERSION
	#define INTERNAL_PLUGIN_VERSION 10006
#endif

#ifndef DC_SEPARATOR
	#define DC_SEPARATOR "|" /** DC Protocol separator */
	#define DC_SEPARATOR_LEN 1
#endif

#ifndef WEB_SEPARATOR
	#define WEB_SEPARATOR "\r\n\r\n" /** Protocol separator */
#endif

using namespace ::std;

namespace nDCServer {

typedef enum { /** Params with null values flags */
	eMYINFO_TAG       = 1 << 0,  //< Tag
	eMYINFO_CLIENT    = 1 << 1,  //< Client name
	eMYINFO_VERSION   = 1 << 2,  //< Client version
	eMYINFO_MODE      = 1 << 3,  //< Mode
	eMYINFO_UNREG     = 1 << 4,  //< Usual hubs
	eMYINFO_REG       = 1 << 5,  //< Reg hubs
	eMYINFO_OP        = 1 << 6,  //< Op hubs
	eMYINFO_SLOT      = 1 << 7,  //< Slots
	eMYINFO_LIMIT     = 1 << 8,  //< Limit
	eMYINFO_OPEN      = 1 << 9,  //< Open
	eMYINFO_BANDWIDTH = 1 << 10, //< Bandwidth
	eMYINFO_DOWNLOAD  = 1 << 11, //< Download
	eMYINFO_FRACTION  = 1 << 12, //< Fraction
} tMYINFONilType;

namespace nProtoEnums {

typedef enum { /** Types of the commands (for field mType) */
	eDC_NO = -1,
	eDC_MSEARCH,      //< 0  = $MultiSearch
	eDC_MSEARCH_PAS,  //< 1  = $MultiSearch Hub:
	eDC_SEARCH_PAS,   //< 2  = $Search Hub:
	eDC_SEARCH,       //< 3  = $Search
	eDC_SR,           //< 4  = $SR
	eDC_MYNIFO,       //< 5  = $MyNIFO
	eDC_SUPPORTS,     //< 6  = $Support
	eDC_KEY,          //< 7  = $Key
	eDC_VALIDATENICK, //< 8  = $ValidateNick
	eDC_VERSION,      //< 9  = $Version
	eDC_GETNICKLIST,  //< 10 = $GetNickList
	eDC_CHAT,         //< 11 = Chat
	eDC_TO,           //< 12 = $To
	eDC_QUIT,         //< 13 = $Quit
	eDC_MYPASS,       //< 14 = $MyPass
	eDC_CONNECTTOME,  //< 15 = $ConnecToMe
	eDC_RCONNECTTOME, //< 16 = $RevConnectToMe
	eDC_MCONNECTTOME, //< 17 = $MultiConnectToMe
	eDC_KICK,         //< 18 = $Kick
	eDC_OPFORCEMOVE,  //< 19 = $OpForceMove
	eDC_GETINFO,      //< 20 = $GetINFO
	eDC_MCTO,         //< 21 = $MCTo
	eDC_PING,         //< 22 = |
	eDC_UNKNOWN       //< 23 = $Unknown
} tDCType;


/** The Following constants were developed for accomodation 
	corresponding to parameter for each enterring commands of 
	the protocol DC in variable mChunks, are used in function 
	cDCServer::DC_* as well as in cDCParser::SplitChunks... 
	they must correspond
*/

/** A number of the chunks for simple commands (without parameter) */
enum { eCH_0_ALL };

/** A number of the chunks for commands with one parameter.
		$Key [key], $ValidateNick [nick], $Version [1,0091], $Quit [nick], $MyPass [pass], $Kick [nick]
*/
enum { eCH_1_ALL, eCH_1_PARAM }; 

/** A number of the chunks for partition chat message
		<[nick]> [msg]
*/
enum { eCH_CH_ALL, eCH_CH_NICK, eCH_CH_MSG };

/** A number of the chunks for the $GetINFO command
		$GetINFO [remote_nick] [nick]
*/
enum { eCH_GI_ALL, eCH_GI_OTHER, eCH_GI_NICK };

/** A number of the chunks for the $RevConnectToMe command
		$RevConnectToMe [nick] [remote_nick]
*/
enum { eCH_RC_ALL, eCH_RC_NICK, eCH_RC_OTHER };

/** A number of the chunks for the private message
		$To: [remote_nick] From: [nick] $<[[nick]> [msg]]
*/
enum { eCH_PM_ALL, eCH_PM_TO, eCH_PM_FROM, eCH_PM_CHMSG, eCH_PM_NICK, eCH_PM_MSG };

/** A number of the chunks for the $MyINFO command
		$MyINFO $ALL [nick] [[desc]$ $[speed]$[email]$[share]$]
*/
enum { eCH_MI_ALL, eCH_MI_DEST, eCH_MI_NICK, eCH_MI_INFO, eCH_MI_DESC, eCH_MI_SPEED, eCH_MI_MAIL, eCH_MI_SIZE };

/** A number of the chunks for the $ConnectToMe command
		$ConnectToMe [remote_nick] [ip]:[port]
*/
enum { eCH_CM_ALL, eCH_CM_NICK, eCH_CM_ACTIVE, eCH_CM_IP, eCH_CM_PORT };

/** A number of the chunks for the $OpForceMove command
		$OpForceMove $Who:[remote_nick]$Where:[address]$Msg:[reason]
*/
enum { eCH_FM_ALL, eCH_FM_NICK, eCH_FM_DEST, eCH_FM_REASON };

/** A number of the chunks for the active search command
		$Search [[ip]:[port]] [[sizerestricted?isminimumsize?size?datatype]?[searchpattern]]
*/
enum { eCH_AS_ALL, eCH_AS_ADDR, eCH_AS_IP, eCH_AS_PORT, eCH_AS_QUERY, eCH_AS_SEARCHLIMITS, eCH_AS_SEARCHPATTERN };

/** A number of the chunks for the passive search command
		$Search Hub:[nick] [[sizerestricted?isminimumsize?size?datatype]?[searchpattern]]
*/
enum { eCH_PS_ALL, eCH_PS_NICK, eCH_PS_QUERY, eCH_PS_SEARCHLIMITS, eCH_PS_SEARCHPATTERN };

/** A number of the chunks for the search results command
		$SR [nick] [file/path][0x05][filesize] [freeslots]/[totalslots][0x05][hubname] ([hubhost][:[hubport]])[0x05][searching_nick]
*/
enum { eCH_SR_ALL, eCH_SR_FROM, eCH_SR_PATH, eCH_SR_SIZE, eCH_SR_SLOTS, eCH_SR_SL_FR, eCH_SR_SL_TO, eCH_SR_HUBINFO, eCH_SR_TO };

/** A number of the chunks for the private message
		$MCTo: [remote_nick] $[nick] [msg]
*/
enum { eCH_MC_ALL, eCH_MC_TO, eCH_MC_FROM, eCH_MC_MSG };

}; // nProtoEnums


class cDCUserBase;


// ================ cDCConnBase ================

class cDCConnBase {

public:
	cDCUserBase * mDCUserBase; //< User
	int _miConnType; //< Connection type (for protection and compatibility)

public:
	cDCConnBase() : mDCUserBase(NULL), _miConnType(1){}
	~cDCConnBase(){}
	virtual int Send(const string & sData, bool bAddSep = false, bool bFlush = true) = 0; //< Sending RAW cmd to the client
	virtual const string & GetVersion() const = 0; //< Client's protocol version
	virtual const string & GetIp() const = 0; //< Get string of IP
	virtual const string & GetData() const = 0; //< Get some user data
	virtual const string & GetMacAddr() const = 0; //< Get mac address
	virtual const string & GetSupports() const = 0;
	virtual int GetPort() const = 0; //< Get real port
	virtual int GetPortConn() const = 0; //< Get connection port
	virtual int GetProfile() const = 0; //< Get profile
	virtual unsigned long GetNetIp() const = 0; //< Get numeric IP
	virtual void SetProfile(int) = 0;
	virtual void SetData(const string &) = 0;
	virtual void Disconnect() = 0;
	virtual long GetEnterTime() const = 0; //< Get enter time

}; // cDCConnBase


// ================ cDCUserBase ================

/** Base user class */
class cDCUserBase {

public:

	cDCConnBase * mDCConnBase;
	unsigned int mNil;

public:

	cDCUserBase(){}
	virtual ~cDCUserBase(){}

	virtual const string & GetNick() const = 0;
	virtual const string & GetMyINFO() const = 0;
	virtual bool IsInUserList() const = 0;
	virtual bool IsInOpList() const = 0;
	virtual bool IsInIpList() const = 0;
	virtual bool IsHide() const = 0;

	virtual bool SetMyINFO(const string &sMyINFO, const string & sNick) = 0;
	virtual void SetOpList(bool) = 0;
	virtual void SetIpList(bool) = 0;
	virtual void SetHide(bool) = 0;

	virtual const string & GetDesc() const = 0;
	virtual const string & GetEmail() const = 0;
	virtual const string & GetConnection() const = 0;
	virtual unsigned GetByte() const = 0;
	virtual __int64 GetShare() const = 0;

	virtual const string & GetTag() const = 0;
	virtual const string & GetClient() const = 0;
	virtual const string & GetVersion() const = 0;
	virtual unsigned GetUnRegHubs() const = 0;
	virtual unsigned GetRegHubs() const = 0;
	virtual unsigned GetOpHubs() const = 0;
	virtual unsigned GetSlots() const = 0;
	virtual unsigned GetLimit() const = 0;
	virtual unsigned GetOpen() const = 0;
	virtual unsigned GetBandwidth() const = 0;
	virtual unsigned GetDownload() const = 0;
	virtual const string & GetFraction() const = 0;
	virtual const string & GetMode() const = 0;

}; // cDCUserBase


// ================ cDCConnListIterator ================

class cDCConnListIterator {

public:

	cDCConnListIterator(){}
	virtual ~cDCConnListIterator(){}
	virtual cDCConnBase * operator() (void) = 0;

}; // cDCConnListIterator


// ================ cDCServerBase ================

class cDCServerBase {

public:

	typedef vector<string> List_t;

	virtual const string & GetMainDir() const = 0;
	virtual const string & GetTime() = 0;
	virtual const string & GetHubInfo() const = 0; /** Name and version of the hub */
	virtual const string & GetLocale() const = 0;
	virtual const string & GetSystemVersion() const = 0;
	virtual int GetMSec() const = 0;
	virtual int GetUpTime() const = 0; /** Work time (sec) */
	virtual int GetUsersCount() const = 0;
	virtual __int64 GetTotalShare() const = 0;

	virtual int CheckCmd(const string &) = 0;

	virtual bool SendToUser(cDCConnBase *DCConn, const char *sData, char *sNick = NULL, char *sFrom = NULL) = 0;
	virtual bool SendToNick(const char *sTo, const char *sData, char *sNick = NULL, char *sFrom = NULL) = 0;
	virtual bool SendToAll(const char *sData, char *sNick = NULL, char *sFrom = NULL) = 0;
	virtual bool SendToProfiles(unsigned long iProfile, const char *sData, char *sNick = NULL, char *sFrom = NULL) = 0;
	virtual bool SendToIP(const char *sIP, const char *sData, unsigned long iProfile = 0, char *sNick = NULL, char *sFrom = NULL) = 0;
	virtual bool SendToAllExceptNicks(List_t & NickList, const char *sData, char *sNick = NULL, char *sFrom = NULL) = 0;
	virtual bool SendToAllExceptIps(List_t & IPList, const char *sData, char *sNick = NULL, char *sFrom = NULL) = 0;

	virtual void GetDCConnBase(const char * sIP, vector<cDCConnBase *> & vconn) = 0;
	virtual cDCUserBase * GetDCUserBase(const char *sNick) = 0;
	virtual cDCConnListIterator * GetDCConnListIterator() = 0;

	virtual void GetConfig(vector<string> & vec) = 0;
	virtual const char * GetConfig(const string & sName) = 0;
	virtual const char * GetLang(const string & sName) = 0;
	virtual bool SetConfig(const string & sName, const string & sValue) = 0;
	virtual bool SetLang(const string & sName, const string & sValue) = 0;

	virtual int RegBot(const string & sNick, const string & sMyINFO, const string & sIP, bool bKey = true) = 0;
	virtual int UnregBot(const string & sNick) = 0;

	virtual void StopHub() = 0;
	virtual void RestartHub() = 0;

}; // cDCServerBase


// ================ cDCParserBase ================

class cDCParserBase {

public:

	string & msStr; //< address of the string with command

public:

	cDCParserBase(string & sStr) : msStr(sStr) {}
	virtual string & ChunkString(unsigned int n) = 0; /** Get string address for the chunk of command */
	virtual int GetType() const = 0; /** Get command type */

}; // cDCParserBase

}; // nDCServer



namespace nWebServer {


// ================ cWebParserBase ================

class cWebParserBase {

public:

	string & msStr; /** Ref to string with cmd */

public:

	cWebParserBase(string & sStr):msStr(sStr) {}

}; // cWebParserBase

}; // nWebServer



namespace nPlugin {

using namespace ::nDCServer;
using namespace nProtoEnums;
using namespace nWebServer;

class cPlugin;


// ================ cPluginListBase ================

class cPluginListBase {

public:

	cPluginListBase(){}
	virtual ~cPluginListBase(){}

	virtual const string & GetPluginDir() = 0; /** Plugins dir */
	virtual bool RegCallList(const char * sId, cPlugin *) = 0; /** Reg plugin in list */
	virtual bool UnregCallList(const char * sId, cPlugin *) = 0; /** Unreg plugin in list */

}; // cPluginListBase


// ================ cPlugin ================

/** Plugin base class */
class cPlugin {

private:
	bool mbIsAlive; /** State of plugin (loaded or not loaded) */

public:
	int miVersion; /** Interface version of all plugins */
	cDCServerBase * mDCServer;

public:
	cPlugin() : mbIsAlive(true), miVersion(0), mDCServer(NULL), mPluginList(NULL) {
		miVersion = INTERNAL_PLUGIN_VERSION;
	}
	virtual ~cPlugin() {}

	virtual bool RegAll(cPluginListBase *) = 0; /** Reg function in all call lists */
	virtual void OnLoad(cDCServerBase * DCServer) { mDCServer = DCServer; }
	virtual int OnUserConnected(cDCConnBase *) { return 1; }
	virtual int OnUserDisconnected(cDCConnBase *) { return 1; }
	virtual int OnUserEnter(cDCConnBase *) { return 1; }
	virtual int OnUserExit(cDCConnBase *) { return 1; }
	virtual int OnSupports(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnKey(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnValidateNick(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnMyPass(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnVersion(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnGetNickList(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnMyINFO(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnChat(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnTo(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnConnectToMe(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnRevConnectToMe(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnSearch(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnSR(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnKick(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnOpForceMove(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnGetINFO(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnMCTo(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnTimer() { return 1; }
	virtual int OnAny(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnUnknown(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnFlood(cDCConnBase *, int, int) { return 1; }
	virtual int OnWebData(cDCConnBase *, cWebParserBase *) { return 1; }


	const string &Name() { return msName; } /** Get name of the plugin */
	const string &Version() { return msVersion; } /** Get version of the plugin */
	void Suicide() { mbIsAlive = true; } /** Destruction plugin */
	bool IsAlive() { return mbIsAlive; } /** Check state */
	void SetPluginList(cPluginListBase * PluginList) { mPluginList = PluginList; } /** Set plugin list for plugin */
	virtual const string & GetPluginDir() { return mPluginList->GetPluginDir(); }; /** Plugin's dir */

protected:

	string msName; /** Name of the plugin */
	string msVersion; /** Version of the plugin */
	cPluginListBase * mPluginList; /** Pointer to list of all plugins */

}; // cPlugin

}; // nPlugin

#endif // CPLUGIN_H
