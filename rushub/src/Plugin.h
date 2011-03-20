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

#ifndef PLUGIN_H
#define PLUGIN_H

#include <string>
#include <iostream>
#include <vector>

#ifndef _WIN32
	#ifndef __int64
		#define __int64 long long
	#endif
#endif

namespace plugin {
	class Plugin;
};

using ::plugin::Plugin;
using namespace ::std;

#ifndef REG_PLUGIN
	#ifdef _WIN32
		#define BUILDING_DLL 1
		#if BUILDING_DLL
			#define DLLIMPORT __declspec (dllexport)
		#else
			#define DLLIMPORT __declspec (dllimport)
		#endif
		//< Macros for registration/unregistration plugin
		#define REG_PLUGIN(__classname) \
		extern "C" { \
			DLLIMPORT Plugin * get_plugin() { \
				return new (__classname); \
			} \
			DLLIMPORT void del_plugin(Plugin * plugin) { \
				if (plugin) { \
					delete plugin; \
				} \
			} \
		}
	#else
		//< Macros for unregistration/unregistration plugin
		#define REG_PLUGIN(__classname) \
		extern "C" { \
			Plugin * get_plugin() { \
				return new (__classname); \
			} \
			void del_plugin(Plugin * plugin) { \
				if (plugin) { \
					delete plugin; \
				} \
			} \
		}
	#endif // _WIN32
#endif // REG_PLUGIN

//< Internal plugin version
#ifndef INTERNAL_PLUGIN_VERSION
	#define INTERNAL_PLUGIN_VERSION 10011
#endif

//< NMDC protocol separator
#ifndef NMDC_SEPARATOR
	#define NMDC_SEPARATOR "|"
	#define NMDC_SEPARATOR_LEN 1
#endif

//< Web protocol separator
#ifndef WEB_SEPARATOR
	#define WEB_SEPARATOR "\r\n\r\n"
	#define WEB_SEPARATOR_LEN 4
#endif



namespace dcserver {

enum ClientType {
	CLIENT_TYPE_NMDC,
	CLIENT_TYPE_WEB,
}; // enum ClientType

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
}; // enum TagNil



