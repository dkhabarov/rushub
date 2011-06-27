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
		/// Macros for registration/unregistration plugin
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
		/// Macros for unregistration/unregistration plugin
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

/// Internal plugin version
#ifndef INTERNAL_PLUGIN_VERSION
	#define INTERNAL_PLUGIN_VERSION 10023
#endif

/// NMDC protocol separator
#ifndef NMDC_SEPARATOR
	#define NMDC_SEPARATOR "|"
	#define NMDC_SEPARATOR_LEN 1
#endif

/// Web protocol separator
#ifndef WEB_SEPARATOR
	#define WEB_SEPARATOR "\r\n\r\n"
	#define WEB_SEPARATOR_LEN 4
#endif



namespace dcserver {

enum ClientType {
	CLIENT_TYPE_NMDC = 1000,
	CLIENT_TYPE_WEB = 1001,
}; // enum ClientType

/** Params with null values flags */
enum TagNil {
	TAGNIL_NO        = 0,        ///< No
	TAGNIL_TAG       = 1 << 0,   ///< Tag
	TAGNIL_CLIENT    = 1 << 1,   ///< Client name
	TAGNIL_VERSION   = 1 << 2,   ///< Client version
	TAGNIL_MODE      = 1 << 3,   ///< Mode
	TAGNIL_UNREG     = 1 << 4,   ///< Usual hubs
	TAGNIL_REG       = 1 << 5,   ///< Reg hubs
	TAGNIL_OP        = 1 << 6,   ///< Op hubs
	TAGNIL_SLOT      = 1 << 7,   ///< Slots
	TAGNIL_LIMIT     = 1 << 8,   ///< Limit
	TAGNIL_OPEN      = 1 << 9,   ///< Open
	TAGNIL_BANDWIDTH = 1 << 10,  ///< Bandwidth
	TAGNIL_DOWNLOAD  = 1 << 11,  ///< Download
	TAGNIL_FRACTION  = 1 << 12,  ///< Fraction
}; // enum TagNil



class DcConnBase;



// ================ DcUserBase ================

/** Base DC user */
class DcUserBase {

public:

	/// Connection
	DcConnBase * mDcConnBase;

public:

	virtual ~DcUserBase() {
	}

	/// Get user's nick
	virtual const string & getUid() const = 0;

	/// User in user-list
	virtual bool getInUserList() const = 0;


	/// User in op-list (has op-key)
	virtual bool getInOpList() const = 0;

	/// Add user in op-list
	virtual void setInOpList(bool) = 0;


	/// User in ip-list (can receive ip addresses of users)
	virtual bool getInIpList() const = 0;

	/// Add user in ip-list
	virtual void setInIpList(bool) = 0;


	/// User is hidden
	virtual bool getHide() const = 0;

	/// Hide the user
	virtual void setHide(bool) = 0;


	/// User can redirect
	virtual bool getForceMove() const = 0;

	/// Redirect flag (user can redirect)
	virtual void setForceMove(bool) = 0;


	/// User can kick
	virtual bool getKick() const = 0;

	/// Kick flag (user can kick)
	virtual void setKick(bool) = 0;


	/// Get user's profile
	virtual int getProfile() const = 0;

	/// Set client profile
	virtual void setProfile(int) = 0;


	/// Get user's MyINFO cmd
	virtual const string & getMyInfo(/*bool real = false*/) const = 0;

	/// Set user's MyINFO cmd
	virtual bool setMyInfo(const string & myInfo) = 0;


	/// Get user's description
	virtual const string & getDesc(/*bool real = false*/) const = 0;

	/// Get user's email address
	virtual const string & getEmail(/*bool real = false*/) const = 0;

	/// Get user's connection flag
	virtual const string & getConnection(/*bool real = false*/) const = 0;

	/// Get user's magic byte
	virtual unsigned getByte(/*bool real = false*/) const = 0;

	/// Get user's share size
	virtual __int64 getShare(/*bool real = false*/) const = 0;


	/// Get user's tag
	virtual const string & getTag(/*bool real = false*/) const = 0;

	/// Get user's client
	virtual const string & getClient(/*bool real = false*/) const = 0;

	/// Get user's client version
	virtual const string & getClientVersion(/*bool real = false*/) const = 0;

	/// Get user's mode
	virtual const string & getMode(/*bool real = false*/) const = 0;

	/// Get user's unreg-hubs
	virtual unsigned getUnregHubs(/*bool real = false*/) const = 0;

	/// Get user's reg-hubs
	virtual unsigned getRegHubs(/*bool real = false*/) const = 0;

	/// Get user's op-hubs
	virtual unsigned getOpHubs(/*bool real = false*/) const = 0;

	/// Get user's slots
	virtual unsigned getSlots(/*bool real = false*/) const = 0;

