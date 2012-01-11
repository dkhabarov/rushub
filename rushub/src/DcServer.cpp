/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz
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

#include "DcServer.h"
#include "WebConn.h"

#include <string.h>


#ifdef _WIN32
	#if defined(_MSC_VER) && (_MSC_VER >= 1400)
		#pragma warning(disable:4996) // Disable "This function or variable may be unsafe."
	#endif
#else
	#include <sys/utsname.h> // for utsname

	// include from autoconf
	#include "config.h"

	// capabilit

	#if HAVE_LIBCAP
		#include <grp.h>
		#include <pwd.h>
		#include <sys/types.h>
		#include <unistd.h>
		#include <sys/capability.h>
		#include <sys/prctl.h>
	#endif // HAVE_CAPABILITY
#endif


using namespace ::webserver;


namespace dcserver {



DcListIterator::DcListIterator(DcServer * dcServer) :
	mIt(dcServer->mClientList.begin()),
	mEnd(dcServer->mClientList.end())
{
}



DcServer * DcServer::currentDcServer = NULL;
string DcServer::mSysVersion;



DcServer::DcServer(const string & configFile, const string &) :
	Server(),
	mDcConfig(&mDcConfigLoader, mServer, configFile.c_str()),
	mDcLang(&mDcConfigLoader, &mDcConfig),
	mSystemLoad(SYSTEM_LOAD_OK),
	mStartTime(true),
	mDcUserList("UserList"),
	mBotList("BotList"),
	mOpList("OpList"),
	mIpList("IpList"),
	mActiveList("ActiveList"),
	mHelloList("HelloList"),
	mEnterList("EnterList"),
	mChatList("ChatList"),
	mAdcUserList("AdcUserList"),
	miTotalUserCount(0),
	miTotalShare(0),
	mPluginList(mDcConfig.mPluginPath),
	mHubName(INTERNALNAME " " INTERNALVERSION " " __DATE__ " " __TIME__),
	mWebProtocol(NULL),
	mIpEnterFlood(mDcConfig.mFloodCountReconnIp, mDcConfig.mFloodTimeReconnIp),
	mCalls(&mPluginList)
{
	setClassName("DcServer");

	// Current server
	currentDcServer = this;

	// Put some capabilities
	setCapabilities();

	// Define OS
	if (mSysVersion.empty()) {
		mSysVersion = getSysVersion();
	}

	// DcIpList
	mIpListConn = new DcIpList();

	mAdcProtocol.setServer(this);
	mNmdcProtocol.setServer(this);
	mPluginList.setServer(this);

	mPluginList.loadAll(); // Load plugins


	mDcUserList.addUserListItem(NmdcProtocol::nickList, "$NickList ");
	mDcUserList.addUserListItem(NmdcProtocol::myInfoList, "");
	mDcUserList.addUserListItem(NmdcProtocol::ipList, "$UserIP ");
	mOpList.addUserListItem(NmdcProtocol::nickList, "$OpList ");

	mAdcUserList.addUserListItem(AdcProtocol::infList, "");


	if (mDcConfig.mRegMainBot) { // Main bot registration
		if (log(DEBUG)) {
			logStream() << "Reg main bot '" << mDcConfig.mHubBot << "'" << endl;
		}
		regBot(mDcConfig.mHubBot, mDcConfig.mMainBotMyinfo,
			mDcConfig.mMainBotIp, mDcConfig.mMainBotKey);
	}
}



DcServer::~DcServer() {
	if (log(INFO)) {
		logStream() << "Destruct DcServer" << endl;
	}

	mPluginList.unloadAll(); // Unload all plugins

	if (mDcConfig.mRegMainBot) { // Main bot unreg
		if (log(DEBUG)) {
			logStream() << "Unreg main bot '" << mDcConfig.mHubBot << "'" << endl;
		}
		unregBot(mDcConfig.mHubBot);
	}

	DcUser * Us = NULL;
	UserList::iterator it = mDcUserList.begin();
	UserList::iterator it_e = mDcUserList.end();
	while (it != it_e) {
		Us = static_cast<DcUser *> (*it++);
		if (Us->mDcConn) {
			delConnection(Us->mDcConn);
		} else {
			if (Us->isTrueBoolParam(USER_PARAM_IN_USER_LIST)) {
				removeFromDcUserList(Us);
			}
			delete Us;
		}
	}

	it = mAdcUserList.begin();
	it_e = mAdcUserList.end();
	while (it != it_e) {
		Us = static_cast<DcUser *> (*it++);
		if (Us->mDcConn) {
			delConnection(Us->mDcConn);
		} else {
			if (Us->isTrueBoolParam(USER_PARAM_IN_USER_LIST)) {
				removeFromDcUserList(Us);
			}
			delete Us;
		}
	}

	deleteAll(); // Delete all other conn


	// Delete all ConnFactory of server
	ConnFactory * connFactory = NULL;
	vector<ConnFactory *>::iterator cf_it = mConnFactories.begin();
	vector<ConnFactory *>::iterator cf_it_e = mConnFactories.end();
	while (cf_it != cf_it_e) {
		connFactory = (*cf_it++);
		delete connFactory;
	}
	mConnFactories.clear();


	if (mIpListConn != NULL) {
		delete mIpListConn;
		mIpListConn = NULL;
	}

	if (mWebProtocol != NULL) {
		delete mWebProtocol;
		mWebProtocol = NULL;
	}
}



// static
void DcServer::getAddresses(
	const char * sAddresses,
	vector<pair<string, string> > & vec,
	const char * defaultPort
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
		vec.push_back(pair<string, string>(vIpPort[0], vIpPort.size() == 2 ? vIpPort[1] : defaultPort));
	}
}