namespace protoenums {

typedef enum { /** Types of the commands */
	NMDC_TYPE_NO = -1,
	NMDC_TYPE_MSEARCH,        //< 0  = $MultiSearch
	NMDC_TYPE_MSEARCH_PAS,    //< 1  = $MultiSearch Hub:
	NMDC_TYPE_SEARCH_PAS,     //< 2  = $Search Hub:
	NMDC_TYPE_SEARCH,         //< 3  = $Search
	NMDC_TYPE_SR,             //< 4  = $SR
	NMDC_TYPE_SR_UDP,         //< 5  = $SR UDP
	NMDC_TYPE_MYNIFO,         //< 6  = $MyNIFO
	NMDC_TYPE_SUPPORTS,       //< 7  = $Support
	NMDC_TYPE_KEY,            //< 8  = $Key
	NMDC_TYPE_VALIDATENICK,   //< 9  = $ValidateNick
	NMDC_TYPE_VERSION,        //< 10 = $Version
	NMDC_TYPE_GETNICKLIST,    //< 11 = $GetNickList
	NMDC_TYPE_CHAT,           //< 12 = Chat
	NMDC_TYPE_TO,             //< 13 = $To
	NMDC_TYPE_QUIT,           //< 14 = $Quit
	NMDC_TYPE_MYPASS,         //< 15 = $MyPass
	NMDC_TYPE_CONNECTTOME,    //< 16 = $ConnecToMe
	NMDC_TYPE_RCONNECTTOME,   //< 17 = $RevConnectToMe
	NMDC_TYPE_MCONNECTTOME,   //< 18 = $MultiConnectToMe
	NMDC_TYPE_KICK,           //< 19 = $Kick
	NMDC_TYPE_OPFORCEMOVE,    //< 20 = $OpForceMove
	NMDC_TYPE_GETINFO,        //< 21 = $GetINFO
	NMDC_TYPE_MCTO,           //< 22 = $MCTo
	NMDC_TYPE_USERIP,         //< 23 = $UserIP
	NMDC_TYPE_PING,           //< 24 = |
	NMDC_TYPE_UNKNOWN,        //< 25 = $Unknown
} NmdcType;


/** The Following constants were developed for accomodation 
	corresponding to parameter for each enterring commands of 
	the protocol DC in variable mChunks, are used in function 
	DcServer::DC_* as well as in DcParser::SplitChunks... 
	they must correspond
*/

/** A number of the chunks for simple commands (without parameter) */
enum {
	CHUNK_0_ALL,
};

/** A number of the chunks for commands with one parameter.
		$Key [key],
		$ValidateNick [nick],
		$Version [1,0091],
		$Quit [nick],
		$MyPass [pass],
		$Kick [nick]
*/
enum {
	CHUNK_1_ALL,
	CHUNK_1_PARAM,
};

/** A number of the chunks for partition chat message
		<[nick]> [msg]
*/
enum {
	CHUNK_CH_ALL,
	CHUNK_CH_NICK,
	CHUNK_CH_MSG,
};

/** A number of the chunks for the $GetINFO command
		$GetINFO [remote_nick] [nick]
*/
enum {
	CHUNK_GI_ALL,
	CHUNK_GI_OTHER,
	CHUNK_GI_NICK,
};

/** A number of the chunks for the $RevConnectToMe command
		$RevConnectToMe [nick] [remote_nick]
*/
enum {
	CHUNK_RC_ALL,
	CHUNK_RC_NICK,
	CHUNK_RC_OTHER,
};

/** A number of the chunks for the private message
		$To: [remote_nick] From: [nick] $<[[nick]> [msg]]
*/
enum {
	CHUNK_PM_ALL,
	CHUNK_PM_TO,
	CHUNK_PM_FROM,
	CHUNK_PM_CHMSG,
	CHUNK_PM_NICK,
	CHUNK_PM_MSG,
};

/** A number of the chunks for the $MyINFO command
		$MyINFO $ALL [nick] [[desc]$ $[speed]$[email]$[share]$]
*/
enum {
	CHUNK_MI_ALL,
	CHUNK_MI_DEST,
	CHUNK_MI_NICK,
	CHUNK_MI_INFO,
	CHUNK_MI_DESC,
	CHUNK_MI_SPEED,
	CHUNK_MI_MAIL,
	CHUNK_MI_SIZE,
};

/** A number of the chunks for the $ConnectToMe command
		$ConnectToMe [remote_nick] [ip]:[port]
*/
enum {
	CHUNK_CM_ALL,
	CHUNK_CM_NICK,
	CHUNK_CM_ACTIVE,
	CHUNK_CM_IP,
	CHUNK_CM_PORT,
};

/** A number of the chunks for the $OpForceMove command
		$OpForceMove $Who:[remote_nick]$Where:[address]$Msg:[reason]
*/
enum {
	CHUNK_FM_ALL,
	CHUNK_FM_NICK,
	CHUNK_FM_DEST,
	CHUNK_FM_REASON,
};

/** A number of the chunks for the active search command
		$Search [[ip]:[port]] [[sizerestricted?isminimumsize?size?datatype]?[searchpattern]]
*/
enum {
	CHUNK_AS_ALL,
	CHUNK_AS_ADDR,
	CHUNK_AS_IP,
	CHUNK_AS_PORT,
	CHUNK_AS_QUERY,
	CHUNK_AS_SEARCHLIMITS,
	CHUNK_AS_SEARCHPATTERN,
};

/** A number of the chunks for the passive search command
		$Search Hub:[nick] [[sizerestricted?isminimumsize?size?datatype]?[searchpattern]]
*/
enum {
	CHUNK_PS_ALL,
	CHUNK_PS_NICK,
	CHUNK_PS_QUERY,
	CHUNK_PS_SEARCHLIMITS,
	CHUNK_PS_SEARCHPATTERN,
};

/** A number of the chunks for the search results command
		$SR [nick] [file/path][0x05][filesize] [freeslots]/[totalslots][0x05][hubname] ([hubhost][:[hubport]])[0x05][searching_nick]
*/
enum {
	CHUNK_SR_ALL,
	CHUNK_SR_FROM,
	CHUNK_SR_PATH,
	CHUNK_SR_SIZE,
	CHUNK_SR_SLOTS,
	CHUNK_SR_SL_FR,
	CHUNK_SR_SL_TO,
	CHUNK_SR_HUBINFO,
	CHUNK_SR_TO,
};

/** A number of the chunks for the private message
		$MCTo: [remote_nick] $[nick] [msg]
*/
enum {
	CHUNK_MC_ALL,
	CHUNK_MC_TO,
	CHUNK_MC_FROM,
	CHUNK_MC_MSG,
};

}; // namespace protoenums



class DcUserBase;



// ================ DcConnBase ================

/** Base DC connection */
class DcConnBase {

public:

	//< User
	DcUserBase * mDcUserBase;

	//< Connection type (for protection and compatibility)
	const int mType;

public:

	DcConnBase(int type) :
		mDcUserBase(NULL),
		mType(type)
	{
	}