	/// Get user's L-limit
	virtual unsigned getLimit(/*bool real = false*/) const = 0;

	/// Get user's O-limit
	virtual unsigned getOpen(/*bool real = false*/) const = 0;

	/// Get user's B-limit
	virtual unsigned getBandwidth(/*bool real = false*/) const = 0;

	/// Get user's D-limit
	virtual unsigned getDownload(/*bool real = false*/) const = 0;

	/// Get user's F-limit
	virtual const string & getFraction(/*bool real = false*/) const = 0;


	/// Get user's tagNil param
	virtual unsigned int getTagNil(/*bool real = false*/) const = 0;


	/// Get some user data
	virtual const string & getData() const = 0;

	/// Set some user data
	virtual void setData(const string &) = 0;

	/// Get all support cmd parameters, except of cmd name (PROTOCOL NMDC)
	virtual const string & getSupports() const = 0;

	/// User's protocol version
	virtual const string & getVersion() const = 0;

	/// Disconnect this client
	virtual void disconnect() = 0;


	/// Get string of IP
	virtual const string & getIp() const = 0;

	/// Get string of server ip (host)
	virtual const string & getIpConn() const = 0;

	/// Get enter time (in unix time sec)
	virtual long getConnectTime() const = 0;

	/// Get real clients port
	virtual int getPort() const = 0;

	/// Get connection port
	virtual int getPortConn() const = 0;

	/// Get mac address
	virtual const string & getMacAddress() const = 0;

}; // class DcUserBase



/** Base DC connection */
class DcConnBase {

public:

	/// User
	DcUserBase * mDcUserBase;

	/// Connection type (for protection and compatibility)
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

	/// Disconnect this client
	virtual void disconnect() {
		if (mDcUserBase != NULL) {
			mDcUserBase->disconnect();
		}
	}


	virtual bool parseCommand(const char * cmd) = 0;

	virtual const char * getCommand() = 0;

}; // class DcConnBase



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

	/// Get main hub path
	virtual const string & getMainDir() const = 0;

	/// Get system date-time string (now)
	virtual const string & getTime() = 0;

	/// Get name and version of the hub
	virtual const string & getHubInfo() const = 0;

	/// Get main locale
	virtual const string & getLocale() const = 0;

	/// Get OS name and version
	virtual const string & getSystemVersion() const = 0;

	/// Get time in milliseconds (now)
	virtual int getMSec() const = 0;

	/// Get hub work time in sec
	virtual int getUpTime() const = 0;

	/// Get total hub users count
	virtual int getUsersCount() const = 0;

	/// Get total hub share size
	virtual __int64 getTotalShare() const = 0;

	/// Send comand to user
	virtual bool sendToUser(
		DcUserBase *,
		const char * data,
		const char * nick = NULL,
		const char * from = NULL
	) = 0;

	/// Send comand to nick
	virtual bool sendToNick(
		const char * to,
		const char * data,
		const char * nick = NULL,
		const char * from = NULL
	) = 0;

	/// Send comand to all
	virtual bool sendToAll(
		const char * data,
		const char * nick = NULL,
		const char * from = NULL
	) = 0;

	/// Send comand to profiles
	virtual bool sendToProfiles(
		unsigned long profile,
		const char * data,
		const char * nick = NULL,
		const char * from = NULL
	) = 0;

	/// Send comand to ip
	virtual bool sendToIp(
		const char * ip,
		const char * data,
		unsigned long profile = 0,
		const char * nick = NULL,
		const char * from = NULL
	) = 0;

	/// Send comand to all except nicks
	virtual bool sendToAllExceptNicks(
		const vector<string> & nickList,
		const char * data,
		const char * nick = NULL,
		const char * from = NULL
	) = 0;

	/// Send comand to all except ips
	virtual bool sendToAllExceptIps(
		const vector<string> & ipList,
		const char * data,
		const char * nick = NULL,
		const char * from = NULL
	) = 0;


	/// Redirection client
	virtual void forceMove(
		DcUserBase *,
		const char * address,
		const char * reason = NULL
	) = 0;


	/// Get conn base by ip
	virtual const vector<DcConnBase*> & getDcConnBase(const char * ip) = 0;

	/// Get user base by nick
	virtual DcUserBase * getDcUserBase(const char * nick) = 0;

	/// Get iterator of conn base
	virtual DcConnListIterator * getDcConnListIterator() = 0;


	/// Get all configs names
	virtual const vector<string> & getConfig() = 0;

	/// Get config value by name
	virtual const char * getConfig(const string & name) = 0;

	/// Get lang value by name
	virtual const char * getLang(const string & name) = 0;

	/// Set config value by name
	virtual bool setConfig(const string & name, const string & value) = 0;

