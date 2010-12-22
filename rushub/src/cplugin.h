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

namespace nPlugin {
	class cPlugin;
};
using nPlugin::cPlugin;
using namespace ::std;

#ifndef REG_PLUGIN
	#ifdef _WIN32
		#define BUILDING_DLL 1
		#if BUILDING_DLL
			#define DLLIMPORT __declspec (dllexport)
		#else
			#define DLLIMPORT __declspec (dllimport)
		#endif
		//< Macros for registration plugin
		#define REG_PLUGIN(__classname) \
		extern "C" { \
			DLLIMPORT cPlugin * get_plugin() { return new (__classname); } \
			DLLIMPORT void del_plugin(cPlugin *Plugin) { if(Plugin) delete Plugin; } \
		}
	#else
		//< Macros for unregistration plugin
		#define REG_PLUGIN(__classname) \
		extern "C" { \
			cPlugin * get_plugin(void) { return new (__classname); } \
			void del_plugin(cPlugin *plugin) { if(plugin) delete plugin; } \
		}
	#endif // _WIN32
#endif // REG_PLUGIN

#ifndef INTERNAL_PLUGIN_VERSION
	#define INTERNAL_PLUGIN_VERSION 10008  //< Internal plugin version
#endif

#ifndef DC_SEPARATOR
	#define DC_SEPARATOR "|"               //< DC protocol separator
	#define DC_SEPARATOR_LEN 1             //< Length of DC protocol separator
#endif

#ifndef WEB_SEPARATOR
	#define WEB_SEPARATOR "\r\n\r\n"       //< Web protocol separator
	#define WEB_SEPARATOR_LEN 4            //< Length of web protocol separator
#endif