	virtual ~DcConnBase() {
	}

	virtual DcConnBase & operator = (const DcConnBase &) {
		return *this;
	}


	//< Sending RAW cmd to the client
	virtual int send(
		const string & data,
		bool addSep = false,
		bool flush = true
	) = 0;

	//< Disconnect this client
	virtual void disconnect() = 0;


	//< Get real clients port
	virtual int getPort() const = 0;

	//< Get connection port
	virtual int getPortConn() const = 0;

	//< Get numeric IP
	virtual unsigned long getNetIp() const = 0;

	//< Get string of IP
	virtual const string & getIp() const = 0;

	//< Get string of server ip (host)
	virtual const string & getIpConn() const = 0;

	//< Get mac address
	virtual const string & getMacAddress() const = 0;

	//< Get enter time
	virtual long getEnterTime() const = 0;

	//< Client's protocol version
	virtual const string & getVersion() const = 0;

	//< Get all support cmd parameters, except of cmd name
	virtual const string & getSupports() const = 0;


	//< Get client profile
	virtual int getProfile() const = 0;

	//< Set client profile
	virtual void setProfile(int) = 0;


	//< Get some client data
	virtual const string & getData() const = 0;

	//< Set some client data
	virtual void setData(const string &) = 0;

}; // class DcConnBase



// ================ DcUserBase ================

/** Base DC user */
class DcUserBase {

public:

	//< Connection
	DcConnBase * mDcConnBase;

public:

	virtual ~DcUserBase() {
	}

	//< Get user's nick
	virtual const string & getNick() const = 0;

	//< User in user-list
	virtual bool getInUserList() const = 0;


	//< User in op-list (has op-key)
	virtual bool getInOpList() const = 0;

	//< Add user in op-list
	virtual void setInOpList(bool) = 0;


	//< User in ip-list (can receive ip addresses of users)
	virtual bool getInIpList() const = 0;

	//< Add user in ip-list
	virtual void setInIpList(bool) = 0;


	//< User is hidden
	virtual bool getHide() const = 0;

	//< Hide the user
	virtual void setHide(bool) = 0;


	//< User can redirect
	virtual bool getForceMove() const = 0;

	//< Redirect flag (user can redirect)
	virtual void setForceMove(bool) = 0;


	//< User can kick
	virtual bool getKick() const = 0;

	//< Kick flag (user can kick)
	virtual void setKick(bool) = 0;


	//< Get user's MyINFO cmd
	virtual const string & getMyINFO(/*bool real = false*/) const = 0;

	//< Set user's MyINFO cmd
	virtual bool setMyINFO(const string & myInfo, const string & nick) = 0;


	//< Get user's description
	virtual const string & getDesc(/*bool real = false*/) const = 0;

	//< Get user's email address
	virtual const string & getEmail(/*bool real = false*/) const = 0;

	//< Get user's connection flag
	virtual const string & getConnection(/*bool real = false*/) const = 0;

	//< Get user's magic byte
	virtual unsigned getByte(/*bool real = false*/) const = 0;

	//< Get user's share size
	virtual __int64 getShare(/*bool real = false*/) const = 0;


	//< Get user's tag
	virtual const string & getTag(/*bool real = false*/) const = 0;

	//< Get user's client
	virtual const string & getClient(/*bool real = false*/) const = 0;

	//< Get user's client version
	virtual const string & getVersion(/*bool real = false*/) const = 0;

	//< Get user's mode
	virtual const string & getMode(/*bool real = false*/) const = 0;

	//< Get user's unreg-hubs
	virtual unsigned getUnregHubs(/*bool real = false*/) const = 0;

	//< Get user's reg-hubs
	virtual unsigned getRegHubs(/*bool real = false*/) const = 0;

	//< Get user's op-hubs
	virtual unsigned getOpHubs(/*bool real = false*/) const = 0;

	//< Get user's slots
	virtual unsigned getSlots(/*bool real = false*/) const = 0;

	//< Get user's L-limit
	virtual unsigned getLimit(/*bool real = false*/) const = 0;

	//< Get user's O-limit
	virtual unsigned getOpen(/*bool real = false*/) const = 0;

	//< Get user's B-limit
	virtual unsigned getBandwidth(/*bool real = false*/) const = 0;

	//< Get user's D-limit
	virtual unsigned getDownload(/*bool real = false*/) const = 0;

	//< Get user's F-limit
	virtual const string & getFraction(/*bool real = false*/) const = 0;


	//< Get user's tagNil param
	virtual unsigned int getTagNil(/*bool real = false*/) const = 0;


}; // class DcUserBase



