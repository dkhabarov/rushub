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


DcConn::DcConn(int type, tSocket sock, Server *s) : 
	Conn(sock, s),
	DcConnBase(type),
	mFeatures(0),
	miProfile(-1),
	mbSendNickList(false),
	mbIpRecv(false),
	mbNickListInProgress(false),
	mDcUser(NULL),
	miSRCounter(0),
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



int DcConn::send(const string & data, bool addSep, bool flush) {
	int iRet;
	if (!mbWritable) {
		return 0;
	}

	if (data.size() >= miSendBufMax) {
		string msg(data);
		if (Log(0)) {
			LogStream() << "Too long message. Size: " << msg.size() << ". Max size: "
				<< miSendBufMax << " Message starts with: " << msg.substr(0, 25) << endl;
		}
		msg.resize(miSendBufMax - 1);

		if (addSep) {
			WriteData(msg, false);
			iRet = WriteData(NMDC_SEPARATOR, flush);
		} else {
			iRet = WriteData(msg, flush);
		}
	} else {
		if (addSep) {
			WriteData(data, false);
			iRet = WriteData(NMDC_SEPARATOR, flush);
		} else {
			iRet = WriteData(data, flush);
		}
	}
	return iRet;
}



void DcConn::disconnect() {
	CloseNice(9000, CLOSE_REASON_PLUGIN);
}

//< Get real port
int DcConn::getPort() const {
	return miPort;
}



//< Get connection port
int DcConn::getPortConn() const {
	return miPortConn;
}



//< Get numeric IP
unsigned long DcConn::getNetIp() const {
	return miNetIp;
}



//< Get string of IP
const string & DcConn::getIp() const {
	return msIp;
}



//< Get string of server IP (host)
const string & DcConn::getIpConn() const {
	return msIpConn;
}



//< Get mac address
const string & DcConn::getMacAddress() const {
	return msMAC;
}



