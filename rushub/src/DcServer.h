/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz

 * modified: 27 Aug 2009
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

#ifndef DC_SERVER_H
#define DC_SERVER_H

//#define WITHOUT_PLUGINS 1

#include "Server.h"
#include "DcConn.h"
#include "NmdcProtocol.h"
#include "Plugin.h"

#include "DcConfig.h"
#include "UserList.h"
#include "PluginList.h"

#include "DcConfigLoader.h"
#include "AntiFlood.h"
#include "DcIpList.h"

#include "stringutils.h" // for StringReplace

#include <istream> // for istringstream


using namespace ::plugin;

namespace dcserver {

using namespace ::server; /** for Server */
using namespace ::dcserver::protocol;
using namespace ::utils; /** for StringReplace */



/** Индикаторы загруженности хаба */
typedef enum {

	SYSTEM_LOAD_OK,         //< OK
	SYSTEM_LOAD_LOWER,      //< LOWER
	SYSTEM_LOAD_MIDDLE,     //< MIDDLE
	SYSTEM_LOAD_CRITICAL,   //< CRITICAL
	SYSTEM_LOAD_SYSTEM_DOWN //< SYSTEM_DOWN

} SystemLoad;



enum FloodType { /** Flood types */

	FLOOD_TYPE_NO = -1,
	FLOOD_TYPE_CHAT,
	FLOOD_TYPE_TO,
	FLOOD_TYPE_MYNIFO,
	FLOOD_TYPE_NICKLIST,
	FLOOD_TYPE_SEARCH,
	FLOOD_TYPE_SR,
	FLOOD_TYPE_CTM,
	FLOOD_TYPE_RCTM,
	FLOOD_TYPE_MCTO,
	FLOOD_TYPE_PING,
	FLOOD_TYPE_UNKNOWN

}; // enum FloodType



class DcServer;
class DcConfigLoader;



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

class DcServer : public Server, public DcServerBase {

	friend class ::dcserver::DcListIterator; // for mConnList
	friend class ::dcserver::DcConn; // for DoUserEnter in DcConn::onFlush and MinDelay in DcConn::onTimer
	friend class ::dcserver::DcConnFactory; // for RemoveFromDCUserList in DcConnFactory::deleteConn
	friend class ::dcserver::protocol::NmdcProtocol; // for BeforeUserEnter in NmdcProtocol::eventMyInfo
	friend class ::webserver::WebConnFactory; // for call plugins

public:

	typedef vector<string> List_t;
	typedef UserList::Key UserKey;

	/** Current server */
	static DcServer * currentDcServer;

	/** Config loader */
	DcConfigLoader mDcConfigLoader;

	/** Config settings of the server */
	DcConfig mDcConfig;

	/** Settings of language */
	DcLang mDCLang;

	SystemLoad mSystemLoad; /** Indicator of the system overloading */
	static string mSysVersion; /** Verion of OS System */

	Time mStartTime; /** Start time of the hub */
	NmdcProtocol mDcProtocol; /** DC Protocol */

	FullUserList mDcUserList; /** User list */
	UserList mBotList; /** Bot list */
	UserList mOpList; /** Op list */
	UserList mIpList; /** ip list */
	UserList mActiveList; /** Active user list */
	UserList mHelloList; /** Hello user list */
	UserList mEnterList; /** Enter list */
	UserList mChatList; /** Chat list */

	int miTotalUserCount; /** Total number of the users */
	__int64 miTotalShare; /** Total hub share size */

	PluginList mPluginList;

	string mTimeBuf; /** Time buffer for plugins */

	DcIpList * mIPListConn; /** IP list of connections */

public:

	DcServer(const string & configFile, const string & execFile);

	virtual ~DcServer();

	virtual DcServer & operator = (const DcServer &) {
		return *this;
	}

	const string & getMainDir() const {
		return mDcConfig.mMainPath;
	}

	const string & getTime() {
		stringstream oss;
		oss << mTime.AsDate();
		mTimeBuf = oss.str();
		return mTimeBuf;
	}

	const string & getHubInfo() const {
		return mHubName;
	}

	const string & getLocale() const {
		return mDcConfig.mLocale;
	}

	const string & getSystemVersion() const {
		return mSysVersion;
	}

	int getMSec() const {
		Time tm;
		return tm;
	}

	/** Work time (sec) */
	int getUpTime() const {
		Time tm;
		tm -= mStartTime;
		return tm.Sec();
	}

	int getUsersCount() const {
		return miTotalUserCount;
	}

	__int64 getTotalShare() const {
		return miTotalShare;
	}

	int checkCmd(const string & cmd, DcUserBase * dcUserBase = NULL);

	/** Listebing of ports */
	int Listening();

	/** Main timer */
	int onTimer(Time &now);

	/** Function checks min interval */
	bool MinDelay(Time &then, double sec);

	/** Pointer on the user (or NULL) */
	DcUser * GetDCUser(const char *sNick);
	const vector<DcConnBase*> & getDcConnBase(const char * sIP);
	DcUserBase * getDcUserBase(const char *sNick);

