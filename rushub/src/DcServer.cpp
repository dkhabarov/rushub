/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz
 *
 * modified: 27 Aug 2009
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

#include "DcServer.h"
#include "WebConn.h"
#include "DcCmd.h"

#include <string.h>
#include <functional>


#ifdef _WIN32
	#if defined(_MSC_VER) && (_MSC_VER >= 1400)
		#pragma warning(disable:4996) // Disable "This function or variable may be unsafe."
	#endif
#else
	#include <sys/utsname.h> // for utsname

	// include from autoconf
	#ifdef HAVE_CONFIG_H
		#include <config.h>
	#endif

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
using namespace ::dcserver::protocol;


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
	mStartTime(true),
	mDcUserList("UserList"),
	mOpList("OpList"),
	mIpList("IpList"),
	mActiveList("ActiveList"),
	mHelloList("HelloList"),
	mEnterList("EnterList"),
	mChatList("ChatList"),
	miTotalUserCount(0),
	miTotalShare(0),
	mPluginList(mDcConfig.mPluginPath),
	mHubName(INTERNALNAME " " INTERNALVERSION " " __DATE__ " " __TIME__),
	mSystemLoad(SYSTEM_LOAD_OK),
	mWebProtocol(NULL),
	mIpEnterFlood(mDcConfig.mFloodCountReconnIp, mDcConfig.mFloodTimeReconnIp),
	mStopSync(false),
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

	// Protocol dependence
	mAdcProtocol.setServer(this); // ADC
	mNmdcProtocol.setServer(this); // NMDC

	mPluginList.setServer(this);
	mPluginList.loadAll(); // Load plugins

	// Preparing all lists
	mDcUserList.addUserListItem(NmdcProtocol::nickList, "$NickList "); // USER_LIST_NICK
	mDcUserList.addUserListItem(NmdcProtocol::myInfoList, ""); // USER_LIST_MYINFO
	mDcUserList.addUserListItem(NmdcProtocol::ipList, "$UserIP "); // USER_LIST_IP
	mDcUserList.addUserListItem(AdcProtocol::infList, ""); // USER_LIST_ADC_INFO
	mOpList.addUserListItem(NmdcProtocol::nickList, "$OpList "); // 0

	if (mDcConfig.mRegMainBot) { // Main bot registration
		LOG(LEVEL_DEBUG, "Reg main bot '" << mDcConfig.mHubBot << "'");
		regBot(mDcConfig.mHubBot, mDcConfig.mMainBotMyinfo,
			mDcConfig.mMainBotIp, mDcConfig.mMainBotKey);
	}

	#ifdef USE_DCSERVER_THREADS
		LOG(LEVEL_INFO, "Used multi-threads mode: yes");
		Thread::start(&DcServer::syncTimer, this);
	#else
		LOG(LEVEL_INFO, "Used multi-threads mode: no");
	#endif
}


void DcServer::delAllUsers(UserBase * userBase) {
	DcUser * dcUser = static_cast<DcUser *> (userBase);
	if (dcUser != NULL) {
		if (dcUser->mDcConn) {
			delConnection(dcUser->mDcConn);
		} else {
			// remove bots!
			if (dcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST)) {
				removeFromDcUserList(dcUser);
			}
			delete dcUser;
		}
	}
}

