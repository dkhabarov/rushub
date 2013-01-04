/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz
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

#ifndef DC_CONN_H
#define DC_CONN_H

#include "Conn.h" // first (def winsock2.h)
#include "Plugin.h"
#include "DcUser.h"
#include "TimeOut.h"

using namespace ::server;
using namespace ::utils;

namespace dcserver {


/// Supports features (SupportFeature)
enum SupportFeature {

	SUPPORT_FEATUER_USERCOMMAND = 1 << 0, ///< UserCommand feature
	SUPPORT_FEATUER_NOGETINFO   = 1 << 1, ///< NoGetINFO feature
	SUPPORT_FEATURE_NOHELLO     = 1 << 2, ///< NoHello feature
	SUPPORT_FEATUER_USERIP2     = 1 << 3, ///< UserIP2 feature
	SUPPORT_FEATUER_ZPIPE       = 1 << 4, ///< ZPipe0 or ZPipe feature
	SUPPORT_FEATUER_QUICKLIST   = 1 << 5, ///< Quicklist feature
	SUPPORT_FEATUER_PASSIVE     = 1 << 6, ///< Passive mode feature
	SUPPORT_FEATUER_USERIP      = 1 << 7  ///< UserIP feature

}; // enum SupportFeature



/// Enumeration of reasons to closing connection (CloseReason)
enum CloseReason {

	CLOSE_REASON_CMD_STATE = CLOSE_REASON_MAX,
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
	CLOSE_REASON_TIMEOUT_LOGIN,
	CLOSE_REASON_TIMEOUT_ANYACTION,
	CLOSE_REASON_NOT_LOGIN,
	CLOSE_REASON_PLUGIN,
	CLOSE_REASON_HUB_LOAD, // System down, do not take new users
	CLOSE_REASON_WEB

}; // enum CloseReason



class DcServer; // server()

namespace protocol {
	class DcProtocol;
}



/// Factory of DC connection
class DcConnFactory : public ConnFactory {

public:

	DcConnFactory(Protocol *, Server *);
	virtual ~DcConnFactory();
	virtual Conn * createConn(tSocket sock = 0);
	virtual void deleteConn(Conn * &);
	virtual int onNewConn(Conn *);

}; // DcConnFactory



/// Main DC connection
class DcConn : public Conn {

	friend class DcUser; // for Param

public:

	unsigned int mFeatures;    ///< Features (PROTOCOL NMDC)

	bool mSendNickList;        ///< Sending user list when login
	bool mIpRecv;              ///< Permit on reception of the messages, sending on my ip
	volatile bool mNickListInProgress;  ///< True while sending first nicklist
	DcUser * mDcUser;          ///< User object

	/// Atniflood struct
	struct Antiflood {
		Time mTime, mTime2;
		unsigned mCount, mCount2;
		Antiflood() : mCount(0), mCount2(0) {
		}
	} mAntiflood[NMDC_TYPE_UNKNOWN + 1]; // PROTOCOL NMDC !

public:

	DcConn(tSocket sock = 0, Server * server = NULL);
	virtual ~DcConn();


	/// Sending RAW command to the client
	inline size_t send(const string & data, bool addSep = false, bool flush = true) {
		return send(data.c_str(), data.size(), addSep, flush);
	}
	virtual size_t send(const char * data, size_t len, bool addSep = false, bool flush = true);

	/// Sending command with zlib compression
	inline void sendZpipe(const string & data, bool flush = true) {
		sendZpipe(data.c_str(), data.size(), flush);
	}
	void sendZpipe(const char * data, size_t len, bool flush);

	virtual bool parseCommand(const char * cmd);

	virtual const char * getCommand();

	// =====================

	virtual void closeNow(int reason = 0);

	virtual void closeNice(int msec, int reason = 0);


	/// Pointer to the server
	DcServer * server();

	/// Pointer to the dcProtocol
	DcProtocol * dcProtocol();

	/// Set user object for current connection
	bool setUser(DcUser * dcUser);

	unsigned int getSrCounter() const;

	void increaseSrCounter();
	void emptySrCounter();

	/// Is state
	bool isState(unsigned int state) const;

	/// Get state
	unsigned int getState() const;

	/// Set state
	void setState(unsigned int state);

	/// Reset state
	void resetState(unsigned int state);

	/// Set login timeout
	void setLoginTimeOut(double sec, Time & now);

	/// Clear login timeout
	void clearLoginTimeOut();

protected:

	virtual void onOk(bool);

private:

	/// Time last ping from server to client
	Time mPingServer;

	/// Timeouts
	TimeOut mLoginTimeOut;

	/// Counter search results
	unsigned int mSrCounter;

	/// Protocol state
	unsigned int mState;

private:

	/// Timer for current connection
	virtual int onTimer(Time & now);

	DcConn(const DcConn &);
	DcConn & operator = (const DcConn &);

}; // class DcConn

} // namespace dcserver

#endif // DC_CONN_H

/**
 * $Id$
 * $HeadURL$
 */
