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

#include "DcConn.h"
#include "DcServer.h" // server() and UserList
#include "DcUser.h" // for mDcUser

namespace dcserver {


DcConn::DcConn(int type, tSocket sock, Server * server) : 
	Conn(sock, server),
	DcConnBase(type),
	mFeatures(0),
	miProfile(-1),
	mbSendNickList(false),
	mbIpRecv(false),
	mbNickListInProgress(false),
	mDcUser(NULL),
	mSrCounter(0),
	mLoginStatus(0)
{
	mDcUserBase = NULL;
	SetClassName("DcConn");
}



DcConn::~DcConn() {
	if (mDcUser) {
		delete mDcUser;
	}
	mDcUser = NULL;
	mDcUserBase = NULL;
}

DcServer * DcConn::server() {
	return static_cast<DcServer *> (mServer);
}


int DcConn::send(const string & data, bool addSep, bool flush) {
	int iRet;
	if (!mWritable) {
		return 0;
	}

	if (data.size() >= mSendBufMax) {
		string msg(data);
		if (Log(0)) {
			LogStream() << "Too long message. Size: " << msg.size() << ". Max size: "
				<< mSendBufMax << " Message starts with: " << msg.substr(0, 25) << endl;
		}
		msg.resize(mSendBufMax - 1);

		if (addSep) {
			writeData(msg, false);
			iRet = writeData(NMDC_SEPARATOR, flush);
		} else {
			iRet = writeData(msg, flush);
		}
	} else {
		if (addSep) {
			writeData(data, false);
			iRet = writeData(NMDC_SEPARATOR, flush);
		} else {
			iRet = writeData(data, flush);
		}
	}
	return iRet;
}



void DcConn::disconnect() {
	closeNice(9000, CLOSE_REASON_PLUGIN);
}

//< Get real port
int DcConn::getPort() const {
	return mPort;
}



//< Get connection port
int DcConn::getPortConn() const {
	return mPortConn;
}



//< Get numeric IP
unsigned long DcConn::getNetIp() const {
	return mNetIp;
}



//< Get string of IP
const string & DcConn::getIp() const {
	return mIp;
}



//< Get string of server IP (host)
const string & DcConn::getIpConn() const {
	return mIpConn;
}



//< Get mac address
const string & DcConn::getMacAddress() const {
	return mMac;
}



long DcConn::getEnterTime() const {
	return mTimes.mKey.Sec();
}



void DcConn::setEnterTimeNow() {
	mTimes.mKey.Get();
}



//< Client's protocol version
const string & DcConn::getVersion() const {
	return mVersion;
}



const string & DcConn::getSupports() const {
	return msSupports;
}



//< Get profile
int DcConn::getProfile() const {
	return miProfile;
}



void DcConn::setProfile(int iProfile) {
	miProfile = iProfile;
}



//< Get some user data
const string & DcConn::getData() const {
	return msData;
}



void DcConn::setData(const string & sData) {
	msData = sData;
}



/** onFlush sending buffer */
void DcConn::onFlush() {
	if (mbNickListInProgress) {
		setLoginStatusFlag(LOGIN_STATUS_NICKLST);
		mbNickListInProgress = false;
		if (!mOk || !mWritable) {
			if (Log(2)) {
				LogStream() << "Connection closed during nicklist" << endl;
			}
		} else {
			if (Log(3)) {
				LogStream() << "Enter after nicklist" << endl;
			}
			server()->doUserEnter(this);
		}
	}
}

/** Set timeout for this connection */
void DcConn::setTimeOut(HubTimeOut to, double Sec, Time &now) {
	mTimeOut[to].setMaxDelay(Sec);
	mTimeOut[to].reset(now);
}

/** Clear timeout */
void DcConn::clearTimeOut(HubTimeOut to) {
	mTimeOut[to].disable();
}

/** Check timeout */
int DcConn::checkTimeOut(HubTimeOut to, Time &now) {
	return 0 == mTimeOut[to].check(now);
}

/** Timer for the current connection */
int DcConn::onTimer(Time &now) {
	DcServer * dcServer = server();

	/** Check timeouts. For entering only */
	if (!mDcUser || !mDcUser->getInUserList()) { // Optimisation
		for (int i = 0; i < HUB_TIME_OUT_MAX; ++i) {
			if (!checkTimeOut(HubTimeOut(i), now)) {
				string sMsg;
				if (Log(2)) {
					LogStream() << "Operation timeout (" << HubTimeOut(i) << ")" << endl;
				}
				stringReplace(dcServer->mDCLang.mTimeout, string("reason"), sMsg, dcServer->mDCLang.mTimeoutCmd[i]);
				dcServer->sendToUser(this, sMsg.c_str(), dcServer->mDcConfig.mHubBot.c_str());
				this->closeNice(9000, CLOSE_REASON_TIMEOUT);
				return 1;
			}
		}
	}

	/*Time lastRecv(mLastRecv);
	if (dcServer->minDelay(lastRecv, dcServer->mDcConfig.mTimeoutAny)) {
		if (Log(2)) {
			LogStream() << "Any action timeout..." << endl;
		}
		dcServer->sendToUser(this, dcServer->mDCLang.mTimeoutAny.c_str(), dcServer->mDcConfig.mHubBot.c_str());
		closeNice(9000, CLOSE_REASON_TIMEOUT_ANYACTION);
		return 2;
	}*/

	/** Check user on freeze.
		Sending void msg to all users, starting on mStartPing sec after entry,
		every mPingInterval sec
	*/
	Time Ago(now);
	Ago -= dcServer->mDcConfig.mStartPing;
	if (
		dcServer->minDelay(mTimes.mPingServer, dcServer->mDcConfig.mPingInterval) &&
		mDcUser && mDcUser->getInUserList() && mDcUser->mTimeEnter < Ago
	) {
		string s;
		send(s, true, true);
	}
	return 0;
}

void DcConn::closeNice(int msec, int iReason) {
	Conn::closeNice(msec, iReason);
}

void DcConn::closeNow(int iReason) {
	Conn::closeNow(iReason);
}

/** Set user object for current connection */
bool DcConn::setUser(DcUser * dcUser) {
	if (!dcUser) {
		if (ErrLog(1)) {
			LogStream() << "Trying to add a NULL user" << endl;
		}
		return false;
	}
	if (mDcUser) {
		if (ErrLog(1)) {
			LogStream() << "Trying to add user when it's actually done" << endl;
		}
		delete dcUser;
		return false;
	}
	mDcUser = dcUser;
	mDcUserBase = dcUser;
	dcUser->setIp(mIp);
	dcUser->mDcConn = this;
	dcUser->mDcConnBase = this;
	dcUser->mDcServer = server();
	if (Log(3)) {
		LogStream() << "User " << dcUser->msNick << " connected ... " << endl;
	}
	return true;
}



void DcConn::increaseSrCounter() {
	++mSrCounter;
}



void DcConn::emptySrCounter() {
	mSrCounter = 0;
}



void DcConn::onOk(bool ok) {
	if (mDcUser) {
		mDcUser->setCanSend(ok);
	}
}

bool DcConn::parseCommand(const char * cmd) {

	// ToDo: set command pointer
	if (getCommandPtr() == NULL || mParser == NULL) {
		return false;
	}
	string & command = *getCommandPtr();

	int type = mParser->mType;
	string oldCmd = command;
	command = cmd;
	if (mParser->parse() != type) {
		command = oldCmd;
		mParser->parse();
		return false;
	}
	return true;
}

const char * DcConn::getCommand() {
	string * command = getCommandPtr();
	if (command == NULL) {
		return NULL;
	}
	return (*command).c_str();
}





DcConnFactory::DcConnFactory(Protocol * protocol, Server * server) : 
	ConnFactory(protocol, server)
{
}

DcConnFactory::~DcConnFactory() {
}

Conn * DcConnFactory::createConn(tSocket sock) {
	if (!mServer) {
		return NULL;
	}

	DcConn * dcConn = new DcConn(CLIENT_TYPE_NMDC, sock, mServer);
	dcConn->setCreatedByFactory(true);
	dcConn->mConnFactory = this; /** Connection factory for current connection (DcConnFactory) */
	dcConn->mProtocol = mProtocol; /** Protocol pointer */

	DcServer * dcServer = static_cast<DcServer *> (mServer);
	if (!dcServer) {
		return NULL;
	}
	dcServer->mIPListConn->add(dcConn); /** Adding connection in IP-list */

	return static_cast<Conn *> (dcConn);
}

void DcConnFactory::deleteConn(Conn * &conn) {
	DcConn * dcConn = static_cast<DcConn *> (conn);
	DcServer * dcServer = static_cast<DcServer *> (mServer);
	if (dcConn && dcServer) {
		dcServer->mIPListConn->remove(dcConn);
		if (dcConn->getLoginStatusFlag(LOGIN_STATUS_ALOWED)) {
			dcServer->miTotalUserCount --;
			if (dcConn->mDcUser) {
				dcServer->miTotalShare -= dcConn->mDcUser->getShare();
			} else if (conn->Log(3)) {
					conn->LogStream() << "Del conn without user" << endl;
			}
		} else if (conn->Log(3)) {
			conn->LogStream() << "Del conn without ALOWED flag: " << dcConn->getLoginStatusFlag(LOGIN_STATUS_LOGIN_DONE) << endl;
		}
		if (dcConn->mDcUser) {
			if (dcConn->mDcUser->getInUserList()) {
				dcServer->removeFromDcUserList(static_cast<DcUser *> (dcConn->mDcUser));
			} else { // remove from enter list, if user was already added in it, but user was not added in user list
				dcServer->mEnterList.removeByNick(dcConn->mDcUser->getNick());
			}
			delete dcConn->mDcUser;
			dcConn->mDcUser = NULL;
			dcConn->mDcUserBase = NULL;
		}
		#ifndef WITHOUT_PLUGINS
			dcServer->mCalls.mOnUserDisconnected.callAll(dcConn);
		#endif
	} else if (conn->ErrLog(0)) {
		conn->LogStream() << "Fail error in deleteConn: dcConn = " <<
		(dcConn == NULL ? "NULL" : "not NULL") << ", dcServer = " << 
		(dcServer == NULL ? "NULL" : "not NULL") << endl;
	}
	ConnFactory::deleteConn(conn);
}

}; // namespace dcserver
