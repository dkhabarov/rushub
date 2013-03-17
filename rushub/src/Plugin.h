/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * Copyright (C) 2009-2013 by Setuper
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
#include <sstream>

#ifdef _WIN32
	#ifndef STD_TYPES
		#define STD_TYPES
		typedef unsigned __int64 uint64_t;
		typedef unsigned __int32 uint32_t;
		typedef unsigned __int16 uint16_t;
		typedef unsigned __int8 uint8_t;
		typedef __int64 int64_t;
		typedef __int32 int32_t;
		typedef __int16 int16_t;
		typedef __int8 int8_t;
	#endif // STD_TYPES
#else
	#include <sys/types.h>
	#include <stdint.h>
#endif

namespace plugin {
	class Plugin;
}

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

// Internal plugin version
#ifndef INTERNAL_PLUGIN_VERSION
	#define INTERNAL_PLUGIN_VERSION 10044
#endif

// NMDC protocol separator
#ifndef NMDC_SEPARATOR
	#define NMDC_SEPARATOR "|"
	#define NMDC_SEPARATOR_LEN 1
#endif

// ADC protocol separator
#ifndef ADC_SEPARATOR
	#define ADC_SEPARATOR "\n"
	#define ADC_SEPARATOR_LEN 1
#endif

// Web protocol separator
#ifndef WEB_SEPARATOR
	#define WEB_SEPARATOR "\r\n\r\n"
	#define WEB_SEPARATOR_LEN 4
#endif

enum DcProtocolType {
	DC_PROTOCOL_TYPE_ALL = -1,
	DC_PROTOCOL_TYPE_NMDC,
	DC_PROTOCOL_TYPE_ADC,
	DC_PROTOCOL_TYPE_SIZE // used as a size only!
};



/// DC Server namespace
namespace dcserver {

enum ClientType {
	CLIENT_TYPE_DC = 1000,
	CLIENT_TYPE_WEB = 1001
}; // enum ClientType


#define USER_PARAM_DESC "sDesc"
#define USER_PARAM_TAG "sTag"
#define USER_PARAM_CLIENT_NAME "sClientName"
#define USER_PARAM_CLIENT_VERSION "sClientVersion"
#define USER_PARAM_MODE "sMode"
#define USER_PARAM_UNREG_HUBS "iUsHubs"
#define USER_PARAM_REG_HUBS "iRegHubs"
#define USER_PARAM_OP_HUBS "iOpHubs"
#define USER_PARAM_SLOTS "iSlots"
#define USER_PARAM_LIMIT "iLimit"
#define USER_PARAM_OPEN "iOpen"
#define USER_PARAM_BANDWIDTH "iBandwidth"
#define USER_PARAM_DOWNLOAD "iDownload"
#define USER_PARAM_FRACTION "sFraction"
#define USER_PARAM_CONNECTION "sConn"
#define USER_PARAM_BYTE "iByte"
#define USER_PARAM_EMAIL "sEmail"
#define USER_PARAM_SHARE "iShare"
#define USER_PARAM_SUPPORTS "sSupports"
#define USER_PARAM_VERSION "sVersion"

#define USER_PARAM_NICK "sNick"
#define USER_PARAM_PROFILE "iProfile"
#define USER_PARAM_IP "sIP"
#define USER_PARAM_IP_CONN "sIPConn"
#define USER_PARAM_PORT "iPort"
#define USER_PARAM_PORT_CONN "iPortConn"
#define USER_PARAM_MAC_ADDRESS "sMacAddress"
#define USER_PARAM_ENTER_TIME "iEnterTime"
#define USER_PARAM_CAN_KICK "bKick"
#define USER_PARAM_CAN_REDIRECT "bRedirect"
#define USER_PARAM_CAN_HIDE "bHide"
#define USER_PARAM_IN_USER_LIST "bInUserList"
#define USER_PARAM_IN_IP_LIST "bInIpList"
#define USER_PARAM_IN_OP_LIST "bInOpList"



/// Base param class
class ParamBase {

public:

	enum {
		TYPE_NONE = 0,
		TYPE_STRING,
		TYPE_INT,
		TYPE_BOOL,
		TYPE_DOUBLE,
		TYPE_LONG,
		TYPE_INT64
	};

public:

	virtual ~ParamBase() {
	}

	virtual const string & getName() const = 0;
	virtual int getType() const = 0;
	virtual const string & toString() = 0;

	virtual const string & getString() const = 0;
	virtual int setString(const string &) = 0;
	virtual const int & getInt() const = 0;
	virtual int setInt(int) = 0;
	virtual const bool & getBool() const = 0;
	virtual int setBool(bool) = 0;
	virtual const double & getDouble() const = 0;
	virtual int setDouble(double) = 0;
	virtual const long & getLong() const = 0;
	virtual int setLong(long) = 0;
	virtual const int64_t & getInt64() const = 0;
	virtual int setInt64(int64_t) = 0;

}; // class ParamBase



// ================ DcUserBase ================

/** Base DC user */
class DcUserBase {

public:

	const int mType; ///< Type (for protection and compatibility)

public:

	DcUserBase(int type) :
		mType(type)
	{
	}

	virtual ~DcUserBase() {
	}

	virtual ParamBase * getParam(const char * name) const = 0;
	virtual ParamBase * getParamForce(const char * name) = 0;
	virtual bool removeParam(const char * name) = 0;

	/// Disconnect this client
	virtual void disconnect() = 0;

	/// Get user's Nick
	virtual const string & getNick() const = 0;