bool DcServer::listeningServer(const char * name, const char * addresses, const char * defaultPort, ConnFactory * connFactory, bool udp /*= false*/) {
	vector<pair<string, string> > vAddresses;
	getAddresses(addresses, vAddresses, defaultPort);

	if (vAddresses.size() == 0) {
		if (log(FATAL)) {
			logStream() << "Incorrect address of the " << name << endl;
		}
	}

	bool ret = false;
	for (vector<pair<string, string> >::iterator it = vAddresses.begin(); it != vAddresses.end(); ++it) {
		if (Server::listening(connFactory, ((*it).first).c_str(), ((*it).second).c_str(), udp) != NULL) {
			ret = true;
			if (log(INFO)) {
				logStream() << name << " is running on [" 
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



/// Listening all servers
int DcServer::listening() {

	if (mDcConfig.mAdcOn) {

		ConnFactory * adcConnFactory = new DcConnFactory(&mAdcProtocol, this); // ADC PROTOCOL
		mConnFactories.push_back(adcConnFactory);

		// ADC Server
		if (!listeningServer("ADC Server " INTERNALNAME " " INTERNALVERSION, mDcConfig.mAddresses.c_str(), "411", adcConnFactory)) {
			return -1;
		}

	} else {

		ConnFactory * nmdcConnFactory = new DcConnFactory(&mNmdcProtocol, this); // NMDC PROTOCOL
		mConnFactories.push_back(nmdcConnFactory);

		// NMDC Server
		if (!listeningServer("NMDC Server " INTERNALNAME " " INTERNALVERSION, mDcConfig.mAddresses.c_str(), "411", nmdcConnFactory)) {
			return -1;
		}

		// UDP NMDC Server
		if (mDcConfig.mUdpServer) {
			listeningServer("DC Server (UDP)", mDcConfig.mUdpAddresses.c_str(), "1209", nmdcConnFactory, true);
		}

	}

	if (!mWebProtocol) {
		mWebProtocol = new WebProtocol(mDcConfig.mMaxWebCommandLength);
	}

	ConnFactory * webConnFactory = new WebConnFactory(mWebProtocol, this);
	mConnFactories.push_back(webConnFactory);

	// Web Server
	if (mDcConfig.mWebServer) {
		listeningServer("Web Server", mDcConfig.mWebAddresses.c_str(), "80", webConnFactory);
	}

	return 0;
}



int DcServer::onTimer(Time & now) {

	// Execute each second
	if ((__int64) (now - mChecker) >= mTimerServPeriod) {

		mChecker = now;

		if (!mDcConfig.mAdcOn) {
			mHelloList.flushCache(); // NMDC
			mDcUserList.flushCache(); // NMDC
			mBotList.flushCache();
			mEnterList.flushCache();
			mOpList.flushCache();
			mIpList.flushCache();
			mChatList.flushCache();
			mActiveList.flushCache();
		} else {
			mAdcUserList.flushCacheAdc(); // ADC
			mBotList.flushCacheAdc();
			mEnterList.flushCacheAdc();
			mOpList.flushCacheAdc();
			mIpList.flushCacheAdc();
			mChatList.flushCacheAdc();
			mActiveList.flushCacheAdc();
		}


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
			if (log(WARN)) {
				logStream() << "System loading: " 
					<< mSystemLoad << " level (was " 
					<< SysLoading << " level)" << endl;
			}
		}

		mIpEnterFlood.del(now); // Removing ip addresses, which already long ago entered

		mDcUserList.autoResize(); // NMDC
		mAdcUserList.autoResize(); // ADC
		mHelloList.autoResize(); // NMDC
		mBotList.autoResize();
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



/// Function action after joining the client
int DcServer::onNewConn(Conn * conn) {
	DcConn * dcConn = static_cast<DcConn *> (conn);

	if (mSystemLoad == SYSTEM_LOAD_SYSTEM_DOWN) {
		if (dcConn->log(WARN)) {
			dcConn->logStream() << "System down, close" << endl;
		}
		dcConn->closeNow(CLOSE_REASON_HUB_LOAD);
		return -1;
	}

	// Checking flood-entry (by ip)
	if (mIpEnterFlood.check(dcConn->getIp(), mTime)) {
		dcConn->closeNow(CLOSE_REASON_FLOOD_IP_ENTRY);
		return -2;
	}

	// TODO fix me (refactoring)
	mIpListConn->add(dcConn); // Adding connection in IP-list
	dcConn->mDcUser->setIp(dcConn->getIp());

	if (dcConn->log(TRACE)) {
		dcConn->logStream() << "[S]Stage onNewConn" << endl;
	}

	if (dcConn->mProtocol != NULL) {
		dcConn->mProtocol->onNewConn(dcConn);
	}

	if (dcConn->log(TRACE)) {
		dcConn->logStream() << "[E]Stage onNewConn" << endl;
	}
	return 0;
}



/// Returns pointer to line of the connection, in which will be recorded got data
string * DcServer::createCommandPtr(Conn * conn) {
	return conn->getParserCommandPtr();
}



/// Function of the processing enterring data
void DcServer::onNewData(Conn * conn, string * data) {

	if (conn->log(TRACE)) {
		conn->logStream() << "IN: " << (*data) << endl;
	}

	Parser * parser = conn->mParser;
	Protocol * protocol = conn->mProtocol;

	if (parser != NULL && protocol != NULL) {

		parser->parse();

		if (conn->getConnType() == CONN_TYPE_INCOMING_UDP) {
			onNewUdpData(conn, parser);
		} else {
			protocol->doCommand(parser, conn); // Do protocol command
		}
	}
}



void DcServer::onNewUdpData(Conn * conn, Parser * parser) {

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



/// Function checks min interval
bool DcServer::minDelay(Time & time, double sec) {
	if (::fabs(double(mTime - time)) >= sec) {
		time = mTime;
		return true;
	}
	return false;
}



/// Antiflood function
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



/// Checking for this nick used
bool DcServer::checkNick(DcConn *dcConn) {

	// check empty nick!
	if (dcConn->mDcUser->getUid().empty()) {
		return false;
	}

	unsigned long uidHash = dcConn->mDcUser->getUidHash();
	if (mDcUserList.contain(uidHash)) {
		DcUser * us = static_cast<DcUser *> (mDcUserList.find(uidHash));

		if (!us->mDcConn || (us->getParamForce(USER_PARAM_PROFILE)->getInt() != -1 && us->getIp() != dcConn->getIp())) {
			if (dcConn->log(DEBUG)) {
				dcConn->logStream() << "Bad nick (used): '" 
					<< dcConn->mDcUser->getUid() << "'["
					<< dcConn->getIp() << "] vs '" << us->getUid() 
					<< "'[" << us->getIp() << "]" << endl;
			}
			string msg;
			stringReplace(mDcLang.mUsedNick, "nick", msg, dcConn->mDcUser->getUid());
			sendToUser(dcConn->mDcUser, msg.c_str(), mDcConfig.mHubBot.c_str());
			dcConn->send(mNmdcProtocol.appendValidateDenied(msg.erase(), dcConn->mDcUser->getUid())); // refactoring to DcProtocol pointer
			return false;
		}
		if (us->mDcConn->log(DEBUG)) {
			us->mDcConn->logStream() << "removed old user" << endl;
		}
		removeFromDcUserList(us);
		us->mDcConn->closeNow(CLOSE_REASON_USER_OLD);
	}
	return true;
}



bool DcServer::beforeUserEnter(DcConn * dcConn) {
	if (dcConn->log(DEBUG)) {
		dcConn->logStream() << "Begin login" << endl;
	}

	if (!mDcConfig.mAdcOn) {
		// check empty nick!
		if (!checkNick(dcConn)) {
			dcConn->closeNice(9000, CLOSE_REASON_NICK_INVALID);
			return false;
		}
	}

	if (dcConn->mSendNickList) {
		if (!mDcConfig.mDelayedLogin) {
			doUserEnter(dcConn);
		} else {
			mEnterList.add(dcConn->mDcUser->getUidHash(), dcConn->mDcUser);
		}

		// Can happen so that list not to send at a time
		if (!mDcConfig.mAdcOn) {
			mNmdcProtocol.sendNickList(dcConn);
		} else {
			mAdcProtocol.sendNickList(dcConn);
		}

		dcConn->mSendNickList = false;
	} else if (!dcConn->mDcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST)) {
		doUserEnter(dcConn);
	}
	return true;
}



/// User entry
void DcServer::doUserEnter(DcConn * dcConn) {

	// Check entry stages
	if (!mDcConfig.mAdcOn) {
		if (LOGIN_STATUS_LOGIN_DONE != dcConn->getLoginStatusFlag(LOGIN_STATUS_LOGIN_DONE)) {
			if (dcConn->log(DEBUG)) {
				dcConn->logStream() << "User Login when not all done (" 
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
	}

	unsigned long uidHash = dcConn->mDcUser->getUidHash();

	// User is already considered came
	if (mEnterList.contain(uidHash)) {
		// We send user contents of cache without clear this cache
		mEnterList.flushForUser(dcConn->mDcUser);
		mEnterList.remove(uidHash);
	}

	// Adding user to the user list
	if (!addToUserList(static_cast<DcUser *> (dcConn->mDcUser))) {
		dcConn->closeNow(CLOSE_REASON_USER_ADD);
		return;
	}

	// Show to all
	showUserToAll(dcConn->mDcUser);

	afterUserEnter(dcConn);

	if (!mDcConfig.mAdcOn) {
		dcConn->clearTimeOut(HUB_TIME_OUT_LOGIN);
		(static_cast<DcUser *> (dcConn->mDcUser))->mTimeEnter.get();
	}
}



/// Adding user in the user list
bool DcServer::addToUserList(DcUser * dcUser) {
	if (!dcUser) {
		if (log(ERR)) {
			logStream() << "Adding a NULL user to userlist" << endl;
		}
		return false;
	}
	if (dcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST)) {
		if (log(ERR)) {
			logStream() << "User is already in the user list" << endl;
		}
		return false;
	}

	unsigned long uidHash = dcUser->getUidHash();

	UserList * mainUserList = NULL;
	if (!mDcConfig.mAdcOn) {
		mainUserList = &mDcUserList;
	} else {
		mainUserList = &mAdcUserList;
	}

	if (mainUserList->log(TRACE)) {
		mainUserList->logStream() << "Before add: " << dcUser->getUid() << " Size: " << mainUserList->size() << endl;
	}

	if (!mainUserList->add(uidHash, dcUser)) {
		if (log(DEBUG)) {
			logStream() << "Adding twice user with same nick " << dcUser->getUid() << " (" << mainUserList->find(uidHash)->getUid() << ")" << endl;
		}
		dcUser->setInUserList(false);
		return false;
	}

	if (mainUserList->log(TRACE)) {
		mainUserList->logStream() << "After add: " << dcUser->getUid() << " Size: " << mainUserList->size() << endl;
	}


	if (!mDcConfig.mAdcOn) {
		if (!dcUser->isPassive()) {
			mActiveList.add(uidHash, dcUser);
		}
		if (dcUser->isTrueBoolParam(USER_PARAM_IN_OP_LIST)) {
			mOpList.add(uidHash, dcUser);
		}
		if (dcUser->isTrueBoolParam(USER_PARAM_IN_IP_LIST)) {
			mIpList.add(uidHash, dcUser);
		}
	}

	dcUser->setInUserList(true);
	dcUser->setCanSend(true);

	if (dcUser->mDcConn) {

		++ miTotalUserCount; // add except bot

		dcUser->mDcConn->mIpRecv = true; // Installing the permit on reception of the messages on ip
		mChatList.add(uidHash, dcUser);

		if (!mDcConfig.mAdcOn) {
			if (!(dcUser->mDcConn->mFeatures & SUPPORT_FEATURE_NOHELLO)) {
				mHelloList.add(uidHash, dcUser);
			}
		}

		if (dcUser->mDcConn->log(DEBUG)) {
			dcUser->mDcConn->logStream() << "Adding at the end of Nicklist" << endl;
		}
	}
	return true;
}



/// Removing user from the user list
bool DcServer::removeFromDcUserList(DcUser * dcUser) {
	unsigned long uidHash = dcUser->getUidHash();

	UserList * mainUserList = NULL;
	if (!mDcConfig.mAdcOn) {
		mainUserList = &mDcUserList;
	} else {
		mainUserList = &mAdcUserList;
	}

	if (mainUserList->log(TRACE)) {
		mainUserList->logStream() << "Before leave: " << dcUser->getUid() << " Size: " << mainUserList->size() << endl;
	}
	if (mainUserList->contain(uidHash)) {

		if (!mDcConfig.mAdcOn) {
			#ifndef WITHOUT_PLUGINS
				if (dcUser->mDcConn) {
					mCalls.mOnUserExit.callAll(dcUser);
				}
			#endif
		}

		if (dcUser->mDcConn != NULL) {
			-- miTotalUserCount;
		}

		// We make sure that user with such nick one!
		DcUser * other = static_cast<DcUser *> (mainUserList->find(dcUser->getUidHash()));
		if (!dcUser->mDcConn) { // Removing the bot
			mainUserList->remove(uidHash);
		} else if (other && other->mDcConn && dcUser->mDcConn && other->mDcConn == dcUser->mDcConn) {
			mainUserList->remove(uidHash);
			if (mainUserList->log(TRACE)) {
				mainUserList->logStream() << "After leave: " << dcUser->getUid() << " Size: " << mainUserList->size() << endl;
			}
		} else {
			// Such can happen only for users without connection or with different connection
			if (dcUser->log(ERR)) {
				dcUser->logStream() << "Not found the correct user for nick: " << dcUser->getUid() << endl;
			}
			return false;
		}
	}

	// Removing from lists
	mOpList.remove(uidHash);
	mIpList.remove(uidHash);
	mEnterList.remove(uidHash);
	mActiveList.remove(uidHash);
	mChatList.remove(uidHash);
	mBotList.remove(uidHash);
	if (!mDcConfig.mAdcOn) {
		mHelloList.remove(uidHash); // NMDC
	}

	if (dcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST)) {
		dcUser->setInUserList(false);

		if (!dcUser->isTrueBoolParam(USER_PARAM_CAN_HIDE)) {
			string msg;

			if (!mDcConfig.mAdcOn) {
				mNmdcProtocol.appendQuit(msg, dcUser->getUid());
				mDcUserList.sendToAll(msg, true/*mDcConfig.mDelayedMyinfo*/, false); // Delay in sending MyINFO (and Quit)
			} else {
				msg.append("IQUI ").append(dcUser->getUid()).append(ADC_SEPARATOR);
				mAdcUserList.sendToAllAdc(msg, true/*mDcConfig.mDelayedMyinfo*/, false);
			}
		}
	}
	return true;
}



/// Show user to all
bool DcServer::showUserToAll(DcUser * dcUser) {

	if (!mDcConfig.mAdcOn) {
		string hello;
		if (dcUser->mDcConn && dcUser->isTrueBoolParam(USER_PARAM_CAN_HIDE)) {
			if (dcUser->mDcConn->mFeatures & SUPPORT_FEATURE_NOHELLO) {
				dcUser->mDcConn->send(dcUser->getInfo(), true, false);
			} else if (dcUser->mDcConn->mFeatures & SUPPORT_FEATUER_NOGETINFO) {
				dcUser->mDcConn->send(mNmdcProtocol.appendHello(hello, dcUser->getUid()), false, false); // refactoring to DcProtocol pointer
				dcUser->mDcConn->send(dcUser->getInfo(), true, false);
			} else {
				dcUser->mDcConn->send(mNmdcProtocol.appendHello(hello, dcUser->getUid()), false, false); // refactoring to DcProtocol pointer
			}

			if (dcUser->isTrueBoolParam(USER_PARAM_IN_OP_LIST)) {
				string opList;
				dcUser->mDcConn->send(mNmdcProtocol.appendOpList(opList, dcUser->getUid()), false, false); // refactoring to DcProtocol pointer
			}
		} else {

			// Sending the greeting for all users, not supporting feature NoHello (except enterring users)
			mHelloList.sendToAll(mNmdcProtocol.appendHello(hello, dcUser->getUid()), true/*mDcConfig.mDelayedMyinfo*/, false); // refactoring to DcProtocol pointer

			// Show MyINFO string to all users
			mDcUserList.sendToAll(dcUser->getInfo(), true/*mDcConfig.mDelayedMyinfo*/); // use cache -> so this can be after user is added

			// Show MyINFO string of the current user to all enterring users
			mEnterList.sendToAll(dcUser->getInfo(), true/*mDcConfig.mDelayedMyinfo*/);

			// Op entry
			if (dcUser->isTrueBoolParam(USER_PARAM_IN_OP_LIST)) {
				string opList;
				mDcUserList.sendToAll(mNmdcProtocol.appendOpList(opList, dcUser->getUid()), true/*mDcConfig.mDelayedMyinfo*/, false); // refactoring to DcProtocol pointer
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
			mNmdcProtocol.appendUserIp(ipList, dcUser->getUid(), dcUser->getIp()); // refactoring to DcProtocol pointer
			if (ipList.size()) {
				mIpList.sendToAll(ipList, true, false);
			}

			if (dcUser->isTrueBoolParam(USER_PARAM_IN_IP_LIST)) {
				dcUser->send(mDcUserList.getList(USER_LIST_IP), true, false);
			} else if (ipList.size() && dcUser->mDcConn && (dcUser->mDcConn->mFeatures & SUPPORT_FEATUER_USERIP2)) { // UserIP2
				dcUser->send(ipList, false, false);
			}
		}

		dcUser->send("", 0, false, true);
	} else {

		bool canSend = dcUser->isCanSend();

		if (!dcUser->isTrueBoolParam(USER_PARAM_CAN_HIDE)) {

			// Show MyINFO string to all users
			mAdcUserList.sendToAllAdc(dcUser->getInfo(), true/*mDcConfig.mDelayedMyinfo*/); // use cache -> so this can be after user is added

			// Show INF string of the current user to all enterring users
			mEnterList.sendToAllAdc(dcUser->getInfo(), true/*mDcConfig.mDelayedMyinfo*/);

		}

		// Prevention of the double sending
		if (!mDcConfig.mDelayedLogin) {
			dcUser->setCanSend(false);
			mAdcUserList.flushCacheAdc();
			mEnterList.flushCacheAdc();
			dcUser->setCanSend(canSend);
		}

		dcUser->send("", 0, false, true);
	}
	return true;
}



void DcServer::afterUserEnter(DcConn *dcConn) {
	if (dcConn->log(DEBUG)) {
		dcConn->logStream() << "Entered on the hub" << endl;
	}
	if (!mDcConfig.mAdcOn) {
		#ifndef WITHOUT_PLUGINS
			mCalls.mOnUserEnter.callAll(dcConn->mDcUser);
		#endif
	}
}



/// Get user by nick (or NULL)
DcUser * DcServer::getDcUser(const char * nick) {
	if (nick == NULL) {
		return NULL;
	}
	string nickStr(nick);
	if (nickStr.size()) {
		// NMDC
		UserBase * userBase = mDcUserList.getUserBaseByUid(nickStr);
		if (userBase) {
			return static_cast<DcUser *> (userBase);
		}
		DcConn * dcConn = NULL;
		for (tCLIt it = mClientList.begin(); it != mClientList.end(); ++it) {
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



/// Send data to user
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
		dcConn->send(mNmdcProtocol.appendPm(str, to, from, nick, data)); // refactoring to DcProtocol pointer
		return true;
	}

	// Chat
	if (nick) {
		string str;
		dcConn->send(mNmdcProtocol.appendChat(str, nick, data)); // refactoring to DcProtocol pointer
		return true;
	}

	// Simple Msg
	dcConn->send(data, strlen(data), dcConn->mType == CLIENT_TYPE_NMDC); // TODO refactoring len
	return true;
}



/// Send data to nick
bool DcServer::sendToNick(const char * to, const char * data, const char * nick, const char * from) {
	DcUser * dcUser = getDcUser(to);
	if (!dcUser || !dcUser->mDcConn) { // Check exist and not bot
		return false;
	}
	return sendToUser(dcUser, data, nick, from);
}



/// Send data to all
bool DcServer::sendToAll(const char * data, const char * nick, const char * from) {
	if (!data) {
		return false;
	}

	// PM
	if (from && nick) {
		string start, end;
		mNmdcProtocol.appendPmToAll(start, end, from, nick, data); // refactoring to DcProtocol pointer
		mDcUserList.sendWithNick(start, end);
		return true;
	}

	// Chat
	if (nick) {
		string str;
		mDcUserList.sendToAll(mNmdcProtocol.appendChat(str, nick, data), false, false); // refactoring to DcProtocol pointer
		return true;
	}

	mDcUserList.sendToAll(data, false, true);
	return true;
}



/// Send data to profiles
bool DcServer::sendToProfiles(unsigned long profile, const char * data, const char * nick, const char * from) {
	if (!data) {
		return false;
	}

	// PM
	if (from && nick) {
		string start, end;
		mNmdcProtocol.appendPmToAll(start, end, from, nick, data); // refactoring to DcProtocol pointer
		mDcUserList.sendWithNick(start, end, profile);
		return true;
	}

	// Chat
	if (nick) {
		string str;
		mDcUserList.sendToProfiles(profile, mNmdcProtocol.appendChat(str, nick, data), false); // refactoring to DcProtocol pointer
		return true;
	}

	// Simple Msg
	mDcUserList.sendToProfiles(profile, data, true);
	return true;
}



bool DcServer::sendToIp(const char * ip, const char * data, unsigned long profile, const char * nick, const char * from) {
	if (!ip || !data || !Conn::checkIp(ip)) {
		return false;
	}

	// PM
	if (from && nick) {
		string start, end;
		mNmdcProtocol.appendPmToAll(start, end, from, nick, data); // refactoring to DcProtocol pointer
		mIpListConn->sendToIpWithNick(ip, start.c_str(), end.c_str(), profile);
		return true;
	}

	// Chat
	if (nick) {
		string str;
		mIpListConn->sendToIp(ip, mNmdcProtocol.appendChat(str, nick, data).c_str(), profile); // refactoring to DcProtocol pointer
		return true;
	}

	// Simple Msg
	mIpListConn->sendToIp(ip, data, profile);
	return true;
}



/// Send data to all except nick list
bool DcServer::sendToAllExceptNicks(const vector<string> & nickList, const char * data, const char * nick, const char * from) {
	if (!data) {
		return false;
	}

	DcUser * dcUser = NULL;
	vector<DcUser *> ul;
	for (List_t::const_iterator it = nickList.begin(); it != nickList.end(); ++it) {
		// NMDC
		dcUser = static_cast<DcUser *> (mDcUserList.getUserBaseByUid(*it));
		if (dcUser && dcUser->isCanSend()) {
			dcUser->setCanSend(false);
			ul.push_back(dcUser);
		}
	}

	if (from && nick) { // PM
		string start, end;
		mNmdcProtocol.appendPmToAll(start, end, from, nick, data); // refactoring to DcProtocol pointer
		mDcUserList.sendWithNick(start, end);
	} else if (nick) { // Chat
		string str;
		mDcUserList.sendToAll(mNmdcProtocol.appendChat(str, nick, data), false, false); // refactoring to DcProtocol pointer
	} else { // Simple Msg
		mDcUserList.sendToAll(data, false, true);
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
			mNmdcProtocol.appendPmToAll(start, end, from, nick, data); // refactoring to DcProtocol pointer
			mDcUserList.sendWithNick(start, end);
		} else if (nick) { // Chat
			string str;
			mDcUserList.sendToAll(mNmdcProtocol.appendChat(str, nick, data), false, false); // refactoring to DcProtocol pointer
		} else { // Simple Msg
			mDcUserList.sendToAll(data, false, true);
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
	mNmdcProtocol.appendPm(msg, nick, mDcConfig.mHubBot, mDcConfig.mHubBot, force); // refactoring to DcProtocol pointer
	mNmdcProtocol.appendChat(msg, mDcConfig.mHubBot, force); // refactoring to DcProtocol pointer
	dcConn->send(mNmdcProtocol.appendForceMove(msg, address)); // refactoring to DcProtocol pointer
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



const char * DcServer::getConfig(const string & name) {
	ConfigItem * configItem = mDcConfig[name];
	if (configItem == NULL) {
		return NULL;
	}
	configItem->convertTo(sBuf);
	return sBuf.c_str();
}



bool DcServer::setConfig(const string & name, const string & value) {
	if (name == "sAddresses" || name == "bAdcOn") {
		return false;
	}

	if (name == "sLocale" && !setlocale(LC_ALL, value.c_str())) {
		return false;
	}

	ConfigItem * configItem = mDcConfig[name];
	if (configItem == NULL) {
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

	configItem->convertFrom(value);

	if (name == "sHubBot") {
		if (mDcConfig.mRegMainBot) { // Registration bot
			regBot(mDcConfig.mHubBot, mDcConfig.mMainBotMyinfo, 
				mDcConfig.mMainBotIp, mDcConfig.mMainBotKey);
		}
	} else if (name == "sHubName" || name == "sTopic") {
		// NMDC
		string msg;
		sendToAll(mNmdcProtocol.appendHubName(msg, mDcConfig.mHubName, mDcConfig.mTopic).c_str()); // use cache ?
	}

	mDcConfig.save();
	return true;
}



const char * DcServer::getLang(const string & name) {
	ConfigItem * configItem = mDcLang[name];
	if (configItem == NULL) {
		return NULL;
	}
	configItem->convertTo(sBuf);
	return sBuf.c_str();
}



bool DcServer::setLang(const string & name, const string & value) {
	ConfigItem * configItem = mDcLang[name];
	if (configItem == NULL) {
		return false;
	}
	configItem->convertFrom(value);
	mDcLang.save();
	return true;
}



int DcServer::regBot(const string & uid, const string & info, const string & ip, bool key) {
	DcUser * dcUser = new DcUser(NULL);
	dcUser->mDcServer = this;
	string sid;
	if (!mDcConfig.mAdcOn) {
		dcUser->setUid(uid);
	} else {
		sid = mAdcProtocol.getSid(1); // TODO: set bot SID!
		dcUser->setUid(sid);
	}
	dcUser->getParamForce(USER_PARAM_PROFILE)->setInt(30);
	dcUser->getParamForce(USER_PARAM_IN_OP_LIST)->setBool(key);

	if (DcConn::checkIp(ip)) {
		dcUser->setIp(ip);
	}

	// Protocol dependence
	if (!mDcConfig.mAdcOn) {
		if (!uid.size() || uid.size() > 0x40 || uid.find_first_of(" |$") != uid.npos) {
			delete dcUser;
			return -1;
		}
		string myInfo("$MyINFO $ALL ");
		if (!dcUser->setInfo(myInfo.append(uid).append(" ", 1).append(info))) {
			myInfo = "$MyINFO $ALL ";
			if (!dcUser->setInfo(myInfo.append(uid).append(" $ $$$0$", 9))) {
				delete dcUser;
				return -2;
			}
		}
	} else {
		const char * ct = "1";
		if (key) {
			ct = "5";
		}
		string inf("IINF ");
		inf.append(sid).append(" CT").append(ct).append(" NI").append(uid);
		inf.append(" SS0 HN0 HR0 HO1 VEBot\\sV:1.0 SL0 DERusHub\\sbot");
		dcUser->setInfo(inf);
	}

	if (log(DEBUG)) {
		logStream() << "Reg bot: " << uid << endl;
	}

	if (!addToUserList(dcUser)) {
		delete dcUser;
		return -3;
	}
	mBotList.add(dcUser->getUidHash(), dcUser);
	showUserToAll(dcUser);
	return 0;
}



int DcServer::unregBot(const string & uid) {

	if (log(DEBUG)) {
		logStream() << "Unreg bot: " << uid << endl;
	}

	DcUser * dcUser = NULL;
	if (!mDcConfig.mAdcOn) {
		dcUser = static_cast<DcUser *> (mDcUserList.getUserBaseByUid(uid));
	} else {
		dcUser = static_cast<DcUser *> (mAdcUserList.getUserBaseByUid(uid));
	}
	if (!dcUser) {
		return -1;
	}
	if (dcUser->mDcConn) {
		if (log(DEBUG)) {
			logStream() << "Attempt delete user" << endl;
		}
		return -2;
	}
	removeFromDcUserList(dcUser);
	delete dcUser;
	return 0;
}



/** Get normal share size */
void DcServer::getNormalShare(__int64 share, string & normalShare) {
	ostringstream os;
	float s(static_cast<float>(share));
	int i(0);
	for (; ((s >= 1024) && (i < 7)); ++i) {
		s /= 1024;
	}
	os << ::std::floor(s * 1000 + 0.5) / 1000 << " " << DcServer::currentDcServer->mDcLang.mUnits[i];
	normalShare = os.str();
}



string DcServer::getSysVersion() {

#ifdef _WIN32

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

#else

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

#endif

}



bool DcServer::setCapabilities() {

#if (!defined _WIN32) && HAVE_LIBCAP

	if (getuid()) {
		if (log(WARN)) {
			logStream() << "Cannot set capabilities. Hub started from common user, not root." << endl;
		}
		return false;
	}

	struct passwd * user = getpwnam(mDcConfig.mUserName.c_str());
	struct group * grp = getgrnam(mDcConfig.mGroupName.c_str());

	if(user && grp) {
		// Keep capabilities across UID change
		if(prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0) != 0) {
			if (log(WARN)) {
				logStream() << "prctl(PR_SET_KEEPCAPS) failed" << endl;
			}
			return false;
		}

		// Change supplementary groups
		if(initgroups(user->pw_name, user->pw_gid) != 0) {
			if (log(WARN)) {
				logStream() << "initgroups() for user " << user->pw_name << " failed"<< endl;
			}
			return false;
		}

		// Change GID
		if(setgid(grp->gr_gid) != 0) {
			if (log(WARN)) {
				logStream() << "Cannot set GID to " << grp->gr_gid << endl;
			}
			return false;
		}

		// Change UID
		if(setuid(user->pw_uid) != 0) {
			if (log(WARN)) {
				logStream() << "Cannot set UID to " << user->pw_uid << endl;
			}
			return false;
		}
	} else {
		if (log(WARN)) {
			logStream() << "Bad user name. Cannot get pam structs. Check user and group name." << endl;
		}
		return false;
	}

	// Check capability to bind privileged ports
	cap_t caps = cap_get_proc();
	if(!caps) {
		if (log(WARN)) {
			logStream() << "cap_get_proc() failed to get capabilities" << endl;
		}
		return false;
	}

	cap_flag_value_t nbs_flag;
	if(cap_get_flag(caps, CAP_NET_BIND_SERVICE, CAP_PERMITTED, &nbs_flag) != 0) {
		if (log(WARN)) {
			logStream() << "cap_get_flag() failed to get CAP_NET_BIND_SERVICE state" << endl;
		}
		cap_free(caps);
		return false;
	}

	// Drop all capabilities except privileged ports binding
	caps = cap_from_text(nbs_flag == CAP_SET ? "cap_net_bind_service=ep" : "=");
	if(!caps) {
		if (log(WARN)) {
			logStream() << "cap_from_text() failed to parse capabilities string" << endl;
		}
		return false;
	}

	if(cap_set_proc(caps) != 0) {
		if (log(WARN)) {
			logStream() << "cap_set_proc() failed to set capabilities" << endl;
		}
		cap_free(caps);
		return false;
	}
	cap_free(caps);

#endif

	return true;

}


}; // namespace dcserver

/**
 * $Id$
 * $HeadURL$
 */
