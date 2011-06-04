/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz
 *
 * modified: 27 Aug 2009
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

#include "DcServer.h"
#include "WebConn.h"

#ifdef _WIN32
	#if defined(_MSC_VER) && (_MSC_VER >= 1400)
		#pragma warning(disable:4996) // Disable "This function or variable may be unsafe."
	#endif
#else
	#include <sys/utsname.h> // for utsname
#endif


using namespace ::webserver;


namespace dcserver {



DcListIterator::DcListIterator(DcServer * dcServer) : 
	mIt(dcServer->mConnList.begin()),
	mEnd(dcServer->mConnList.end())
{
}



DcServer * DcServer::currentDcServer = NULL;
string DcServer::mSysVersion;



DcServer::DcServer(const string & configFile, const string &):
	Server(),
	mDcConfig(&mDcConfigLoader, mServer, configFile.c_str()),
	mDcLang(&mDcConfigLoader, &mDcConfig),
	mSystemLoad(SYSTEM_LOAD_OK),
	mDcUserList("UserList", true, true, true),
	mBotList("BotList", true),
	mOpList("OpList", true),
	mIpList("IpList"),
	mActiveList("ActiveList"),
	mHelloList("HelloList"),
	mEnterList("EnterList"),
	mChatList("ChatList"),
	miTotalUserCount(0),
	miTotalShare(0),
	mPluginList(mDcConfig.mPluginPath),
	mHubName(INTERNALNAME " " INTERNALVERSION " " __DATE__ " " __TIME__),
	mDcConnFactory(NULL),
	mWebConnFactory(NULL),
	mWebProtocol(NULL),
	mIpEnterFlood(mDcConfig.mFloodCountReconnIp, mDcConfig.mFloodTimeReconnIp),
	mCalls(&mPluginList)
{
	SetClassName("DcServer");

	/** Current server */
	currentDcServer = this;

	/** Define OS */
	if (mSysVersion.empty()) {
		mSysVersion = getSysVersion();
	}

	/** DcIpList */
	mIpListConn = new DcIpList();

	mDcProtocol.setServer(this);
	mPluginList.setServer(this);

	mPluginList.loadAll(); /** Load plugins */


	mDcUserList.setNickListStart("$NickList ");
	mDcUserList.setNickListSeparator("$$");

	mOpList.setNickListStart("$OpList ");
	mOpList.setNickListSeparator("$$");


	if (mDcConfig.mRegMainBot) { /** Main bot registration */
		if (Log(3)) {
			LogStream() << "Reg main bot '" << mDcConfig.mHubBot << "'" << endl;
		}
		regBot(mDcConfig.mHubBot, mDcConfig.mMainBotMyinfo,
			mDcConfig.mMainBotIp, mDcConfig.mMainBotKey);
	}
}



DcServer::~DcServer() {
	if (Log(1)) {
		LogStream() << "Destruct DcServer" << endl;
	}

	if (mDcConfig.mRegMainBot) { /** Main bot unreg */
		if (Log(3)) {
			LogStream() << "Unreg main bot '" << mDcConfig.mHubBot << "'" << endl;
		}
		unregBot(mDcConfig.mHubBot);
	}

	DcUser * Us = NULL;
	UserList::iterator it, it_e = mDcUserList.end();	
	for (it = mDcUserList.begin(); it != it_e;) {
		Us = static_cast<DcUser *> (*it);
		++it;
		if (Us->mDcConn) {
			delConnection(Us->mDcConn);
		} else {
			if (Us->getInUserList()) {
				this->removeFromDcUserList(Us);
			}
			delete Us;
		}
	}

	// Remove users
	close();

	if (mIpListConn != NULL) {
		delete mIpListConn;
		mIpListConn = NULL;
	}

	if (mDcConnFactory != NULL) {
		delete mDcConnFactory;
		mDcConnFactory = NULL;
	}

	if (mWebConnFactory != NULL) {
		delete mWebConnFactory;
		mWebConnFactory = NULL;
	}

	if (mWebProtocol != NULL) {
		delete mWebProtocol;
		mWebProtocol = NULL;
	}
}



/** Close server */
void DcServer::close() {
	mRun = false;
	for (tCLIt it = mConnList.begin(); it != mConnList.end(); ++it) {
		deleteConn(*it);
	}
	for (tLLIt it = mListenList.begin(); it != mListenList.end(); ++it) {
		deleteConn(*it);
	}
}



void DcServer::deleteConn(Conn * conn) {
	if (conn != NULL) {
		mConnChooser.deleteConn(conn);

		ConnFactory * connFactory = conn->mConnFactory;
		if (connFactory != NULL && conn->getCreatedByFactory()) {
			connFactory->deleteConn(conn);
		} else {
			delete conn;
		}
		conn = NULL;
	}
}



void DcServer::getAddresses(
	const string & sAddresses,
	vector<pair<string, int> > & vec,
	int iDefPort
) {
	vector<string> vAddresses;
	stringSplit(sAddresses, ' ', vAddresses);
	for (vector<string>::iterator it = vAddresses.begin(); it != vAddresses.end(); ++it) {
		vector<string> vIpPort;
		stringSplit(*it, ':', vIpPort);
		if (2 < vIpPort.size() || vIpPort.size() < 1 || vIpPort[0].size() == 0) {
			continue;
		}
		int iPort = iDefPort;
		if (vIpPort.size() == 2) {
			iPort = atoi(vIpPort[1].c_str());
			if (iPort < 0 || iPort > 65535) {
				continue;
			}
		}
		vec.push_back(pair<string, int>(vIpPort[0], iPort));
	}
}



bool DcServer::listeningServer(const char * name, const string & addresses, unsigned port, ConnFactory * connFactory, bool udp /*= false*/) {
	vector<pair<string, int> > vAddresses;
	getAddresses(addresses, vAddresses, port);

	if (vAddresses.size() == 0) {
		if (ErrLog(0)) {
			LogStream() << "Incorrect address of the " << name << endl;
		}
	}

	bool ret = false;
	for (vector<pair<string, int> >::iterator it = vAddresses.begin(); it != vAddresses.end(); ++it) {
		if (Server::listening(connFactory, (*it).first, (*it).second, udp) == 0) {
			ret = true;
			if (Log(0)) {
				LogStream() << name << " is running on " 
					<< ((*it).first) << ":" << ((*it).second) 
					<< (udp ? " UDP" : " TCP") << endl;
			}
			cout << name << " is running on " 
				<< ((*it).first) << ":" << ((*it).second) 
				<< (udp ? " UDP" : " TCP") << endl;
		}
	}
	return ret;
}



/** Listening all servers */
int DcServer::listening() {

	if (!mDcConnFactory) {
		mDcConnFactory = new DcConnFactory(&mDcProtocol, this);
	}

	if (!mWebProtocol) {
		mWebProtocol = new WebProtocol(mDcConfig.mMaxWebCommandLength);
	}

	if (!mWebConnFactory) {
		mWebConnFactory = new WebConnFactory(mWebProtocol, this);
	}

	// DC Server
	if (!listeningServer("DC Server " INTERNALNAME " " INTERNALVERSION, mDcConfig.mAddresses, 411, mDcConnFactory)) {
		return -1;
	}

	// Web Server
	if (mDcConfig.mWebServer) {
		listeningServer("Web Server", mDcConfig.mWebAddresses, 80, mWebConnFactory);
	}

	// UDP DC Server
	if (mDcConfig.mUdpServer) {
		listeningServer("DC Server (UDP)", mDcConfig.mUdpAddresses, 1209, mDcConnFactory, true);
	}
	return 0;
}



int DcServer::onTimer(Time & now) {

	/** Execute each second */
	if (abs(int(now - mChecker)) >= mTimerServPeriod) {

		mChecker = now;

		mHelloList.flushCache();
		mDcUserList.flushCache();
		mBotList.flushCache();
		mEnterList.flushCache();
		mOpList.flushCache();
		mIpList.flushCache();
		mChatList.flushCache();
		mActiveList.flushCache();


		int SysLoading = mSystemLoad;

		if (0 < mMeanFrequency.mNumFill) {

			double iFrequency = mMeanFrequency.getMean(mTime);

			if (iFrequency < 0.4 * mDcConfig.mSysLoading) {
				mSystemLoad = SYSTEM_LOAD_SYSTEM_DOWN; // 0.4 (>2000 ms)
			} else if (iFrequency < 0.9 * mDcConfig.mSysLoading) {
				mSystemLoad = SYSTEM_LOAD_CRITICAL;    // 0.9 (>1000 ms)
			} else if (iFrequency < 3.6 * mDcConfig.mSysLoading) {
				mSystemLoad = SYSTEM_LOAD_MIDDLE;      // 3.6 (>250  ms)
			} else if (iFrequency < 18  * mDcConfig.mSysLoading) {
				mSystemLoad = SYSTEM_LOAD_LOWER;       // 18  (>50   ms)
			} else {
				mSystemLoad = SYSTEM_LOAD_OK;
			}

		} else {
			mSystemLoad = SYSTEM_LOAD_OK;
		}

		if (mSystemLoad != SysLoading) {
			if (Log(0)) {
				LogStream() << "System loading: " 
					<< mSystemLoad << " level (was " 
					<< SysLoading << " level)" << endl;
			}
		}

		mIpEnterFlood.del(now); // Removing ip addresses, which already long ago entered

		mDcUserList.autoResize();
		mBotList.autoResize();
		mHelloList.autoResize();
		mEnterList.autoResize();
		mActiveList.autoResize();
		mChatList.autoResize();
		mOpList.autoResize();
		mIpList.autoResize();
	}

	#ifndef WITHOUT_PLUGINS
		if (mCalls.mOnTimer.callAll()) {
			return 1;
		}
	#endif

	return 0;
}



/** Function action after joining the client */
int DcServer::onNewConn(Conn *conn) {
	DcConn * dcConn = static_cast<DcConn *> (conn);

	if (mSystemLoad == SYSTEM_LOAD_SYSTEM_DOWN) {
		if (dcConn->Log(1)) {
			dcConn->LogStream() << "System down, close" << endl;
		}
		dcConn->closeNow(CLOSE_REASON_HUB_LOAD);
		return -1;
	}

	/** Checking flood-entry (by ip) */
	if (mIpEnterFlood.check(dcConn->getNetIp(), mTime)) {
		dcConn->closeNow(CLOSE_REASON_FLOOD_IP_ENTRY);
		return -2;
	}

	if (dcConn->Log(5)) {
		dcConn->LogStream() << "[S]Stage onNewConn" << endl;
	}

	mDcProtocol.onNewDcConn(dcConn);

	if (dcConn->Log(5)) {
		dcConn->LogStream() << "[E]Stage onNewConn" << endl;
	}
	return 0;
}



/** Returns pointer to line of the connection, in which will be recorded got data */
string * DcServer::createCommandPtr(Conn * conn) {
	return conn->getParserCommandPtr();
}



/** Function of the processing enterring data */
void DcServer::onNewData(Conn * conn, string * data) {

	if (conn->Log(4)) {
		conn->LogStream() << "IN: " << (*data) << endl;
	}

	// Protocol parser
	Parser * parser = conn->mParser;

	// ToDo Parser == NULL ?
	// ToDo Protocol == NULL ?

	if (parser != NULL) {

		// Parser
		parser->parse();

		// UDP data
		if (conn->getConnType() == CONN_TYPE_CLIENTUDP) {
			onNewUdpData(conn, data);
			return;
		}

		// Do protocol command
		conn->mProtocol->doCommand(parser, conn);
	}
}



void DcServer::onNewUdpData(Conn * conn, string *) {

	Parser * parser = conn->mParser;

	// UDP redirect
	Conn * userConn = conn->mProtocol->getConnForUdpData(conn, parser);
	if (userConn == NULL) {
		// unknown UDP data
		return;
	}

	// setCommandPtr
	userConn->setCommandPtr(&parser->mCommand);

	// Do protocol command
	conn->mProtocol->doCommand(parser, userConn);

	// Clear state and commandPtr
	userConn->clearCommandPtr();
}



/** Function checks min interval */
bool DcServer::minDelay(Time & time, double sec) {
	if (::fabs(double(mTime - time)) >= sec) {
		time = mTime;
		return true;
	}
	return false;
}



/** Antiflood function */
bool DcServer::antiFlood(unsigned & count, Time & time, const unsigned & countLimit, const double & timeLimit) {
	if (!timeLimit) {
		return false;
	}
	bool ret = false;
	if (::fabs(double(mTime - time)) < timeLimit) {
		if (countLimit < ++count) {
			ret = true;
		} else {
			return false;
		}
	}
	time = mTime;
	count = 0;
	return ret;
}



/** Checking for this nick used */
bool DcServer::checkNick(DcConn *dcConn) {

	// check empty nick!
	if (dcConn->mDcUser->getNick().empty()) {
		return false;
	}

	UserKey key = mDcUserList.nick2Key(dcConn->mDcUser->getNick());
	if (mDcUserList.containsKey(key)) {
		string sMsg;
		DcUser * us = static_cast<DcUser *> (mDcUserList.find(key));

		if (!us->mDcConn || (us->getProfile() == -1 && us->getIp() != dcConn->getIp())) {
			if (dcConn->Log(2)) {
				dcConn->LogStream() << "Bad nick (used): '" 
					<< dcConn->mDcUser->getNick() << "'["
					<< dcConn->getIp() << "] vs '" << us->getNick() 
					<< "'[" << us->getIp() << "]" << endl;
			}

			stringReplace(mDcLang.mUsedNick, string("nick"), sMsg, dcConn->mDcUser->getNick());

			sendToUser(dcConn->mDcUser, sMsg.c_str(), mDcConfig.mHubBot.c_str());

			dcConn->send(NmdcProtocol::appendValidateDenide(sMsg.erase(), dcConn->mDcUser->getNick()));
			return false;
		}
		if (us->mDcConn->Log(3)) {
			us->mDcConn->LogStream() << "removed old user" << endl;
		}
		removeFromDcUserList(us);
		us->mDcConn->closeNow(CLOSE_REASON_USER_OLD);
	}
	return true;
}



bool DcServer::beforeUserEnter(DcConn * dcConn) {
	unsigned iWantedMask;
	if (mDcConfig.mDelayedLogin && dcConn->mSendNickList) {
		iWantedMask = LOGIN_STATUS_LOGIN_DONE - LOGIN_STATUS_NICKLST;
	} else {
		iWantedMask = LOGIN_STATUS_LOGIN_DONE;
	}

	if (iWantedMask == dcConn->getLoginStatusFlag(iWantedMask)) {
		if (dcConn->Log(3)) {
			dcConn->LogStream() << "Begin login" << endl;
		}

		// check empty nick!
		if (!checkNick(dcConn)) {
			dcConn->closeNice(9000, CLOSE_REASON_NICK_INVALID);
			return false;
		}

		if (dcConn->mSendNickList) {
			if (!mDcConfig.mDelayedLogin) {
				doUserEnter(dcConn);
			} else {
				mEnterList.addWithNick(dcConn->mDcUser->getNick(), dcConn->mDcUser);
			}

			/** Can happen so that list not to send at a time */
			mDcProtocol.sendNickList(dcConn);

			dcConn->mSendNickList = false;
			return true;
		}
		if (!dcConn->mDcUser->getInUserList()) {
			doUserEnter(dcConn);
		}
		return true;
	} else { /** Invalid sequence of the sent commands */
		if (dcConn->Log(2)) {
			dcConn->LogStream() << "Invalid sequence of the sent commands (" 
				<< dcConn->getLoginStatusFlag(iWantedMask) << "), wanted: " 
				<< iWantedMask << endl;
		}
		dcConn->closeNow(CLOSE_REASON_CMD_SEQUENCE);
		return false;
	}
}



/** User entry */
void DcServer::doUserEnter(DcConn * dcConn) {
	/** Check entry stages */
	if (LOGIN_STATUS_LOGIN_DONE != dcConn->getLoginStatusFlag(LOGIN_STATUS_LOGIN_DONE)) {
		if (dcConn->Log(2)) {
			dcConn->LogStream() << "User Login when not all done (" 
				<< dcConn->getLoginStatusFlag(LOGIN_STATUS_LOGIN_DONE) << ")" <<endl;
		}
		dcConn->closeNow(CLOSE_REASON_LOGIN_NOT_DONE);
		return;
	}

	// check empty nick!
	if (!checkNick(dcConn)) {
		dcConn->closeNice(9000, CLOSE_REASON_NICK_INVALID);
		return;
	}

	UserKey key = mDcUserList.nick2Key(dcConn->mDcUser->getNick());

	/** User is already considered came */
	if (mEnterList.containsKey(key)) {
		/** We send user contents of cache without clear this cache */
		mEnterList.flushForUser(dcConn->mDcUser);
		mEnterList.removeByKey(key);
	}

	/** Adding user to the user list */
	if (!addToUserList(static_cast<DcUser *> (dcConn->mDcUser))) {
		dcConn->closeNow(CLOSE_REASON_USER_ADD);
		return;
	}

	/** Show to all */
	showUserToAll(dcConn->mDcUser);

	afterUserEnter(dcConn);

	dcConn->clearTimeOut(HUB_TIME_OUT_LOGIN);
	(static_cast<DcUser *> (dcConn->mDcUser))->mTimeEnter.Get();
}



/** Adding user in the user list */
bool DcServer::addToUserList(DcUser * dcUser) {
	if (!dcUser) {
		if (ErrLog(1)) {
			LogStream() << "Adding a NULL user to userlist" << endl;
		}
		return false;
	}
	if (dcUser->getInUserList()) {
		if (ErrLog(2)) {
			LogStream() << "User is already in the user list" << endl;
		}
		return false;
	}

	UserKey key = mDcUserList.nick2Key(dcUser->getNick());

	if (mDcUserList.Log(4)) {
		mDcUserList.LogStream() << "Before add: " << dcUser->getNick() << " Size: " << mDcUserList.size() << endl;
	}

	if (!mDcUserList.addWithKey(key, dcUser)) {
		if (Log(1)) {
			LogStream() << "Adding twice user with same nick " << dcUser->getNick() << " (" << mDcUserList.find(key)->nick() << ")" << endl;
		}
		dcUser->setInUserList(false);
		return false;
	}

	if (mDcUserList.Log(4)) {
		mDcUserList.LogStream() << "After add: " << dcUser->getNick() << " Size: " << mDcUserList.size() << endl;
	}

	dcUser->setInUserList(true);
	dcUser->setCanSend(true);
	if (!dcUser->isPassive()) {
		mActiveList.addWithKey(key, dcUser);
	}
	if (dcUser->mInOpList) {
		mOpList.addWithKey(key, dcUser);
	}
	if (dcUser->mInIpList) {
		mIpList.addWithKey(key, dcUser);
	}

	if (dcUser->mDcConn) {
		dcUser->mDcConn->mIpRecv = true; /** Installing the permit on reception of the messages on ip */
		mChatList.addWithKey(key, dcUser);

		if (!(dcUser->mDcConn->mFeatures & SUPPORT_FEATURE_NOHELLO)) {
			mHelloList.addWithKey(key, dcUser);
		}
		if (dcUser->mDcConn->Log(3)) {
			dcUser->mDcConn->LogStream() << "Adding at the end of Nicklist" << endl;
		}
		if (dcUser->mDcConn->Log(3)) {
			dcUser->mDcConn->LogStream() << "Becomes in list" << endl;
		}
	}
	return true;
}



/** Removing user from the user list */
bool DcServer::removeFromDcUserList(DcUser * dcUser) {
	UserKey key = mDcUserList.nick2Key(dcUser->getNick());
	if (mDcUserList.Log(4)) {
		mDcUserList.LogStream() << "Before leave: " << dcUser->getNick() << " Size: " << mDcUserList.size() << endl;
	}
	if (mDcUserList.containsKey(key)) {
		#ifndef WITHOUT_PLUGINS
			if (dcUser->mDcConn) {
				mCalls.mOnUserExit.callAll(dcUser);
			}
		#endif

		/** We make sure that user with such nick one! */
		DcUser * other = static_cast<DcUser *> (mDcUserList.getUserBaseByNick(dcUser->getNick())); // NMDC
		if (!dcUser->mDcConn) { /** Removing the bot */
			mDcUserList.removeByKey(key);
		} else if (other && other->mDcConn && dcUser->mDcConn && other->mDcConn == dcUser->mDcConn) {
			mDcUserList.removeByKey(key);
			if (mDcUserList.Log(4)) {
				mDcUserList.LogStream() << "After leave: " << dcUser->getNick() << " Size: " << mDcUserList.size() << endl;
			}
		} else {
			/** Such can happen only for users without connection or with different connection */
			if (dcUser->ErrLog(1)) {
				dcUser->LogStream() << "Not found the correct user for nick: " << dcUser->getNick() << endl;
			}
			return false;
		}
	}

	/** Removing from lists */
	mOpList.removeByKey(key);
	mIpList.removeByKey(key);
	mHelloList.removeByKey(key);
	mEnterList.removeByKey(key);
	mChatList.removeByKey(key);
	mActiveList.removeByKey(key);
	mBotList.removeByKey(key);

	if (dcUser->getInUserList()) {
		dcUser->setInUserList(false);

		if (!dcUser->getHide()) {
			string sMsg;
			NmdcProtocol::appendQuit(sMsg, dcUser->getNick());

			/** Delay in sending MyINFO (and Quit) */
			mDcUserList.sendToAll(sMsg, true/*mDcConfig.mDelayedMyinfo*/, false);
		}
	}
	return true;
}



/** Show user to all */
bool DcServer::showUserToAll(DcUser * dcUser) {
	string sMsg1, sMsg2;
	if (dcUser->mHide && dcUser->mDcConn) {
		if (dcUser->mDcConn->mFeatures & SUPPORT_FEATURE_NOHELLO) {
			dcUser->mDcConn->send(string(dcUser->getMyINFO()), true, false);
		} else if (dcUser->mDcConn->mFeatures & SUPPORT_FEATUER_NOGETINFO) {
			dcUser->mDcConn->send(NmdcProtocol::appendHello(sMsg1, dcUser->getNick()), false, false);
			dcUser->mDcConn->send(string(dcUser->getMyINFO()), true, false);
		} else {
			dcUser->mDcConn->send(NmdcProtocol::appendHello(sMsg1, dcUser->getNick()), false, false);
		}

		if (dcUser->mInOpList) {
			dcUser->mDcConn->send(NmdcProtocol::appendOpList(sMsg2, dcUser->getNick()), false, false);
		}
	} else {

		/** Sending the greeting for all users, not supporting feature NoHello (except enterring users) */
		mHelloList.sendToAll(NmdcProtocol::appendHello(sMsg1, dcUser->getNick()), true/*mDcConfig.mDelayedMyinfo*/, false);

		/** Show MyINFO string to all users */
		mDcUserList.sendToAll(dcUser->getMyINFO(), true/*mDcConfig.mDelayedMyinfo*/); // use cache -> so this can be after user is added

		/** Show MyINFO string of the current user to all enterring users */
		mEnterList.sendToAll(dcUser->getMyINFO(), true/*mDcConfig.mDelayedMyinfo*/);

		/** Op entry */
		if (dcUser->mInOpList) {
			mDcUserList.sendToAll(NmdcProtocol::appendOpList(sMsg2, dcUser->getNick()), true/*mDcConfig.mDelayedMyinfo*/, false);
			mEnterList.sendToAll(sMsg2, true/*mDcConfig.mDelayedMyinfo*/, false);
		}
	}

	bool canSend = dcUser->isCanSend();

	/** Prevention of the double sending MyINFO string */
	if (!mDcConfig.mDelayedLogin) {
			dcUser->setCanSend(false);
			mDcUserList.flushCache();
			mEnterList.flushCache();
			dcUser->setCanSend(canSend);
	}

	if (mDcConfig.mSendUserIp) {
		string sStr;
		dcUser->setCanSend(false);
		NmdcProtocol::appendUserIp(sStr, dcUser->getNick(), dcUser->getIp());
		if (sStr.length()) {
			mIpList.sendToAll(sStr, true);
		}
		dcUser->setCanSend(canSend);

		if (dcUser->mInIpList) {
			dcUser->send(mDcUserList.getIpList(), true, false);
		} else if (dcUser->mDcConn && (dcUser->mDcConn->mFeatures & SUPPORT_FEATUER_USERIP2)) { // UserIP2
			dcUser->send(sStr, false, false);
		}
	}

	static string s;
	dcUser->send(s, false, true);
	return true;
}



void DcServer::afterUserEnter(DcConn *dcConn) {
	if (dcConn->Log(3)) {
		dcConn->LogStream() << "Entered the hub." << endl;
	}
	#ifndef WITHOUT_PLUGINS
		mCalls.mOnUserEnter.callAll(dcConn->mDcUser);
	#endif
}



/** Get user by nick (or NULL) */
DcUser * DcServer::getDcUser(const char * nick) {
	string sN(nick);
	if (sN.size()) {
		// NMDC
		UserBase * userBase = mDcUserList.getUserBaseByNick(sN);
		if (userBase) {
			return static_cast<DcUser *> (userBase);
		}
		DcConn * dcConn;
		for (tCLIt it = mConnList.begin(); it != mConnList.end(); ++it) {
			dcConn = static_cast<DcConn *> (*it);
			if (dcConn && dcConn->mType == CLIENT_TYPE_NMDC && dcConn->mDcUser && dcConn->mDcUser->getNick() == sN) {
				return static_cast<DcUser *> (dcConn->mDcUser);
			}
		}
	}
	return NULL;
}



// TODO: return user with conn only?
DcUserBase * DcServer::getDcUserBase(const char * nick) {
	DcUser * dcUser = getDcUser(nick);
	return (dcUser != NULL && dcUser->mDcConn != NULL) ? static_cast<DcUserBase *> (dcUser) : NULL;
}



const vector<DcConnBase *> & DcServer::getDcConnBase(const char * sIP) {
	DcIpList::iterator it;
	mIpConnList.clear();
	for (it = mIpListConn->begin(DcConn::ip2Num(sIP)); it != mIpListConn->end(); ++it) {
		DcConn * dcConn = static_cast<DcConn *> (*it);
		if (dcConn->mType == CLIENT_TYPE_NMDC) {
			mIpConnList.push_back(dcConn);
		}
	}
	return mIpConnList;
}



/** Send data to user */
bool DcServer::sendToUser(DcUserBase * dcUserBase, const char * sData, const char * sNick, const char * sFrom) {
	if (!dcUserBase || !dcUserBase->mDcConnBase || !sData) {
		return false;
	}
	DcConn * dcConn = static_cast<DcConn *> (dcUserBase->mDcConnBase);

	// PM
	if (sFrom && sNick) {
		string sTo("<unknown>"), sStr;
		if (dcConn->mDcUser && !dcConn->mDcUser->getNick().empty()) {
			sTo = dcConn->mDcUser->getNick();
		}
		dcConn->send(NmdcProtocol::appendPm(sStr, sTo, sFrom, sNick, sData));
		return true;
	}

	// Chat
	if (sNick) {
		string sStr;
		dcConn->send(NmdcProtocol::appendChat(sStr, sNick, sData));
		return true;
	}

	// Simple Msg
	string sMsg(sData);
	if (dcConn->mType == CLIENT_TYPE_NMDC && sMsg.substr(sMsg.size() - 1, 1) != NMDC_SEPARATOR) {
		sMsg.append(NMDC_SEPARATOR);
	}
	dcConn->send(sMsg);
	return true;
}



/** Send data to nick */
bool DcServer::sendToNick(const char *sTo, const char *sData, const char *sNick, const char *sFrom) {
	if (!sTo || !sData) {
		return false;
	}
	DcUser * dcUser = getDcUser(sTo);
	if (!dcUser || !dcUser->mDcConn) {
		return false;
	}

	// PM
	if (sFrom && sNick) {
		string sStr;
		dcUser->mDcConn->send(NmdcProtocol::appendPm(sStr, sTo, sFrom, sNick, sData));
		return true;
	}

	// Chat
	if (sNick) {
		string sStr;
		dcUser->mDcConn->send(NmdcProtocol::appendChat(sStr, sNick, sData));
		return true;
	}

	// Simple Msg
	string sMsg(sData);
	if (sMsg.substr(sMsg.size() - 1, 1) != NMDC_SEPARATOR) {
		sMsg.append(NMDC_SEPARATOR);
	}
	dcUser->mDcConn->send(sMsg);
	return true;
}



/** Send data to all */
bool DcServer::sendToAll(const char *sData, const char *sNick, const char *sFrom) {
	if (!sData) {
		return false;
	}

	// PM
	if (sFrom && sNick) {
		string sStart, sEnd;
		NmdcProtocol::appendPmToAll(sStart, sEnd, sFrom, sNick, sData);
		mDcUserList.sendWithNick(sStart, sEnd);
		return true;
	}

	// Chat
	if (sNick) {
		string sStr;
		mDcUserList.sendToAll(NmdcProtocol::appendChat(sStr, sNick, sData), false, false);
		return true;
	}

	// Simple Msg
	string sMsg(sData);
	if (sMsg.substr(sMsg.size() - 1, 1) != NMDC_SEPARATOR) {
		sMsg.append(NMDC_SEPARATOR);
	}
	mDcUserList.sendToAll(sMsg, false, false);
	return true;
}



/** Send data to profiles */
bool DcServer::sendToProfiles(unsigned long iProfile, const char *sData, const char *sNick, const char *sFrom) {
	if (!sData) {
		return false;
	}

	// PM
	if (sFrom && sNick) {
		string sStart, sEnd;
		NmdcProtocol::appendPmToAll(sStart, sEnd, sFrom, sNick, sData);
		mDcUserList.sendWithNick(sStart, sEnd, iProfile);
		return true;
	}

	// Chat
	if (sNick) {
		string sStr;
		mDcUserList.sendToProfiles(iProfile, NmdcProtocol::appendChat(sStr, sNick, sData), false);
		return true;
	}

	// Simple Msg
	string sMsg(sData);
	if (sMsg.substr(sMsg.size() - 1, 1) != NMDC_SEPARATOR) {
		sMsg.append(NMDC_SEPARATOR);
	}
	mDcUserList.sendToProfiles(iProfile, sMsg, false);
	return true;
}



bool DcServer::sendToIp(const char *sIP, const char *sData, unsigned long iProfile, const char *sNick, const char *sFrom) {
	if (!sIP || !sData || !Conn::checkIp(sIP)) {
		return false;
	}

	// PM
	if (sFrom && sNick) {
		string sStart, sEnd;
		NmdcProtocol::appendPmToAll(sStart, sEnd, sFrom, sNick, sData);
		mIpListConn->sendToIpWithNick(sIP, sStart, sEnd, iProfile);
		return true;
	}

	// Chat
	if (sNick) {
		string sStr;
		mIpListConn->sendToIp(sIP, NmdcProtocol::appendChat(sStr, sNick, sData), iProfile); // newPolitic
		return true;
	}

	// Simple Msg
	string sMsg(sData);
	if (sMsg.substr(sMsg.size() - 1, 1) != NMDC_SEPARATOR) {
		sMsg.append(NMDC_SEPARATOR);
	}
	mIpListConn->sendToIp(sIP, sMsg, iProfile); // newPolitic
	return true;
}



/** Send data to all except nick list */
bool DcServer::sendToAllExceptNicks(const vector<string> & NickList, const char *sData, const char *sNick, const char *sFrom) {
	if (!sData) {
		return false;
	}

	DcUser * dcUser;
	vector<DcUser *> ul;
	for (List_t::const_iterator it = NickList.begin(); it != NickList.end(); ++it) {
		// NMDC
		dcUser = static_cast<DcUser *> (mDcUserList.getUserBaseByNick(*it));
		if (dcUser && dcUser->isCanSend()) {
			dcUser->setCanSend(false);
			ul.push_back(dcUser);
		}
	}

	if (sFrom && sNick) { // PM
		string sStart, sEnd;
		NmdcProtocol::appendPmToAll(sStart, sEnd, sFrom, sNick, sData);
		mDcUserList.sendWithNick(sStart, sEnd);
	} else if (sNick) { // Chat
		string sStr;
		mDcUserList.sendToAll(NmdcProtocol::appendChat(sStr, sNick, sData), false, false);
	} else { // Simple Msg
		string sMsg(sData);
		if (sMsg.substr(sMsg.size() - 1, 1) != NMDC_SEPARATOR) {
			sMsg.append(NMDC_SEPARATOR);
		}
		mDcUserList.sendToAll(sMsg, false, false);
	}

	for (vector<DcUser *>::iterator ul_it = ul.begin(); ul_it != ul.end(); ++ul_it) {
		(*ul_it)->setCanSend(true);
	}

	return true;
}



bool DcServer::sendToAllExceptIps(const vector<string> & IPList, const char *sData, const char *sNick, const char *sFrom) {
	if (!sData) {
		return false;
	}

	DcConn * dcConn;
	vector<DcConn*> ul;
	bool bBadIP = false;
	for (List_t::const_iterator it = IPList.begin(); it != IPList.end(); ++it) {
		if (!DcConn::checkIp(*it)) {
			bBadIP = true;
		}
		for (DcIpList::iterator mit = mIpListConn->begin(DcConn::ip2Num((*it).c_str())); mit != mIpListConn->end(); ++mit) {
			dcConn = static_cast<DcConn *> (*mit);
			if (dcConn->mDcUser && dcConn->mDcUser->isCanSend()) {
				dcConn->mDcUser->setCanSend(false);
				ul.push_back(dcConn);
			}
		}
	}

	if (!bBadIP) {
		if (sFrom && sNick) { // PM
			string sStart, sEnd;
			NmdcProtocol::appendPmToAll(sStart, sEnd, sFrom, sNick, sData);
			mDcUserList.sendWithNick(sStart, sEnd);
		} else if (sNick) { // Chat
			string sStr;
			mDcUserList.sendToAll(NmdcProtocol::appendChat(sStr, sNick, sData), false, false);
		} else { // Simple Msg
			string sMsg(sData);
			if (sMsg.substr(sMsg.size() - 1, 1) != NMDC_SEPARATOR) {
				sMsg.append(NMDC_SEPARATOR);
			}
			mDcUserList.sendToAll(sMsg, false, false);
		}
	}

	for (vector<DcConn*>::iterator ul_it = ul.begin(); ul_it != ul.end(); ++ul_it) {
		(*ul_it)->mDcUser->setCanSend(true);
	}

	return (bBadIP == true) ? false : true;
}



void DcServer::forceMove(DcUserBase * dcUserBase, const char * sAddress, const char * sReason /* = NULL */) {
	if (!sAddress) {
		return;
	}
	DcConn * dcConn = static_cast<DcConn *> (dcUserBase->mDcConnBase);

	string sMsg, sForce, sNick("<unknown>");
	if (dcConn->mDcUser && !dcConn->mDcUser->getNick().empty()) {
		sNick = dcConn->mDcUser->getNick();
	}

	stringReplace(mDcLang.mForceMove, string("address"), sForce, string(sAddress));
	stringReplace(sForce, string("reason"), sForce, string(sReason != NULL ? sReason : ""));
	NmdcProtocol::appendPm(sMsg, sNick, mDcConfig.mHubBot, mDcConfig.mHubBot, sForce);
	NmdcProtocol::appendChat(sMsg, mDcConfig.mHubBot, sForce);
	dcConn->send(NmdcProtocol::appendForceMove(sMsg, sAddress));
	dcConn->closeNice(9000, CLOSE_REASON_CMD_FORCE_MOVE);
}



const vector<string> & DcServer::getConfig() {
	if (mConfigNameList.empty()) {
		for (ConfigListBase::tHLMIt it = mDcConfig.mList.begin(); it != mDcConfig.mList.end(); ++it) {
			mConfigNameList.push_back((*it)->mName);
		}
	}
	return mConfigNameList;
}



const char * DcServer::getConfig(const string & sName) {
	Config * config = mDcConfig[sName];
	if (!config) {
		return NULL;
	}
	config->convertTo(sBuf);
	return sBuf.c_str();
}



const char * DcServer::getLang(const string & sName) {
	Config * config = mDcLang[sName];
	if (!config) {
		return NULL;
	}
	config->convertTo(sBuf);
	return sBuf.c_str();
}



bool DcServer::setConfig(const string & sName, const string & sValue) {
	if (sName == "sAddresses") {
		return false;
	}

	if (sName == "sLocale" && !setlocale(LC_ALL, sValue.c_str())) {
		return false;
	}

	Config * config = mDcConfig[sName];
	if (!config) {
		return false;
	}

	if (sName == "sHubBot") {
		unregBot(mDcConfig.mHubBot);
	} else if (sName == "bRegMainBot") {
		if (sValue == "true" || 0 != atoi(sValue.c_str()) ) {
			regBot(mDcConfig.mHubBot, mDcConfig.mMainBotMyinfo, 
				mDcConfig.mMainBotIp, mDcConfig.mMainBotKey);
		} else {
			unregBot(mDcConfig.mHubBot);
		}
	}

	config->convertFrom(sValue);

	if (sName == "sHubBot") {
		if (mDcConfig.mRegMainBot) { /** Registration bot */
			regBot(mDcConfig.mHubBot, mDcConfig.mMainBotMyinfo, 
				mDcConfig.mMainBotIp, mDcConfig.mMainBotKey);
		}
	} else if (sName == "sHubName" || sName == "sTopic") {
		string sMsg;
		sendToAll(NmdcProtocol::appendHubName(sMsg, mDcConfig.mHubName, mDcConfig.mTopic).c_str()); // use cache ?
	}

	mDcConfig.save();
	return true;
}



bool DcServer::setLang(const string & sName, const string & sValue) {
	Config * config = mDcLang[sName];
	if (!config) {
		return false;
	}
	config->convertFrom(sValue);
	mDcLang.save();
	return true;
}



int DcServer::regBot(const string & nick, const string & info, const string & ip, bool key) {
	DcUser * dcUser = new DcUser();
	dcUser->setNick(nick);
	dcUser->mDcServer = this;
	dcUser->mInOpList = key;

	if (DcConn::checkIp(ip)) {
		dcUser->setIp(ip);
	}

	// Protocol dependence
	if (!nick.length() || nick.length() > 64 || nick.find_first_of(" |$") != nick.npos) {
		return -1;
	}
	if (!dcUser->setMyINFO(string("$MyINFO $ALL ") + nick + " " + info)) {
		if (!dcUser->setMyINFO(string("$MyINFO $ALL ") + nick + " $ $$$0$")) {
			delete dcUser;
			return -2;
		}
	}
	////

	if (Log(3)) {
		LogStream() << "Reg bot: " << nick << endl;
	}

	if (!addToUserList(dcUser)) {
		delete dcUser;
		return -3;
	}
	mBotList.addWithNick(nick, dcUser);
	showUserToAll(dcUser);
	return 0;
}



int DcServer::unregBot(const string & nick) {

	if (Log(3)) {
		LogStream() << "Unreg bot: " << nick << endl;
	}

	// NMDC
	DcUser * dcUser = static_cast<DcUser *> (mDcUserList.getUserBaseByNick(nick));
	if (!dcUser) {
		return -1;
	}
	if (dcUser->mDcConn) {
		if (Log(3)) {
			LogStream() << "Attempt delete user" << endl;
		}
		return -2;
	}
	removeFromDcUserList(dcUser);
	delete dcUser;
	return 0;
}



#ifdef _WIN32

string DcServer::getSysVersion() {
	OSVERSIONINFOEX osvi;
	int bOsVersionInfoEx;

	string version;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO *) &osvi);
	if (!bOsVersionInfoEx) {
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if (!GetVersionEx((OSVERSIONINFO *) &osvi)) {
			return string("unknown");
		}
	}

	char buf[256] = { '\0' };
	switch (osvi.dwPlatformId) {

		case VER_PLATFORM_WIN32_NT : // Windows NT

			if (osvi.dwMajorVersion <= 4) {
				version.append("Microsoft Windows NT ");
			}
			if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0) {
				version.append("Microsoft Windows 2000 ");
			}

			if (bOsVersionInfoEx) {

				// Check workstation type
				if (osvi.wProductType == VER_NT_WORKSTATION) {

					if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1) {
						version.append("Microsoft Windows XP ");
					} else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0) {
						version.append("Microsoft Windows Vista ");
					} else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1) {
						version.append("Microsoft Windows 7 ");
					} else {
						version.append("Microsoft Windows (unknown version) ");
					}


					if (osvi.wSuiteMask & VER_SUITE_PERSONAL) {
						version.append("Home Edition ");
					} else {
						version.append("Professional ");
					}

				} else if (osvi.wProductType == VER_NT_SERVER) { // Check server type

					if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2) {
						version.append("Microsoft Windows 2003 ");
					} else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0) {
						version.append("Microsoft Windows Server 2008 ");
					} else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1) {
						version.append("Microsoft Windows Server 2008 R2 ");
					} else {
						version.append("Microsoft Windows (unknown version) ");
					}

					if (osvi.wSuiteMask & VER_SUITE_DATACENTER) {
						version.append("DataCenter Server ");
					} else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE) {
						if (osvi.dwMajorVersion == 4) {
							version.append("Advanced Server ");
						} else {
							version.append("Enterprise Server ");
						}
					} else if (osvi.wSuiteMask == VER_SUITE_BLADE) {
						version.append("Web Server ");
					} else {
						version.append("Server ");
					}

				}

			} else {
				HKEY hKey;
				char szProductType[80] = { '\0' };
				DWORD dwBufLen = 80;
				LONG lRet = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\ProductOptions", 0, KEY_QUERY_VALUE, &hKey);

				if (lRet != ERROR_SUCCESS) {
					return string("unknown");
				}

				lRet = RegQueryValueExA( hKey, "ProductType", NULL, NULL, (LPBYTE) szProductType, &dwBufLen);

				if ((lRet != ERROR_SUCCESS) || (dwBufLen > 80)) {
					return string("unknown");
				}

				RegCloseKey(hKey);

				if (lstrcmpiA("WINNT", szProductType) == 0) {
					version.append("Professional ");
				}
				if (lstrcmpiA("LANMANNT", szProductType) == 0) {
					version.append("Server ");
				}
				if (lstrcmpiA( "SERVERNT", szProductType) == 0) {
					version.append("Advanced Server ");
				}
			}

			// Version, service pack, number of the build
			if (osvi.dwMajorVersion <= 4) {
				sprintf(buf, "version %d.%d %s (Build %d)", 
					osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.szCSDVersion, osvi.dwBuildNumber & 0xFFFF);
				version.append(buf);
			} else {
				sprintf(buf, "%s (Build %d)", osvi.szCSDVersion, osvi.dwBuildNumber & 0xFFFF);
				version.append(buf);
			}

			break;

		case VER_PLATFORM_WIN32_WINDOWS : // Windows 95

			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0) {
				version.append("Microsoft Windows 95 ");
				if (osvi.szCSDVersion[1] == 'C' || osvi.szCSDVersion[1] == 'B') {
					version.append("OSR2 ");
				}
			} 

			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10) {
				version.append("Microsoft Windows 98 ");
				if (osvi.szCSDVersion[1] == 'A') {
					version.append("SE ");
				}
			} 

			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90) {
				version.append("Microsoft Windows Millennium Edition ");
			} 
			break;

		case VER_PLATFORM_WIN32s : // Windows
			version.append("Microsoft Win32s ");
			break;

		default :
			break;
	}
	return version; 
}

#else

string DcServer::getSysVersion() {
	string version;
	utsname osname;
	if (uname(&osname) == 0) {
		version = string(osname.sysname) + " " + 
			string(osname.release) + " (" + 
			string(osname.machine) + ")";
	} else {
		version = "unknown";
	}
	return version;
}

#endif

}; // namespace dcserver

/**
 * $Id$
 * $HeadURL$
 */
