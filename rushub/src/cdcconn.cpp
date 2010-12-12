/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * Copyright (C) 2009-2010 by Setuper
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

#include "cdcconn.h"
#include "cdcserver.h" /** Server() and cUserList */
#include "cdcuser.h" /** for mDCUser */

namespace nDCServer {

cDCConn::cDCConn(int type, tSocket sock, cServer *s) : 
	cConn(sock, s),
	cDCConnBase(type),
	mFeatures(0),
	miProfile(-1),
	mbSendNickList(false),
	mbIpRecv(false),
	mbNickListInProgress(false),
	mDCUser(NULL),
	mLoginStatus(0),
	miSRCounter(0)
{
	mDCUserBase = NULL;
	SetClassName("cDCConn");
}

cDCConn::~cDCConn() {
	if(mDCUser) delete mDCUser;
	mDCUser = NULL;
	mDCUserBase = NULL;
}

int cDCConn::Send(const string & sData, bool bAddSep, bool bFlush) {
	int iRet;
	if(!mbWritable) return 0;

	if(sData.size() >= miSendBufMax) {
		string sMsg(sData);
		if(Log(0))
			LogStream() << "Too long message. Size: " << sMsg.size() << ". Max size: "
				<< miSendBufMax << " Message starts with: " << sMsg.substr(0, 25) << endl;
		sMsg.resize(miSendBufMax - 1);

		if(bAddSep) {
			WriteData(sMsg, false);
			iRet = WriteData(DC_SEPARATOR, bFlush);
		} else iRet = WriteData(sMsg, bFlush);
	} else {
		if(bAddSep) {
			WriteData(sData, false);
			iRet = WriteData(DC_SEPARATOR, bFlush);
		} else iRet = WriteData(sData, bFlush);
	}
	return iRet;
}

/** OnFlush sending buffer */
void cDCConn::OnFlush() {
	if(mbNickListInProgress) {
		SetLSFlag(eLS_NICKLST);
		mbNickListInProgress = false;
		if(!mbOk || !mbWritable) {
			if(Log(2)) LogStream() << "Connection closed during nicklist" << endl;
		} else {
			if(Log(3)) LogStream() << "Enter after nicklist" << endl;
			Server()->DoUserEnter(this);
		}
	}
}

/** Set timeout for this connection */
void cDCConn::SetTimeOut(tTimeOut to, double Sec, cTime &now) {
	mTimeOut[to].SetMaxDelay(Sec);
	mTimeOut[to].Reset(now);
}

/** Clear timeout */
void cDCConn::ClearTimeOut(tTimeOut to) {
	mTimeOut[to].Disable();
}

/** Check timeout */
int cDCConn::CheckTimeOut(tTimeOut to, cTime &now) {
	return 0 == mTimeOut[to].Check(now);
}

/** Timer for the current connection */
int cDCConn::OnTimer(cTime &now) {
	cDCServer * dcserver = Server();

	/** Check timeouts. For entering only */
	if(!mDCUser || !mDCUser->mbInUserList) { // Optimisation
		for(int i = 0; i < eTO_MAX; ++i) {
			if(!CheckTimeOut(tTimeOut(i), now)) {
				string sMsg;
				if(Log(2)) LogStream() << "Operation timeout (" << tTimeOut(i) << ")" << endl;
				StringReplace(dcserver->mDCLang.msTimeout, string("reason"), sMsg, dcserver->mDCLang.msTimeoutCmd[i]);
				dcserver->SendToUser(this, sMsg.c_str(), (char*)dcserver->mDCConfig.msHubBot.c_str());
				this->CloseNice(9000, eCR_TIMEOUT);
				return 1;
			}
		}
	}

	/*cTime lastRecv(mLastRecv);
	if(dcserver->MinDelay(lastRecv, dcserver->mDCConfig.miTimeoutAny)) {
		if(Log(2)) LogStream() << "Any action timeout..." << endl;
		dcserver->SendToUser(this, dcserver->mDCLang.msTimeoutAny.c_str(), (char*)dcserver->mDCConfig.msHubBot.c_str());
		CloseNice(9000, eCR_TO_ANYACTION);
		return 2;
	}*/

	/** Check user on freeze.
		Sending void msg to all users, starting on miStartPing sec after entry,
		every miPingInterval sec
	*/
	cTime Ago(now);
	Ago -= dcserver->mDCConfig.miStartPing;
	if(
		dcserver->MinDelay(mTimes.mPingServer, dcserver->mDCConfig.miPingInterval) &&
		mDCUser && mDCUser->mbInUserList && mDCUser->mTimeEnter < Ago
	) {
		string s;
		Send(s, true, true);
	}
	return 0;
}

void cDCConn::CloseNice(int msec, int iReason) {
	cConn::CloseNice(msec, iReason);
}

void cDCConn::CloseNow(int iReason) {
	cConn::CloseNow(iReason);
}

/** Set user object for current connection */
bool cDCConn::SetUser(cDCUser * User) {
	if(!User) {
		if(ErrLog(1)) LogStream() << "Trying to add a NULL user" << endl;
		return false;
	}
	if(mDCUser) {
		if(ErrLog(1)) LogStream() << "Trying to add user when it's actually done" << endl;
		delete User;
		return false;
	}
	mDCUser = User;
	mDCUserBase = User;
	User->SetIp(msIp);
	User->mDCConn = this;
	User->mDCConnBase = this;
	User->mDCServer = Server();
	if(Log(3)) LogStream() << "User " << User->msNick << " connected ... " << endl;
	return true;
}

cDCConnFactory::cDCConnFactory(cProtocol *protocol, cServer *s) : cConnFactory(protocol, s) {
}

cDCConnFactory::~cDCConnFactory() {
}

cConn *cDCConnFactory::CreateConn(tSocket sock) {
	if(!mServer) return NULL;

	cDCConn * dcconn = new cDCConn(eT_DC_CLIENT, sock, mServer);
	dcconn->mConnFactory = this; /** Connection factory for current connection (cDCConnFactory) */
	dcconn->mProtocol = mProtocol; /** Protocol pointer (cDCProtocol) */

	cDCServer * dcserver = (cDCServer *) mServer;
	if(!dcserver) return NULL;
	dcserver->mIPListConn->Add(dcconn); /** Adding connection in IP-list */

	return (cConn *)dcconn;
}

void cDCConnFactory::DelConn(cConn * &conn) {
	cDCConn * dcconn = (cDCConn *) conn;
	cDCServer * dcserver = (cDCServer *) mServer;
	if(dcconn && dcserver) {
		dcserver->mIPListConn->Remove(dcconn);
		if(dcconn->GetLSFlag(eLS_ALOWED)) {
			dcserver->miTotalUserCount --;
			if(dcconn->mDCUser)
				dcserver->miTotalShare -= dcconn->mDCUser->GetShare();
			else
				if(conn->Log(3)) conn->LogStream() << "Del conn without user" << endl;
		} else {
			if(conn->Log(3)) conn->LogStream() << "Del conn without ALOWED flag: " << dcconn->GetLSFlag(eLS_LOGIN_DONE) << endl;
		}
		if(dcconn->mDCUser) {
			if(dcconn->mDCUser->mbInUserList)
				dcserver->RemoveFromDCUserList((cDCUser*)dcconn->mDCUser);
			else // remove from enter list, if user was already added in it, but user was not added in user list
				dcserver->mEnterList.RemoveByNick(dcconn->mDCUser->GetNick());
			delete dcconn->mDCUser;
			dcconn->mDCUser = NULL;
			dcconn->mDCUserBase = NULL;
		}
		#ifndef WITHOUT_PLUGINS
			dcserver->mCalls.mOnUserDisconnected.CallAll(dcconn);
		#endif
	} else {
		if(conn->ErrLog(0)) conn->LogStream() << "Fail error in DelConn: dcconn = " <<
			(dcconn == NULL ? "NULL" : "not NULL") << ", dcserver = " << 
			(dcserver == NULL ? "NULL" : "not NULL") << endl;
	}
	cConnFactory::DelConn(conn);
}

}; // nDCServer