	/// Get user's SID
	virtual const string & getSid() const = 0;

	virtual const string & getNmdcTag() = 0;

	/// Get user's Info string
	virtual const string & getInfo() = 0;

	/// Set user's MyINFO cmd
	virtual bool setInfo(const string & info) = 0;

	/// Check cmd syntax
	virtual bool parseCommand(const char * cmd) = 0;

	/// Get command
	virtual const char * getCommand() = 0;

private:

	DcUserBase & operator = (const DcUserBase &);

}; // class DcUserBase



// ================ DcListIteratorBase ================

/** Iterator for list of base connections */
class DcListIteratorBase {

public:

	virtual ~DcListIteratorBase() {
	}

	virtual DcUserBase * operator() (void) = 0;

}; // class DcListIteratorBase



// ================ DcServerBase ================

/** Base DC server */
class DcServerBase {

public:

	virtual ~DcServerBase() {
	}

	/// Get main hub path
	virtual const string & getMainDir() const = 0;

	/// Get logs path
	virtual const string & getLogDir() const = 0;

	/// Get plugins path
	virtual const string & getPluginDir() const = 0;

	/// Get system date-time string (now)
	virtual const string & getTime() = 0;

	/// Get name and version of the hub
	virtual const string & getHubInfo() const = 0;

	/// Get main locale
	virtual const string & getLocale() const = 0;

	/// Get OS name and version
	virtual const string & getSystemVersion() const = 0;

	/// Get time in milliseconds (now)
	virtual int64_t getMsec() const = 0;

	/// Get hub work time in sec
	virtual int getUpTime() const = 0;

	/// Get total hub users count
	virtual int getUsersCount() const = 0;

	/// Get total hub share size
	virtual int64_t getTotalShare() const = 0;

	/// Get iterator of conn base
	virtual DcListIteratorBase * getDcListIterator() = 0;

	/// Get conn base by ip
	virtual const vector<DcUserBase *> & getDcUserBaseByIp(const char * ip) = 0;

	/// Get user base by nick
	virtual DcUserBase * getDcUserBase(const char * nick) = 0;

	/// Send command to user
	virtual bool sendToUser(
		DcUserBase *,
		const string & data,
		const char * nick = NULL,
		const char * from = NULL,
		bool flush = true
	) = 0;

	/// Send command to nick
	virtual bool sendToNick(
		const char * to,
		const string & data,
		const char * nick = NULL,
		const char * from = NULL,
		bool flush = true
	) = 0;

	/// Send command to all
	virtual bool sendToAll(
		const string & data,
		const char * nick = NULL,
		const char * from = NULL,
		bool flush = true
	) = 0;

	/// Send command to profiles
	virtual bool sendToProfiles(
		unsigned long profile,
		const string & data,
		const char * nick = NULL,
		const char * from = NULL,
		bool flush = true
	) = 0;

	/// Send command to ip
	virtual bool sendToIp(
		const string & ip,
		const string & data,
		unsigned long profile = 0,
		const char * nick = NULL,
		const char * from = NULL,
		bool flush = true
	) = 0;

	/// Send command to all except nicks
	virtual bool sendToAllExceptNicks(
		const vector<string> & nickList,
		const string & data,
		const char * nick = NULL,
		const char * from = NULL,
		bool flush = true
	) = 0;

	/// Send command to all except ips
	virtual bool sendToAllExceptIps(
		const vector<string> & ipList,
		const string & data,
		const char * nick = NULL,
		const char * from = NULL,
		bool flush = true
	) = 0;


	/// Redirection client
	virtual void forceMove(
		DcUserBase *,
		const char * address,
		const char * reason = NULL
	) = 0;

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
		const string & info,
		const string & ip,
		bool key = true
	) = 0;

	/// Unreg bot
	virtual int unregBot(const string & nick) = 0;


	/// Stop hub
	virtual void stopHub() = 0;

	/// Restarting hub
	virtual void restartHub() = 0;


	/// Reg plugin in list with id
	virtual bool regCallList(const char * id, Plugin *) = 0;

	/// Unreg plugin from list with id
	virtual bool unregCallList(const char * id, Plugin *) = 0;

}; // class DcServerBase



template<class F, class T> T & convert(const F & from, T & to, const T & def) {
    ostringstream oss;
    if (!(oss << from)) {
        return to = def;
    }
    istringstream iss(oss.str());
    if (!(iss >> to)) {
        return to = def;
    }
    return to;
}

} // namespace dcserver



namespace webserver {


// ================ WebUserBase ================


/// Base Web user
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

	virtual const char * getCommand() = 0;

	/// Disconnect this client
	virtual void disconnect() = 0;

private:

	WebUserBase & operator = (const WebUserBase &);

}; // class WebUserBase


} // namespace webserver



/// Plugin namespace
namespace plugin {

using namespace ::dcserver;
using namespace ::webserver;


// ================ Plugin ================

/** Base plugin */
class Plugin {

public:

	/// Version of plugin interface
	const int mInternalPluginVersion;

public:

	Plugin() : 
		mInternalPluginVersion(INTERNAL_PLUGIN_VERSION),
		mIsAlive(true)
	{
	}

	virtual ~Plugin() {
	}

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

protected:

	/// Name of the plugin
	string mName;

	/// Version of the plugin
	string mVersion;

private:

	/// State of plugin (loaded or not loaded)
	bool mIsAlive;

	Plugin & operator = (const Plugin &);

}; // class Plugin

} // namespace plugin

#endif // PLUGIN_H

/**
 * $Id$
 * $HeadURL$
 */
