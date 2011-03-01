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



DcServer::DcServer(const string &, const string &):
	Server(NMDC_SEPARATOR),
	mDcConfig(&mDcConfigLoader, mServer, NULL), // TODO: cfg
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
	msHubName(INTERNALNAME " " INTERNALVERSION " " __DATE__ " " __TIME__),
	mListenFactory(NULL),
	mWebListenFactory(NULL),
	mIPEnterFlood(mDcConfig.miFloodCountReconnIp, mDcConfig.miFloodTimeReconnIp),
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

	if (mDcConfig.mbRegMainBot) { /** Main bot registration */

		if (Log(3)) {
			LogStream() << "Reg main bot '" << mDcConfig.msHubBot << "'" << endl;
		}

		if (
			regBot(
				mDcConfig.msHubBot,
				mDcConfig.msMainBotMyINFO, 
				mDcConfig.msMainBotIP,
				mDcConfig.mbMainBotKey
			) == -2
		) {
			regBot(
				mDcConfig.msHubBot,
				string("$ $$$0$"), 
				mDcConfig.msMainBotIP,
				mDcConfig.mbMainBotKey
			);
		}
	}
}



DcServer::~DcServer() {
	if (Log(1)) {
		LogStream() << "Destruct DcServer" << endl;
	}

	if (mDcConfig.mbRegMainBot) { /** Main bot unreg */
		if (Log(3)) {
			LogStream() << "Unreg main bot '" << mDcConfig.msHubBot << "'" << endl;
		}
		unregBot(mDcConfig.msHubBot);
	}

	DcUser * Us = NULL;
	UserList::iterator it, it_e = mDCUserList.end();	
	for (it = mDCUserList.begin(); it != it_e;) {
		Us = (DcUser *) (*it);
		++it;
		if (Us->mDCConn) {
			DelConnection(Us->mDCConn);
		} else {
			cout << "11" << endl;
			if (Us->mbInUserList) {
				cout << "22" << endl;
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
	mbRun = false;
	for (tCLIt it = mConnList.begin(); it != mConnList.end(); ++it) {
		DelConn(*it);
	}
	for (tLLIt it = mListenList.begin(); it != mListenList.end(); ++it) {
		DelConn(*it);
	}
}



void DcServer::DelConn(Conn * conn) {
	if (conn != NULL) {
		mConnChooser.DelConn(conn);
		if (conn->mConnFactory != NULL) {
			ConnFactory * Factory = conn->mConnFactory;
			Factory->DelConn(conn);
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



/** Listening ports */
int DcServer::Listening(int iPort) {
	if (!mListenFactory) {
		mListenFactory = new ListenFactory(this);
	}

	vector<pair<string, int> > vAddresses;
	getAddresses(msAddresses, vAddresses, 411);

	if (vAddresses.size() == 0) {
		if (ErrLog(0)) {
			LogStream() << "Incorrect address of the hub" << endl;
		}
		return -1;
	}
	for (vector<pair<string, int> >::iterator it = vAddresses.begin(); it != vAddresses.end(); ++it) {
		int iRes = Server::Listening(mListenFactory, (*it).first, (iPort != 0) ? iPort : (*it).second);
		if (iRes != 0) {
			return iRes;
		}
		if (Log(0)) {
			LogStream() << "Server " INTERNALNAME " " INTERNALVERSION " is running on " 
				<< ((*it).first) << ":" << ((iPort != 0) ? iPort : (*it).second) 
				<< " TCP" << endl;
		}
		cout << "Server " INTERNALNAME " " INTERNALVERSION " is running on " 
			<< ((*it).first) << ":" << ((iPort != 0) ? iPort : (*it).second) 
			<< " TCP" << endl;
	}

	if (mDcConfig.mbWebServer) {
		if (!mWebListenFactory) {
			mWebListenFactory = new WebListenFactory(this);
		}

		vAddresses.clear();
		getAddresses(mDcConfig.msWebAddresses, vAddresses, 80);

		if (vAddresses.size() == 0) {
			if (ErrLog(0)) {
				LogStream() << "Incorrect address of the web server" << endl;
			}
		}
		for (vector<pair<string, int> >::iterator it = vAddresses.begin(); it != vAddresses.end(); ++it) {
			Server::Listening(mWebListenFactory, (*it).first, (iPort != 0) ? iPort : (*it).second);
			if (Log(0)) {
				LogStream() << "Web-Server is running on " 
					<< ((*it).first) << ":" << ((*it).second) 
					<< " TCP" << endl;
			}
			cout << "Web-Server is running on " 
				<< ((*it).first) << ":" << ((*it).second) 
				<< " TCP" << endl;
		}
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

			if (iFrequency < 0.4 * mDcConfig.miSysLoading) {
				mSystemLoad = SYSTEM_LOAD_SYSTEM_DOWN; // 0.4 (>2000 ms)
			} else if (iFrequency < 0.9 * mDcConfig.miSysLoading) {
				mSystemLoad = SYSTEM_LOAD_CRITICAL;    // 0.9 (>1000 ms)
			} else if (iFrequency < 3.6 * mDcConfig.miSysLoading) {
				mSystemLoad = SYSTEM_LOAD_MIDDLE;      // 3.6 (>250  ms)
			} else if (iFrequency < 18  * mDcConfig.miSysLoading) {
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
int DcServer::OnNewConn(Conn *conn) {
	DcConn * dcconn = (DcConn *) conn;

	if (mSystemLoad == SYSTEM_LOAD_SYSTEM_DOWN) {
		if (dcconn->Log(1)) {
			dcconn->LogStream() << "System down, close" << endl;
		}
		dcconn->CloseNow(CLOSE_REASON_HUB_LOAD);
		return 1;
	}

	/** Checking flood-entry (by ip) */
	if (mIPEnterFlood.Check(dcconn->getNetIp(), mTime)) {
		dcconn->CloseNow(CLOSE_REASON_IP_FLOOD);
		return 2;
	}

	if (dcconn->Log(5)) {
		dcconn->LogStream() << "[S]Stage OnNewConn" << endl;
	}
	string sMsg;
	dcconn->send(
		DcProtocol::Append_DC_HubName(
			DcProtocol::Append_DC_Lock(sMsg),
			mDcConfig.msHubName,
			mDcConfig.msTopic
		),
		false,
		false
	);

	#ifndef WITHOUT_PLUGINS
		if (mCalls.mOnUserConnected.CallAll(dcconn)) {
			dcconn->Flush();
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
				oss << w << " " << mDCLang.msTimes[0] << " ";
			}
			if (d) {
				oss << d << " " << mDCLang.msTimes[1] << " ";
			}
			if (h) {
				oss << h << " " << mDCLang.msTimes[2] << " ";
			}
			oss << m << " " << mDCLang.msTimes[3];
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
			StringReplace(mDCLang.msFirstMsg, string("HUB"), sCache, string(INTERNALNAME " " INTERNALVERSION));
			StringReplace(sCache, string("uptime"), sCache, sTimeCache);
			StringReplace(sCache, string("users"), sCache, iUsersVal);
			StringReplace(sCache, string("share"), sCache, sShareCache);
		}
		sendToUser(dcconn, sCache.c_str(), (char *) mDcConfig.msHubBot.c_str());
	}
	dcconn->SetTimeOut(HUB_TIME_OUT_LOGIN, mDcConfig.miTimeout[HUB_TIME_OUT_LOGIN], mTime); /** Timeout for enter */
	dcconn->SetTimeOut(HUB_TIME_OUT_KEY, mDcConfig.miTimeout[HUB_TIME_OUT_KEY], mTime);
	if (dcconn->Log(5)) {
		dcconn->LogStream() << "[E]Stage OnNewConn" << endl;
	}
	dcconn->Flush();
	return 0;
}



/** Returns pointer to line of the connection, in which will be recorded got data */
string * DcServer::GetPtrForStr(Conn * conn) {
	return conn->GetPtrForStr();
}



/** Function of the processing enterring data */
void DcServer::OnNewData(Conn * conn, string * data) {

	if (conn->Log(4)) { 
		conn->LogStream() << "IN: " << (*data) << NMDC_SEPARATOR << endl;
	}

	conn->mParser->Parse();
	conn->mProtocol->DoCmd(conn->mParser, conn);
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
bool DcServer::antiFlood(unsigned &iCount, Time & time, const unsigned &iCountLimit, const double &iTimeLimit) {
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
		if (User->mbInUserList) {
			string sMsg;
			mOpList.AddWithNick(User->msNick, User);
			if (User->mbHide) {
				User->send(DcProtocol::Append_DC_OpList(sMsg, User->msNick), false, true);
			} else {
				mDCUserList.sendToAll(DcProtocol::Append_DC_OpList(sMsg, User->msNick), true/*mDcConfig.mbDelayedMyINFO*/, false);
				mEnterList.sendToAll(sMsg, true/*mDcConfig.mbDelayedMyINFO*/, false);
			}
		}
	}
}



void DcServer::DelFromOps(DcUser * User) {
	if (User->mbInOpList) {
		User->mbInOpList = false;
		if (User->mbInUserList) {
			string sMsg1, sMsg2, sMsg3;
			mOpList.RemoveByNick(User->msNick);
			if (User->mbHide) {
				if (User->mDCConn == NULL) {
					return;
				}
				string s = User->getMyINFO();
				User->send(DcProtocol::Append_DC_Quit(sMsg1, User->msNick), false, false);
				if (User->mDCConn->mFeatures & SUPPORT_FEATUER_NOHELLO) {
					User->send(s, true, false);
				} else if (User->mDCConn->mFeatures & SUPPORT_FEATUER_NOGETINFO) {
					User->send(DcProtocol::Append_DC_Hello(sMsg2, User->msNick), false, false);
					User->send(s, true, false);
				} else {
					User->send(DcProtocol::Append_DC_Hello(sMsg2, User->msNick), false, false);
				}

				if ((User->mDCConn->mFeatures & SUPPORT_FEATUER_USERIP2) || User->mbInIpList) {
					User->send(DcProtocol::Append_DC_UserIP(sMsg3, User->msNick, User->getIp()));
				} else {
					s.clear();
					User->send(s);
				}
			} else {
				mDCUserList.sendToAll(DcProtocol::Append_DC_Quit(sMsg1, User->msNick), true/*mDcConfig.mbDelayedMyINFO*/, false);
				mEnterList.sendToAll(sMsg1, true/*mDcConfig.mbDelayedMyINFO*/, false);
				mHelloList.sendToAll(DcProtocol::Append_DC_Hello(sMsg2, User->msNick), true/*mDcConfig.mbDelayedMyINFO*/, false);
				mDCUserList.sendToAll(User->getMyINFO(), true/*mDcConfig.mbDelayedMyINFO*/);
				mEnterList.sendToAll(User->getMyINFO(), true/*mDcConfig.mbDelayedMyINFO*/);
				mIpList.sendToAll(DcProtocol::Append_DC_UserIP(sMsg3, User->msNick, User->getIp()), true, false);
			}
		}
	}
}



void DcServer::AddToIpList(DcUser * User) {
	if (!User->mbInIpList) {
		User->mbInIpList = true;
		if (User->mbInUserList) {
			mIpList.AddWithNick(User->msNick, User);
			User->send(mDCUserList.GetIpList(), true);
		}
	}
}



void DcServer::DelFromIpList(DcUser * User) {
	if (User->mbInIpList) {
		User->mbInIpList = false;
		if (User->mbInUserList) {
			mIpList.RemoveByNick(User->msNick);
		}
	}
}



void DcServer::AddToHide(DcUser * User) {
	if (!User->mbHide) {
		User->mbHide = true;
		if (User->mbInUserList) {
			string sMsg;
			User->mbInUserList = false;
			mDCUserList.sendToAll(DcProtocol::Append_DC_Quit(sMsg, User->msNick), false/*mDcConfig.mbDelayedMyINFO*/, false);
			mEnterList.sendToAll(sMsg, false/*mDcConfig.mbDelayedMyINFO*/, false); // false cache
			User->mbInUserList = true;
			mOpList.Remake();
			mDCUserList.Remake();
		}
	}
}



void DcServer::DelFromHide(DcUser * User) {
	if (User->mbHide) {
		User->mbHide = false;
		if (User->mbInUserList) {
			string sMsg1, sMsg2, sMsg3;
			if (User->mbInOpList) {
				User->mbInUserList = false;
				mHelloList.sendToAll(DcProtocol::Append_DC_Hello(sMsg1, User->msNick), false/*mDcConfig.mbDelayedMyINFO*/, false);
				sMsg2 = string(User->getMyINFO()).append(NMDC_SEPARATOR);
				mDCUserList.sendToAll(DcProtocol::Append_DC_OpList(sMsg2, User->msNick), false/*mDcConfig.mbDelayedMyINFO*/, false);
				mEnterList.sendToAll(sMsg2, false/*mDcConfig.mbDelayedMyINFO*/, false);
				mIpList.sendToAll(DcProtocol::Append_DC_UserIP(sMsg3, User->msNick, User->getIp()), false, false);
				User->mbInUserList = true;
			} else {
				User->mbInUserList = false;
				mHelloList.sendToAll(DcProtocol::Append_DC_Hello(sMsg1, User->msNick), false/*mDcConfig.mbDelayedMyINFO*/, false);
				mDCUserList.sendToAll(User->getMyINFO(), false/*mDcConfig.mbDelayedMyINFO*/);
				mEnterList.sendToAll(User->getMyINFO(), false/*mDcConfig.mbDelayedMyINFO*/);
				mIpList.sendToAll(DcProtocol::Append_DC_UserIP(sMsg3, User->msNick, User->getIp()), false, false);
				User->mbInUserList = true;
			}
			mOpList.Remake();
			mDCUserList.Remake();
		}
	}
}



bool DcServer::ValidateUser(DcConn * dcconn, const string & sNick) {

	/** Checking for bad symbols in nick */
	static string forbidedChars("$| ");
	if (sNick.npos != sNick.find_first_of(forbidedChars)) {
		if (dcconn->Log(2)) {
			dcconn->LogStream() << "Bad nick chars: '" << sNick << "'" << endl;
		}
		sendToUser(dcconn, mDCLang.msBadChars.c_str(), (char *) mDcConfig.msHubBot.c_str());
		return false;
	}

	return true;
}



bool DcServer::CheckNickLength(DcConn * dcconn, const unsigned iLen) {
	if (dcconn->miProfile == -1 && (iLen > mDcConfig.miMaxNickLen || iLen < mDcConfig.miMinNickLen)) {
		string sMsg;

		if (dcconn->Log(2)) {
			dcconn->LogStream() << "Bad nick len: " 
				<< iLen << " (" << dcconn->mDCUser->msNick 
				<< ") [" << mDcConfig.miMinNickLen << ", " 
				<< mDcConfig.miMaxNickLen << "]" << endl;
		}

		StringReplace(mDCLang.msBadNickLen, string("min"), sMsg, (int) mDcConfig.miMinNickLen);
		StringReplace(sMsg, string("max"), sMsg, (int) mDcConfig.miMaxNickLen);

		sendToUser(dcconn, sMsg.c_str(), (char *) mDcConfig.msHubBot.c_str());

		return false;
	}
	return true;
}



/** Checking for this nick used */
bool DcServer::CheckNick(DcConn *dcconn) {
	UserKey key = mDCUserList.Nick2Key(dcconn->mDCUser->msNick);
	if (mDCUserList.ContainsKey(key)) {
		string sMsg;
		DcUser * us = (DcUser *) mDCUserList.Find(key);

		if (!us->mDCConn || (us->getProfile() == -1 && us->getIp() != dcconn->Ip())) {
			if (dcconn->Log(2)) {
				dcconn->LogStream() << "Bad nick (used): '" 
					<< dcconn->mDCUser->msNick << "'["
					<< dcconn->Ip() << "] vs '" << us->msNick 
					<< "'[" << us->getIp() << "]" << endl;
			}

			StringReplace(mDCLang.msUsedNick, string("nick"), sMsg, dcconn->mDCUser->msNick);

			sendToUser(dcconn, sMsg.c_str(), (char *) mDcConfig.msHubBot.c_str());

			dcconn->send(DcProtocol::Append_DC_ValidateDenide(sMsg.erase(), dcconn->mDCUser->msNick));
			return false;
		}
		if (us->mDCConn->Log(3)) {
			us->mDCConn->LogStream() << "removed old user" << endl;
		}
		RemoveFromDCUserList(us);
		us->mDCConn->CloseNow(CLOSE_REASON_OLD_CLIENT);
	}
	return true;
}



bool DcServer::BeforeUserEnter(DcConn * dcconn) {
	unsigned iWantedMask;
	if (mDcConfig.mbDelayedLogin && dcconn->mbSendNickList) {
		iWantedMask = LOGIN_STATUS_LOGIN_DONE - LOGIN_STATUS_NICKLST;
	} else {
		iWantedMask = LOGIN_STATUS_LOGIN_DONE;
	}

	if (iWantedMask == dcconn->GetLSFlag(iWantedMask)) {
		if (dcconn->Log(3)) {
			dcconn->LogStream() << "Begin login" << endl;
		}
		if (!CheckNick(dcconn)) {
			dcconn->CloseNice(9000, CLOSE_REASON_INVALID_NICK);
			return false;
		}
		if (dcconn->mbSendNickList) {
			if (!mDcConfig.mbDelayedLogin) {
				DoUserEnter(dcconn);
			} else {
				mEnterList.Add(dcconn->mDCUser);
			}

			/** Can happen so that list not to send at a time */
			mDCProtocol.SendNickList(dcconn);

			dcconn->mbSendNickList = false;
			return true;
		}
		if (!dcconn->mDCUser->mbInUserList) {
			DoUserEnter(dcconn);
		}
		return true;
	} else { /** Invalid sequence of the sent commands */
		if (dcconn->Log(2)) {
			dcconn->LogStream() << "Invalid sequence of the sent commands (" 
				<< dcconn->GetLSFlag(iWantedMask) << "), wanted: " 
				<< iWantedMask << endl;
		}
		dcconn->CloseNow(CLOSE_REASON_BAD_SEQUENCE);
		return false;
	}
}



/** User entry */
void DcServer::DoUserEnter(DcConn * dcconn) {
	/** Check entry stages */
	if (LOGIN_STATUS_LOGIN_DONE != dcconn->GetLSFlag(LOGIN_STATUS_LOGIN_DONE)) {
		if (dcconn->Log(2)) {
			dcconn->LogStream() << "User Login when not all done (" 
				<< dcconn->GetLSFlag(LOGIN_STATUS_LOGIN_DONE) << ")" <<endl;
		}
		dcconn->CloseNow(CLOSE_REASON_NOT_LOGIN_DONE);
		return;
	}

	if (!CheckNick(dcconn)) {
		dcconn->CloseNice(9000, CLOSE_REASON_INVALID_NICK);
		return;
	}

	UserKey key = mDCUserList.Nick2Key(dcconn->mDCUser->msNick);

	/** User is already considered came */
	if (mEnterList.ContainsKey(key)) {
		/** We send user contents of cache without clear this cache */
		mEnterList.FlushForUser(dcconn->mDCUser);
		mEnterList.RemoveByKey(key);
	}

	/** Adding user to the user list */
	if (!AddToUserList((DcUser *)dcconn->mDCUser)) {
		dcconn->CloseNow(CLOSE_REASON_ADD_USER);
		return;
	}

	/** Show to all */
	ShowUserToAll(dcconn->mDCUser);

	AfterUserEnter(dcconn);

	dcconn->ClearTimeOut(HUB_TIME_OUT_LOGIN);
	((DcUser *)dcconn->mDCUser)->mTimeEnter.Get();
}



/** Adding user in the user list */
bool DcServer::AddToUserList(DcUser * User) {
	if (!User) {
		if (ErrLog(1)) {
			LogStream() << "Adding a NULL user to userlist" << endl;
		}
		return false;
	}
	if (User->mbInUserList) {
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
		User->mbInUserList = false;
		return false;
	}

	if (mDCUserList.Log(4)) {
		mDCUserList.LogStream() << "After add: " << User->msNick << " Size: " << mDCUserList.Size() << endl;
	}

	User->mbInUserList = true;
	if (!User->IsPassive()) {
		mActiveList.AddWithKey(key, User);
	}
	if (User->mbInOpList) {
		mOpList.AddWithKey(key, User);
	}
	if (User->mbInIpList) {
		mIpList.AddWithKey(key, User);
	}

	if (User->mDCConn) {
		User->mDCConn->mbIpRecv = true; /** Installing the permit on reception of the messages on ip */
		mChatList.AddWithKey(key, User);

		if (!(User->mDCConn->mFeatures & SUPPORT_FEATUER_NOHELLO)) {
			mHelloList.AddWithKey(key, User);
		}
		if (User->mDCConn->Log(3)) {
			User->mDCConn->LogStream() << "Adding at the end of Nicklist" << endl;
		}
		if (User->mDCConn->Log(3)) {
			User->mDCConn->LogStream() << "Becomes in list" << endl;
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
			if (User->mDCConn) {
				mCalls.mOnUserExit.CallAll(User->mDCConn);
			}
		#endif

		/** We make sure that user with such nick one! */
		DcUser *other = (DcUser *)mDCUserList.GetUserBaseByNick(User->msNick);
		if (!User->mDCConn) { /** Removing the bot */
			mDCUserList.RemoveByKey(key);
		} else if (other && other->mDCConn && User->mDCConn && other->mDCConn == User->mDCConn) {
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

	if (User->mbInUserList) {
		User->mbInUserList = false;

		if (!User->getHide()) {
			string sMsg;
			DcProtocol::Append_DC_Quit(sMsg, User->msNick);

			/** Delay in sending MyINFO (and Quit) */
			mDCUserList.sendToAll(sMsg, true/*mDcConfig.mbDelayedMyINFO*/, false);
		}
	}
	return true;
}



/** Show user to all */
bool DcServer::ShowUserToAll(DcUser *User) {
	string sMsg1, sMsg2;
	if (User->mbHide && User->mDCConn) {
		if (User->mDCConn->mFeatures & SUPPORT_FEATUER_NOHELLO) {
			User->mDCConn->send(string(User->getMyINFO()), true, false);
		} else if (User->mDCConn->mFeatures & SUPPORT_FEATUER_NOGETINFO) {
			User->mDCConn->send(DcProtocol::Append_DC_Hello(sMsg1, User->msNick), false, false);
			User->mDCConn->send(string(User->getMyINFO()), true, false);
		} else {
			User->mDCConn->send(DcProtocol::Append_DC_Hello(sMsg1, User->msNick), false, false);
		}

		if (User->mbInOpList) {
			User->mDCConn->send(DcProtocol::Append_DC_OpList(sMsg2, User->msNick), false, false);
		}
	} else {

		/** Sending the greeting for all users, not supporting feature NoHello (except enterring users) */
		mHelloList.sendToAll(DcProtocol::Append_DC_Hello(sMsg1, User->msNick), true/*mDcConfig.mbDelayedMyINFO*/, false);

		/** Show MyINFO string to all users */
		mDCUserList.sendToAll(User->getMyINFO(), true/*mDcConfig.mbDelayedMyINFO*/); // use cache -> so this can be after user is added

		/** Show MyINFO string of the current user to all enterring users */
		mEnterList.sendToAll(User->getMyINFO(), true/*mDcConfig.mbDelayedMyINFO*/);

		/** Op entry */
		if (User->mbInOpList) {
			mDCUserList.sendToAll(DcProtocol::Append_DC_OpList(sMsg2, User->msNick), true/*mDcConfig.mbDelayedMyINFO*/, false);
			mEnterList.sendToAll(sMsg2, true/*mDcConfig.mbDelayedMyINFO*/, false);
		}
	}

	bool inUserList = User->mbInUserList;

	/** Prevention of the double sending MyINFO string */
	if (!mDcConfig.mbDelayedLogin) {
			User->mbInUserList = false;
			mDCUserList.FlushCache();
			mEnterList.FlushCache();
			User->mbInUserList = inUserList;
	}

	if (mDcConfig.mbSendUserIp) {
		string sStr;
		User->mbInUserList = false;
		DcProtocol::Append_DC_UserIP(sStr, User->msNick, User->getIp());
		if (sStr.length()) {
			mIpList.sendToAll(sStr, true);
		}
		User->mbInUserList = inUserList;

		if (User->mbInIpList) {
			User->send(mDCUserList.GetIpList(), true, false);
		} else if (User->mDCConn && (User->mDCConn->mFeatures & SUPPORT_FEATUER_USERIP2)) { // UserIP2
			User->send(sStr, false, false);
		}
	}

	static string s;
	User->send(s, false, true);
	return true;
}



void DcServer::AfterUserEnter(DcConn *dcconn) {
	if (dcconn->Log(3)) {
		dcconn->LogStream() << "Entered the hub." << endl;
	}
	#ifndef WITHOUT_PLUGINS
		mCalls.mOnUserEnter.CallAll(dcconn);
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
		DcConn * dcconn;
		for (tCLIt it = mConnList.begin(); it != mConnList.end(); ++it) {
			dcconn = (DcConn *)(*it);
			if (dcconn && dcconn->mType == CLIENT_TYPE_NMDC && dcconn->mDCUser && dcconn->mDCUser->msNick == sN) {
				return (DcUser *)dcconn->mDCUser;
			}
		}
	}
	return NULL;
}



DcUserBase * DcServer::getDcUserBase(const char *sNick) {
	DcUser * User = GetDCUser(sNick);
	if (User) {
		return (DcUserBase *)User;
	}
	return NULL;
}



const vector<DcConnBase*> & DcServer::getDcConnBase(const char * sIP) {
	DcIpList::iterator it;
	mvIPConn.clear();
	for (it = mIPListConn->begin(DcConn::Ip2Num(sIP)); it != mIPListConn->end(); ++it) {
		DcConn * dcconn = (DcConn *)(*it);
		if (dcconn->mType == CLIENT_TYPE_NMDC) {
			mvIPConn.push_back(dcconn);
		}
	}
	return mvIPConn;
}



/** Send data to user */
bool DcServer::sendToUser(DcConnBase *dcConn, const char *sData, const char *sNick, const char *sFrom) {
	if (!dcConn || !sData) {
		return false;
	}

	// PM
	if (sFrom && sNick) {
		string sTo("<unknown>"), sStr;
		if (dcConn->mDcUserBase) {
			sTo = dcConn->mDcUserBase->getNick();
		}
		dcConn->send(DcProtocol::Append_DC_PM(sStr, sTo, sFrom, sNick, sData));
		return true;
	}

	// Chat
	if (sNick) {
		string sStr;
		dcConn->send(DcProtocol::Append_DC_Chat(sStr, sNick, sData));
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
	DcUser *User = GetDCUser(sTo);
	if (!User || !User->mDCConn) {
		return false;
	}

	// PM
	if (sFrom && sNick) {
		string sStr;
		User->mDCConn->send(DcProtocol::Append_DC_PM(sStr, sTo, sFrom, sNick, sData));
		return true;
	}

	// Chat
	if (sNick) {
		string sStr;
		User->mDCConn->send(DcProtocol::Append_DC_Chat(sStr, sNick, sData));
		return true;
	}

	// Simple Msg
	string sMsg(sData);
	if (sMsg.substr(sMsg.size() - 1, 1) != NMDC_SEPARATOR) {
		sMsg.append(NMDC_SEPARATOR);
	}
	User->mDCConn->send(sMsg);
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
	if (!sIP || !sData || !Conn::CheckIp(sIP)) {
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
		if (User && User->mbInUserList) {
			User->mbInUserList = false;
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
		(*ul_it)->mbInUserList = true;
	}

	return true;
}



bool DcServer::sendToAllExceptIps(const vector<string> & IPList, const char *sData, const char *sNick, const char *sFrom) {
	if (!sData) {
		return false;
	}

	DcConn * dcconn;
	vector<DcConn*> ul;
	bool bBadIP = false;
	for (List_t::const_iterator it = IPList.begin(); it != IPList.end(); ++it) {
		if (!DcConn::CheckIp(*it)) {
			bBadIP = true;
		}
		for (DcIpList::iterator mit = mIPListConn->begin(DcConn::Ip2Num((*it).c_str())); mit != mIPListConn->end(); ++mit) {
			dcconn = (DcConn*)(*mit);
			if (dcconn->mDCUser && dcconn->mDCUser->mbInUserList) {
				dcconn->mDCUser->mbInUserList = false;
				ul.push_back(dcconn);
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
		(*ul_it)->mDCUser->mbInUserList = true;
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



void DcServer::forceMove(DcConnBase *dcConn, const char *sAddress, const char *sReason /* = NULL */) {
	if (!dcConn || !sAddress) {
		return;
	}
	DcConn * dcconn = (DcConn *) dcConn;

	string sMsg, sForce, sNick("<unknown>");
	if (dcconn->mDCUser) {
		sNick = dcconn->mDCUser->msNick;
	}

	StringReplace(mDCLang.msForceMove, string("address"), sForce, string(sAddress));
	StringReplace(sForce, string("reason"), sForce, string(sReason != NULL ? sReason : ""));
	DcProtocol::Append_DC_PM(sMsg, sNick, mDcConfig.msHubBot, mDcConfig.msHubBot, sForce);
	DcProtocol::Append_DC_Chat(sMsg, mDcConfig.msHubBot, sForce);
	dcconn->send(DcProtocol::Append_DC_ForceMove(sMsg, sAddress));
	dcconn->CloseNice(9000, CLOSE_REASON_FORCE_MOVE);
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
		unregBot(mDcConfig.msHubBot);
	} else if (sName == "bRegMainBot") {
		if (sValue == "true" || 0 != atoi(sValue.c_str()) ) {
			if (regBot(mDcConfig.msHubBot, mDcConfig.msMainBotMyINFO, 
				mDcConfig.msMainBotIP, mDcConfig.mbMainBotKey) == -2) {
					regBot(mDcConfig.msHubBot, string("$ $$$0$"), 
						mDcConfig.msMainBotIP, mDcConfig.mbMainBotKey);
			}
		} else {
			unregBot(mDcConfig.msHubBot);
		}
	}

	config->convertFrom(sValue);

	if (sName == "sHubBot") {
		if (mDcConfig.mbRegMainBot) { /** Registration bot */
			if (regBot(mDcConfig.msHubBot, mDcConfig.msMainBotMyINFO, 
				mDcConfig.msMainBotIP, mDcConfig.mbMainBotKey) == -2) {
					regBot(mDcConfig.msHubBot, string("$ $$$0$"), 
						mDcConfig.msMainBotIP, mDcConfig.mbMainBotKey);
			}
		}
	} else if (sName == "sHubName" || sName == "sTopic") {
		string sMsg;
		sendToAll(DcProtocol::Append_DC_HubName(sMsg, mDcConfig.msHubName, mDcConfig.msTopic).c_str()); // use cache ?
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



int DcServer::regBot(const string & sNick, const string & sMyINFO, const string & sIP, bool bKey) {
	if (!sNick.length() || sNick.length() > 64 || sNick.find_first_of(" |$") != sNick.npos) {
		return -1;
	}

	string sINFO(sMyINFO);
	DcUser *User = new DcUser(sNick);
	User->mDcServer = this;
	User->mbInOpList = bKey;
	User->SetIp(sIP);
	if (!sINFO.length()) {
		sINFO = "$ $$$0$";
	}
	if (!User->setMyINFO(string("$MyINFO $ALL ") + sNick + " " + sINFO, sNick)) {
		return -2;
	}

	if (Log(3)) {
		LogStream() << "Reg bot: " << sNick << endl;
	}

	if (!AddToUserList(User)) {
		delete User;
		return -3;
	}
	mDCBotList.AddWithNick(sNick, User);
	ShowUserToAll(User);
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
	if (User->mDCConn) {
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