DcServer::~DcServer() {
	mStopSync = true;
	#ifdef USE_DCSERVER_THREADS
		Thread::stop();
	#endif

	LOG(LEVEL_INFO, "Destruct DcServer");

	mPluginList.unloadAll(); // Unload all plugins

	if (mDcConfig.mRegMainBot) { // Main bot unreg
		LOG(LEVEL_DEBUG, "Unreg main bot '" << mDcConfig.mHubBot << "'");
		unregBot(mDcConfig.mHubBot);
	}

	// remove all users and bots
	if (mDcUserList.size() > 0) {
		mDcUserList.doForEach(bind1st(mem_fun(&DcServer::delAllUsers), this), true); // Delete all users
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



const string & DcServer::getMainDir() const {
	return mDcConfig.mMainPath;
}



const string & DcServer::getLogDir() const {
	return mDcConfig.mLogPath;
}



const string & DcServer::getPluginDir() const {
	return mPluginList.getPluginDir();
}



const string & DcServer::getTime() {
	stringstream oss;
	oss << mTime.asDate();
	mTimeBuf = oss.str();
	return mTimeBuf;
}



const string & DcServer::getHubInfo() const {
	return mHubName;
}



const string & DcServer::getLocale() const {
	return mDcConfig.mLocale;
}



const string & DcServer::getSystemVersion() const {
	return mSysVersion;
}



int64_t DcServer::getMsec() const {
	Time tm(true);
	return tm.msec();
}



/// Work time (sec)
int DcServer::getUpTime() const {
	Time tm(true);
	tm -= mStartTime;
	return tm.sec();
}



int DcServer::getUsersCount() const {
	return miTotalUserCount;
}



int64_t DcServer::getTotalShare() const {
	return miTotalShare;
}



DcListIteratorBase * DcServer::getDcListIterator() {
	return new DcListIterator(this);
}



bool DcServer::regCallList(const char * id, Plugin * plugin) {
	return mPluginList.regCallList(id, plugin);
}



bool DcServer::unregCallList(const char * id, Plugin * plugin) {
	return mPluginList.unregCallList(id, plugin);
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
		if (2 < vIpPort.size() || vIpPort.size() < 1 || vIpPort[0].empty()) {
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

	if (vAddresses.empty()) {
		LOG(LEVEL_FATAL, "Incorrect address of the " << name);
	}

	bool ret = false;
	for (vector<pair<string, string> >::iterator it = vAddresses.begin(); it != vAddresses.end(); ++it) {
		if (Server::listening(connFactory, ((*it).first).c_str(), ((*it).second).c_str(), udp) != NULL) {
			ret = true;
			LOG(LEVEL_INFO, name << " is running on [" 
				<< ((*it).first) << "]:" << ((*it).second) 
				<< (udp ? " UDP" : " TCP"));
			cout << name << " is running on [" 
				<< ((*it).first) << "]:" << ((*it).second) 
				<< (udp ? " UDP" : " TCP") << endl;
		}
	}
	return ret;
}



/// Listening all servers
int DcServer::listening() {

	// Protocol dependence
	if (mDcConfig.mAdcOn) { // ADC

		ConnFactory * adcConnFactory = new DcConnFactory(&mAdcProtocol, this); // ADC PROTOCOL
		mConnFactories.push_back(adcConnFactory);

		// ADC Server
		if (!listeningServer("ADC Server " INTERNALNAME " " INTERNALVERSION, mDcConfig.mAddresses.c_str(), "411", adcConnFactory)) {
			return -1;
		}

	} else { // NMDC

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



void DcServer::syncTimer(void * self) {
	DcServer * dcServer = static_cast<DcServer *> (self);

	dcServer->mStopSync = false;

	while (!dcServer->mStopSync) {
		syncActions(dcServer);
		sleep(500); // chances are that we are blocking main thread, if time will be very small
	}
}



void DcServer::syncActions(DcServer * dcServer) {
	dcServer->mHelloList.flushCache(); // NMDC
	dcServer->mDcUserList.flushCache(); // ADC & NMDC
	dcServer->mEnterList.flushCache(); // ADC & NMDC
	dcServer->mOpList.flushCache(); // NMDC
	dcServer->mIpList.flushCache(); // NMDC
	dcServer->mChatList.flushCache(); // ADC & NMDC
	dcServer->mActiveList.flushCache(); // NMDC

	dcServer->mHelloList.autoResize(); // NMDC
	dcServer->mDcUserList.autoResize(); // ADC & NMDC
	dcServer->mEnterList.autoResize(); // ADC & NMDC
	dcServer->mOpList.autoResize(); // NMDC
	dcServer->mIpList.autoResize(); // NMDC
	dcServer->mChatList.autoResize(); // ADC & NMDC
	dcServer->mActiveList.autoResize(); // NMDC
}



int DcServer::onTimer(Time & now) {

	// Execute each second
	if (static_cast<int64_t> (now - mChecker) >= mTimerServPeriod) {
		mChecker = now;

		int sysLoading = mSystemLoad;

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

		if (mSystemLoad != sysLoading) {
			LOG(LEVEL_WARN, "System loading: " 
				<< mSystemLoad << " level (was " 
				<< sysLoading << " level)");
		}

		#ifndef USE_DCSERVER_THREADS
			syncActions(this);
		#endif

		// TODO to sync action
		mIpEnterFlood.del(now); // Removing ip addresses, which already long ago entered
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
		LOG_CLASS(dcConn, LEVEL_WARN, "System down, close");
		dcConn->closeNow(CLOSE_REASON_HUB_LOAD);
		return -1;
	}

	// Checking flood-entry (by ip)
	int ret = mIpEnterFlood.check(dcConn->getIp(), mTime);
	if (ret) {
		if (ret == 1) {
			// TODO: convert ADC errorCode to close reason and move close function into sendError
			dcConn->dcProtocol()->sendError(dcConn, mDcLang.mFloodReEnter, ERROR_CODE_BAN_GENERIC);
			dcConn->closeNice(9000, CLOSE_REASON_FLOOD_IP_ENTRY);
		} else {
			// desconnect without msg
			dcConn->closeNow(CLOSE_REASON_FLOOD_IP_ENTRY);
		}
		return -2;
	}

	// TODO fix me (refactoring)
	mIpListConn->add(dcConn); // Adding connection in IP-list
	dcConn->mDcUser->setIp(dcConn->getIp());

	LOG_CLASS(dcConn, LEVEL_TRACE, "[S]Stage onNewConn");

	if (dcConn->mProtocol != NULL) {
		dcConn->mProtocol->onNewConn(dcConn);
	}

	LOG_CLASS(dcConn, LEVEL_TRACE, "[E]Stage onNewConn");
	return 0;
}



/// Returns pointer to line of the connection, in which will be recorded got data
string * DcServer::createCommandPtr(Conn * conn) {
	return conn->getParserCommandPtr();
}



/// Function of the processing enterring data
void DcServer::onNewData(Conn * conn, string * data) {

	LOG_CLASS(conn, LEVEL_TRACE, "IN: " << (*data));

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
	bool ret = false;
	if (timeLimit > 0.) {
		if (fabs(double(mTime - time)) < timeLimit) {
			if (countLimit < ++count) {
				ret = true;
			} else {
				return false;
			}
		}
		time = mTime;
		count = 0;
	}
	return ret;
}



sockoptval_t DcServer::tcpNodelay() const {
	return mDcConfig.mTcpNodelay ? 1 : 0;
}



/// Checking for this nick used
bool DcServer::checkNick(DcConn * dcConn) {

	// check empty nick!
	if (dcConn->mDcUser->getNick().empty()) {
		return false;
	}

	unsigned long nickHash = dcConn->mDcUser->getNickHash();

	if (mDcUserList.contain(nickHash)) {
		// User on a hub
		DcUser * us = static_cast<DcUser *> (mDcUserList.find(nickHash));

		// Checking nick only for profile -1 (unreg) and bots
		// All other profiles is a reg users and they are not checked
		if (!us->mDcConn || dcConn->mDcUser->getParamForce(USER_PARAM_PROFILE)->getInt() == -1) {
			LOG(LEVEL_DEBUG, "Bad nick (used): '" 
				<< dcConn->mDcUser->getNick() << "'["
				<< dcConn->getIp() << "] vs '" << us->getNick() 
				<< "'[" << us->getIp() << "]");
			string msg;
			stringReplace(mDcLang.mUsedNick, string(STR_LEN("nick")), msg, dcConn->mDcUser->getNick());

			// TODO: convert ADC errorCode to close reason and move close function into sendError
			dcConn->dcProtocol()->sendError(dcConn, msg, ERROR_CODE_NICK_INVALID);

			return false;
		}
		LOG(LEVEL_DEBUG, "removed old user");
		removeFromDcUserList(us);
		us->mDcConn->closeNow(CLOSE_REASON_USER_OLD);
	}
	return true;
}



bool DcServer::beforeUserEnter(DcConn * dcConn) {
	LOG_CLASS(dcConn, LEVEL_DEBUG, "Begin login");

	// Check nick
	if (!checkNick(dcConn)) {
		dcConn->closeNice(9000, CLOSE_REASON_NICK_INVALID);
		return false;
	}

	if (dcConn->mSendNickList) {
		if (!mDcConfig.mDelayedLogin) {
			// Before enter, after send list
			doUserEnter(dcConn);
		} else {
			mEnterList.add(dcConn->mDcUser->getNickHash(), dcConn->mDcUser);
		}

		// Can happen so that list not to send at a time
		dcConn->dcProtocol()->sendNickList(dcConn);

		dcConn->mSendNickList = false;
	} else if (!dcConn->mDcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST)) {
		// User has got list already
		doUserEnter(dcConn);
	}
	return true;
}



/// User entry
void DcServer::doUserEnter(DcConn * dcConn) {

	if (!dcConn->isState(STATE_NORMAL)) {
		LOG_CLASS(dcConn, LEVEL_DEBUG, "User Login when not all done (" << dcConn->getState() << ")");
		dcConn->closeNow(CLOSE_REASON_NOT_LOGIN);
		return;
	}

	// TODO remove it!
	// check empty nick!
	if (!checkNick(dcConn)) {
		dcConn->closeNice(9000, CLOSE_REASON_NICK_INVALID);
		return;
	}

	unsigned long nickHash = dcConn->mDcUser->getNickHash();

	// User is already considered came
	if (mEnterList.contain(nickHash)) {
		// We send user contents of cache without clear this cache
		mEnterList.flushForUser(dcConn->mDcUser);
		mEnterList.remove(nickHash);
	}

	// Adding user to the user list
	if (!addToUserList(dcConn->mDcUser)) {
		dcConn->closeNow(CLOSE_REASON_USER_ADD);
		return;
	}

	// Show to all
	showUserToAll(dcConn->mDcUser);

	dcConn->clearLoginTimeOut();
	dcConn->mDcUser->mTimeEnter.get();

	afterUserEnter(dcConn);
}



void DcServer::afterUserEnter(DcConn * dcConn) {
	LOG_CLASS(dcConn, LEVEL_DEBUG, "Entered on the hub");

	#ifndef WITHOUT_PLUGINS
		mCalls.mOnUserEnter.callAll(dcConn->mDcUser);
	#endif
}



/// Adding user in the user list
bool DcServer::addToUserList(DcUser * dcUser) {
	if (!dcUser) {
		LOG(LEVEL_ERROR, "Adding a NULL user to userlist");
		return false;
	} else if (dcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST)) {
		LOG(LEVEL_ERROR, "User is already in the user list");
		return false;
	}

	unsigned long nickHash = dcUser->getNickHash();

	LOG_CLASS(&mDcUserList, LEVEL_TRACE, "Before add: " << dcUser->getNick() << " Size: " << mDcUserList.size());

	if (!mDcUserList.add(nickHash, dcUser)) {
		LOG(LEVEL_DEBUG, "Adding twice user with same user nick " << dcUser->getNick() << " (" << mDcUserList.find(nickHash)->getNick() << ")");
		dcUser->setInUserList(false);
		return false;
	}

	LOG_CLASS(&mDcUserList, LEVEL_TRACE, "After add: " << dcUser->getNick() << " Size: " << mDcUserList.size());

	if (!mDcConfig.mAdcOn) { // NMDC
		if (dcUser->isTrueBoolParam(USER_PARAM_IN_OP_LIST)) {
			mOpList.add(nickHash, dcUser); // TODO: for bots!
		}
	}

	if (dcUser->mDcConn) {

		mChatList.add(nickHash, dcUser);

		dcUser->mDcConn->dcProtocol()->onAddInUserList(dcUser);

		++ miTotalUserCount; // add except bot
		dcUser->mDcConn->mIpRecv = true; // Installing the permit on reception of the messages on ip

		LOG_CLASS(dcUser->mDcConn, LEVEL_DEBUG, "Adding at the end of Nicklist");
	}

	dcUser->setInUserList(true);
	dcUser->setCanSend(true);

	return true;
}



/// Removing user from the user list
bool DcServer::removeFromDcUserList(DcUser * dcUser) {
	unsigned long nickHash = dcUser->getNickHash();

	LOG_CLASS(&mDcUserList, LEVEL_TRACE, "Before leave: " << dcUser->getNick() << " Size: " << mDcUserList.size());
	if (mDcUserList.contain(nickHash)) {

		if (dcUser->mDcConn) {
			#ifndef WITHOUT_PLUGINS
				mCalls.mOnUserExit.callAll(dcUser);
			#endif

			-- miTotalUserCount;
		}

		// We make sure that user with such nick one!
		DcUser * other = static_cast<DcUser *> (mDcUserList.find(nickHash));
		if (!dcUser->mDcConn) { // Removing the bot
			mDcUserList.remove(nickHash);
		} else if (other && other->mDcConn && dcUser->mDcConn && other->mDcConn == dcUser->mDcConn) {
			mDcUserList.remove(nickHash);
			LOG_CLASS(&mDcUserList, LEVEL_TRACE, "After leave: " << dcUser->getNick() << " Size: " << mDcUserList.size());
		} else {
			// Such can happen only for users without connection or with different connection
			LOG_CLASS(dcUser, LEVEL_ERROR, "Not found the correct user for nick: " << dcUser->getNick());
			return false;
		}
	}

	// Removing from lists
	mEnterList.remove(nickHash);
	mChatList.remove(nickHash);

	mOpList.remove(nickHash);

	mIpList.remove(nickHash); // NMDC only
	mActiveList.remove(nickHash); // NMDC only
	mHelloList.remove(nickHash); // NMDC only

	if (dcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST)) {
		dcUser->setInUserList(false);

		if (!dcUser->isTrueBoolParam(USER_PARAM_CAN_HIDE)) {
			string msg;
			// TODO: for bots?
			if (mDcConfig.mAdcOn) { // ADC
				msg.append(STR_LEN("IQUI ")).append(dcUser->getSid()).append(STR_LEN(ADC_SEPARATOR));
				mDcUserList.sendToAllAdc(msg, false, false/*mDcConfig.mDelayedMyinfo*/); // Delay in sending MyINFO (and Quit)
			} else { // NMDC
				mNmdcProtocol.appendQuit(msg, dcUser->getNick());
				mDcUserList.sendToAll(msg, false, false/*mDcConfig.mDelayedMyinfo*/); // Delay in sending MyINFO (and Quit)
			}
		}
	}
	return true;
}



/// Show user to all
bool DcServer::showUserToAll(DcUser * dcUser) {
	bool canSend = dcUser->isCanSend();

	if (mDcConfig.mAdcOn) { // ADC

		if (dcUser->mDcConn && dcUser->isTrueBoolParam(USER_PARAM_CAN_HIDE)) {

			// TODO

		} else {

			// Show INF string to all users
			mDcUserList.sendToAllAdc(dcUser->getInfo(), true, false/*mDcConfig.mDelayedMyinfo*/); // use cache -> so this can be after user is added

			// Show INF string of the current user to all enterring users
			mEnterList.sendToAllAdc(dcUser->getInfo(), true, false/*mDcConfig.mDelayedMyinfo*/);
		}

	} else { // NMDC

		if (dcUser->mDcConn && dcUser->isTrueBoolParam(USER_PARAM_CAN_HIDE)) {
			if (!(dcUser->mDcConn->mFeatures & SUPPORT_FEATURE_NOHELLO)) {
				string hello;
				mNmdcProtocol.appendHello(hello, dcUser->getNick());
				dcUser->mDcConn->send(hello, false, false);
			}
			if ((dcUser->mDcConn->mFeatures & SUPPORT_FEATURE_NOHELLO) || (dcUser->mDcConn->mFeatures & SUPPORT_FEATUER_NOGETINFO)) {
				dcUser->mDcConn->send(dcUser->getInfo(), true, false);
			}
			if (dcUser->isTrueBoolParam(USER_PARAM_IN_OP_LIST)) {
				string opList;
				mNmdcProtocol.appendOpList(opList, dcUser->getNick());
				dcUser->mDcConn->send(opList, false, false);
			}
		} else {

			// Sending the greeting for all users, not supporting feature NoHello (except enterring users)
			{
				string hello;
				mNmdcProtocol.appendHello(hello, dcUser->getNick());
				mHelloList.sendToAll(hello, false, false/*mDcConfig.mDelayedMyinfo*/);
			}

			// Show MyINFO string to all users
			mDcUserList.sendToAll(dcUser->getInfo(), true, false/*mDcConfig.mDelayedMyinfo*/); // use cache -> so this can be after user is added

			// Show MyINFO string of the current user to all enterring users
			mEnterList.sendToAll(dcUser->getInfo(), true, false/*mDcConfig.mDelayedMyinfo*/);

			// Op entry
			if (dcUser->isTrueBoolParam(USER_PARAM_IN_OP_LIST)) {
				string opList;
				mNmdcProtocol.appendOpList(opList, dcUser->getNick());
				mDcUserList.sendToAll(opList, false, false/*mDcConfig.mDelayedMyinfo*/);
				mEnterList.sendToAll(opList, false, false/*mDcConfig.mDelayedMyinfo*/);
			}
		}

	}

	// Prevention of the double sending
	if (!mDcConfig.mDelayedLogin) {
		dcUser->setCanSend(false);
		mDcUserList.flushCache();
		mEnterList.flushCache();
		dcUser->setCanSend(canSend);
	}

	if (!mDcConfig.mAdcOn) { // NMDC
		if (mDcConfig.mSendUserIp) {
			string ipList;
			// TODO: for bots?
			mNmdcProtocol.appendUserIp(ipList, dcUser->getNick(), dcUser->getIp());
			if (ipList.size()) {
				mIpList.sendToAll(ipList, false, false);
			}

			// ===================================================
			if (dcUser->isTrueBoolParam(USER_PARAM_IN_IP_LIST)) {
				dcUser->send(mDcUserList.getList(USER_LIST_IP), true, false);
			} else if (ipList.size() && dcUser->mDcConn && (dcUser->mDcConn->mFeatures & SUPPORT_FEATUER_USERIP2)) { // UserIP2
				dcUser->send(ipList, false, false);
			}
			// ===================================================
		}
	}

	dcUser->send("", 0, false, true);
	return true;
}



/// Get user by nick (or NULL)
DcUser * DcServer::getDcUser(const char * nick) {
	if (nick == NULL) {
		return NULL;
	}

	string nickStr(nick);
	if (nickStr.size()) {
		UserBase * userBase = mDcUserList.getUserBaseByNick(nickStr);
		if (userBase) {
			return static_cast<DcUser *> (userBase);
		}
		DcConn * dcConn = NULL;
		for (tCLIt it = mClientList.begin(); it != mClientList.end(); ++it) {
			dcConn = static_cast<DcConn *> (*it);
			if (dcConn && dcConn->mDcUser && dcConn->mDcUser->mType == CLIENT_TYPE_DC &&
				dcConn->mDcUser->getNick() == nickStr
			) {
				return dcConn->mDcUser;
			}
		}
	}
	return NULL;
}



// TODO return user with conn only?
DcUserBase * DcServer::getDcUserBase(const char * nick) {
	DcUser * dcUser = getDcUser(nick);
	return (dcUser != NULL && dcUser->mDcConn != NULL) ? static_cast<DcUserBase *> (dcUser) : NULL;
}



const vector<DcUserBase *> & DcServer::getDcUserBaseByIp(const char * ip) {
	// TODO add bots in list (getDcUserBaseByIp)
	mIpUserList.clear();
	for (DcIpList::iterator it = mIpListConn->begin(ip); it != mIpListConn->end(); ++it) {
		DcConn * dcConn = static_cast<DcConn *> (*it);
		if (dcConn->mDcUser->mType == CLIENT_TYPE_DC && dcConn->getIp() == ip) {
			mIpUserList.push_back(dcConn->mDcUser);
		}
	}
	return mIpUserList;
}



/// Send data to user
bool DcServer::sendToUser(DcUserBase * dcUserBase, const string & data, const char * nick, const char * from) {
	DcUser * dcUser = static_cast<DcUser *> (dcUserBase);
	if (!dcUser || !dcUser->mDcConn) { // Check exist and not bot
		return false;
	}

	// Sent chat or pm through user protocol!
	DcConn * dcConn = dcUser->mDcConn;
	if (from && nick) {
		dcConn->mDcUser->sendToPm(data, nick, from, true); // PM
	} else if (nick) {
		dcConn->mDcUser->sendToChat(data, nick, true); // Chat from user
	} else {
		dcConn->mDcUser->sendToChat(data, true); // Simple Chat
	}
	return true;
}



/// Send raw data to user
bool DcServer::sendToUserRaw(DcUserBase * dcUserBase, const string & data) {
	DcUser * dcUser = static_cast<DcUser *> (dcUserBase);
	if (!dcUser || !dcUser->mDcConn) { // Check exist and not bot
		return false;
	}

	// TODO: parse cmd
	// new object:
	// enum cmdDirect
	// enum cmdType
	// vector cmdParts by protocol type

	// cmd(data); // simple parse cmd

	dcUser->mDcConn->send(data, true, false);
	return true;
}



/// Send data to nick
bool DcServer::sendToNick(const char * to, const string & data, const char * nick, const char * from) {
	return sendToUser(getDcUser(to), data, nick, from);
}



/// Send raw data to nick
bool DcServer::sendToNickRaw(const char * to, const string & data) {
	return sendToUserRaw(getDcUser(to), data);
}



void DcServer::sendToAllRaw(const string & data, bool flush) {
	// TODO: optimization now
	// TODO: problem with sent cache for both protocol
	// append msg to cmd, but not string
	//DcCmd dcCmd(mDcConfig.mAdcOn ? DC_PROTOCOL_TYPE_ADC : DC_PROTOCOL_TYPE_NMDC);
	//dcCmd.parse(data);
	//mDcUserList.sendToAll(&dcCmd, flush);

	// Protocol dependence
	if (mDcConfig.mAdcOn) { // ADC
		mDcUserList.sendToAllAdc(data, true, flush);
	} else { // NMDC
		mDcUserList.sendToAll(data, true, flush);
	}
}



/// Send to all chat or pm through user protocol
bool DcServer::sendToAll(const string & data, const char * nick, const char * from) {
	// TODO: optimization now
	//DcCmd dcCmd(mDcConfig.mAdcOn ? DC_PROTOCOL_TYPE_ADC : DC_PROTOCOL_TYPE_NMDC);
	//if (from && nick) {
	//	dcCmd.buildPm(data, string(nick), string(from));
	//} else if (nick) {
	//	dcCmd.buildChat(data, string(nick), true);
	//} else {
	//	dcCmd.buildChat(data, string(""), true);
	//}
	//mDcUserList.sendToAll(&dcCmd, true);

	// Sent chat or pm through user protocol!
	if (from && nick) {
		mDcUserList.sendToAllPm(data, nick, from); // PM
	} else if (nick) {
		mDcUserList.sendToAllChat(data, nick); // Chat from user
	} else {
		if (mDcConfig.mAdcOn) { // ADC
			mChatList.sendToAllAdc(data, true, true); // mChatList
		} else { // NMDC
			mChatList.sendToAll(data, true, true); // mChatList
		}
	}
	return true;
}



/// Send raw data to all
bool DcServer::sendToAllRaw(const string & data) {
	sendToAllRaw(data, false);
	return true;
}



/// Send data to profiles
bool DcServer::sendToProfiles(unsigned long profile, const string & data, const char * nick, const char * from) {

	// Sent chat or pm through user protocol!
	if (from && nick) {
		mDcUserList.sendToAllPm(data, nick, from, &profile); // PM
	} else if (nick) {
		mDcUserList.sendToAllChat(data, nick, &profile); // Chat from user
	} else {
		// TODO
		mDcUserList.sendToProfiles(profile, data, true); // Simple Chat
	}
	return true;
}



/// Send raw data to profiles
bool DcServer::sendToProfilesRaw(unsigned long profile, const string & data) {

	// TODO: parse cmd
	mDcUserList.sendToProfiles(profile, data, true);
	return true;
}



/// Send data to ip
bool DcServer::sendToIp(const string & ip, const string & data, unsigned long profile, const char * nick, const char * from) {
	if (!Conn::checkIp(ip)) {
		return false;
	}

	// Sent chat or pm through user protocol!
	if (from && nick) {
		mIpListConn->sendToIpPm(ip, data, nick, from, profile, true); // PM
	} else if (nick) {
		mIpListConn->sendToIpChat(ip, data, nick, profile, true); // Chat from user
	} else {
		// TODO
		mIpListConn->sendToIp(ip, data, profile, true); // Simple Chat
	}
	return true;
}



/// Send raw data to ip
bool DcServer::sendToIpRaw(const string & ip, const string & data, unsigned long profile) {
	if (!Conn::checkIp(ip)) {
		return false;
	}

	// TODO: parse cmd
	mIpListConn->sendToIp(ip, data, profile, true);
	return true;
}



/// Send data to all except nick list
bool DcServer::sendToAllExceptNicks(const vector<string> & nickList, const string & data, const char * nick, const char * from) {

	DcUser * dcUser = NULL;
	vector<DcUser *> ul;
	for (List_t::const_iterator it = nickList.begin(); it != nickList.end(); ++it) {
		dcUser = static_cast<DcUser *> (mDcUserList.getUserBaseByNick(*it));
		if (dcUser && dcUser->isCanSend()) {
			dcUser->setCanSend(false);
			ul.push_back(dcUser);
		}
	}

	// Sent chat or pm through user protocol!
	sendToAll(data, nick, from);

	for (vector<DcUser *>::iterator ul_it = ul.begin(); ul_it != ul.end(); ++ul_it) {
		(*ul_it)->setCanSend(true);
	}
	return true;
}



/// Send raw data to all except nick list
bool DcServer::sendToAllExceptNicksRaw(const vector<string> & nickList, const string & data) {

	DcUser * dcUser = NULL;
	vector<DcUser *> ul;
	for (List_t::const_iterator it = nickList.begin(); it != nickList.end(); ++it) {
		dcUser = static_cast<DcUser *> (mDcUserList.getUserBaseByNick(*it));
		if (dcUser && dcUser->isCanSend()) {
			dcUser->setCanSend(false);
			ul.push_back(dcUser);
		}
	}

	// TODO: parse cmd
	sendToAllRaw(data);

	for (vector<DcUser *>::iterator ul_it = ul.begin(); ul_it != ul.end(); ++ul_it) {
		(*ul_it)->setCanSend(true);
	}
	return true;
}



/// Send data to all except ip list
bool DcServer::sendToAllExceptIps(const vector<string> & ipList, const string & data, const char * nick, const char * from) {

	DcConn * dcConn = NULL;
	vector<DcConn*> ul;
	for (List_t::const_iterator it = ipList.begin(); it != ipList.end(); ++it) {
		if (DcConn::checkIp(*it)) {
			for (DcIpList::iterator mit = mIpListConn->begin((*it).c_str()); mit != mIpListConn->end(); ++mit) {
				dcConn = static_cast<DcConn *> (*mit);
				if (dcConn->mDcUser && dcConn->mDcUser->isCanSend() && dcConn->getIp() == (*it)) {
					dcConn->mDcUser->setCanSend(false);
					ul.push_back(dcConn);
				}
			}
		}
	}

	// Sent chat or pm through user protocol!
	sendToAll(data, nick, from);

	for (vector<DcConn*>::iterator ul_it = ul.begin(); ul_it != ul.end(); ++ul_it) {
		(*ul_it)->mDcUser->setCanSend(true);
	}
	return true;
}



/// Send raw data to all except ip list
bool DcServer::sendToAllExceptIpsRaw(const vector<string> & ipList, const string & data) {

	DcConn * dcConn = NULL;
	vector<DcConn*> ul;
	for (List_t::const_iterator it = ipList.begin(); it != ipList.end(); ++it) {
		if (DcConn::checkIp(*it)) {
			for (DcIpList::iterator mit = mIpListConn->begin((*it).c_str()); mit != mIpListConn->end(); ++mit) {
				dcConn = static_cast<DcConn *> (*mit);
				if (dcConn->mDcUser && dcConn->mDcUser->isCanSend() && dcConn->getIp() == (*it)) {
					dcConn->mDcUser->setCanSend(false);
					ul.push_back(dcConn);
				}
			}
		}
	}

	// TODO: parse cmd
	sendToAllRaw(data);

	for (vector<DcConn*>::iterator ul_it = ul.begin(); ul_it != ul.end(); ++ul_it) {
		(*ul_it)->mDcUser->setCanSend(true);
	}
	return true;
}



void DcServer::forceMove(DcUserBase * dcUserBase, const char * address, const char * reason /* = NULL */) {
	DcUser * dcUser = static_cast<DcUser *> (dcUserBase);
	if (!address || !dcUser || !dcUser->mDcConn) {
		return;
	}
	dcUser->mDcConn->dcProtocol()->forceMove(dcUser->mDcConn, address, reason);
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
	configItem->convertTo(mBuf);
	return mBuf.c_str();
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

		// Protocol dependence
		if (mDcConfig.mAdcOn) { // ADC

			// TODO for ADC

		} else { // NMDC
			string msg;
			sendToAllRaw(mNmdcProtocol.appendHubName(msg, mDcConfig.mHubName, mDcConfig.mTopic));
		}
	}
	mDcConfig.save();
	return true;
}



const char * DcServer::getLang(const string & name) {
	ConfigItem * configItem = mDcLang[name];
	if (configItem == NULL) {
		return NULL;
	}
	configItem->convertTo(mBuf);
	return mBuf.c_str();
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



int DcServer::regBot(const string & nick, const string & info, const string & ip, bool key) {
	DcUser * dcUser = new DcUser(CLIENT_TYPE_DC, NULL);
	dcUser->mDcServer = this;

	dcUser->setSid(string(mAdcProtocol.genNewSid()));
	dcUser->setNick(nick);

	dcUser->getParamForce(USER_PARAM_PROFILE)->setInt(30);
	dcUser->getParamForce(USER_PARAM_IN_OP_LIST)->setBool(key);

	if (DcConn::checkIp(ip)) {
		dcUser->setIp(ip);
	}

	// Protocol dependence
	if (mDcConfig.mAdcOn) { // ADC
		const char * ct = "1";
		if (key) {
			ct = "5";
		}
		string inf(STR_LEN("IINF "));
		inf.reserve(79);
		inf.append(dcUser->getSid()).append(STR_LEN(" CT")).append(ct, 1).append(STR_LEN(" NI")).append(nick);
		inf.append(STR_LEN(" IDAONWQSVCXNJKW7L4HLB5O24TYW55555KAEK7WRY SS0 HN0 HR0 HO1 VEBot\\sV:1.0 SL0 DERusHub\\sbot"));
		dcUser->setInfo(inf);
	} else { // NMDC
		if (nick.empty() || nick.size() > 0x40 || nick.find_first_of(" |$") != string::npos) {
			delete dcUser;
			return -1;
		}
		string myInfo(STR_LEN("$MyINFO $ALL "));
		if (!dcUser->setInfo(myInfo.append(nick).append(STR_LEN(" ")).append(info))) {
			myInfo.assign(STR_LEN("$MyINFO $ALL "));
			if (!dcUser->setInfo(myInfo.append(nick).append(STR_LEN(" $ $$$0$")))) {
				delete dcUser;
				return -2;
			}
		}
	}

	LOG(LEVEL_DEBUG, "Reg bot: " << nick);

	if (!addToUserList(dcUser)) {
		delete dcUser;
		return -3;
	}
	showUserToAll(dcUser);
	return 0;
}



int DcServer::unregBot(const string & nick) {

	LOG(LEVEL_DEBUG, "Unreg bot: " << nick);

	// TODO: remove it! Replace to botlist
	DcUser * dcUser = static_cast<DcUser *> (mDcUserList.getUserBaseByNick(nick));
	if (!dcUser) { // Not found
		return -1;
	}

	if (dcUser->mDcConn) { // It's not bot!
		LOG(LEVEL_DEBUG, "Attempt delete user");
		return -2;
	}
	removeFromDcUserList(dcUser);
	delete dcUser;
	return 0;
}



/** Get normal share size */
void DcServer::getNormalShare(int64_t share, string & normalShare) {
	ostringstream os;
	float s(static_cast<float>(share));
	int i(0);
	for (; ((s >= 1024) && (i < 7)); ++i) {
		s /= 1024;
	}
	os << ::std::floor(s * 1000 + 0.5) / 1000 << " " << DcServer::currentDcServer->mDcLang.mUnits[6 - i];
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
				version.append(STR_LEN("Microsoft Windows NT "));
			}
			if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0) {
				version.append(STR_LEN("Microsoft Windows 2000 "));
			}

			if (osVersionInfoEx) {

				// Check workstation type
				if (osvi.wProductType == VER_NT_WORKSTATION) {

					if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1) {
						version.append(STR_LEN("Microsoft Windows XP "));
					} else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0) {
						version.append(STR_LEN("Microsoft Windows Vista "));
					} else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1) {
						version.append(STR_LEN("Microsoft Windows 7 "));
					} else {
						version.append(STR_LEN("Microsoft Windows (unknown version) "));
					}


					if (osvi.wSuiteMask & VER_SUITE_PERSONAL) {
						version.append(STR_LEN("Home Edition "));
					} else {
						version.append(STR_LEN("Professional "));
					}

				} else if (osvi.wProductType == VER_NT_SERVER) { // Check server type

					if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2) {
						version.append(STR_LEN("Microsoft Windows 2003 "));
					} else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0) {
						version.append(STR_LEN("Microsoft Windows Server 2008 "));
					} else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1) {
						version.append(STR_LEN("Microsoft Windows Server 2008 R2 "));
					} else {
						version.append(STR_LEN("Microsoft Windows (unknown version) "));
					}

					if (osvi.wSuiteMask & VER_SUITE_DATACENTER) {
						version.append(STR_LEN("DataCenter Server "));
					} else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE) {
						if (osvi.dwMajorVersion == 4) {
							version.append(STR_LEN("Advanced Server "));
						} else {
							version.append(STR_LEN("Enterprise Server "));
						}
					} else if (osvi.wSuiteMask == VER_SUITE_BLADE) {
						version.append(STR_LEN("Web Server "));
					} else {
						version.append(STR_LEN("Server "));
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

				lRet = RegQueryValueExA(hKey, "ProductType", NULL, NULL, (LPBYTE) szProductType, &dwBufLen);

				if ((lRet != ERROR_SUCCESS) || (dwBufLen > 80)) {
					return "unknown";
				}

				RegCloseKey(hKey);

				if (lstrcmpiA("WINNT", szProductType) == 0) {
					version.append(STR_LEN("Professional "));
				}
				if (lstrcmpiA("LANMANNT", szProductType) == 0) {
					version.append(STR_LEN("Server "));
				}
				if (lstrcmpiA("SERVERNT", szProductType) == 0) {
					version.append(STR_LEN("Advanced Server "));
				}
			}

			// Version, service pack, number of the build
			if (osvi.dwMajorVersion <= 4) {
				sprintf(buf, "version %u.%u %s (Build %u)", 
					osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.szCSDVersion, osvi.dwBuildNumber & 0xFFFF);
				version.append(buf);
			} else {
				sprintf(buf, "%s (Build %u)", osvi.szCSDVersion, osvi.dwBuildNumber & 0xFFFF);
				version.append(buf);
			}

			break;

		case VER_PLATFORM_WIN32_WINDOWS : // Windows 95

			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0) {
				version.append(STR_LEN("Microsoft Windows 95 "));
				if (osvi.szCSDVersion[1] == 'C' || osvi.szCSDVersion[1] == 'B') {
					version.append(STR_LEN("OSR2 "));
				}
			}

			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10) {
				version.append(STR_LEN("Microsoft Windows 98 "));
				if (osvi.szCSDVersion[1] == 'A') {
					version.append(STR_LEN("SE "));
				}
			}

			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90) {
				version.append(STR_LEN("Microsoft Windows Millennium Edition "));
			}
			break;

		case VER_PLATFORM_WIN32s : // Windows
			version.append(STR_LEN("Microsoft Win32s "));
			break;

		default :
			break;
	}
	return version;