// ================ DcConnListIterator ================

/** Iterator for list of base connections */
class DcConnListIterator {

public:

	virtual ~DcConnListIterator() {
	}

	virtual DcConnBase * operator() (void) = 0;

}; // class DcConnListIterator



// ================ DcServerBase ================

/** Base DC server */
class DcServerBase {

public:

	//< Get main hub path
	virtual const string & getMainDir() const = 0;

	//< Get system date-time string (now)
	virtual const string & getTime() = 0;

	//< Get name and version of the hub
	virtual const string & getHubInfo() const = 0;

	//< Get main locale
	virtual const string & getLocale() const = 0;

	//< Get OS name and version
	virtual const string & getSystemVersion() const = 0;

	//< Get time in milliseconds (now)
	virtual int getMSec() const = 0;

	//< Get hub work time in sec
	virtual int getUpTime() const = 0;

	//< Get total hub users count
	virtual int getUsersCount() const = 0;

	//< Get total hub share size
	virtual __int64 getTotalShare() const = 0;

	//< Checking syntax of cmd. Returned NmdcType enum of checked cmd
	virtual int checkCmd(const string &) = 0;

	//< Send comand to user
	virtual bool sendToUser(
		DcConnBase *,
		const char * data,
		const char * nick = NULL,
		const char * from = NULL
	) = 0;

	//< Send comand to nick
	virtual bool sendToNick(
		const char * to,
		const char * data,
		const char * nick = NULL,
		const char * from = NULL
	) = 0;

	//< Send comand to all
	virtual bool sendToAll(
		const char * data,
		const char * nick = NULL,
		const char * from = NULL
	) = 0;

	//< Send comand to profiles
	virtual bool sendToProfiles(
		unsigned long profile,
		const char * data,
		const char * nick = NULL,
		const char * from = NULL
	) = 0;

	//< Send comand to ip
	virtual bool sendToIp(
		const char * ip,
		const char * data,
		unsigned long profile = 0,
		const char * nick = NULL,
		const char * from = NULL
	) = 0;

	//< Send comand to all except nicks
	virtual bool sendToAllExceptNicks(
		const vector<string> & nickList,
		const char * data,
		const char * nick = NULL,
		const char * from = NULL
	) = 0;

	//< Send comand to all except ips
	virtual bool sendToAllExceptIps(
		const vector<string> & ipList,
		const char * data,
		const char * nick = NULL,
		const char * from = NULL
	) = 0;


	//< Redirection client
	virtual void forceMove(
		DcConnBase *,
		const char * address,
		const char * reason = NULL
	) = 0;


	//< Get conn base by ip
	virtual const vector<DcConnBase*> & getDcConnBase(const char * ip) = 0;

	//< Get user base by nick
	virtual DcUserBase * getDcUserBase(const char * nick) = 0;

	//< Get iterator of conn base
	virtual DcConnListIterator * getDcConnListIterator() = 0;


	//< Get all configs names
	virtual const vector<string> & getConfig() = 0;

	//< Get config value by name
	virtual const char * getConfig(const string & name) = 0;

	//< Get lang value by name
	virtual const char * getLang(const string & name) = 0;

	//< Set config value by name
	virtual bool setConfig(const string & name, const string & value) = 0;

	//< Set lang value by name
	virtual bool setLang(const string & name, const string & value) = 0;


	//< Registration bot
	virtual int regBot(
		const string & nick,
		const string & myInfo,
		const string & ip,
		bool key = true
	) = 0;

	//< Unreg bot
	virtual int unregBot(const string & nick) = 0;


	//< Stop hub
	virtual void stopHub() = 0;

	//< Restarting hub
	virtual void restartHub() = 0;


}; // class DcServerBase


// ================ DcParserBase ================

/** Base DC parser */
class DcParserBase {

public:

	//< Ref to string with command
	string & mParseString;

public:

	DcParserBase(string & parseString) : 
		mParseString(parseString)
	{
	}

	virtual ~DcParserBase() {
	}

	virtual DcParserBase & operator = (const DcParserBase &) {
		return *this;
	}

	//< Get string address for the chunk of command
	virtual string & chunkString(unsigned int n) = 0;

	//< Get command type
	virtual int getCommandType() const = 0;

}; // class DcParserBase

}; // namespace dcserver



namespace webserver {


// ================ WebParserBase ================

/** Base web parser */
class WebParserBase {

public:

	//< Ref to string with command
	string & mParseString;

public:

	WebParserBase(string & parseString) :
		mParseString(parseString)
	{
	}

	virtual ~WebParserBase() {
	}

