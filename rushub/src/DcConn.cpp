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

#include "DcConn.h"
#include "DcServer.h" // server() and UserList
#include "DcUser.h" // for mDcUser

namespace dcserver {


DcConn::DcConn(int type, tSocket sock, Server * server) : 
	Conn(sock, server, CONN_TYPE_INCOMING_TCP),
	DcConnBase(type),
	mFeatures(0),
	mSendNickList(false),
	mIpRecv(false),
	mNickListInProgress(false),
	mDcUser(NULL),
	mPingServer(true),
	mSrCounter(0),
	mLoginStatus(0)
{
	mDcUserBase = NULL;
	setClassName("DcConn");
}



DcConn::~DcConn() {
	if (mDcUser) {
		delete mDcUser;
	}
}

DcServer * DcConn::server() {
	return static_cast<DcServer *> (mServer);
}



size_t DcConn::send(const char * data, size_t len, bool addSep, bool flush) {
	size_t ret = 0;
	if (mWritable) {
		if (len >= mSendBufMax) {
			len = mSendBufMax;
			if (log(WARN)) {
				logStream() << "Too long message. Size: " << len << ". Max size: " << mSendBufMax << endl;
			}
		}

		const char * sep = getSeparator();
		size_t sep_len = getSeparatorLen();

		// if addSep then check for separator at end of data
		if (addSep && len >= sep_len && strstr(data + len - sep_len, sep) == NULL) {
			writeData(data, len, false);
			ret = writeData(sep, sep_len, flush);
		} else {
			ret = writeData(data, len, flush);
		}
	}
	return ret;
}



/// Set timeout for this connection
void DcConn::setTimeOut(HubTimeOut to, double Sec, Time &now) {
	mTimeOut[to].setMaxDelay(Sec);
	mTimeOut[to].reset(now);
}

/// Clear timeout
void DcConn::clearTimeOut(HubTimeOut to) {
	mTimeOut[to].disable();
}

/// Check timeout
int DcConn::checkTimeOut(HubTimeOut to, Time &now) {
	return 0 == mTimeOut[to].check(now);
}

/// Timer for the current connection
int DcConn::onTimer(Time &now) {
	DcServer * dcServer = server();

	// Check timeouts. For entering only
	if (!mDcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST)) { // Optimisation
		for (int i = 0; i < HUB_TIME_OUT_MAX; ++i) {
			if (!checkTimeOut(HubTimeOut(i), now)) {
				if (log(DEBUG)) {
					logStream() << "Operation timeout (" << HubTimeOut(i) << ")" << endl;
				}
				string msg;
				stringReplace(dcServer->mDcLang.mTimeout, "reason", msg, dcServer->mDcLang.mTimeoutCmd[i]);
				dcServer->sendToUser(mDcUser, msg.c_str(), dcServer->mDcConfig.mHubBot.c_str());
				closeNice(9000, CLOSE_REASON_TIMEOUT);
				return 1;
			}
		}
	}

	/*Time lastRecv(mLastRecv);
	if (dcServer->minDelay(lastRecv, dcServer->mDcConfig.mTimeoutAny)) {
		if (log(DEBUG)) {
			logStream() << "Any action timeout..." << endl;
		}
		dcServer->sendToUser(mDcUser, dcServer->mDcLang.mTimeoutAny.c_str(), dcServer->mDcConfig.mHubBot.c_str());
		closeNice(9000, CLOSE_REASON_TIMEOUT_ANYACTION);
		return 2;
	}*/

	/* Check user on freeze.
		 Sending void msg to all users, starting on mStartPing sec after entry,
		 every mPingInterval sec
	*/
	Time Ago(now);
	Ago -= dcServer->mDcConfig.mStartPing;
	if (
		dcServer->minDelay(mPingServer, dcServer->mDcConfig.mPingInterval) &&
		mDcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST) && mDcUser->mTimeEnter < Ago
	) {
		send("", 0, true, true);
	}
	return 0;
}

void DcConn::closeNice(int msec, int iReason) {
	Conn::closeNice(msec, iReason);
}

void DcConn::closeNow(int iReason) {
	Conn::closeNow(iReason);
}

/// Set user object for current connection
bool DcConn::setUser(DcUser * dcUser) {
	mDcUser = dcUser;
	mDcUserBase = dcUser;
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

	// TODO: set command pointer
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
	dcConn->mSelfConnFactory = this; // Connection factory for current connection (DcConnFactory)

	// Create DcUser
	DcUser * dcUser = new DcUser(dcConn);
	dcConn->setUser(dcUser);

	return static_cast<Conn *> (dcConn);
}

void DcConnFactory::deleteConn(Conn * &conn) {
	DcConn * dcConn = static_cast<DcConn *> (conn);
	DcServer * dcServer = static_cast<DcServer *> (mServer);
	if (dcConn && dcServer) {

		#ifndef WITHOUT_PLUGINS
			dcServer->mCalls.mOnUserDisconnected.callAll(dcConn->mDcUser);
		#endif
		
		dcServer->mIpListConn->remove(dcConn);

		if (dcConn->mDcUser != NULL) {

			Param * share = (Param *) dcConn->mDcUser->getParam(USER_PARAM_SHARE);
			if (share != NULL) {
				share->setInt64(__int64(0)); // for remove from total share
			}

			if (dcConn->mDcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST)) {
				dcServer->removeFromDcUserList(static_cast<DcUser *> (dcConn->mDcUser));
			} else { // remove from enter list, if user was already added in it, but user was not added in user list
				dcServer->mEnterList.remove(dcConn->mDcUser->getUidHash());
			}

			// Remove DcUser
			delete dcConn->mDcUser;
			dcConn->mDcUser = NULL;
			dcConn->mDcUserBase = NULL;
		} else {
			if (conn->log(DEBUG)) {
				conn->logStream() << "Del conn without user" << endl;
			}
		}
	} else if (conn->log(FATAL)) {
		conn->logStream() << "Fail error in deleteConn: dcConn = " <<
		(dcConn == NULL ? "NULL" : "not NULL") << ", dcServer = " << 
		(dcServer == NULL ? "NULL" : "not NULL") << endl;
	}
	ConnFactory::deleteConn(conn);
}



int DcConnFactory::onNewConn(Conn * conn) {
	return ConnFactory::onNewConn(conn);
}


}; // namespace dcserver

/**
 * $Id$
 * $HeadURL$
 */
