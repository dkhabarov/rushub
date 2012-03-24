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

#include "DcConn.h"
#include "DcServer.h" // server() and UserList
#include "DcUser.h" // for mDcUser
#include "ZlibFilter.h"

namespace dcserver {


DcConn::DcConn(tSocket sock, Server * server) : 
	Conn(sock, server, CONN_TYPE_INCOMING_TCP),
	mFeatures(0),
	mSendNickList(false),
	mIpRecv(false),
	mNickListInProgress(false),
	mDcUser(NULL),
	mPingServer(true),
	mSrCounter(0),
	mState(0)
{
	setClassName("DcConn");
}



DcConn::~DcConn() {
	if (mDcUser) {
		delete mDcUser;
	}
}




DcServer * DcConn::server() {
	if (!mServer) {
		if (log(LEVEL_FATAL)) {
			logStream() << "Server is NULL" << endl;
			throw "Server is NULL";
		}
	}
	return static_cast<DcServer *> (mServer);
}



DcProtocol * DcConn::dcProtocol() {
	if (!mProtocol) {
		if (log(LEVEL_FATAL)) {
			logStream() << "Protocol is NULL" << endl;
			throw "Protocol is NULL";
		}
	}
	return static_cast<DcProtocol *> (mProtocol);
}



size_t DcConn::send(const char * data, size_t len, bool addSep, bool flush) {
	if (!isWritable()) {
		return 0;
	}

	if (len >= mSendBufMax) {
		len = mSendBufMax;
		if (log(LEVEL_WARN)) {
			logStream() << "Too long message. Size: " << len << ". Max size: " << mSendBufMax << endl;
		}
	}

	// check for separator at end of data
	if (addSep) {
		const char * sep = getSeparator();
		size_t sepLen = getSeparatorLen();
		if (len < sepLen || strstr(data + len - sepLen, sep) == NULL) {
			size_t ret = writeData(data, len, false);
			ret += writeData(sep, sepLen, flush);
			return ret;
		}
	}
	return writeData(data, len, flush);
}



void DcConn::sendZpipe(const char * data, size_t len, bool flush) {
	string out;
	if ((mFeatures & SUPPORT_FEATUER_ZPIPE) && ZlibFilter::compressFull(data, len, out)) {
		send(STR_LEN("$ZOn"), true, false);
		send(out, true, flush);
	} else {
		send(data, len, false, flush);
	}
}



/// Set timeout for this connection
void DcConn::setLoginTimeOut(double sec, Time & now) {
	mLoginTimeOut.setMaxDelay(sec);
	mLoginTimeOut.reset(now);
}



/// Clear timeout
void DcConn::clearLoginTimeOut() {
	mLoginTimeOut.disable();
}



/// Timer for the current connection
int DcConn::onTimer(Time & now) {
	DcServer * dcServer = server();

	// Check timeouts. For entering only
	if (!mDcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST)) { // Optimisation
		if (mLoginTimeOut.check(now) != 0) {
			if (log(LEVEL_DEBUG)) {
				logStream() << "Timeout login" << endl;
			}
			dcServer->sendToUser(mDcUser, dcServer->mDcLang.mTimeoutLogon, dcServer->mDcConfig.mHubBot.c_str());
			closeNice(9000, CLOSE_REASON_TIMEOUT_LOGIN);
			return 1;
		}
	}

	/*
		Connection timeout. It's equal 600 sec by default.
		Some older clients can't ping the hub. So if hub will often drop clients, 
		you need increase this setting.
	*/
	/*Time lastRecv(mLastRecv);
	if (dcServer->minDelay(lastRecv, dcServer->mDcConfig.mTimeoutAny)) {
		if (log(LEVEL_DEBUG)) {
			logStream() << "Any action timeout..." << endl;
		}
		dcServer->sendToUser(mDcUser, dcServer->mDcLang.mTimeoutAny, dcServer->mDcConfig.mHubBot.c_str());
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



void DcConn::closeNice(int msec, int reason) {
	Conn::closeNice(msec, reason);
}



void DcConn::closeNow(int reason) {
	Conn::closeNow(reason);
}



/// Set user object for current connection
bool DcConn::setUser(DcUser * dcUser) {
	mDcUser = dcUser;
	return true;
}



unsigned int DcConn::getSrCounter() const {
	return mSrCounter;
}



void DcConn::increaseSrCounter() {
	++mSrCounter;
}



void DcConn::emptySrCounter() {
	mSrCounter = 0;
}



/// Is state
bool DcConn::isState(unsigned int state) const {
	return (mState & state) == state;
}



/// Get state
unsigned int DcConn::getState() const {
	return mState;
}



/// Set state
void DcConn::setState(unsigned int state) {
	mState |= state;
}



/// Reset state
void DcConn::resetState(unsigned int state) {
	mState = state;
}



void DcConn::onOk(bool ok) {
	if (mDcUser) {
		mDcUser->setCanSend(ok);
	}
}



bool DcConn::parseCommand(const char * cmd) {

	// TODO deprecated? Set command pointer
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

	DcConn * dcConn = new DcConn(sock, mServer);
	dcConn->mSelfConnFactory = this; // Connection factory for current connection (DcConnFactory)

	// Create DcUser
	DcUser * dcUser = new DcUser(CLIENT_TYPE_DC, dcConn);
	dcConn->setUser(dcUser);

	return dcConn;
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
				int64_t n = 0;
				share->setInt64(n); // for remove from total share
			}

			if (dcConn->mDcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST)) {
				dcServer->removeFromDcUserList(dcConn->mDcUser);
			} else { // remove from enter list, if user was already added in it, but user was not added in user list
				dcServer->mEnterList.remove(dcConn->mDcUser->getUidHash());
			}

			// Remove DcUser
			delete dcConn->mDcUser;
			dcConn->mDcUser = NULL;
		} else {
			if (conn->log(LEVEL_DEBUG)) {
				conn->logStream() << "Del conn without user" << endl;
			}
		}
	} else if (conn->log(LEVEL_FATAL)) {
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
