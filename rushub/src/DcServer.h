/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz
 *
 * modified: 27 Aug 2009
 * Copyright (C) 2009-2012 by Setuper
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

#ifndef DC_SERVER_H
#define DC_SERVER_H

#include "Server.h" // first (def winsock2.h)
#include "DcConn.h"
#include "NmdcProtocol.h"
#include "AdcProtocol.h"
#include "Plugin.h"

#include "DcConfig.h"
#include "UserList.h"
#include "PluginList.h"

#include "DcConfigLoader.h"
#include "AntiFlood.h"
#include "DcIpList.h"
#include "WebProtocol.h"
#include "WebConn.h"

#include "stringutils.h" // for stringReplace

#include <istream> // for istringstream

using namespace ::plugin;
using namespace ::webserver::protocol;

namespace dcserver {

using namespace ::server; // for Server
using namespace ::dcserver::protocol;
using namespace ::utils; // for stringReplace



/// Busy indicators
typedef enum {

	SYSTEM_LOAD_OK,         ///< OK
	SYSTEM_LOAD_LOWER,      ///< LOWER
	SYSTEM_LOAD_MIDDLE,     ///< MIDDLE
	SYSTEM_LOAD_CRITICAL,   ///< CRITICAL
	SYSTEM_LOAD_SYSTEM_DOWN ///< SYSTEM_DOWN

} SystemLoad;


enum {
	USER_LIST_NICK,
	USER_LIST_MYINFO,
	USER_LIST_IP
};



class DcServer;
class DcConfigLoader;



/// List for get users into plugins
class DcListIterator : public DcConnListIterator {

public:

	DcListIterator(DcServer * dcServer);

	~DcListIterator() {
	}

	virtual DcConnBase * operator() (void) {
		if (mIt == mEnd) {
			return NULL;
		}
		DcConn * dcConn = static_cast<DcConn *> (*mIt);
		if (!dcConn) {
			++mIt;
			return this->operator() ();
		}
		++mIt;
		return dcConn;
	}

private:

	list<Conn *>::iterator mIt, mEnd;

}; // DcListIterator



/// Main DC Server class
class DcServer : public Server, public DcServerBase, private NonCopyable {

	friend class ::dcserver::DcListIterator; // for mClientList
	friend class ::dcserver::DcConn; // for minDelay in DcConn::onTimer
	friend class ::dcserver::DcConnFactory; // for removeFromDcUserList in DcConnFactory::deleteConn
	friend class ::dcserver::protocol::NmdcProtocol; // for beforeUserEnter
	friend class ::dcserver::protocol::AdcProtocol; // for beforeUserEnter
	friend class ::webserver::WebConnFactory; // for call plugins

public:

	typedef vector<string> List_t;

	/// Current server
	static DcServer * currentDcServer;

	/// Config loader
	DcConfigLoader mDcConfigLoader;

	/// Config settings of the server
	DcConfig mDcConfig;

	/// Settings of language
	DcLang mDcLang;

	SystemLoad mSystemLoad; ///< Indicator of the system overloading
	static string mSysVersion; ///< Verion of OS System

	Time mStartTime; ///< Start time of the hub
	NmdcProtocol mNmdcProtocol; ///< NMDC Protocol
	AdcProtocol mAdcProtocol; ///< ADC Protocol

	UserList mDcUserList; ///< User list
	UserList mBotList; ///< Bot list
	UserList mOpList; ///< Op list
	UserList mIpList; ///< ip list
	UserList mActiveList; ///< Active user list
	UserList mHelloList; ///< Hello user list (NMDC)
	UserList mEnterList; ///< Enter list (NMDC)
	UserList mChatList; ///< Chat list

	UserList mAdcUserList; ///< ADC User list

	int miTotalUserCount; ///< Total number of the users
	__int64 miTotalShare; ///< Total hub share size

	PluginList mPluginList;

	string mTimeBuf; ///< Time buffer for plugins

	DcIpList * mIpListConn; ///< IP list of connections

public:

