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
	#define INTERNAL_PLUGIN_VERSION 10010  //< Internal plugin version
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

/** Params with null values flags */
enum TagNil {
	TAGNIL_NO        = 0,        //< No
	TAGNIL_TAG       = 1 << 0,   //< Tag
	TAGNIL_CLIENT    = 1 << 1,   //< Client name
	TAGNIL_VERSION   = 1 << 2,   //< Client version
	TAGNIL_MODE      = 1 << 3,   //< Mode
	TAGNIL_UNREG     = 1 << 4,   //< Usual hubs
	TAGNIL_REG       = 1 << 5,   //< Reg hubs
	TAGNIL_OP        = 1 << 6,   //< Op hubs
	TAGNIL_SLOT      = 1 << 7,   //< Slots
	TAGNIL_LIMIT     = 1 << 8,   //< Limit
	TAGNIL_OPEN      = 1 << 9,   //< Open
	TAGNIL_BANDWIDTH = 1 << 10,  //< Bandwidth
	TAGNIL_DOWNLOAD  = 1 << 11,  //< Download
	TAGNIL_FRACTION  = 1 << 12,  //< Fraction
};

namespace protoenums {

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

}; // protoenums


class cDCUserBase;


// ================ cDCConnBase ================

/** Base DC connection */
class cDCConnBase {

public:

	cDCUserBase * mDCUserBase;  //< User
	const int mType;            //< Connection type (for protection and compatibility)

public:

	cDCConnBase(int type) : mDCUserBase(NULL), mType(type) {}
	virtual ~cDCConnBase() {}
	virtual cDCConnBase & operator=(const cDCConnBase &) { return *this; }

	virtual int Send(const string & data, bool addSep = false, bool flush = true) = 0; //< Sending RAW cmd to the client
	virtual int getPort() const = 0;                 //< Get real clients port
	virtual int getPortConn() const = 0;             //< Get connection port
	virtual int getProfile() const = 0;              //< Get client profile
	virtual long getEnterTime() const = 0;           //< Get enter time
	virtual unsigned long getNetIp() const = 0;      //< Get numeric IP
	virtual void setProfile(int) = 0;                //< Set client profile
	virtual void setData(const string &) = 0;        //< Set some client data
	virtual void disconnect() = 0;                   //< Disconnect this client
	virtual const string & getVersion() const = 0;   //< Client's protocol version
	virtual const string & getIp() const = 0;        //< Get string of IP
	virtual const string & getIpConn() const = 0;    //< Get string of server ip (host)
	virtual const string & getData() const = 0;      //< Get some client data
	virtual const string & getMacAddress() const = 0;//< Get mac address
	virtual const string & getSupports() const = 0;  //< Get all support cmd parameters, except of cmd name

}; // cDCConnBase


// ================ cDCUserBase ================

/** Base DC user */
class cDCUserBase {

public:

	cDCConnBase * mDCConnBase;  //< Connection

public:

	cDCUserBase() {}
	virtual ~cDCUserBase() {}
	virtual cDCUserBase & operator=(const cDCUserBase &) { return *this; }

	virtual const string & getNick() const = 0; //< Get user's nick

	virtual bool getInUserList() const = 0;     //< User in user-list

	virtual bool getInOpList() const = 0;       //< User in op-list (has op-key)
	virtual void setInOpList(bool) = 0;         //< Add user in op-list

	virtual bool getInIpList() const = 0;       //< User in ip-list (can receive ip addresses of users)
	virtual void setInIpList(bool) = 0;         //< Add user in ip-list

	virtual bool getHide() const = 0;           //< User is hidden
	virtual void setHide(bool) = 0;             //< Hide the user

	virtual bool getForceMove() const = 0;      //< User can redirect
	virtual void setForceMove(bool) = 0;        //< Redirect flag (user can redirect)

	virtual bool getKick() const = 0;           //< User can kick
	virtual void setKick(bool) = 0;             //< Kick flag (user can kick)

	virtual const string & GetMyINFO(/*bool real = false*/) const = 0;      //< Get user's MyINFO cmd
	virtual bool SetMyINFO(const string & myInfo, const string & nick) = 0; //< Set user's MyINFO cmd