	DcConnListIterator * getDcConnListIterator() {
		return new DcListIterator(this);
	}

	bool sendToUser(DcConnBase * dcConnBase, const char * sData, const char * sNick = NULL, const char * sFrom = NULL);
	bool sendToNick(const char * sTo, const char * sData, const char * sNick = NULL, const char * sFrom = NULL);
	bool sendToAll(const char * sData, const char * sNick = NULL, const char * sFrom = NULL);
	bool sendToProfiles(unsigned long iProfile, const char * sData, const char * sNick = NULL, const char * sFrom = NULL);
	bool sendToIp(const char * sIP, const char * sData, unsigned long iProfile = 0, const char * sNick = NULL, const char * sFrom = NULL);
	bool sendToAllExceptNicks(const vector<string> & NickList, const char * sData, const char * sNick = NULL, const char * sFrom = NULL);
	bool sendToAllExceptIps(const vector<string> & IPList, const char * sData, const char * sNick = NULL, const char * sFrom = NULL);

	void forceMove(DcConnBase * dcConnBase, const char * sAddress, const char * sReason = NULL); //< Redirection client

	const vector<string> & getConfig();
	const char * getConfig(const string & sName);
	const char * getLang(const string & sName);
	bool setConfig(const string & sName, const string & sValue);
	bool setLang(const string & sName, const string & sValue);

	int regBot(const string & sNick, const string & sMyINFO, const string & sIP, bool bKey = true);
	int unregBot(const string & sNick);

	void stopHub() {
		stop(0);
	}

	void restartHub() {
		stop(1);
	}

	static void getAddresses(const string & sAddresses, vector<pair<string, int> > & vec, int iDefPort);

protected:

	/** Function action when joining the client */
	int onNewConn(Conn *);

	/** Returns pointer to line of the connection, in which will be recorded got data */
	string * getPtrForStr(Conn *);

	/** Function of the processing enterring data */
	void onNewData(Conn *, string *);

	/** Antiflood function */
	bool antiFlood(unsigned &iCount, Time & time, const unsigned &iCountLimit, const double &iTimeLimit);

	/** Check nick used */
	bool CheckNick(DcConn *dcConn);

	/** Actions before user entry */
	bool BeforeUserEnter(DcConn *);

	/** User entry */
	void DoUserEnter(DcConn *);

	/** Adding user in the user list */
	bool AddToUserList(DcUser *);

	/** Removing user from the user list */
	bool RemoveFromDCUserList(DcUser *);

	/** Show user to all */
	bool ShowUserToAll(DcUser *);

	/** Actions after user entry */
	void AfterUserEnter(DcConn *);

	/** Close server */
	void close();

private:

	Time mChecker; /** Checking time */
	tCLIt conn_it; /** Iterator for optimum */
	string mHubName; /** Hub name for plugins */
	string sBuf; /** Temp buffer */
	vector<DcConnBase *> mvIPConn; /** Conn with same ip for plugins */
	vector<string> mvConfigNames; /** Config names for plugins */

	ListenFactory * mListenFactory;
	WebListenFactory * mWebListenFactory;

	AntiFlood mIPEnterFlood;
	struct IpEnter {
		Time mTime;
		unsigned miCount;
		IpEnter() : miCount(0) {
		}
	};

	/** List recently came ip addresses */
	typedef List<unsigned long, IpEnter *> tIPEnterList;

	tIPEnterList * mIPEnterList;

private:

	void OnNewUdpData(Conn * conn, string * data);

	void deleteConn(Conn * conn);

	bool ListeningServer(const char * name, const string & addresses, unsigned port, ListenFactory * listenFactory, bool udp = false);

	static string getSysVersion();

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

		CallListConnection    mOnUserConnected;
		CallListConnection    mOnUserDisconnected;
		CallListConnection    mOnUserEnter;
		CallListConnection    mOnUserExit;
		CallListConnParser    mOnSupports;
		CallListConnParser    mOnKey;
		CallListConnParser    mOnValidateNick;
		CallListConnParser    mOnMyPass;
		CallListConnParser    mOnVersion;
		CallListConnParser    mOnGetNickList;
		CallListConnParser    mOnMyINFO;
		CallListConnParser    mOnChat;
		CallListConnParser    mOnTo;
		CallListConnParser    mOnConnectToMe;
		CallListConnParser    mOnRevConnectToMe;
		CallListConnParser    mOnSearch;
		CallListConnParser    mOnSR;
		CallListConnParser    mOnKick;
		CallListConnParser    mOnOpForceMove;
		CallListConnParser    mOnGetINFO;
		CallListConnParser    mOnMCTo;
		CallListSimple        mOnTimer;
		CallListConnParser    mOnAny;
		CallListConnParser    mOnUnknown;
		CallListConnIntInt    mOnFlood;
		CallListConnWebParser mOnWebData;

	} mCalls;

}; // class DcServer

}; // namespace dcserver

#endif // DC_SERVER_H