	/// Set lang value by name
	virtual bool setLang(const string & name, const string & value) = 0;


	/// Registration bot
	virtual int regBot(
		const string & nick,
		const string & myInfo,
		const string & ip,
		bool key = true
	) = 0;

	/// Unreg bot
	virtual int unregBot(const string & nick) = 0;


	/// Stop hub
	virtual void stopHub() = 0;

	/// Restarting hub
	virtual void restartHub() = 0;


}; // class DcServerBase


}; // namespace dcserver



namespace webserver {


// ================ WebUserBase ================

class WebUserBase {

public:

	/// Connection type (for protection and compatibility)
	const int mType;

public:

	WebUserBase(int type) :
		mType(type)
	{
	}

	virtual ~WebUserBase() {
	}

	virtual WebUserBase & operator = (const WebUserBase &) {
		return *this;
	}

	virtual const char * getCommand() = 0;

	/// Disconnect this client
	virtual void disconnect() = 0;

}; // class WebUserBase


}; // namespace webserver



namespace plugin {

using namespace ::dcserver;
using namespace ::webserver;

class Plugin;


// ================ PluginListBase ================

/** Base plugin list */
class PluginListBase {

public:

	/// Get plugins path
	virtual const string & getPluginDir() const = 0;

	/// Reg plugin in list with id
	virtual bool regCallList(const char * id, Plugin *) = 0;

	/// Unreg plugin from list with id
	virtual bool unregCallList(const char * id, Plugin *) = 0;

}; // class PluginListBase


// ================ Plugin ================

/** Base plugin */
class Plugin {

public:

	/// Version of plugin interface
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

	/// Reg function in all call lists
	virtual bool regAll(PluginListBase *) = 0;

	/// OnLoad plugin function
	virtual void onLoad(DcServerBase *) {
	}


	/// Events

	virtual int onUserConnected(DcUserBase *) {
		return 1;
	}

	virtual int onUserDisconnected(DcUserBase *) {
		return 1;
	}

	virtual int onUserEnter(DcUserBase *) {
		return 1;
	}

	virtual int onUserExit(DcUserBase *) {
		return 1;
	}

	virtual int onSupports(DcUserBase *) {
		return 1;
	}

	virtual int onKey(DcUserBase *) {
		return 1;
	}

	virtual int onValidateNick(DcUserBase *) {
		return 1;
	}

	virtual int onMyPass(DcUserBase *) {
		return 1;
	}

	virtual int onVersion(DcUserBase *) {
		return 1;
	}

	virtual int onGetNickList(DcUserBase *) {
		return 1;
	}

	virtual int onMyINFO(DcUserBase *) {
		return 1;
	}

	virtual int onChat(DcUserBase *) {
		return 1;
	}

	virtual int onTo(DcUserBase *) {
		return 1;
	}

	virtual int onConnectToMe(DcUserBase *) {
		return 1;
	}

	virtual int onRevConnectToMe(DcUserBase *) {
		return 1;
	}

	virtual int onSearch(DcUserBase *) {
		return 1;
	}

	virtual int onSR(DcUserBase *) {
		return 1;
	}

	virtual int onKick(DcUserBase *) {
		return 1;
	}

	virtual int onOpForceMove(DcUserBase *) {
		return 1;
	}

	virtual int onGetINFO(DcUserBase *) {
		return 1;
	}

	virtual int onMCTo(DcUserBase *) {
		return 1;
	}

	virtual int onTimer() {
		return 1;
	}

	virtual int onAny(DcUserBase *, int) {
		return 1;
	}

	virtual int onUnknown(DcUserBase *) {
		return 1;
	}

	virtual int onFlood(DcUserBase *, int, int) {
		return 1;
	}

	virtual int onWebData(WebUserBase *) {
		return 1;
	}


	/// Get name of plugin
	const string & getName() const {
		return mName;
	}

	/// Get version of plugin
	const string & getVersion() const {
		return mVersion;
	}

	/// Destruction of plugin
	void suicide() {
		mIsAlive = false;
	}

	/// Check state
	bool isAlive() const {
		return mIsAlive;
	}

	/// Set plugin-list for plugin
	void setPluginList(PluginListBase * pluginListBase) {
		mPluginListBase = pluginListBase;
	}

	/// Get plugins path
	virtual const string & getPluginDir() const {
		return mPluginListBase->getPluginDir();
	}

protected:

	/// Name of the plugin
	string mName;

	/// Version of the plugin
	string mVersion;

private:

	/// State of plugin (loaded or not loaded)
	bool mIsAlive;

	/// Pointer to list of all plugins
	PluginListBase * mPluginListBase;


}; // class Plugin

}; // namespace plugin

#endif // PLUGIN_H

/**
 * $Id$
 * $HeadURL$
 */