	virtual const string & getDesc(/*bool real = false*/) const = 0;        //< Get user's description
	virtual const string & getEmail(/*bool real = false*/) const = 0;       //< Get user's email address
	virtual const string & getConnection(/*bool real = false*/) const = 0;  //< Get user's connection flag
	virtual unsigned getByte(/*bool real = false*/) const = 0;              //< Get user's magic byte
	virtual __int64 getShare(/*bool real = false*/) const = 0;              //< Get user's share size

	virtual const string & getTag(/*bool real = false*/) const = 0;         //< Get user's tag
	virtual const string & getClient(/*bool real = false*/) const = 0;      //< Get user's client
	virtual const string & getVersion(/*bool real = false*/) const = 0;     //< Get user's client version
	virtual const string & getMode(/*bool real = false*/) const = 0;        //< Get user's mode
	virtual unsigned getUnregHubs(/*bool real = false*/) const = 0;         //< Get user's unreg-hubs
	virtual unsigned getRegHubs(/*bool real = false*/) const = 0;           //< Get user's reg-hubs
	virtual unsigned getOpHubs(/*bool real = false*/) const = 0;            //< Get user's op-hubs
	virtual unsigned getSlots(/*bool real = false*/) const = 0;             //< Get user's slots
	virtual unsigned getLimit(/*bool real = false*/) const = 0;             //< Get user's L-limit
	virtual unsigned getOpen(/*bool real = false*/) const = 0;              //< Get user's O-limit
	virtual unsigned getBandwidth(/*bool real = false*/) const = 0;         //< Get user's B-limit
	virtual unsigned getDownload(/*bool real = false*/) const = 0;          //< Get user's D-limit
	virtual const string & getFraction(/*bool real = false*/) const = 0;    //< Get user's F-limit

	virtual unsigned int getTagNil(/*bool real = false*/) const = 0;              //< Get user's tagNil param

}; // cDCUserBase


// ================ cDCConnListIterator ================

/** Iterator for list of base connections */
class cDCConnListIterator {

public:

	cDCConnListIterator() {}
	virtual ~cDCConnListIterator() {}
	virtual cDCConnBase * operator() (void) = 0;
	virtual cDCConnListIterator & operator=(const cDCConnListIterator &) { return *this; }

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
	virtual ~cDCParserBase() {}
	virtual cDCParserBase & operator=(const cDCParserBase &) { return *this; }
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
	virtual ~cWebParserBase() {}
	virtual cWebParserBase & operator=(const cWebParserBase &) { return *this; }

}; // cWebParserBase

}; // nWebServer



namespace nPlugin {

using namespace ::nDCServer;
using namespace protoenums;
using namespace nWebServer;

class cPlugin;


// ================ cPluginListBase ================

/** Base plugin list */
class cPluginListBase {

public:

	cPluginListBase() {}
	virtual ~cPluginListBase() {}
	virtual cPluginListBase & operator=(const cPluginListBase &) { return *this; }

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
	virtual cPlugin & operator=(const cPlugin &) { return *this; }

	virtual bool RegAll(cPluginListBase *) = 0; //< Reg function in all call lists
	virtual void onLoad(cDCServerBase * DCServer) { mDCServer = DCServer; } //< OnLoad plugin function

	/** Events */
	virtual int onUserConnected(cDCConnBase *) { return 1; }
	virtual int onUserDisconnected(cDCConnBase *) { return 1; }
	virtual int onUserEnter(cDCConnBase *) { return 1; }
	virtual int onUserExit(cDCConnBase *) { return 1; }
	virtual int onSupports(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int onKey(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int onValidateNick(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int onMyPass(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int onVersion(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int onGetNickList(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int onMyINFO(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int onChat(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int onTo(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int onConnectToMe(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int onRevConnectToMe(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int onSearch(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int onSR(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int onKick(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int onOpForceMove(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int onGetINFO(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int onMCTo(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int onTimer() { return 1; }
	virtual int onAny(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int onUnknown(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int onFlood(cDCConnBase *, int, int) { return 1; }
	virtual int onWebData(cDCConnBase *, cWebParserBase *) { return 1; }


	const string &Name() const { return msName; }       //< Get name of plugin
	const string &Version() const { return msVersion; } //< Get version of plugin
	void Suicide() { mbIsAlive = false; }               //< Destruction of plugin
	bool IsAlive() const { return mbIsAlive; }          //< Check state
	void SetPluginList(cPluginListBase * pluginList) { mPluginList = pluginList; } //< Set plugin-list for plugin
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
