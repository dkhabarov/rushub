/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz
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

#ifndef DC_CONN_H
#define DC_CONN_H

#include "Conn.h" // first (def winsock2.h)
#include "Plugin.h"
#include "DcUser.h"
#include "TimeOut.h"

using namespace ::server;
using namespace ::utils;

namespace dcserver {



/// Time outs during entering on hub only (HubTimeOut)
typedef enum {

	HUB_TIME_OUT_KEY = 0, ///< Waiting $Key after $Lock
	HUB_TIME_OUT_VALNICK, ///< Waiting $ValidateNick after $Lock
	HUB_TIME_OUT_LOGIN,   ///< Life time of the connection object before full entry (doUserEnter)
	HUB_TIME_OUT_MYINFO,  ///< After $ValidateNick and before $MyINFO timeout
	HUB_TIME_OUT_PASS,    ///< Waiting pass
	HUB_TIME_OUT_MAX      ///< Max timeout type

} HubTimeOut;



/// Login steps (LoginStatus)
enum LoginStatus {

	LOGIN_STATUS_KEY        = 1 << 0, ///< Key was checked (once)
	LOGIN_STATUS_ALOWED     = 1 << 1, ///< It allow for entry
	LOGIN_STATUS_VALNICK    = 1 << 2, ///< Nick was checked (once)
	LOGIN_STATUS_PASSWD     = 1 << 3, ///< Password was right or password was not need
	LOGIN_STATUS_VERSION    = 1 << 4, ///< Version was checked
	LOGIN_STATUS_MYINFO     = 1 << 5, ///< MyINFO string was received
	LOGIN_STATUS_NICKLST    = 1 << 6, ///< GetNickList flag
	LOGIN_STATUS_LOGIN_DONE = LOGIN_STATUS_KEY|LOGIN_STATUS_ALOWED|LOGIN_STATUS_VALNICK|LOGIN_STATUS_PASSWD|LOGIN_STATUS_VERSION|LOGIN_STATUS_MYINFO|LOGIN_STATUS_NICKLST

}; // enum LoginStatus



/// Supports features (SupportFeature)
enum SupportFeature {

	SUPPORT_FEATUER_USERCOMMAND = 1     , ///< UserCommand feature
	SUPPORT_FEATUER_NOGETINFO   = 1 << 1, ///< NoGetINFO feature
	SUPPORT_FEATURE_NOHELLO     = 1 << 2, ///< NoHello feature
	SUPPORT_FEATUER_USERIP2     = 1 << 3, ///< UserIP2 feature
	SUPPORT_FEATUER_TTHSEARCH   = 1 << 4, ///< TTHSearch feature
	SUPPORT_FEATUER_QUICKLIST   = 1 << 5, ///< Quicklist feature
	SUPPORT_FEATUER_PASSIVE     = 1 << 6, ///< Passive mode feature
	SUPPORT_FEATUER_USERIP      = 1 << 7, ///< UserIP feature

}; // enum SupportFeature



/// Enumeration of reasons to closing connection (CloseReason)
enum CloseReason {

	CLOSE_REASON_CMD_REPEAT = CLOSE_REASON_MAX,
	CLOSE_REASON_CMD_LENGTH,
	CLOSE_REASON_CMD_NULL,
	CLOSE_REASON_CMD_QUIT,
	CLOSE_REASON_CMD_KICK,
	CLOSE_REASON_CMD_FORCE_MOVE,
	CLOSE_REASON_CMD_PASSWORD_ERR,
	CLOSE_REASON_CMD_MYINFO_WITHOUT_USER,
	CLOSE_REASON_CMD_VERSION,
	CLOSE_REASON_CMD_UNKNOWN,
	CLOSE_REASON_CMD_SYNTAX,
	CLOSE_REASON_CMD_SEQUENCE,
	CLOSE_REASON_NICK_LEN,
	CLOSE_REASON_NICK_LONG,
	CLOSE_REASON_NICK_INVALID,
	CLOSE_REASON_NICK_MYINFO,
	CLOSE_REASON_NICK_SEARCH,
	CLOSE_REASON_NICK_SR,
	CLOSE_REASON_NICK_CTM,
	CLOSE_REASON_NICK_RCTM,
	CLOSE_REASON_NICK_CHAT,
	CLOSE_REASON_NICK_PM,
	CLOSE_REASON_NICK_MCTO,
	CLOSE_REASON_USER_INVALID,
	CLOSE_REASON_USER_ADD,
	CLOSE_REASON_USER_OLD,
	CLOSE_REASON_USERS_LIMIT,
	CLOSE_REASON_FLOOD,
	CLOSE_REASON_FLOOD_IP_ENTRY,
	CLOSE_REASON_TIMEOUT,
	CLOSE_REASON_TIMEOUT_ANYACTION,
	CLOSE_REASON_LOGIN_NOT_DONE,
	CLOSE_REASON_PLUGIN,
	CLOSE_REASON_HUB_LOAD, // System down, do not take new users
	CLOSE_REASON_WEB

}; // enum CloseReason



class DcServer; // server()



class DcConnFactory : public ConnFactory {

public:

	DcConnFactory(Protocol * protocol, Server * server);
	virtual ~DcConnFactory();
	virtual Conn * createConn(tSocket sock = 0);
	virtual void deleteConn(Conn * &);

}; // DcConnFactory



class DcConn : public Conn, public DcConnBase {

public:

	unsigned mFeatures;         ///< Features (PROTOCOL NMDC)

	bool mSendNickList;        ///< Sending user list when login
	bool mIpRecv;              ///< Permit on reception of the messages, sending on my ip
	bool mNickListInProgress;  ///< True while sending first nicklist
	DcUser * mDcUser;           ///< User object

	struct Timers { ///< Timers

		Time mTime[NMDC_TYPE_UNKNOWN]; // PROTOCOL NMDC !
		unsigned mCount[NMDC_TYPE_UNKNOWN]; // PROTOCOL NMDC !

		Timers() {
			for (int i = 0; i <= NMDC_TYPE_UNKNOWN; ++i) { // PROTOCOL NMDC !
				mCount[i] = 0;
			}
		}

	} mTimes1, mTimes2;


public:

	DcConn(int type, tSocket sock = 0, Server * server = NULL);
	virtual ~DcConn();


	/// Sending RAW command to the client
	inline size_t send(const string & data, bool addSep = false, bool flush = true) {
		return send(data.c_str(), data.size(), addSep, flush);
	}
	virtual size_t send(const char * data, size_t len, bool addSep = false, bool flush = true);

	virtual bool parseCommand(const char * cmd);

	virtual const char * getCommand();

	// =====================



	/// Timer for current connection
	virtual int onTimer(Time & now);

	virtual void closeNow(int iReason = 0);
	virtual void closeNice(int msec, int iReason = 0);



	/// Pointer to the server
	DcServer * server();

	/// Set user object for current connection
	bool setUser(DcUser * dcUser);

	unsigned int getSrCounter() {
		return mSrCounter;
	}

	void increaseSrCounter();
	void emptySrCounter();



	/// Setting entry status flag
	void setLoginStatusFlag(unsigned int s) {
		mLoginStatus |= s;
	}

	/// Reset flag
	void resetLoginStatusFlag(unsigned int s) {
		mLoginStatus = s;
	}

	/// Get flag
	unsigned int getLoginStatusFlag(unsigned int s) {
		return mLoginStatus & s;
	}

	/// Set timeout
	void setTimeOut(HubTimeOut, double Sec, Time & now);

	/// Clear timeout
	void clearTimeOut(HubTimeOut);

	/// Check timeout
	int checkTimeOut(HubTimeOut t, Time & now);


protected:

	TimeOut mTimeOut[HUB_TIME_OUT_MAX];

	/// Time last ping from server to client
	Time mPingServer;

protected:

	/// Flush sending buffer
	virtual void onFlush();

	virtual void onOk(bool);

private:

	/// Counter search results
	unsigned int mSrCounter;

	/// Login status
	unsigned int mLoginStatus;


}; // class DcConn

}; // namespace dcserver

#endif // DC_CONN_H

/**
 * $Id$
 * $HeadURL$
 */
