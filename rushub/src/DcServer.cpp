/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz

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
string DcServer::msSysVersion;



DcServer::DcServer(const string & configFile, const string &):
	Server(NMDC_SEPARATOR),
	mDcConfig(&mDcConfigLoader, mServer, configFile.c_str()),
	mDCLang(&mDcConfigLoader, &mDcConfig),
	mSystemLoad(SYSTEM_LOAD_OK),
	mDCUserList("UserList", true, true, true),
	mDCBotList("BotList", true),
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
	mListenFactory(NULL),
	mWebListenFactory(NULL),
	mIPEnterFlood(mDcConfig.mFloodCountReconnIp, mDcConfig.mFloodTimeReconnIp),
	mCalls(&mPluginList)
{
	SetClassName("DcServer");

	/** Current server */
	currentDcServer = this;

	/** Define OS */
	if (!GetSysVersion()) {
		msSysVersion = "unknown";
	}

	/** ConnFactory */
	mConnFactory = new DcConnFactory(&mDCProtocol, this);

	/** DcIpList */
	mIPListConn = new DcIpList();

	mDCProtocol.SetServer(this);
	mPluginList.SetServer(this);

	mPluginList.LoadAll(); /** Load plugins */

	/** Set params of lists */
	string nmdc;

	nmdc = "$NickList ";
	mDCUserList.SetNickListStart(nmdc);

	nmdc = "$OpList ";
	mOpList.SetNickListStart(nmdc);

	nmdc = "$$";
	mDCUserList.SetNickListSeparator(nmdc);
	mOpList.SetNickListSeparator(nmdc);

	nmdc = "$ActiveList ";
	mActiveList.SetNickListStart(nmdc);

	if (mDcConfig.mRegMainBot) { /** Main bot registration */

		if (Log(3)) {
			LogStream() << "Reg main bot '" << mDcConfig.mHubBot << "'" << endl;
		}

		if (
			regBot(
				mDcConfig.mHubBot,
				mDcConfig.mMainBotMyinfo, 
				mDcConfig.mMainBotIp,
				mDcConfig.mMainBotKey
			) == -2
		) {
			regBot(
				mDcConfig.mHubBot,
				string("$ $$$0$"), 
				mDcConfig.mMainBotIp,
				mDcConfig.mMainBotKey
			);
		}
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
	UserList::iterator it, it_e = mDCUserList.end();	
	for (it = mDCUserList.begin(); it != it_e;) {
		Us = (DcUser *) (*it);
		++it;
		if (Us->mDcConn) {
			delConnection(Us->mDcConn);
		} else {
			if (Us->getInUserList()) {
				this->RemoveFromDCUserList(Us);
			}
			delete Us;
		}
	}

	// Remove users
	close();

	if (mIPListConn != NULL) {
		delete mIPListConn;
		mIPListConn = NULL;
	}

	if (mListenFactory != NULL) {
		delete mListenFactory;
		mListenFactory = NULL;
	}

	if (mWebListenFactory != NULL) {
		delete mWebListenFactory;
		mWebListenFactory = NULL;
	}

	if (mConnFactory != NULL) {
		delete mConnFactory;
		mConnFactory = NULL;
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
		if (conn->mConnFactory != NULL) {
			ConnFactory * Factory = conn->mConnFactory;
			Factory->deleteConn(conn);
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
	StringSplit(sAddresses, ' ', vAddresses);
	for (vector<string>::iterator it = vAddresses.begin(); it != vAddresses.end(); ++it) {
		vector<string> vIpPort;
		StringSplit(*it, ':', vIpPort);
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



bool DcServer::ListeningServer(const char * name, const string & addresses, unsigned port, ListenFactory * listenFactory, bool udp /*= false*/) {
	vector<pair<string, int> > vAddresses;
	getAddresses(addresses, vAddresses, port);

	if (vAddresses.size() == 0) {
		if (ErrLog(0)) {
			LogStream() << "Incorrect address of the " << name << endl;
		}
	}

	bool ret = false;
	for (vector<pair<string, int> >::iterator it = vAddresses.begin(); it != vAddresses.end(); ++it) {
		if (Server::Listening(listenFactory, (*it).first, (*it).second, udp) == 0) {
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



/** Listening ports */
int DcServer::Listening() {

	if (!mListenFactory) {
		mListenFactory = new ListenFactory(this);
	}
	if (!mWebListenFactory) {
		mWebListenFactory = new WebListenFactory(this);
	}

	// DC Server
	if (!ListeningServer("Server " INTERNALNAME " " INTERNALVERSION, mDcConfig.mAddresses, 411, mListenFactory)) {
		return -1;
	}

	// Web Server
	if (mDcConfig.mWebServer) {
		ListeningServer("Web-Server", mDcConfig.mWebAddresses, 80, mWebListenFactory);
	}

	// UDP Server
	if (mDcConfig.mUdpServer) {
		ListeningServer("UDP-Server", mDcConfig.mUdpAddresses, 1209, mListenFactory, true);
	}
	return 0;
}



int DcServer::onTimer(Time & now) {

	/** Execute each second */
	if (abs(int(now - mChecker)) >= mTimerServPeriod) {

		mChecker = now;

		mHelloList.FlushCache();
		mDCUserList.FlushCache();
		mDCBotList.FlushCache();
		mEnterList.FlushCache();
		mOpList.FlushCache();
		mIpList.FlushCache();
		mChatList.FlushCache();
		mActiveList.FlushCache();


		int SysLoading = mSystemLoad;

		if (0 < mMeanFrequency.mNumFill) {

			double iFrequency = mMeanFrequency.GetMean(mTime);

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

		mIPEnterFlood.Del(now); // Removing ip addresses, which already long ago entered

		mDCUserList.AutoResize();
		mDCBotList.AutoResize();
		mHelloList.AutoResize();
		mEnterList.AutoResize();
		mActiveList.AutoResize();
		mChatList.AutoResize();
		mOpList.AutoResize();
		mIpList.AutoResize();
	}

	#ifndef WITHOUT_PLUGINS
		if (mCalls.mOnTimer.CallAll()) {
			return 1;
		}
	#endif

	return 0;
}



/** Function action after joining the client */
int DcServer::onNewConn(Conn *conn) {
	DcConn * dcConn = (DcConn *) conn;

	if (mSystemLoad == SYSTEM_LOAD_SYSTEM_DOWN) {
		if (dcConn->Log(1)) {
			dcConn->LogStream() << "System down, close" << endl;
		}
		dcConn->closeNow(CLOSE_REASON_HUB_LOAD);
		return 1;
	}

	/** Checking flood-entry (by ip) */
	if (mIPEnterFlood.Check(dcConn->getNetIp(), mTime)) {
		dcConn->closeNow(CLOSE_REASON_FLOOD_IP_ENTRY);
		return 2;
	}

	if (dcConn->Log(5)) {
		dcConn->LogStream() << "[S]Stage onNewConn" << endl;
	}
	string sMsg;
	dcConn->send(
		DcProtocol::Append_DC_HubName(
			DcProtocol::Append_DC_Lock(sMsg),
			mDcConfig.mHubName,
			mDcConfig.mTopic
		),
		false,
		false
	);

	#ifndef WITHOUT_PLUGINS
		if (mCalls.mOnUserConnected.CallAll(dcConn)) {
			dcConn->flush();
		} else
	#endif
	{
		static __int64 iShareVal = -1;
		static int iUsersVal = -1;
		static long iTimeVal = -1;
		static string sTimeCache, sShareCache, sCache;
		bool useCache = true;
		Time Uptime(mTime);
		Uptime -= mStartTime;
		long min = Uptime.Sec() / 60;
		if (iTimeVal != min) {
			iTimeVal = min;
			useCache = false;
			stringstream oss;
			int w, d, h, m;
			Uptime.AsTimeVals(w, d, h, m);
			if (w) {
				oss << w << " " << mDCLang.mTimes[0] << " ";
			}
			if (d) {
				oss << d << " " << mDCLang.mTimes[1] << " ";
			}
			if (h) {
				oss << h << " " << mDCLang.mTimes[2] << " ";
			}
			oss << m << " " << mDCLang.mTimes[3];
			sTimeCache = oss.str();
		}
		if (iShareVal != miTotalShare) {
			iShareVal = miTotalShare;
			useCache = false;
			sShareCache = DcProtocol::GetNormalShare(iShareVal);
		}
		if (iUsersVal != miTotalUserCount) {
			iUsersVal = miTotalUserCount;
			useCache = false;
		}
		if (!useCache) {
			StringReplace(mDCLang.mFirstMsg, string("HUB"), sCache, string(INTERNALNAME " " INTERNALVERSION));
			StringReplace(sCache, string("uptime"), sCache, sTimeCache);
			StringReplace(sCache, string("users"), sCache, iUsersVal);
			StringReplace(sCache, string("share"), sCache, sShareCache);
		}
		sendToUser(dcConn, sCache.c_str(), mDcConfig.mHubBot.c_str());
	}
	dcConn->SetTimeOut(HUB_TIME_OUT_LOGIN, mDcConfig.mTimeout[HUB_TIME_OUT_LOGIN], mTime); /** Timeout for enter */
	dcConn->SetTimeOut(HUB_TIME_OUT_KEY, mDcConfig.mTimeout[HUB_TIME_OUT_KEY], mTime);
	if (dcConn->Log(5)) {
		dcConn->LogStream() << "[E]Stage onNewConn" << endl;
	}
	dcConn->flush();
	return 0;
}



/** Returns pointer to line of the connection, in which will be recorded got data */
string * DcServer::getPtrForStr(Conn * conn) {
	return conn->getPtrForStr();
}



/** Function of the processing enterring data */
void DcServer::onNewData(Conn * conn, string * data) {

	// Parse data
	conn->mParser->Parse();

	// Check TCP conn
	if (conn->getConnType() == CONN_TYPE_CLIENTTCP) {
		if (conn->Log(4)) {
			conn->LogStream() << "IN: " << (*data) << NMDC_SEPARATOR << endl;
		}
		conn->mProtocol->DoCmd(conn->mParser, conn);
	} else {
		OnNewUdpData(conn, data);
	}
}



void DcServer::OnNewUdpData(Conn * conn, string * data) {

	if (conn->Log(4)) {
		conn->LogStream() << "IN [UDP]: " << (*data) << NMDC_SEPARATOR << endl;
	}

	DcParser * dcparser = (DcParser *)conn->mParser;
	if (dcparser->miType == NMDC_TYPE_SR) {
		dcparser->miType = NMDC_TYPE_SR_UDP; // Set type for parse
		if (!dcparser->SplitChunks()) {
			dcparser->miType = NMDC_TYPE_SR; // Set simple SR type

			DcUser * User = (DcUser *)mDCUserList.GetUserBaseByNick(dcparser->chunkString(CHUNK_SR_FROM));
			if (User && User->mDcConn && conn->ipUdp() == User->getIp()) {
				conn->mProtocol->DoCmd(dcparser, User->mDcConn);
			}
		}
	} else {
		if (conn->Log(5)) {
			conn->LogStream() << "Unknown UDP data" << endl;
		}
	}
}



/** Function checks min interval */
bool DcServer::MinDelay(Time &time, double sec) {
	if (::fabs(double(mTime - time)) >= sec) {
		time = mTime;
		return true;
	}
	return false;
}



/** Antiflood function */
bool DcServer::antiFlood(unsigned & iCount, Time & time, const unsigned & iCountLimit, const double & iTimeLimit) {
	if (!iTimeLimit) {
		return false;
	}
	bool bRet = false;
	if (::fabs(double(mTime - time)) < iTimeLimit) {
		if (iCountLimit < ++iCount) {
			bRet = true;
		} else {
			return false;
		}
	}
	time = mTime;
	iCount = 0;
	return bRet;
}



void DcServer::AddToOps(DcUser * User) {
	if (!User->mbInOpList) {
		User->mbInOpList = true;
		if (User->getInUserList()) {
			string sMsg;
			mOpList.AddWithNick(User->msNick, User);
			if (User->mbHide) {
				User->send(DcProtocol::Append_DC_OpList(sMsg, User->msNick), false, true);
			} else {
				mDCUserList.sendToAll(DcProtocol::Append_DC_OpList(sMsg, User->msNick), true/*mDcConfig.mDelayedMyinfo*/, false);
				mEnterList.sendToAll(sMsg, true/*mDcConfig.mDelayedMyinfo*/, false);
			}
		}
	}
}



void DcServer::DelFromOps(DcUser * User) {
	if (User->mbInOpList) {
		User->mbInOpList = false;
		if (User->getInUserList()) {
			string sMsg1, sMsg2, sMsg3;
			mOpList.RemoveByNick(User->msNick);
			if (User->mbHide) {
				if (User->mDcConn == NULL) {
					return;
				}
				string s = User->getMyINFO();
				User->send(DcProtocol::Append_DC_Quit(sMsg1, User->msNick), false, false);
				if (User->mDcConn->mFeatures & SUPPORT_FEATUER_NOHELLO) {
					User->send(s, true, false);
				} else if (User->mDcConn->mFeatures & SUPPORT_FEATUER_NOGETINFO) {
					User->send(DcProtocol::Append_DC_Hello(sMsg2, User->msNick), false, false);
					User->send(s, true, false);
				} else {
					User->send(DcProtocol::Append_DC_Hello(sMsg2, User->msNick), false, false);
				}

				if ((User->mDcConn->mFeatures & SUPPORT_FEATUER_USERIP2) || User->mbInIpList) {
					User->send(DcProtocol::Append_DC_UserIP(sMsg3, User->msNick, User->getIp()));
				} else {
					s.clear();
					User->send(s);
				}
			} else {
				mDCUserList.sendToAll(DcProtocol::Append_DC_Quit(sMsg1, User->msNick), true/*mDcConfig.mDelayedMyinfo*/, false);
				mEnterList.sendToAll(sMsg1, true/*mDcConfig.mDelayedMyinfo*/, false);
				mHelloList.sendToAll(DcProtocol::Append_DC_Hello(sMsg2, User->msNick), true/*mDcConfig.mDelayedMyinfo*/, false);
				mDCUserList.sendToAll(User->getMyINFO(), true/*mDcConfig.mDelayedMyinfo*/);
				mEnterList.sendToAll(User->getMyINFO(), true/*mDcConfig.mDelayedMyinfo*/);
				mIpList.sendToAll(DcProtocol::Append_DC_UserIP(sMsg3, User->msNick, User->getIp()), true, false);
			}
		}
	}
}



void DcServer::AddToIpList(DcUser * User) {
	if (!User->mbInIpList) {
		User->mbInIpList = true;
		if (User->getInUserList()) {
			mIpList.AddWithNick(User->msNick, User);
			User->send(mDCUserList.GetIpList(), true);
		}
	}
}



void DcServer::DelFromIpList(DcUser * User) {
	if (User->mbInIpList) {
		User->mbInIpList = false;
		if (User->getInUserList()) {
			mIpList.RemoveByNick(User->msNick);
		}
	}
}



void DcServer::AddToHide(DcUser * User) {
	if (!User->mbHide) {
		User->mbHide = true;
		if (User->isCanSend()) {
			string sMsg;
			User->setCanSend(false);
			mDCUserList.sendToAll(DcProtocol::Append_DC_Quit(sMsg, User->msNick), false/*mDcConfig.mDelayedMyinfo*/, false);
			mEnterList.sendToAll(sMsg, false/*mDcConfig.mDelayedMyinfo*/, false); // false cache
			User->setCanSend(true);
			mOpList.Remake();
			mDCUserList.Remake();
		}
	}
}



void DcServer::DelFromHide(DcUser * User) {
	if (User->mbHide) {
		User->mbHide = false;
		if (User->isCanSend()) {
			string sMsg1, sMsg2, sMsg3;
			if (User->mbInOpList) {
				User->setCanSend(false);
				mHelloList.sendToAll(DcProtocol::Append_DC_Hello(sMsg1, User->msNick), false/*mDcConfig.mDelayedMyinfo*/, false);
				sMsg2 = string(User->getMyINFO()).append(NMDC_SEPARATOR);
				mDCUserList.sendToAll(DcProtocol::Append_DC_OpList(sMsg2, User->msNick), false/*mDcConfig.mDelayedMyinfo*/, false);
				mEnterList.sendToAll(sMsg2, false/*mDcConfig.mDelayedMyinfo*/, false);
				mIpList.sendToAll(DcProtocol::Append_DC_UserIP(sMsg3, User->msNick, User->getIp()), false, false);
				User->setCanSend(true);
			} else {
				User->setCanSend(false);
				mHelloList.sendToAll(DcProtocol::Append_DC_Hello(sMsg1, User->msNick), false/*mDcConfig.mDelayedMyinfo*/, false);
				mDCUserList.sendToAll(User->getMyINFO(), false/*mDcConfig.mDelayedMyinfo*/);
				mEnterList.sendToAll(User->getMyINFO(), false/*mDcConfig.mDelayedMyinfo*/);
				mIpList.sendToAll(DcProtocol::Append_DC_UserIP(sMsg3, User->msNick, User->getIp()), false, false);
				User->setCanSend(true);
			}
			mOpList.Remake();
			mDCUserList.Remake();
		}
	}
}



bool DcServer::ValidateUser(DcConn * dcConn, const string & sNick) {

	/** Checking for bad symbols in nick */
	static string forbidedChars("$| ");
	if (sNick.npos != sNick.find_first_of(forbidedChars)) {
		if (dcConn->Log(2)) {
			dcConn->LogStream() << "Bad nick chars: '" << sNick << "'" << endl;
		}
		sendToUser(dcConn, mDCLang.mBadChars.c_str(), mDcConfig.mHubBot.c_str());
		return false;
	}

	return true;
}



bool DcServer::CheckNickLength(DcConn * dcConn, const unsigned iLen) {
	if (dcConn->miProfile == -1 && (iLen > mDcConfig.mMaxNickLen || iLen < mDcConfig.mMinNickLen)) {
		string sMsg;

		if (dcConn->Log(2)) {
			dcConn->LogStream() << "Bad nick len: " 
				<< iLen << " (" << dcConn->mDcUser->msNick 
				<< ") [" << mDcConfig.mMinNickLen << ", " 
				<< mDcConfig.mMaxNickLen << "]" << endl;
		}

		StringReplace(mDCLang.mBadNickLen, string("min"), sMsg, (int) mDcConfig.mMinNickLen);
		StringReplace(sMsg, string("max"), sMsg, (int) mDcConfig.mMaxNickLen);

		sendToUser(dcConn, sMsg.c_str(), mDcConfig.mHubBot.c_str());

		return false;
	}
	return true;
}



/** Checking for this nick used */
bool DcServer::CheckNick(DcConn *dcConn) {
	UserKey key = mDCUserList.Nick2Key(dcConn->mDcUser->msNick);
	if (mDCUserList.ContainsKey(key)) {
		string sMsg;
		DcUser * us = (DcUser *) mDCUserList.Find(key);

		if (!us->mDcConn || (us->getProfile() == -1 && us->getIp() != dcConn->ip())) {
			if (dcConn->Log(2)) {
				dcConn->LogStream() << "Bad nick (used): '" 
					<< dcConn->mDcUser->msNick << "'["
					<< dcConn->ip() << "] vs '" << us->msNick 
					<< "'[" << us->getIp() << "]" << endl;
			}

			StringReplace(mDCLang.mUsedNick, string("nick"), sMsg, dcConn->mDcUser->msNick);

			sendToUser(dcConn, sMsg.c_str(), mDcConfig.mHubBot.c_str());

			dcConn->send(DcProtocol::Append_DC_ValidateDenide(sMsg.erase(), dcConn->mDcUser->msNick));
			return false;
		}
		if (us->mDcConn->Log(3)) {
			us->mDcConn->LogStream() << "removed old user" << endl;
		}
		RemoveFromDCUserList(us);
		us->mDcConn->closeNow(CLOSE_REASON_USER_OLD);
	}
	return true;
}



bool DcServer::BeforeUserEnter(DcConn * dcConn) {
	unsigned iWantedMask;
	if (mDcConfig.mDelayedLogin && dcConn->mbSendNickList) {
		iWantedMask = LOGIN_STATUS_LOGIN_DONE - LOGIN_STATUS_NICKLST;
	} else {
		iWantedMask = LOGIN_STATUS_LOGIN_DONE;
	}

	if (iWantedMask == dcConn->GetLSFlag(iWantedMask)) {
		if (dcConn->Log(3)) {
			dcConn->LogStream() << "Begin login" << endl;
		}
		if (!CheckNick(dcConn)) {
			dcConn->closeNice(9000, CLOSE_REASON_NICK_INVALID);
			return false;
		}
		if (dcConn->mbSendNickList) {
			if (!mDcConfig.mDelayedLogin) {
				DoUserEnter(dcConn);
			} else {
				mEnterList.Add(dcConn->mDcUser);
			}

			/** Can happen so that list not to send at a time */
			mDCProtocol.SendNickList(dcConn);

			dcConn->mbSendNickList = false;
			return true;
		}
		if (!dcConn->mDcUser->getInUserList()) {
			DoUserEnter(dcConn);
		}
		return true;
	} else { /** Invalid sequence of the sent commands */
		if (dcConn->Log(2)) {
			dcConn->LogStream() << "Invalid sequence of the sent commands (" 
				<< dcConn->GetLSFlag(iWantedMask) << "), wanted: " 
				<< iWantedMask << endl;
		}
		dcConn->closeNow(CLOSE_REASON_CMD_SEQUENCE);
		return false;
	}
}



/** User entry */
void DcServer::DoUserEnter(DcConn * dcConn) {
	/** Check entry stages */
	if (LOGIN_STATUS_LOGIN_DONE != dcConn->GetLSFlag(LOGIN_STATUS_LOGIN_DONE)) {
		if (dcConn->Log(2)) {
			dcConn->LogStream() << "User Login when not all done (" 
				<< dcConn->GetLSFlag(LOGIN_STATUS_LOGIN_DONE) << ")" <<endl;
		}
		dcConn->closeNow(CLOSE_REASON_LOGIN_NOT_DONE);
		return;
	}

	if (!CheckNick(dcConn)) {
		dcConn->closeNice(9000, CLOSE_REASON_NICK_INVALID);
		return;
	}

	UserKey key = mDCUserList.Nick2Key(dcConn->mDcUser->msNick);

	/** User is already considered came */
	if (mEnterList.ContainsKey(key)) {
		/** We send user contents of cache without clear this cache */
		mEnterList.FlushForUser(dcConn->mDcUser);
		mEnterList.RemoveByKey(key);
	}

	/** Adding user to the user list */
	if (!AddToUserList((DcUser *)dcConn->mDcUser)) {
		dcConn->closeNow(CLOSE_REASON_USER_ADD);
		return;
	}

	/** Show to all */
	ShowUserToAll(dcConn->mDcUser);

	AfterUserEnter(dcConn);

	dcConn->ClearTimeOut(HUB_TIME_OUT_LOGIN);
	((DcUser *)dcConn->mDcUser)->mTimeEnter.Get();
}



/** Adding user in the user list */
bool DcServer::AddToUserList(DcUser * User) {
	if (!User) {
		if (ErrLog(1)) {
			LogStream() << "Adding a NULL user to userlist" << endl;
		}
		return false;
	}
	if (User->getInUserList()) {
		if (ErrLog(2)) {
			LogStream() << "User is already in the user list" << endl;
		}
		return false;
	}

	UserKey key = mDCUserList.Nick2Key(User->msNick);

	if (mDCUserList.Log(4)) {
		mDCUserList.LogStream() << "Before add: " << User->msNick << " Size: " << mDCUserList.Size() << endl;
	}

	if (!mDCUserList.AddWithKey(key, User)) {
		if (Log(1)) {
			LogStream() << "Adding twice user with same nick " << User->msNick << " (" << mDCUserList.Find(key)->Nick() << ")" << endl;
		}
		User->setInUserList(false);
		return false;
	}

	if (mDCUserList.Log(4)) {
		mDCUserList.LogStream() << "After add: " << User->msNick << " Size: " << mDCUserList.Size() << endl;
	}

	User->setInUserList(true);
	User->setCanSend(true);
	if (!User->IsPassive()) {
		mActiveList.AddWithKey(key, User);
	}
	if (User->mbInOpList) {
		mOpList.AddWithKey(key, User);
	}
	if (User->mbInIpList) {
		mIpList.AddWithKey(key, User);
	}

	if (User->mDcConn) {
		User->mDcConn->mbIpRecv = true; /** Installing the permit on reception of the messages on ip */
		mChatList.AddWithKey(key, User);

		if (!(User->mDcConn->mFeatures & SUPPORT_FEATUER_NOHELLO)) {
			mHelloList.AddWithKey(key, User);
		}
		if (User->mDcConn->Log(3)) {
			User->mDcConn->LogStream() << "Adding at the end of Nicklist" << endl;
		}
		if (User->mDcConn->Log(3)) {
			User->mDcConn->LogStream() << "Becomes in list" << endl;
		}
	}
	return true;
}



/** Removing user from the user list */
bool DcServer::RemoveFromDCUserList(DcUser *User) {
	UserKey key = mDCUserList.Nick2Key(User->msNick);
	if (mDCUserList.Log(4)) {
		mDCUserList.LogStream() << "Before leave: " << User->msNick << " Size: " << mDCUserList.Size() << endl;
	}
	if (mDCUserList.ContainsKey(key)) {
		#ifndef WITHOUT_PLUGINS
			if (User->mDcConn) {
				mCalls.mOnUserExit.CallAll(User->mDcConn);
			}
		#endif

		/** We make sure that user with such nick one! */
		DcUser *other = (DcUser *)mDCUserList.GetUserBaseByNick(User->msNick);
		if (!User->mDcConn) { /** Removing the bot */
			mDCUserList.RemoveByKey(key);
		} else if (other && other->mDcConn && User->mDcConn && other->mDcConn == User->mDcConn) {
			mDCUserList.RemoveByKey(key);
			if (mDCUserList.Log(4)) {
				mDCUserList.LogStream() << "After leave: " << User->msNick << " Size: " << mDCUserList.Size() << endl;
			}
		} else {
			/** Such can happen only for users without connection or with different connection */
			if (User->ErrLog(1)) {
				User->LogStream() << "Not found the correct user for nick: " << User->msNick << endl;
			}
			return false;
		}
	}

	/** Removing from lists */
	mOpList.RemoveByKey(key);
	mIpList.RemoveByKey(key);
	mHelloList.RemoveByKey(key);
	mEnterList.RemoveByKey(key);
	mChatList.RemoveByKey(key);
	mActiveList.RemoveByKey(key);
	mDCBotList.RemoveByKey(key);

	if (User->getInUserList()) {
		User->setInUserList(false);

		if (!User->getHide()) {
			string sMsg;
			DcProtocol::Append_DC_Quit(sMsg, User->msNick);

			/** Delay in sending MyINFO (and Quit) */
			mDCUserList.sendToAll(sMsg, true/*mDcConfig.mDelayedMyinfo*/, false);
		}
	}
	return true;
}



/** Show user to all */
bool DcServer::ShowUserToAll(DcUser *User) {
	string sMsg1, sMsg2;
	if (User->mbHide && User->mDcConn) {
		if (User->mDcConn->mFeatures & SUPPORT_FEATUER_NOHELLO) {
			User->mDcConn->send(string(User->getMyINFO()), true, false);
		} else if (User->mDcConn->mFeatures & SUPPORT_FEATUER_NOGETINFO) {
			User->mDcConn->send(DcProtocol::Append_DC_Hello(sMsg1, User->msNick), false, false);
			User->mDcConn->send(string(User->getMyINFO()), true, false);
		} else {
			User->mDcConn->send(DcProtocol::Append_DC_Hello(sMsg1, User->msNick), false, false);
		}

		if (User->mbInOpList) {
			User->mDcConn->send(DcProtocol::Append_DC_OpList(sMsg2, User->msNick), false, false);
		}
	} else {

		/** Sending the greeting for all users, not supporting feature NoHello (except enterring users) */
		mHelloList.sendToAll(DcProtocol::Append_DC_Hello(sMsg1, User->msNick), true/*mDcConfig.mDelayedMyinfo*/, false);

		/** Show MyINFO string to all users */
		mDCUserList.sendToAll(User->getMyINFO(), true/*mDcConfig.mDelayedMyinfo*/); // use cache -> so this can be after user is added

		/** Show MyINFO string of the current user to all enterring users */
		mEnterList.sendToAll(User->getMyINFO(), true/*mDcConfig.mDelayedMyinfo*/);

		/** Op entry */
		if (User->mbInOpList) {
			mDCUserList.sendToAll(DcProtocol::Append_DC_OpList(sMsg2, User->msNick), true/*mDcConfig.mDelayedMyinfo*/, false);
			mEnterList.sendToAll(sMsg2, true/*mDcConfig.mDelayedMyinfo*/, false);
		}
	}

	bool canSend = User->isCanSend();

	/** Prevention of the double sending MyINFO string */
	if (!mDcConfig.mDelayedLogin) {
			User->setCanSend(false);
			mDCUserList.FlushCache();
			mEnterList.FlushCache();
			User->setCanSend(canSend);
	}

	if (mDcConfig.mSendUserIp) {
		string sStr;
		User->setCanSend(false);
		DcProtocol::Append_DC_UserIP(sStr, User->msNick, User->getIp());
		if (sStr.length()) {
			mIpList.sendToAll(sStr, true);
		}
		User->setCanSend(canSend);

		if (User->mbInIpList) {
			User->send(mDCUserList.GetIpList(), true, false);
		} else if (User->mDcConn && (User->mDcConn->mFeatures & SUPPORT_FEATUER_USERIP2)) { // UserIP2
			User->send(sStr, false, false);
		}
	}

	static string s;
	User->send(s, false, true);
	return true;
}



void DcServer::AfterUserEnter(DcConn *dcConn) {
	if (dcConn->Log(3)) {
		dcConn->LogStream() << "Entered the hub." << endl;
	}
	#ifndef WITHOUT_PLUGINS
		mCalls.mOnUserEnter.CallAll(dcConn);
	#endif
}



/** Get user by nick (or NULL) */
DcUser * DcServer::GetDCUser(const char *sNick) {
	string sN(sNick);
	if (sN.size()) {
		UserBase * User = mDCUserList.GetUserBaseByNick(sN);
		if (User) {
			return (DcUser *)User;
		}
		DcConn * dcConn;
		for (tCLIt it = mConnList.begin(); it != mConnList.end(); ++it) {
			dcConn = (DcConn *)(*it);
			if (dcConn && dcConn->mType == CLIENT_TYPE_NMDC && dcConn->mDcUser && dcConn->mDcUser->msNick == sN) {
				return (DcUser *)dcConn->mDcUser;
			}
		}
	}
	return NULL;
}



DcUserBase * DcServer::getDcUserBase(const char *sNick) {
	DcUser * User = GetDCUser(sNick);
	return User != NULL ? (DcUserBase *)User : NULL;
}



const vector<DcConnBase *> & DcServer::getDcConnBase(const char * sIP) {
	DcIpList::iterator it;
	mvIPConn.clear();
	for (it = mIPListConn->begin(DcConn::ip2Num(sIP)); it != mIPListConn->end(); ++it) {
		DcConn * dcConn = (DcConn *)(*it);
		if (dcConn->mType == CLIENT_TYPE_NMDC) {
			mvIPConn.push_back(dcConn);
		}
	}
	return mvIPConn;
}



/** Send data to user */
bool DcServer::sendToUser(DcConnBase * dcConnBase, const char * sData, const char * sNick, const char * sFrom) {
	if (!dcConnBase || !sData) {
		return false;
	}

	// PM
	if (sFrom && sNick) {
		string sTo("<unknown>"), sStr;
		if (dcConnBase->mDcUserBase) {
			sTo = dcConnBase->mDcUserBase->getNick();
		}
		dcConnBase->send(DcProtocol::Append_DC_PM(sStr, sTo, sFrom, sNick, sData));
		return true;
	}

	// Chat
	if (sNick) {
		string sStr;
		dcConnBase->send(DcProtocol::Append_DC_Chat(sStr, sNick, sData));
		return true;
	}

	// Simple Msg
	string sMsg(sData);
	if (dcConnBase->mType == CLIENT_TYPE_NMDC && sMsg.substr(sMsg.size() - 1, 1) != NMDC_SEPARATOR) {
		sMsg.append(NMDC_SEPARATOR);
	}
	dcConnBase->send(sMsg);
	return true;
}



/** Send data to nick */
bool DcServer::sendToNick(const char *sTo, const char *sData, const char *sNick, const char *sFrom) {
	if (!sTo || !sData) {
		return false;
	}
	DcUser *User = GetDCUser(sTo);
	if (!User || !User->mDcConn) {
		return false;
	}

	// PM
	if (sFrom && sNick) {
		string sStr;
		User->mDcConn->send(DcProtocol::Append_DC_PM(sStr, sTo, sFrom, sNick, sData));
		return true;
	}

	// Chat
	if (sNick) {
		string sStr;
		User->mDcConn->send(DcProtocol::Append_DC_Chat(sStr, sNick, sData));
		return true;
	}

	// Simple Msg
	string sMsg(sData);
	if (sMsg.substr(sMsg.size() - 1, 1) != NMDC_SEPARATOR) {
		sMsg.append(NMDC_SEPARATOR);
	}
	User->mDcConn->send(sMsg);
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
		DcProtocol::Append_DC_PMToAll(sStart, sEnd, sFrom, sNick, sData);
		mDCUserList.SendToWithNick(sStart, sEnd);
		return true;
	}

	// Chat
	if (sNick) {
		string sStr;
		mDCUserList.sendToAll(DcProtocol::Append_DC_Chat(sStr, sNick, sData), false, false);
		return true;
	}

	// Simple Msg
	string sMsg(sData);
	if (sMsg.substr(sMsg.size() - 1, 1) != NMDC_SEPARATOR) {
		sMsg.append(NMDC_SEPARATOR);
	}
	mDCUserList.sendToAll(sMsg, false, false);
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
		DcProtocol::Append_DC_PMToAll(sStart, sEnd, sFrom, sNick, sData);
		mDCUserList.SendToWithNick(sStart, sEnd, iProfile);
		return true;
	}

	// Chat
	if (sNick) {
		string sStr;
		mDCUserList.sendToProfiles(iProfile, DcProtocol::Append_DC_Chat(sStr, sNick, sData), false);
		return true;
	}

	// Simple Msg
	string sMsg(sData);
	if (sMsg.substr(sMsg.size() - 1, 1) != NMDC_SEPARATOR) {
		sMsg.append(NMDC_SEPARATOR);
	}
	mDCUserList.sendToProfiles(iProfile, sMsg, false);
	return true;
}



bool DcServer::sendToIp(const char *sIP, const char *sData, unsigned long iProfile, const char *sNick, const char *sFrom) {
	if (!sIP || !sData || !Conn::checkIp(sIP)) {
		return false;
	}

	// PM
	if (sFrom && sNick) {
		string sStart, sEnd;
		DcProtocol::Append_DC_PMToAll(sStart, sEnd, sFrom, sNick, sData);
		mIPListConn->SendToIPWithNick(sIP, sStart, sEnd, iProfile);
		return true;
	}

	// Chat
	if (sNick) {
		string sStr;
		mIPListConn->sendToIp(sIP, DcProtocol::Append_DC_Chat(sStr, sNick, sData), iProfile); // newPolitic
		return true;
	}

	// Simple Msg
	string sMsg(sData);
	if (sMsg.substr(sMsg.size() - 1, 1) != NMDC_SEPARATOR) {
		sMsg.append(NMDC_SEPARATOR);
	}
	mIPListConn->sendToIp(sIP, sMsg, iProfile); // newPolitic
	return true;
}



/** Send data to all except nick list */
bool DcServer::sendToAllExceptNicks(const vector<string> & NickList, const char *sData, const char *sNick, const char *sFrom) {
	if (!sData) {
		return false;
	}

	DcUser * User;
	vector<DcUser *> ul;
	for (List_t::const_iterator it = NickList.begin(); it != NickList.end(); ++it) {
		User = (DcUser*)mDCUserList.GetUserBaseByNick(*it);
		if (User && User->isCanSend()) {
			User->setCanSend(false);
			ul.push_back(User);
		}
	}

	if (sFrom && sNick) { // PM
		string sStart, sEnd;
		DcProtocol::Append_DC_PMToAll(sStart, sEnd, sFrom, sNick, sData);
		mDCUserList.SendToWithNick(sStart, sEnd);
	} else if (sNick) { // Chat
		string sStr;
		mDCUserList.sendToAll(DcProtocol::Append_DC_Chat(sStr, sNick, sData), false, false);
	} else { // Simple Msg
		string sMsg(sData);
		if (sMsg.substr(sMsg.size() - 1, 1) != NMDC_SEPARATOR) {
			sMsg.append(NMDC_SEPARATOR);
		}
		mDCUserList.sendToAll(sMsg, false, false);
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
		for (DcIpList::iterator mit = mIPListConn->begin(DcConn::ip2Num((*it).c_str())); mit != mIPListConn->end(); ++mit) {
			dcConn = (DcConn*)(*mit);
			if (dcConn->mDcUser && dcConn->mDcUser->isCanSend()) {
				dcConn->mDcUser->setCanSend(false);
				ul.push_back(dcConn);
			}
		}
	}

	if (!bBadIP) {
		if (sFrom && sNick) { // PM
			string sStart, sEnd;
			DcProtocol::Append_DC_PMToAll(sStart, sEnd, sFrom, sNick, sData);
			mDCUserList.SendToWithNick(sStart, sEnd);
		} else if (sNick) { // Chat
			string sStr;
			mDCUserList.sendToAll(DcProtocol::Append_DC_Chat(sStr, sNick, sData), false, false);
		} else { // Simple Msg
			string sMsg(sData);
			if (sMsg.substr(sMsg.size() - 1, 1) != NMDC_SEPARATOR) {
				sMsg.append(NMDC_SEPARATOR);
			}
			mDCUserList.sendToAll(sMsg, false, false);
		}
	}

	for (vector<DcConn*>::iterator ul_it = ul.begin(); ul_it != ul.end(); ++ul_it) {
		(*ul_it)->mDcUser->setCanSend(true);
	}

	return (bBadIP == true) ? false : true;
}



int DcServer::checkCmd(const string & sData) {
	mDCParser.ReInit();
	mDCParser.mCommand = sData;
	mDCParser.Parse();
	if (mDCParser.SplitChunks()) {
		return -1;
	}
	if (mDCParser.miType > 0 && mDCParser.miType < 3) {
		return 3;
	}
	return mDCParser.miType;
}



void DcServer::forceMove(DcConnBase * dcConnBase, const char * sAddress, const char * sReason /* = NULL */) {
	if (!dcConnBase || !sAddress) {
		return;
	}
	DcConn * dcConn = (DcConn *) dcConnBase;

	string sMsg, sForce, sNick("<unknown>");
	if (dcConn->mDcUser) {
		sNick = dcConn->mDcUser->msNick;
	}

	StringReplace(mDCLang.mForceMove, string("address"), sForce, string(sAddress));
	StringReplace(sForce, string("reason"), sForce, string(sReason != NULL ? sReason : ""));
	DcProtocol::Append_DC_PM(sMsg, sNick, mDcConfig.mHubBot, mDcConfig.mHubBot, sForce);
	DcProtocol::Append_DC_Chat(sMsg, mDcConfig.mHubBot, sForce);
	dcConn->send(DcProtocol::Append_DC_ForceMove(sMsg, sAddress));
	dcConn->closeNice(9000, CLOSE_REASON_CMD_FORCE_MOVE);
}



const vector<string> & DcServer::getConfig() {
	if (mvConfigNames.empty()) {
		for (ConfigListBase::tHLMIt it = mDcConfig.mList.begin(); it != mDcConfig.mList.end(); ++it) {
			mvConfigNames.push_back((*it)->mName);
		}
	}
	return mvConfigNames;
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
	Config * config = mDCLang[sName];
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
			if (regBot(mDcConfig.mHubBot, mDcConfig.mMainBotMyinfo, 
				mDcConfig.mMainBotIp, mDcConfig.mMainBotKey) == -2) {
					regBot(mDcConfig.mHubBot, string("$ $$$0$"), 
						mDcConfig.mMainBotIp, mDcConfig.mMainBotKey);
			}
		} else {
			unregBot(mDcConfig.mHubBot);
		}
	}

	config->convertFrom(sValue);

	if (sName == "sHubBot") {
		if (mDcConfig.mRegMainBot) { /** Registration bot */
			if (regBot(mDcConfig.mHubBot, mDcConfig.mMainBotMyinfo, 
				mDcConfig.mMainBotIp, mDcConfig.mMainBotKey) == -2) {
					regBot(mDcConfig.mHubBot, string("$ $$$0$"), 
						mDcConfig.mMainBotIp, mDcConfig.mMainBotKey);
			}
		}
	} else if (sName == "sHubName" || sName == "sTopic") {
		string sMsg;
		sendToAll(DcProtocol::Append_DC_HubName(sMsg, mDcConfig.mHubName, mDcConfig.mTopic).c_str()); // use cache ?
	}

	mDcConfig.save();
	return true;
}



bool DcServer::setLang(const string & sName, const string & sValue) {
	Config * config = mDCLang[sName];
	if (!config) {
		return false;
	}
	config->convertFrom(sValue);
	mDCLang.save();
	return true;
}



int DcServer::regBot(const string & nick, const string & myInfo, const string & ip, bool key) {
	if (!nick.length() || nick.length() > 64 || nick.find_first_of(" |$") != nick.npos) {
		return -1;
	}

	string info(myInfo);
	DcUser * dcUser = new DcUser(nick);
	dcUser->mDcServer = this;
	dcUser->mbInOpList = key;
	dcUser->SetIp(ip, true);
	if (!info.size()) {
		info = "$ $$$0$";
	}
	if (!dcUser->setMyINFO(string("$MyINFO $ALL ") + nick + " " + info, nick)) {
		delete dcUser;
		return -2;
	}

	if (Log(3)) {
		LogStream() << "Reg bot: " << nick << endl;
	}

	if (!AddToUserList(dcUser)) {
		delete dcUser;
		return -3;
	}
	mDCBotList.AddWithNick(nick, dcUser);
	ShowUserToAll(dcUser);
	return 0;
}



int DcServer::unregBot(const string & sNick) {

	if (Log(3)) {
		LogStream() << "Unreg bot: " << sNick << endl;
	}

	DcUser * User = (DcUser*)mDCUserList.GetUserBaseByNick(sNick);
	if (!User) {
		return -1;
	}
	if (User->mDcConn) {
		if (Log(3)) {
			LogStream() << "Attempt delete user" << endl;
		}
		return -2;
	}
	RemoveFromDCUserList(User);
	delete User;
	return 0;
}



#ifdef _WIN32

bool DcServer::GetSysVersion() {
	OSVERSIONINFOEX osvi;
	int bOsVersionInfoEx;
	if (!msSysVersion.empty()) {
		return true;
	}

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO *) &osvi);
	if (!bOsVersionInfoEx) {
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if (!GetVersionEx((OSVERSIONINFO *) &osvi)) {
			return false;
		}
	}

	char buf[256] = { '\0' };
	switch (osvi.dwPlatformId) {

		case VER_PLATFORM_WIN32_NT : // Windows NT

			if (osvi.dwMajorVersion <= 4) {
				msSysVersion.append("Microsoft Windows NT ");
			}
			if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0) {
				msSysVersion.append("Microsoft Windows 2000 ");
			}

			if (bOsVersionInfoEx) {

				// Check workstation type
				if (osvi.wProductType == VER_NT_WORKSTATION) {

					if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1) {
						msSysVersion.append("Microsoft Windows XP ");
					} else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0) {
						msSysVersion.append("Microsoft Windows Vista ");
					} else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1) {
						msSysVersion.append("Microsoft Windows 7 ");
					} else {
						msSysVersion.append("Microsoft Windows (unknown version) ");
					}


					if (osvi.wSuiteMask & VER_SUITE_PERSONAL) {
						msSysVersion.append("Home Edition ");
					} else {
						msSysVersion.append("Professional ");
					}

				} else if (osvi.wProductType == VER_NT_SERVER) { // Check server type

					if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2) {
						msSysVersion.append("Microsoft Windows 2003 ");
					} else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0) {
						msSysVersion.append("Microsoft Windows Server 2008 ");
					} else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1) {
						msSysVersion.append("Microsoft Windows Server 2008 R2 ");
					} else {
						msSysVersion.append("Microsoft Windows (unknown version) ");
					}

					if (osvi.wSuiteMask & VER_SUITE_DATACENTER) {
						msSysVersion.append("DataCenter Server ");
					} else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE) {
						if (osvi.dwMajorVersion == 4) {
							msSysVersion.append("Advanced Server ");
						} else {
							msSysVersion.append("Enterprise Server ");
						}
					} else if (osvi.wSuiteMask == VER_SUITE_BLADE) {
						msSysVersion.append("Web Server ");
					} else {
						msSysVersion.append("Server ");
					}

				}

			} else {
				HKEY hKey;
				char szProductType[80] = { '\0' };
				DWORD dwBufLen = 80;
				LONG lRet = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\ProductOptions", 0, KEY_QUERY_VALUE, &hKey);

				if (lRet != ERROR_SUCCESS) {
					return false;
				}

				lRet = RegQueryValueExA( hKey, "ProductType", NULL, NULL, (LPBYTE) szProductType, &dwBufLen);

				if ((lRet != ERROR_SUCCESS) || (dwBufLen > 80)) {
					return false;
				}

				RegCloseKey(hKey);

				if (lstrcmpiA("WINNT", szProductType) == 0) {
					msSysVersion.append("Professional ");
				}
				if (lstrcmpiA("LANMANNT", szProductType) == 0) {
					msSysVersion.append("Server ");
				}
				if (lstrcmpiA( "SERVERNT", szProductType) == 0) {
					msSysVersion.append("Advanced Server ");
				}
			}

			// Version, service pack, number of the build
			if (osvi.dwMajorVersion <= 4) {
				sprintf(buf, "version %d.%d %s (Build %d)", 
					osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.szCSDVersion, osvi.dwBuildNumber & 0xFFFF);
				msSysVersion.append(buf);
			} else {
				sprintf(buf, "%s (Build %d)", osvi.szCSDVersion, osvi.dwBuildNumber & 0xFFFF);
				msSysVersion.append(buf);
			}

			break;

		case VER_PLATFORM_WIN32_WINDOWS : // Windows 95

			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0) {
				msSysVersion.append("Microsoft Windows 95 ");
				if (osvi.szCSDVersion[1] == 'C' || osvi.szCSDVersion[1] == 'B') {
					msSysVersion.append("OSR2 ");
				}
			} 

			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10) {
				msSysVersion.append("Microsoft Windows 98 ");
				if (osvi.szCSDVersion[1] == 'A') {
					msSysVersion.append("SE ");
				}
			} 

			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90) {
				msSysVersion.append("Microsoft Windows Millennium Edition ");
			} 
			break;

		case VER_PLATFORM_WIN32s : // Windows
			msSysVersion.append("Microsoft Win32s ");
			break;

		default :
			break;
	}
	return true; 
}

#else

bool DcServer::GetSysVersion() {
	utsname osname;
	if (uname(&osname) == 0) {
		msSysVersion = string(osname.sysname) + " " + 
			string(osname.release) + " (" + 
			string(osname.machine) + ")";
	}
	return true;
}

#endif


}; // namespace dcserver