namespace nDCServer {

enum {
	eT_DC_CLIENT,
	eT_WEB_CLIENT
};

typedef enum { /** Params with null values flags */
	eMYINFO_TAG       = 1 << 0,   //< Tag
	eMYINFO_CLIENT    = 1 << 1,   //< Client name
	eMYINFO_VERSION   = 1 << 2,   //< Client version
	eMYINFO_MODE      = 1 << 3,   //< Mode
	eMYINFO_UNREG     = 1 << 4,   //< Usual hubs
	eMYINFO_REG       = 1 << 5,   //< Reg hubs
	eMYINFO_OP        = 1 << 6,   //< Op hubs
	eMYINFO_SLOT      = 1 << 7,   //< Slots
	eMYINFO_LIMIT     = 1 << 8,   //< Limit
	eMYINFO_OPEN      = 1 << 9,   //< Open
	eMYINFO_BANDWIDTH = 1 << 10,  //< Bandwidth
	eMYINFO_DOWNLOAD  = 1 << 11,  //< Download
	eMYINFO_FRACTION  = 1 << 12,  //< Fraction
} tMYINFONilType;

namespace nProtoEnums {

typedef enum { /** Types of the commands */
	eDC_NO = -1,
	eDC_MSEARCH,        //< 0  = $MultiSearch
	eDC_MSEARCH_PAS,    //< 1  = $MultiSearch Hub:
	eDC_SEARCH_PAS,     //< 2  = $Search Hub:
	eDC_SEARCH,         //< 3  = $Search
	eDC_SR,             //< 4  = $SR
	eDC_MYNIFO,         //< 5  = $MyNIFO
	eDC_SUPPORTS,       //< 6  = $Support
	eDC_KEY,            //< 7  = $Key
	eDC_VALIDATENICK,   //< 8  = $ValidateNick
	eDC_VERSION,        //< 9  = $Version
	eDC_GETNICKLIST,    //< 10 = $GetNickList
	eDC_CHAT,           //< 11 = Chat
	eDC_TO,             //< 12 = $To
	eDC_QUIT,           //< 13 = $Quit
	eDC_MYPASS,         //< 14 = $MyPass
	eDC_CONNECTTOME,    //< 15 = $ConnecToMe
	eDC_RCONNECTTOME,   //< 16 = $RevConnectToMe
	eDC_MCONNECTTOME,   //< 17 = $MultiConnectToMe
	eDC_KICK,           //< 18 = $Kick
	eDC_OPFORCEMOVE,    //< 19 = $OpForceMove
	eDC_GETINFO,        //< 20 = $GetINFO
	eDC_MCTO,           //< 21 = $MCTo
	eDC_PING,           //< 22 = |
	eDC_UNKNOWN         //< 23 = $Unknown
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

/** Base DC connection */
class cDCConnBase {

public:

	cDCUserBase * mDCUserBase;  //< User
	const int mType;            //< Connection type (for protection and compatibility)

public:

	cDCConnBase(int type) : mDCUserBase(NULL), mType(type) {}
	~cDCConnBase() {}

	virtual int Send(const string & sData, bool bAddSep = false, bool bFlush = true) = 0; //< Sending RAW cmd to the client
	virtual int GetPort() const = 0;                 //< Get real clients port
	virtual int GetPortConn() const = 0;             //< Get connection port
	virtual int GetProfile() const = 0;              //< Get client profile
	virtual long GetEnterTime() const = 0;           //< Get enter time
	virtual unsigned long GetNetIp() const = 0;      //< Get numeric IP
	virtual void SetProfile(int) = 0;                //< Set client profile
	virtual void SetData(const string &) = 0;        //< Set some client data
	virtual void Disconnect() = 0;                   //< Disconnect this client
	virtual const string & GetVersion() const = 0;   //< Client's protocol version
	virtual const string & GetIp() const = 0;        //< Get string of IP
	virtual const string & GetIpConn() const = 0;    //< Get string of server ip (host)
	virtual const string & GetData() const = 0;      //< Get some client data
	virtual const string & GetMacAddr() const = 0;   //< Get mac address
	virtual const string & GetSupports() const = 0;  //< Get all support cmd parameters, except of cmd name

}; // cDCConnBase


// ================ cDCUserBase ================

/** Base DC user */
class cDCUserBase {

public:

	cDCConnBase * mDCConnBase;  //< Connection
	unsigned int mNil;          //< Empty MyINFO command parameters

public:

	cDCUserBase() {}
	virtual ~cDCUserBase() {}

	virtual const string & GetNick() const = 0;        //< Get user's nick
	virtual const string & GetMyINFO() const = 0;      //< Get user's MyINFO cmd

	virtual bool IsInUserList() const = 0;             //< User in user-list
	virtual bool IsInOpList() const = 0;               //< User in op-list (has op-key)
	virtual bool IsInIpList() const = 0;               //< User in ip-list (can receive ip addresses of users)
	virtual bool IsHide() const = 0;                   //< User is hidden
	virtual bool IsForceMove() const = 0;              //< User can redirect
	virtual bool IsKick() const = 0;                   //< User can kick

	virtual bool SetMyINFO(const string &sMyINFO, const string & sNick) = 0; //< Set user's MyINFO cmd
	virtual void SetOpList(bool) = 0;                  //< Add user in op-list
	virtual void SetIpList(bool) = 0;                  //< Add user in ip-list
	virtual void SetHide(bool) = 0;                    //< Hide the user
	virtual void SetForceMove(bool) = 0;               //< Redirect flag (user can redirect)
	virtual void SetKick(bool) = 0;                    //< Kick flag (user can kick)

	virtual const string & GetDesc() const = 0;        //< Get user's description
	virtual const string & GetEmail() const = 0;       //< Get user's email address
	virtual const string & GetConnection() const = 0;  //< Get user's connection flag
	virtual unsigned GetByte() const = 0;              //< Get user's magic byte
	virtual __int64 GetShare() const = 0;              //< Get user's share size

	virtual const string & GetTag() const = 0;         //< Get user's tag
	virtual const string & GetClient() const = 0;      //< Get user's client
	virtual const string & GetVersion() const = 0;     //< Get user's client version
	virtual const string & GetMode() const = 0;        //< Get user's mode
	virtual unsigned GetUnRegHubs() const = 0;         //< Get user's unreg-hubs
	virtual unsigned GetRegHubs() const = 0;           //< Get user's reg-hubs
	virtual unsigned GetOpHubs() const = 0;            //< Get user's op-hubs
	virtual unsigned GetSlots() const = 0;             //< Get user's slots
	virtual unsigned GetLimit() const = 0;             //< Get user's L-limit
	virtual unsigned GetOpen() const = 0;              //< Get user's O-limit
	virtual unsigned GetBandwidth() const = 0;         //< Get user's B-limit
	virtual unsigned GetDownload() const = 0;          //< Get user's D-limit
	virtual const string & GetFraction() const = 0;    //< Get user's F-limit

}; // cDCUserBase


// ================ cDCConnListIterator ================

/** Iterator for list of base connections */
class cDCConnListIterator {

public:

	cDCConnListIterator() {}
	virtual ~cDCConnListIterator() {}
	virtual cDCConnBase * operator() (void) = 0;

}; // cDCConnListIterator


// ================ cDCServerBase ================

/** Base DC server */
class cDCServerBase {

public:

	virtual const string & GetMainDir() const = 0;         //< Get main hub path
	virtual const string & GetTime() = 0;                  //< Get system date-time string (now)
	virtual const string & GetHubInfo() const = 0;         //< Get name and version of the hub
	virtual const string & GetLocale() const = 0;          //< Get main locale
	virtual const string & GetSystemVersion() const = 0;   //< Get OS name and version
	virtual int GetMSec() const = 0;                       //< Get time in milliseconds (now)
	virtual int GetUpTime() const = 0;                     //< Get hub work time in sec
	virtual int GetUsersCount() const = 0;                 //< Get total hub users count
	virtual __int64 GetTotalShare() const = 0;             //< Get total hub share size

	virtual int CheckCmd(const string &) = 0;              //< Checking syntax of cmd. Returned tDCType enum of checked cmd

	virtual bool SendToUser(cDCConnBase *DCConn, const char *sData, const char *sNick = NULL, const char *sFrom = NULL) = 0; //< Send comand to user
	virtual bool SendToNick(const char *sTo, const char *sData, const char *sNick = NULL, const char *sFrom = NULL) = 0;     //< Send comand to nick
	virtual bool SendToAll(const char *sData, const char *sNick = NULL, const char *sFrom = NULL) = 0;                       //< Send comand to all
	virtual bool SendToProfiles(unsigned long iProfile, const char *sData, const char *sNick = NULL, const char *sFrom = NULL) = 0; //< Send comand to profiles
	virtual bool SendToIP(const char *sIP, const char *sData, unsigned long iProfile = 0, const char *sNick = NULL, const char *sFrom = NULL) = 0; //< Send comand to ip
	virtual bool SendToAllExceptNicks(const vector<string> & NickList, const char *sData, const char *sNick = NULL, const char *sFrom = NULL) = 0; //< Send comand to all except nicks
	virtual bool SendToAllExceptIps(const vector<string> & IPList, const char *sData, const char *sNick = NULL, const char *sFrom = NULL) = 0; //< Send comand to all except ips

	virtual void ForceMove(cDCConnBase *DCConn, const char *sAddress, const char *sReason = NULL) = 0; //< Redirection client

	virtual const vector<cDCConnBase*> & GetDCConnBase(const char * sIP) = 0; //< Get conn base by ip
	virtual cDCUserBase * GetDCUserBase(const char *sNick) = 0; //< Get user base by nick
	virtual cDCConnListIterator * GetDCConnListIterator() = 0;  //< Get iterator of conn base

	virtual const vector<string> & GetConfig() = 0;             //< Get all configs names
	virtual const char * GetConfig(const string & sName) = 0;   //< Get config value by name
	virtual const char * GetLang(const string & sName) = 0;     //< Get lang value by name
	virtual bool SetConfig(const string & sName, const string & sValue) = 0;  //< Set config value by name
	virtual bool SetLang(const string & sName, const string & sValue) = 0;    //< Set lang value by name

	virtual int RegBot(const string & sNick, const string & sMyINFO, const string & sIP, bool bKey = true) = 0; //< Registration bot
	virtual int UnregBot(const string & sNick) = 0; //< Unreg bot

	virtual void StopHub() = 0;         //< Stop hub
	virtual void RestartHub() = 0;      //< Restarting hub

}; // cDCServerBase


// ================ cDCParserBase ================

/** Base DC parser */
class cDCParserBase {

public:

	string & msStr; //< Ref to string with command

public:

	cDCParserBase(string & sStr) : msStr(sStr) {}
	virtual string & ChunkString(unsigned int n) = 0; //< Get string address for the chunk of command
	virtual int GetType() const = 0;                  //< Get command type

}; // cDCParserBase

}; // nDCServer



namespace nWebServer {


// ================ cWebParserBase ================

/** Base web parser */
class cWebParserBase {

public:

	string & msStr; //< Ref to string with cmd

public:

	cWebParserBase(string & sStr) : msStr(sStr) {}

}; // cWebParserBase

}; // nWebServer



namespace nPlugin {

using namespace ::nDCServer;
using namespace nProtoEnums;
using namespace nWebServer;

class cPlugin;


// ================ cPluginListBase ================

/** Base plugin list */
class cPluginListBase {

public:

	cPluginListBase() {}
	virtual ~cPluginListBase() {}

	virtual const string & GetPluginDir() const = 0;                   //< Get plugins path
	virtual bool RegCallList(const char * sId, cPlugin *) = 0;   //< Reg plugin in list with id
	virtual bool UnregCallList(const char * sId, cPlugin *) = 0; //< Unreg plugin from list with id

}; // cPluginListBase


// ================ cPlugin ================

/** Base plugin */
class cPlugin {

public:

	const int miVersion;               //< Version of plugin interface
	cDCServerBase * mDCServer;   //< Main DC Server

public:

	cPlugin() : 
		miVersion(INTERNAL_PLUGIN_VERSION),
		mDCServer(NULL),
		mbIsAlive(true),
		mPluginList(NULL)
	{}

	virtual ~cPlugin() {}

	virtual bool RegAll(cPluginListBase *) = 0; //< Reg function in all call lists
	virtual void OnLoad(cDCServerBase * DCServer) { mDCServer = DCServer; } //< OnLoad plugin function

	/** Events */
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


	const string &Name() const { return msName; }       //< Get name of plugin
	const string &Version() const { return msVersion; } //< Get version of plugin
	void Suicide() { mbIsAlive = false; }               //< Destruction of plugin
	bool IsAlive() const { return mbIsAlive; }          //< Check state
	void SetPluginList(cPluginListBase * PluginList) { mPluginList = PluginList; } //< Set plugin-list for plugin
	virtual const string & GetPluginDir() const { return mPluginList->GetPluginDir(); }  //< Get plugins path

private:

	bool mbIsAlive;                  //< State of plugin (loaded or not loaded)

protected:

	string msName;                   //< Name of the plugin
	string msVersion;                //< Version of the plugin
	cPluginListBase * mPluginList;   //< Pointer to list of all plugins

}; // cPlugin

}; // nPlugin

#endif // CPLUGIN_H