long DcConn::getEnterTime() const {
	return mTimes.mKey.Sec();
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



/** OnFlush sending buffer */
void DcConn::OnFlush() {
	if (mbNickListInProgress) {
		SetLSFlag(LOGIN_STATUS_NICKLST);
		mbNickListInProgress = false;
		if (!mbOk || !mbWritable) {
			if (Log(2)) {
				LogStream() << "Connection closed during nicklist" << endl;
			}
		} else {
			if (Log(3)) {
				LogStream() << "Enter after nicklist" << endl;
			}
			server()->DoUserEnter(this);
		}
	}
}

/** Set timeout for this connection */
void DcConn::SetTimeOut(HubTimeOut to, double Sec, Time &now) {
	mTimeOut[to].SetMaxDelay(Sec);
	mTimeOut[to].Reset(now);
}

/** Clear timeout */
void DcConn::ClearTimeOut(HubTimeOut to) {
	mTimeOut[to].Disable();
}

/** Check timeout */
int DcConn::CheckTimeOut(HubTimeOut to, Time &now) {
	return 0 == mTimeOut[to].Check(now);
}

/** Timer for the current connection */
int DcConn::onTimer(Time &now) {
	DcServer * dcServer = server();

	/** Check timeouts. For entering only */
	if (!mDcUser || !mDcUser->getInUserList()) { // Optimisation
		for (int i = 0; i < HUB_TIME_OUT_MAX; ++i) {
			if (!CheckTimeOut(HubTimeOut(i), now)) {
				string sMsg;
				if (Log(2)) {
					LogStream() << "Operation timeout (" << HubTimeOut(i) << ")" << endl;
				}
				StringReplace(dcServer->mDCLang.msTimeout, string("reason"), sMsg, dcServer->mDCLang.msTimeoutCmd[i]);
				dcServer->sendToUser(this, sMsg.c_str(), (char*)dcServer->mDcConfig.msHubBot.c_str());
				this->CloseNice(9000, CLOSE_REASON_TIMEOUT);
				return 1;
			}
		}
	}

	/*Time lastRecv(mLastRecv);
	if (dcServer->MinDelay(lastRecv, dcServer->mDcConfig.miTimeoutAny)) {
		if (Log(2)) {
			LogStream() << "Any action timeout..." << endl;
		}
		dcServer->sendToUser(this, dcServer->mDCLang.msTimeoutAny.c_str(), (char*)dcServer->mDcConfig.msHubBot.c_str());
		CloseNice(9000, CLOSE_REASON_TO_ANYACTION);
		return 2;
	}*/

	/** Check user on freeze.
		Sending void msg to all users, starting on miStartPing sec after entry,
		every miPingInterval sec
	*/
	Time Ago(now);
	Ago -= dcServer->mDcConfig.miStartPing;
	if (
		dcServer->MinDelay(mTimes.mPingServer, dcServer->mDcConfig.miPingInterval) &&
		mDcUser && mDcUser->getInUserList() && mDcUser->mTimeEnter < Ago
	) {
		string s;
		send(s, true, true);
	}
	return 0;
}

void DcConn::CloseNice(int msec, int iReason) {
	Conn::CloseNice(msec, iReason);
}

void DcConn::CloseNow(int iReason) {
	Conn::CloseNow(iReason);
}

/** Set user object for current connection */
bool DcConn::SetUser(DcUser * User) {
	if (!User) {
		if (ErrLog(1)) {
			LogStream() << "Trying to add a NULL user" << endl;
		}
		return false;
	}
	if (mDcUser) {
		if (ErrLog(1)) {
			LogStream() << "Trying to add user when it's actually done" << endl;
		}
		delete User;
		return false;
	}
	mDcUser = User;
	mDcUserBase = User;
	User->SetIp(msIp);
	User->mDcConn = this;
	User->mDcConnBase = this;
	User->mDcServer = server();
	if (Log(3)) {
		LogStream() << "User " << User->msNick << " connected ... " << endl;
	}
	return true;
}

DcConnFactory::DcConnFactory(Protocol *protocol, Server *s) : ConnFactory(protocol, s) {
}

DcConnFactory::~DcConnFactory() {
}

Conn *DcConnFactory::CreateConn(tSocket sock) {
	if (!mServer) {
		return NULL;
	}

	DcConn * dcconn = new DcConn(CLIENT_TYPE_NMDC, sock, mServer);
	dcconn->mConnFactory = this; /** Connection factory for current connection (DcConnFactory) */
	dcconn->mProtocol = mProtocol; /** Protocol pointer (DcProtocol) */

	DcServer * dcServer = (DcServer *) mServer;
	if (!dcServer) {
		return NULL;
	}
	dcServer->mIPListConn->Add(dcconn); /** Adding connection in IP-list */

	return (Conn *)dcconn;
}

void DcConnFactory::DelConn(Conn * &conn) {
	DcConn * dcconn = (DcConn *) conn;
	DcServer * dcServer = (DcServer *) mServer;
	if (dcconn && dcServer) {
		dcServer->mIPListConn->Remove(dcconn);
		if (dcconn->GetLSFlag(LOGIN_STATUS_ALOWED)) {
			dcServer->miTotalUserCount --;
			if (dcconn->mDcUser) {
				dcServer->miTotalShare -= dcconn->mDcUser->getShare();
			} else if (conn->Log(3)) {
					conn->LogStream() << "Del conn without user" << endl;
			}
		} else if (conn->Log(3)) {
			conn->LogStream() << "Del conn without ALOWED flag: " << dcconn->GetLSFlag(LOGIN_STATUS_LOGIN_DONE) << endl;
		}
		if (dcconn->mDcUser) {
			if (dcconn->mDcUser->getInUserList()) {
				dcServer->RemoveFromDCUserList((DcUser*)dcconn->mDcUser);
			} else { // remove from enter list, if user was already added in it, but user was not added in user list
				dcServer->mEnterList.RemoveByNick(dcconn->mDcUser->getNick());
			}
			delete dcconn->mDcUser;
			dcconn->mDcUser = NULL;
			dcconn->mDcUserBase = NULL;
		}
		#ifndef WITHOUT_PLUGINS
			dcServer->mCalls.mOnUserDisconnected.CallAll(dcconn);
		#endif
	} else if (conn->ErrLog(0)) {
		conn->LogStream() << "Fail error in DelConn: dcconn = " <<
		(dcconn == NULL ? "NULL" : "not NULL") << ", dcServer = " << 
		(dcServer == NULL ? "NULL" : "not NULL") << endl;
	}
	ConnFactory::DelConn(conn);
}

}; // namespace dcserver