	virtual WebParserBase & operator = (const WebParserBase &) {
		return *this;
	}

}; // class WebParserBase

}; // namespace webserver



namespace plugin {

using namespace ::dcserver;
using namespace ::dcserver::protoenums;
using namespace ::webserver;

class Plugin;


// ================ PluginListBase ================

/** Base plugin list */
class PluginListBase {

public:

	//< Get plugins path
	virtual const string & getPluginDir() const = 0;

	//< Reg plugin in list with id
	virtual bool regCallList(const char * id, Plugin *) = 0;

	//< Unreg plugin from list with id
	virtual bool unregCallList(const char * id, Plugin *) = 0;

}; // class PluginListBase


// ================ Plugin ================

/** Base plugin */
class Plugin {

public:

	//< Version of plugin interface
	const int mInternalPluginVersion;

public:

	Plugin() : 
		mInternalPluginVersion(INTERNAL_PLUGIN_VERSION),
		mIsAlive(true),
		mPluginListBase(NULL)
	{
	}

	virtual ~Plugin() {
	}

	virtual Plugin & operator = (const Plugin &) {
		return *this;
	}

	//< Reg function in all call lists
	virtual bool regAll(PluginListBase *) = 0;

	//< OnLoad plugin function
	virtual void onLoad(DcServerBase *) {
	}


	/// Events

	virtual int onUserConnected(DcConnBase *) {
		return 1;
	}

	virtual int onUserDisconnected(DcConnBase *) {
		return 1;
	}

	virtual int onUserEnter(DcConnBase *) {
		return 1;
	}

	virtual int onUserExit(DcConnBase *) {
		return 1;
	}

	virtual int onSupports(DcConnBase *, DcParserBase *) {
		return 1;
	}

	virtual int onKey(DcConnBase *, DcParserBase *) {
		return 1;
	}

	virtual int onValidateNick(DcConnBase *, DcParserBase *) {
		return 1;
	}

	virtual int onMyPass(DcConnBase *, DcParserBase *) {
		return 1;
	}

	virtual int onVersion(DcConnBase *, DcParserBase *) {
		return 1;
	}

	virtual int onGetNickList(DcConnBase *, DcParserBase *) {
		return 1;
	}

	virtual int onMyINFO(DcConnBase *, DcParserBase *) {
		return 1;
	}

	virtual int onChat(DcConnBase *, DcParserBase *) {
		return 1;
	}

	virtual int onTo(DcConnBase *, DcParserBase *) {
		return 1;
	}

	virtual int onConnectToMe(DcConnBase *, DcParserBase *) {
		return 1;
	}

	virtual int onRevConnectToMe(DcConnBase *, DcParserBase *) {
		return 1;
	}

	virtual int onSearch(DcConnBase *, DcParserBase *) {
		return 1;
	}

	virtual int onSR(DcConnBase *, DcParserBase *) {
		return 1;
	}

	virtual int onKick(DcConnBase *, DcParserBase *) {
		return 1;
	}

	virtual int onOpForceMove(DcConnBase *, DcParserBase *) {
		return 1;
	}

	virtual int onGetINFO(DcConnBase *, DcParserBase *) {
		return 1;
	}

	virtual int onMCTo(DcConnBase *, DcParserBase *) {
		return 1;
	}

	virtual int onTimer() {
		return 1;
	}

	virtual int onAny(DcConnBase *, DcParserBase *) {
		return 1;
	}

	virtual int onUnknown(DcConnBase *, DcParserBase *) {
		return 1;
	}

	virtual int onFlood(DcConnBase *, int, int) {
		return 1;
	}

	virtual int onWebData(DcConnBase *, WebParserBase *) {
		return 1;
	}


	//< Get name of plugin
	const string & getName() const {
		return mName;
	}

	//< Get version of plugin
	const string & getVersion() const {
		return mVersion;
	}

	//< Destruction of plugin
	void suicide() {
		mIsAlive = false;
	}

	//< Check state
	bool isAlive() const {
		return mIsAlive;
	}

	//< Set plugin-list for plugin
	void setPluginList(PluginListBase * pluginListBase) {
		mPluginListBase = pluginListBase;
	}

	//< Get plugins path
	virtual const string & getPluginDir() const {
		return mPluginListBase->getPluginDir();
	}

protected:

	//< Name of the plugin
	string mName;

	//< Version of the plugin
	string mVersion;

private:

	//< State of plugin (loaded or not loaded)
	bool mIsAlive;

	//< Pointer to list of all plugins
	PluginListBase * mPluginListBase;


}; // class Plugin

}; // namespace plugin

#endif // PLUGIN_H