#else

#if defined(SOLARIS) || defined(_SOLARIS)
	return "Solaris";
#else
	struct utsname osname;
	if (uname(&osname) == 0) {
		string version(osname.sysname);
		version.append(STR_LEN(" "));
		version.append(osname.release);
		version.append(STR_LEN(" ("));
		version.append(osname.machine);
		version.append(STR_LEN(")"));
		return version;
	}
	return "unknown";
#endif

#endif

}



bool DcServer::setCapabilities() {

#if (!defined _WIN32) && HAVE_LIBCAP

	if (getuid()) {
		LOG(LEVEL_WARN, "Cannot set capabilities. Hub started from common user, not root.");
		return false;
	}

	struct passwd * user = getpwnam(mDcConfig.mUserName.c_str());
	struct group * grp = getgrnam(mDcConfig.mGroupName.c_str());

	if(user && grp) {
		// Keep capabilities across UID change
		if(prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0) != 0) {
			LOG(LEVEL_WARN, "prctl(PR_SET_KEEPCAPS) failed");
			return false;
		}

		// Change supplementary groups
		if(initgroups(user->pw_name, user->pw_gid) != 0) {
			LOG(LEVEL_WARN, "initgroups() for user " << user->pw_name << " failed");
			return false;
		}

		// Change GID
		if(setgid(grp->gr_gid) != 0) {
			LOG(LEVEL_WARN, "Cannot set GID to " << grp->gr_gid);
			return false;
		}

		// Change UID
		if(setuid(user->pw_uid) != 0) {
			LOG(LEVEL_WARN, "Cannot set UID to " << user->pw_uid);
			return false;
		}
	} else {
		LOG(LEVEL_WARN, "Bad user name. Cannot get pam structs. Check user and group name.");
		return false;
	}

	// Check capability to bind privileged ports
	cap_t caps = cap_get_proc();
	if(!caps) {
		LOG(LEVEL_WARN, "cap_get_proc() failed to get capabilities");
		return false;
	}

	cap_flag_value_t nbs_flag;
	if(cap_get_flag(caps, CAP_NET_BIND_SERVICE, CAP_PERMITTED, &nbs_flag) != 0) {
		LOG(LEVEL_WARN, "cap_get_flag() failed to get CAP_NET_BIND_SERVICE state");
		cap_free(caps);
		return false;
	}

	// Drop all capabilities except privileged ports binding
	caps = cap_from_text(nbs_flag == CAP_SET ? "cap_net_bind_service=ep" : "=");
	if(!caps) {
		LOG(LEVEL_WARN, "cap_from_text() failed to parse capabilities string");
		return false;
	}

	if(cap_set_proc(caps) != 0) {
		LOG(LEVEL_WARN, "cap_set_proc() failed to set capabilities");
		cap_free(caps);
		return false;
	}
	cap_free(caps);

#endif

	return true;

}


} // namespace dcserver

/**
 * $Id$
 * $HeadURL$
 */
