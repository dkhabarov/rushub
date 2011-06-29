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
	mStartTime(true),
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

	// Current server
	currentDcServer = this;

	// Define OS
	if (mSysVersion.empty()) {
		mSysVersion = getSysVersion();
	}

	// DcIpList
	mIpListConn = new DcIpList();

	mDcProtocol.setServer(this);
	mPluginList.setServer(this);

	mPluginList.loadAll(); // Load plugins


	mDcUserList.setNickListStart("$NickList ");
	mDcUserList.setNickListSeparator("$$");

	mOpList.setNickListStart("$OpList ");
	mOpList.setNickListSeparator("$$");


	if (mDcConfig.mRegMainBot) { // Main bot registration
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

	mPluginList.unloadAll(); // Unload all plugins

	if (mDcConfig.mRegMainBot) { // Main bot unreg
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
	const char * sAddresses,
	vector<pair<string, string> > & vec,
	const char * defPort
) {
	vector<string> vAddresses;
	stringSplit(sAddresses, " ", vAddresses);
	for (vector<string>::iterator it = vAddresses.begin(); it != vAddresses.end(); ++it) {
		vector<string> vIpPort;
		string part = (*it);
		size_t pos = part.find('[');
		const char * delim = ":";
		if (pos == 0) {
			pos = part.assign(part, 1, part.size() - 1).find("]:");
			delim = "]:";
		}
		stringSplit(part, delim, vIpPort);
		if (2 < vIpPort.size() || vIpPort.size() < 1 || vIpPort[0].size() == 0) {
			continue;
		}
		size_t last = vIpPort[0].size() - 1;
		if (vIpPort[0].at(last) == ']') {
			vIpPort[0].assign(vIpPort[0].c_str(), last);
		}
		vec.push_back(pair<string, string>(vIpPort[0], vIpPort.size() == 2 ? vIpPort[1] : defPort));
	}
}



bool DcServer::listeningServer(const char * name, const char * addresses, const char * port, ConnFactory * connFactory, bool udp /*= false*/) {
	vector<pair<string, string> > vAddresses;
	getAddresses(addresses, vAddresses, port);

	if (vAddresses.size() == 0) {
		if (ErrLog(0)) {
			LogStream() << "Incorrect address of the " << name << endl;
		}
	}

	bool ret = false;
	for (vector<pair<string, string> >::iterator it = vAddresses.begin(); it != vAddresses.end(); ++it) {
		if (Server::listening(connFactory, ((*it).first).c_str(), ((*it).second).c_str(), udp) == 0) {
			ret = true;
			if (Log(0)) {
				LogStream() << name << " is running on [" 
					<< ((*it).first) << "]:" << ((*it).second) 
					<< (udp ? " UDP" : " TCP") << endl;
			}
			cout << name << " is running on [" 
				<< ((*it).first) << "]:" << ((*it).second) 
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
	if (!listeningServer("DC Server " INTERNALNAME " " INTERNALVERSION, mDcConfig.mAddresses.c_str(), "411", mDcConnFactory)) {
		return -1;
	}

	// Web Server
	if (mDcConfig.mWebServer) {
		listeningServer("Web Server", mDcConfig.mWebAddresses.c_str(), "80", mWebConnFactory);
	}

	// UDP DC Server
	if (mDcConfig.mUdpServer) {
		listeningServer("DC Server (UDP)", mDcConfig.mUdpAddresses.c_str(), "1209", mDcConnFactory, true);
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
	if (mIpEnterFlood.check(dcConn->getIp(), mTime)) {
		dcConn->closeNow(CLOSE_REASON_FLOOD_IP_ENTRY);
		return -2;
	}

	// TODO fix me (refactoring)
	mIpListConn->add(dcConn); // Adding connection in IP-list
	dcConn->mDcUser->setIp(dcConn->getIp());

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
	if (dcConn->mDcUser->getUid().empty()) {
		return false;
	}

	unsigned long uidHash = dcConn->mDcUser->getUidHash();
	if (mDcUserList.contain(uidHash)) {
		DcUser * us = static_cast<DcUser *> (mDcUserList.find(uidHash));

		if (!us->mDcConn || (us->getProfile() == -1 && us->getIp() != dcConn->getIp())) {
			if (dcConn->Log(2)) {
				dcConn->LogStream() << "Bad nick (used): '" 
					<< dcConn->mDcUser->getUid() << "'["
					<< dcConn->getIp() << "] vs '" << us->getUid() 
					<< "'[" << us->getIp() << "]" << endl;
			}
			string msg;
			stringReplace(mDcLang.mUsedNick, "nick", msg, dcConn->mDcUser->getUid());
			sendToUser(dcConn->mDcUser, msg.c_str(), mDcConfig.mHubBot.c_str());
			dcConn->send(NmdcProtocol::appendValidateDenide(msg.erase(), dcConn->mDcUser->getUid()));
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
				mEnterList.add(dcConn->mDcUser->getUidHash(), dcConn->mDcUser);
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

	unsigned long uidHash = dcConn->mDcUser->getUidHash();

	/** User is already considered came */
	if (mEnterList.contain(uidHash)) {
		/** We send user contents of cache without clear this cache */
		mEnterList.flushForUser(dcConn->mDcUser);
		mEnterList.remove(uidHash);
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

	unsigned long uidHash = dcUser->getUidHash();

	if (mDcUserList.Log(4)) {
		mDcUserList.LogStream() << "Before add: " << dcUser->getUid() << " Size: " << mDcUserList.size() << endl;
	}

	if (!mDcUserList.add(uidHash, dcUser)) {
		if (Log(1)) {
			LogStream() << "Adding twice user with same nick " << dcUser->getUid() << " (" << mDcUserList.find(uidHash)->uid() << ")" << endl;
		}
		dcUser->setInUserList(false);
		return false;
	}

	if (mDcUserList.Log(4)) {
		mDcUserList.LogStream() << "After add: " << dcUser->getUid() << " Size: " << mDcUserList.size() << endl;
	}

	dcUser->setInUserList(true);
	dcUser->setCanSend(true);
	if (!dcUser->isPassive()) {
		mActiveList.add(uidHash, dcUser);
	}
	if (dcUser->getInOpList()) {
		mOpList.add(uidHash, dcUser);
	}
	if (dcUser->getInIpList()) {
		mIpList.add(uidHash, dcUser);
	}

	if (dcUser->mDcConn) {
		dcUser->mDcConn->mIpRecv = true; /** Installing the permit on reception of the messages on ip */
		mChatList.add(uidHash, dcUser);

		if (!(dcUser->mDcConn->mFeatures & SUPPORT_FEATURE_NOHELLO)) {
			mHelloList.add(uidHash, dcUser);
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
	unsigned long uidHash = dcUser->getUidHash();
	if (mDcUserList.Log(4)) {
		mDcUserList.LogStream() << "Before leave: " << dcUser->getUid() << " Size: " << mDcUserList.size() << endl;
	}
	if (mDcUserList.contain(uidHash)) {
		#ifndef WITHOUT_PLUGINS
			if (dcUser->mDcConn) {
				mCalls.mOnUserExit.callAll(dcUser);
			}
		#endif

		// We make sure that user with such nick one!
		DcUser * other = static_cast<DcUser *> (mDcUserList.find(dcUser->getUidHash()));
		if (!dcUser->mDcConn) { /** Removing the bot */
			mDcUserList.remove(uidHash);
		} else if (other && other->mDcConn && dcUser->mDcConn && other->mDcConn == dcUser->mDcConn) {
			mDcUserList.remove(uidHash);
			if (mDcUserList.Log(4)) {
				mDcUserList.LogStream() << "After leave: " << dcUser->getUid() << " Size: " << mDcUserList.size() << endl;
			}
		} else {
			// Such can happen only for users without connection or with different connection
			if (dcUser->ErrLog(1)) {
				dcUser->LogStream() << "Not found the correct user for nick: " << dcUser->getUid() << endl;
			}
			return false;
		}
	}

	// Removing from lists
	mOpList.remove(uidHash);
	mIpList.remove(uidHash);
	mHelloList.remove(uidHash);
	mEnterList.remove(uidHash);
	mChatList.remove(uidHash);
	mActiveList.remove(uidHash);
	mBotList.remove(uidHash);

	if (dcUser->getInUserList()) {
		dcUser->setInUserList(false);

		if (!dcUser->getHide()) {
			string msg;
			NmdcProtocol::appendQuit(msg, dcUser->getUid());

			// Delay in sending MyINFO (and Quit)
			mDcUserList.sendToAll(msg, true/*mDcConfig.mDelayedMyinfo*/, false);
		}
	}
	return true;
}



/// Show user to all
bool DcServer::showUserToAll(DcUser * dcUser) {
	string hello;
	if (dcUser->getHide() && dcUser->mDcConn) {
		if (dcUser->mDcConn->mFeatures & SUPPORT_FEATURE_NOHELLO) {
			dcUser->mDcConn->send(dcUser->getMyInfo(), true, false);
		} else if (dcUser->mDcConn->mFeatures & SUPPORT_FEATUER_NOGETINFO) {
			dcUser->mDcConn->send(NmdcProtocol::appendHello(hello, dcUser->getUid()), false, false);
			dcUser->mDcConn->send(dcUser->getMyInfo(), true, false);
		} else {
			dcUser->mDcConn->send(NmdcProtocol::appendHello(hello, dcUser->getUid()), false, false);
		}

		if (dcUser->getInOpList()) {
			string opList;
			dcUser->mDcConn->send(NmdcProtocol::appendOpList(opList, dcUser->getUid()), false, false);
		}
	} else {

		// Sending the greeting for all users, not supporting feature NoHello (except enterring users)
		mHelloList.sendToAll(NmdcProtocol::appendHello(hello, dcUser->getUid()), true/*mDcConfig.mDelayedMyinfo*/, false);

		// Show MyINFO string to all users
		mDcUserList.sendToAll(dcUser->getMyInfo(), true/*mDcConfig.mDelayedMyinfo*/); // use cache -> so this can be after user is added

		// Show MyINFO string of the current user to all enterring users
		mEnterList.sendToAll(dcUser->getMyInfo(), true/*mDcConfig.mDelayedMyinfo*/);

		// Op entry
		if (dcUser->getInOpList()) {
			string opList;
			mDcUserList.sendToAll(NmdcProtocol::appendOpList(opList, dcUser->getUid()), true/*mDcConfig.mDelayedMyinfo*/, false);
			mEnterList.sendToAll(opList, true/*mDcConfig.mDelayedMyinfo*/, false);
		}
	}

	bool canSend = dcUser->isCanSend();

	// Prevention of the double sending
	if (!mDcConfig.mDelayedLogin) {
			dcUser->setCanSend(false);
			mDcUserList.flushCache();
			mEnterList.flushCache();
			dcUser->setCanSend(canSend);
	}

	if (mDcConfig.mSendUserIp) {
		string ipList;
		NmdcProtocol::appendUserIp(ipList, dcUser->getUid(), dcUser->getIp());
		if (ipList.length()) {
			mIpList.sendToAll(ipList, true, false);
		}

		if (dcUser->getInIpList()) {
			dcUser->send(mDcUserList.getIpList(), true, false);
		} else if (ipList.length() && dcUser->mDcConn && (dcUser->mDcConn->mFeatures & SUPPORT_FEATUER_USERIP2)) { // UserIP2
			dcUser->send(ipList, false, false);
		}
	}

	dcUser->send("", 0, false, true);
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



/// Get user by nick (or NULL)
DcUser * DcServer::getDcUser(const char * nick) {
	if (nick == NULL) {
		return NULL;
	}
	string nickStr(nick);
	if (nickStr.size()) {
		// NMDC
		UserBase * userBase = mDcUserList.getUserBaseByNick(nickStr);
		if (userBase) {
			return static_cast<DcUser *> (userBase);
		}
		DcConn * dcConn = NULL;
		for (tCLIt it = mConnList.begin(); it != mConnList.end(); ++it) {
			dcConn = static_cast<DcConn *> (*it);
			if (dcConn && dcConn->mType == CLIENT_TYPE_NMDC && 
				dcConn->mDcUser && dcConn->mDcUser->getUid() == nickStr
			) {
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



const vector<DcConnBase *> & DcServer::getDcConnBase(const char * ip) {
	mIpConnList.clear();
	for (DcIpList::iterator it = mIpListConn->begin(ip); it != mIpListConn->end(); ++it) {
		DcConn * dcConn = static_cast<DcConn *> (*it);
		if (dcConn->mType == CLIENT_TYPE_NMDC && dcConn->getIp() == ip) {
			mIpConnList.push_back(dcConn);
		}
	}
	return mIpConnList;
}



/** Send data to user */
bool DcServer::sendToUser(DcUserBase * dcUserBase, const char * data, const char * nick, const char * from) {
	if (!dcUserBase || !dcUserBase->mDcConnBase || !data) {
		return false;
	}
	DcConn * dcConn = static_cast<DcConn *> (dcUserBase->mDcConnBase);

	// PM
	if (from && nick) {
		string to("<unknown>"), str;
		if (dcConn->mDcUser && !dcConn->mDcUser->getUid().empty()) {
			to = dcConn->mDcUser->getUid();
		}
		dcConn->send(NmdcProtocol::appendPm(str, to, from, nick, data));
		return true;
	}

	// Chat
	if (nick) {
		string str;
		dcConn->send(NmdcProtocol::appendChat(str, nick, data));
		return true;
	}

	// Simple Msg
	string msg(data);
	if (dcConn->mType == CLIENT_TYPE_NMDC && 
		msg.find(NMDC_SEPARATOR, msg.size() - NMDC_SEPARATOR_LEN) == msg.npos
	) {
		dcConn->send(msg, true);
	} else {
		dcConn->send(msg);
	}
	return true;
}



/** Send data to nick */
bool DcServer::sendToNick(const char * to, const char * data, const char * nick, const char * from) {
	DcUser * dcUser = getDcUser(to);
	if (!dcUser || !dcUser->mDcConn) { // Check exist and not bot
		return false;
	}
	return sendToUser(dcUser, data, nick, from);
}



/** Send data to all */
bool DcServer::sendToAll(const char * data, const char * nick, const char * from) {
	if (!data) {
		return false;
	}

	// PM
	if (from && nick) {
		string start, end;
		NmdcProtocol::appendPmToAll(start, end, from, nick, data);
		mDcUserList.sendWithNick(start, end);
		return true;
	}

	// Chat
	if (nick) {
		string str;
		mDcUserList.sendToAll(NmdcProtocol::appendChat(str, nick, data), false, false);
		return true;
	}

	// Simple Msg
	string msg(data);
	if (msg.find(NMDC_SEPARATOR, msg.size() - NMDC_SEPARATOR_LEN) == msg.npos) {
		msg.append(NMDC_SEPARATOR);
	}
	mDcUserList.sendToAll(msg, false, false);
	return true;
}



/** Send data to profiles */
bool DcServer::sendToProfiles(unsigned long profile, const char * data, const char * nick, const char * from) {
	if (!data) {
		return false;
	}

	// PM
	if (from && nick) {
		string start, end;
		NmdcProtocol::appendPmToAll(start, end, from, nick, data);
		mDcUserList.sendWithNick(start, end, profile);
		return true;
	}

	// Chat
	if (nick) {
		string str;
		mDcUserList.sendToProfiles(profile, NmdcProtocol::appendChat(str, nick, data), false);
		return true;
	}

	// Simple Msg
	string msg(data);
	if (msg.find(NMDC_SEPARATOR, msg.size() - NMDC_SEPARATOR_LEN) == msg.npos) {
		msg.append(NMDC_SEPARATOR);
	}
	mDcUserList.sendToProfiles(profile, msg, false);
	return true;
}



bool DcServer::sendToIp(const char * ip, const char * data, unsigned long profile, const char * nick, const char * from) {
	if (!ip || !data || !Conn::checkIp(ip)) {
		return false;
	}

	// PM
	if (from && nick) {
		string start, end;
		NmdcProtocol::appendPmToAll(start, end, from, nick, data);
		mIpListConn->sendToIpWithNick(ip, start, end, profile);
		return true;
	}

	// Chat
	if (nick) {
		string str;
		mIpListConn->sendToIp(ip, NmdcProtocol::appendChat(str, nick, data), profile); // newPolitic
		return true;
	}

	// Simple Msg
	string msg(data);
	if (msg.find(NMDC_SEPARATOR, msg.size() - NMDC_SEPARATOR_LEN) == msg.npos) {
		msg.append(NMDC_SEPARATOR);
	}
	mIpListConn->sendToIp(ip, msg, profile); // newPolitic
	return true;
}



/** Send data to all except nick list */
bool DcServer::sendToAllExceptNicks(const vector<string> & nickList, const char * data, const char * nick, const char * from) {
	if (!data) {
		return false;
	}

	DcUser * dcUser = NULL;
	vector<DcUser *> ul;
	for (List_t::const_iterator it = nickList.begin(); it != nickList.end(); ++it) {
		// NMDC
		dcUser = static_cast<DcUser *> (mDcUserList.getUserBaseByNick(*it));
		if (dcUser && dcUser->isCanSend()) {
			dcUser->setCanSend(false);
			ul.push_back(dcUser);
		}
	}

	if (from && nick) { // PM
		string start, end;
		NmdcProtocol::appendPmToAll(start, end, from, nick, data);
		mDcUserList.sendWithNick(start, end);
	} else if (nick) { // Chat
		string str;
		mDcUserList.sendToAll(NmdcProtocol::appendChat(str, nick, data), false, false);
	} else { // Simple Msg
		string msg(data);
		if (msg.find(NMDC_SEPARATOR, msg.size() - NMDC_SEPARATOR_LEN) == msg.npos) {
			msg.append(NMDC_SEPARATOR);
		}
		mDcUserList.sendToAll(msg, false, false);
	}

	for (vector<DcUser *>::iterator ul_it = ul.begin(); ul_it != ul.end(); ++ul_it) {
		(*ul_it)->setCanSend(true);
	}

	return true;
}



bool DcServer::sendToAllExceptIps(const vector<string> & ipList, const char * data, const char * nick, const char * from) {
	if (!data) {
		return false;
	}

	DcConn * dcConn = NULL;
	vector<DcConn*> ul;
	bool badIp = false;
	for (List_t::const_iterator it = ipList.begin(); it != ipList.end(); ++it) {
		if (!DcConn::checkIp(*it)) {
			badIp = true;
		}
		for (DcIpList::iterator mit = mIpListConn->begin((*it).c_str()); mit != mIpListConn->end(); ++mit) {
			dcConn = static_cast<DcConn *> (*mit);
			if (dcConn->mDcUser && dcConn->mDcUser->isCanSend() && dcConn->getIp() == (*it)) {
				dcConn->mDcUser->setCanSend(false);
				ul.push_back(dcConn);
			}
		}
	}

	if (!badIp) {
		if (from && nick) { // PM
			string start, end;
			NmdcProtocol::appendPmToAll(start, end, from, nick, data);
			mDcUserList.sendWithNick(start, end);
		} else if (nick) { // Chat
			string str;
			mDcUserList.sendToAll(NmdcProtocol::appendChat(str, nick, data), false, false);
		} else { // Simple Msg
			string msg(data);
			if (msg.find(NMDC_SEPARATOR, msg.size() - NMDC_SEPARATOR_LEN) == msg.npos) {
				msg.append(NMDC_SEPARATOR);
			}
			mDcUserList.sendToAll(msg, false, false);
		}
	}

	for (vector<DcConn*>::iterator ul_it = ul.begin(); ul_it != ul.end(); ++ul_it) {
		(*ul_it)->mDcUser->setCanSend(true);
	}

	return (badIp == true) ? false : true;
}



void DcServer::forceMove(DcUserBase * dcUserBase, const char * address, const char * reason /* = NULL */) {
	if (!address) {
		return;
	}
	DcConn * dcConn = static_cast<DcConn *> (dcUserBase->mDcConnBase);

	string msg, force, nick("<unknown>");
	if (dcConn->mDcUser && !dcConn->mDcUser->getUid().empty()) {
		nick = dcConn->mDcUser->getUid();
	}

	stringReplace(mDcLang.mForceMove, "address", force, address);
	stringReplace(force, "reason", force, reason != NULL ? reason : "");
	NmdcProtocol::appendPm(msg, nick, mDcConfig.mHubBot, mDcConfig.mHubBot, force);
	NmdcProtocol::appendChat(msg, mDcConfig.mHubBot, force);
	dcConn->send(NmdcProtocol::appendForceMove(msg, address));
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



bool DcServer::setConfig(const string & name, const string & value) {
	if (name == "sAddresses") {
		return false;
	}

	if (name == "sLocale" && !setlocale(LC_ALL, value.c_str())) {
		return false;
	}

	Config * config = mDcConfig[name];
	if (!config) {
		return false;
	}

	if (name == "sHubBot") {
		unregBot(mDcConfig.mHubBot);
	} else if (name == "bRegMainBot") {
		if (value == "true" || 0 != atoi(value.c_str()) ) {
			regBot(mDcConfig.mHubBot, mDcConfig.mMainBotMyinfo, 
				mDcConfig.mMainBotIp, mDcConfig.mMainBotKey);
		} else {
			unregBot(mDcConfig.mHubBot);
		}
	}

	config->convertFrom(value);

	if (name == "sHubBot") {
		if (mDcConfig.mRegMainBot) { /** Registration bot */
			regBot(mDcConfig.mHubBot, mDcConfig.mMainBotMyinfo, 
				mDcConfig.mMainBotIp, mDcConfig.mMainBotKey);
		}
	} else if (name == "sHubName" || name == "sTopic") {
		// NMDC
		string msg;
		sendToAll(NmdcProtocol::appendHubName(msg, mDcConfig.mHubName, mDcConfig.mTopic).c_str()); // use cache ?
	}

	mDcConfig.save();
	return true;
}



bool DcServer::setLang(const string & name, const string & value) {
	Config * config = mDcLang[name];
	if (!config) {
		return false;
	}
	config->convertFrom(value);
	mDcLang.save();
	return true;
}



int DcServer::regBot(const string & nick, const string & info, const string & ip, bool key) {
	DcUser * dcUser = new DcUser();
	dcUser->setUid(nick);
	dcUser->setProfile(30);
	dcUser->mDcServer = this;
	dcUser->setInOpList(key);

	if (DcConn::checkIp(ip)) {
		dcUser->setIp(ip);
	}

	// Protocol dependence
	if (!nick.length() || nick.length() > 64 || nick.find_first_of(" |$") != nick.npos) {
		return -1;
	}
	string myInfo("$MyINFO $ALL ");
	if (!dcUser->setMyInfo(myInfo.append(nick).append(" ", 1).append(info))) {
		myInfo = "$MyINFO $ALL ";
		if (!dcUser->setMyInfo(myInfo.append(nick).append(" $ $$$0$", 9))) {
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
	mBotList.add(dcUser->getUidHash(), dcUser);
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
	int osVersionInfoEx;

	string version;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	osVersionInfoEx = GetVersionEx((OSVERSIONINFO *) &osvi);
	if (!osVersionInfoEx) {
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if (!GetVersionEx((OSVERSIONINFO *) &osvi)) {
			return "unknown";
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

			if (osVersionInfoEx) {

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
					return "unknown";
				}

				lRet = RegQueryValueExA( hKey, "ProductType", NULL, NULL, (LPBYTE) szProductType, &dwBufLen);

				if ((lRet != ERROR_SUCCESS) || (dwBufLen > 80)) {
					return "unknown";
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
	utsname osname;
	if (uname(&osname) == 0) {
		string version(osname.sysname);
		version.append(" ", 1);
		version.append(osname.release);
		version.append(" (", 2);
		version.append(osname.machine);
		version.append(")", 1);
		return version;
	}
	return "unknown";
}

#endif

}; // namespace dcserver

/**
 * $Id$
 * $HeadURL$
 */