	DcServer(const string & configFile, const string & execFile);

	virtual ~DcServer();

	virtual const string & getMainDir() const;

	virtual const string & getLogDir() const;

	virtual const string & getTime();

	virtual const string & getHubInfo() const;

	virtual const string & getLocale() const;

	virtual const string & getSystemVersion() const;

	virtual __int64 getMsec() const;

	/// Work time (sec)
	virtual int getUpTime() const;

	virtual int getUsersCount() const;

	virtual __int64 getTotalShare() const;
	
	virtual DcConnListIterator * getDcConnListIterator();

	virtual const vector<DcConnBase*> & getDcConnBase(const char * ip);
	virtual DcUserBase * getDcUserBase(const char * nick);

	virtual bool sendToUser(DcUserBase *, const char * data, const char * nick = NULL, const char * from = NULL);
	virtual bool sendToNick(const char * to, const char * data, const char * nick = NULL, const char * from = NULL);
	virtual bool sendToAll(const char * data, const char * nick = NULL, const char * from = NULL);
	virtual bool sendToProfiles(unsigned long profile, const char * data, const char * nick = NULL, const char * from = NULL);
	virtual bool sendToIp(const char * ip, const char * data, unsigned long profile = 0, const char * nick = NULL, const char * from = NULL);
	virtual bool sendToAllExceptNicks(const vector<string> & nickList, const char * data, const char * nick = NULL, const char * from = NULL);
	virtual bool sendToAllExceptIps(const vector<string> & ipList, const char * data, const char * nick = NULL, const char * from = NULL);

	virtual void forceMove(DcUserBase *, const char * address, const char * reason = NULL); ///< Redirection client

	virtual const vector<string> & getConfig();
	virtual const char * getConfig(const string & name);
	virtual const char * getLang(const string & name);
	virtual bool setConfig(const string & name, const string & value);
	virtual bool setLang(const string & name, const string & value);

	virtual int regBot(const string & nick, const string & info, const string & ip, bool key = true);
	virtual int unregBot(const string & nick);

	virtual void stopHub() {
		stop(0);
	}

	virtual void restartHub() {
		stop(1);
	}



	static void getNormalShare(__int64, string &); ///< Get normal share size
	static void getAddresses(const char * addresses, vector<pair<string, string> > &, const char * defaultPort);

	/// Pointer on the user (or NULL)
	DcUser * getDcUser(const char * nick);

	/// Listebing of ports
	int listening();

	/// Function checks min interval
	bool minDelay(Time &, double sec);

	/// Function action when joining the client
	int onNewConn(Conn *);

protected:

	/// Returns pointer to line of the connection, in which will be recorded got data
	string * createCommandPtr(Conn *);

	/// Function of the processing enterring data
	void onNewData(Conn *, string *);

	void onNewUdpData(Conn *, Parser *);

	/// Antiflood function
	bool antiFlood(unsigned & iCount, Time &, const unsigned & countLimit, const double & timeLimit);

	/// Check nick used
	bool checkNick(DcConn *);

	/// Actions before user entry
	bool beforeUserEnter(DcConn *);

	/// User entry
	void doUserEnter(DcConn *);

	/// Adding user in the user list
	bool addToUserList(DcUser *);

	/// Removing user from the user list
	bool removeFromDcUserList(DcUser *);

private:

	Time mChecker; ///< Checking time
	string mHubName; ///< Hub name for plugins
	string mBuf; ///< Temp buffer
	vector<DcConnBase *> mIpConnList; ///< Conn with same ip for plugins
	vector<string> mConfigNameList; ///< Config names for plugins

	vector<ConnFactory *> mConnFactories; ///< Server Conn Factories

	/// Web Protocol
	WebProtocol * mWebProtocol;

	AntiFlood mIpEnterFlood;

	/*
	struct IpEnter {
		Time mTime;
		unsigned int mCount;
		IpEnter() : mTime(true), mCount(0) {
		}
	};

	// List recently came ip addresses
	typedef List<unsigned long, IpEnter *> tIPEnterList;

	tIPEnterList * mIpEnterList;
	*/

private:

	bool listeningServer(const char * name, const char * addresses, const char * defaultPort, ConnFactory * connFactory, bool udp = false);

	static string getSysVersion();

	bool setCapabilities();

	/// Main timer
	int onTimer(Time & now);

	/// Show user to all
	bool showUserToAll(DcUser *);

	/// Actions after user entry
	void afterUserEnter(DcConn *);

	/// Call list struct for registration actions into the plugins
	struct PluginCallList {

		PluginCallList(PluginList * pluginList) :
			mOnUserConnected(pluginList, "Conn", &Plugin::onUserConnected),
			mOnUserDisconnected(pluginList, "Disconn", &Plugin::onUserDisconnected),
			mOnUserEnter(pluginList, "Enter", &Plugin::onUserEnter),
			mOnUserExit(pluginList, "Exit", &Plugin::onUserExit),
			mOnSupports(pluginList, "Supports", &Plugin::onSupports),
			mOnKey(pluginList, "Key", &Plugin::onKey),
			mOnValidateNick(pluginList, "Validate", &Plugin::onValidateNick),
			mOnMyPass(pluginList, "MyPass", &Plugin::onMyPass),
			mOnVersion(pluginList, "Version", &Plugin::onVersion),
			mOnGetNickList(pluginList, "GetNickList", &Plugin::onGetNickList),
			mOnMyINFO(pluginList, "MyINFO", &Plugin::onMyINFO),
			mOnChat(pluginList, "Chat", &Plugin::onChat),
			mOnTo(pluginList, "To", &Plugin::onTo),
			mOnConnectToMe(pluginList, "CTM", &Plugin::onConnectToMe),
			mOnRevConnectToMe(pluginList, "RCTM", &Plugin::onRevConnectToMe),
			mOnSearch(pluginList, "Search", &Plugin::onSearch),
			mOnSR(pluginList, "SR", &Plugin::onSR),
			mOnKick(pluginList, "Kick", &Plugin::onKick),
			mOnOpForceMove(pluginList, "OpForce", &Plugin::onOpForceMove),
			mOnGetINFO(pluginList, "GetINFO", &Plugin::onGetINFO),
			mOnMCTo(pluginList, "MCTo", &Plugin::onMCTo),
			mOnTimer(pluginList, "Timer", &Plugin::onTimer),
			mOnAny(pluginList, "Any", &Plugin::onAny),
			mOnUnknown(pluginList, "Unknown", &Plugin::onUnknown),
			mOnFlood(pluginList, "Flood", &Plugin::onFlood),
			mOnWebData(pluginList, "WebData", &Plugin::onWebData)
		{
		}

		CallListUser       mOnUserConnected;
		CallListUser       mOnUserDisconnected;
		CallListUser       mOnUserEnter;
		CallListUser       mOnUserExit;
		CallListUser       mOnSupports;
		CallListUser       mOnKey;
		CallListUser       mOnValidateNick;
		CallListUser       mOnMyPass;
		CallListUser       mOnVersion;
		CallListUser       mOnGetNickList;
		CallListUser       mOnMyINFO;
		CallListUser       mOnChat;
		CallListUser       mOnTo;
		CallListUser       mOnConnectToMe;
		CallListUser       mOnRevConnectToMe;
		CallListUser       mOnSearch;
		CallListUser       mOnSR;
		CallListUser       mOnKick;
		CallListUser       mOnOpForceMove;
		CallListUser       mOnGetINFO;
		CallListUser       mOnMCTo;
		CallListSimple     mOnTimer;
		CallListUserInt    mOnAny;
		CallListUser       mOnUnknown;
		CallListUserIntInt mOnFlood;
		CallListWebUser    mOnWebData;

	} mCalls;

}; // class DcServer


}; // namespace dcserver

#endif // DC_SERVER_H

/**
 * $Id$
 * $HeadURL$
 */
