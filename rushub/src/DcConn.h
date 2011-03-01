/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz

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

#ifndef DC_CONN_H
#define DC_CONN_H

#include "Plugin.h"
#include "Conn.h"
#include "DcUser.h"
#include "TimeOut.h"

using namespace ::server;
using namespace ::utils;

namespace dcserver {



/** Time outs during entering on hub only (HubTimeOut) */
typedef enum {

	HUB_TIME_OUT_KEY = 0, //< Waiting $Key after $Lock
	HUB_TIME_OUT_VALNICK, //< Waiting $ValidateNick after $Lock
	HUB_TIME_OUT_LOGIN,   //< Life time of the connection object before full entry (DoUserEnter)
	HUB_TIME_OUT_MYINFO,  //< After $ValidateNick and before $MyINFO timeout
	HUB_TIME_OUT_PASS,    //< Waiting pass
	HUB_TIME_OUT_MAX      //< Max timeout type

} HubTimeOut;



/** Login steps (LoginStatus) */
enum LoginStatus {

	LOGIN_STATUS_KEY        = 1 << 0, //< Key was checked (once)
	LOGIN_STATUS_ALOWED     = 1 << 1, //< It allow for entry
	LOGIN_STATUS_VALNICK    = 1 << 2, //< Nick was checked (once)
	LOGIN_STATUS_PASSWD     = 1 << 3, //< Password was right or password was not need
	LOGIN_STATUS_VERSION    = 1 << 4, //< Version was checked
	LOGIN_STATUS_MYINFO     = 1 << 5, //< MyINFO string was received
	LOGIN_STATUS_NICKLST    = 1 << 6, //< GetNickList flag
	LOGIN_STATUS_LOGIN_DONE = LOGIN_STATUS_KEY|LOGIN_STATUS_ALOWED|LOGIN_STATUS_VALNICK|LOGIN_STATUS_PASSWD|LOGIN_STATUS_VERSION|LOGIN_STATUS_MYINFO|LOGIN_STATUS_NICKLST

}; // enum LoginStatus



/** Supports features (SupportFeature) */
enum SupportFeature {

	SUPPORT_FEATUER_USERCOMMAND = 1     , //< UserCommand feature
	SUPPORT_FEATUER_NOGETINFO   = 1 << 1, //< NoGetINFO feature
	SUPPORT_FEATUER_NOHELLO     = 1 << 2, //< NoHello feature
	SUPPORT_FEATUER_USERIP2     = 1 << 3, //< UserIP2 feature
	SUPPORT_FEATUER_TTHSEARCH   = 1 << 4, //< TTHSearch feature
	SUPPORT_FEATUER_QUICKLIST   = 1 << 5, //< Quicklist feature
	SUPPORT_FEATUER_PASSIVE     = 1 << 6, //< Passive mode feature

}; // enum SupportFeature



/** Enumeration of reasons to closing connection (CloseReason) */
enum CloseReason {

	CLOSE_REASON_BAD_FLAG = CLOSE_REASON_MAX,
	CLOSE_REASON_BAD_CMD_PARAM,
	CLOSE_REASON_BAD_CMD_LENGTH,
	CLOSE_REASON_BAD_CMD_NULL,
	CLOSE_REASON_LONG_NICK,
	CLOSE_REASON_SETUSER,
	CLOSE_REASON_SYNTAX_CHAT,
	CLOSE_REASON_SYNTAX_TO,
	CLOSE_REASON_BAD_NICK_PM,
	CLOSE_REASON_SYNTAX_MCTO,
	CLOSE_REASON_BAD_NICK_MCTO,
	CLOSE_REASON_SYNTAX_SEARCH,
	CLOSE_REASON_SYNTAX_SR,
	CLOSE_REASON_SYNTAX_CTM,
	CLOSE_REASON_SYNTAX_RCTM,
	CLOSE_REASON_SYNTAX_KICK,
	CLOSE_REASON_SYNTAX_OFM,
	CLOSE_REASON_SYNTAX_GETINFO,
	CLOSE_REASON_BAD_DC_CMD,
	CLOSE_REASON_IP_FLOOD,
	CLOSE_REASON_OLD_CLIENT,
	CLOSE_REASON_BAD_SEQUENCE,
	CLOSE_REASON_NOT_LOGIN_DONE,
	CLOSE_REASON_ADD_USER,
	CLOSE_REASON_PLUGIN,
	CLOSE_REASON_TIMEOUT,
	CLOSE_REASON_TO_ANYACTION,
	CLOSE_REASON_FLOOD,
	CLOSE_REASON_INVALID_USER,
	CLOSE_REASON_INVALID_NICK,
	CLOSE_REASON_USERS_LIMIT,
	CLOSE_REASON_NICK_LEN,
	CLOSE_REASON_LOGIN_ERR,
	CLOSE_REASON_SYNTAX_VERSION,
	CLOSE_REASON_SYNTAX_MYINFO,
	CLOSE_REASON_MYINFO_WITHOUT_USER,
	CLOSE_REASON_BAD_MYINFO_NICK,
	CLOSE_REASON_CHAT_NICK,
	CLOSE_REASON_NICK_SEARCH,
	CLOSE_REASON_NICK_SR,
	CLOSE_REASON_NICK_CTM,
	CLOSE_REASON_NICK_RCTM,
	CLOSE_REASON_UNKNOWN_CMD,
	CLOSE_REASON_QUIT,
	CLOSE_REASON_WEB,
	CLOSE_REASON_HUB_LOAD, // System down, do not take new users	
	CLOSE_REASON_FORCE_MOVE,
	CLOSE_REASON_KICK

}; // enum CloseReason



class DcServer; /** server() */


namespace protocol {

	class DcProtocol;

}; // namespace protocol


using ::dcserver::protocol::DcProtocol;



class DcConnFactory : public ConnFactory {

public:

	DcConnFactory(Protocol * protocol, Server * server);
	virtual ~DcConnFactory();
	virtual Conn * CreateConn(tSocket sock = 0);
	virtual void DelConn(Conn * &);

}; // DcConnFactory



class DcConn : public Conn, public DcConnBase {

	friend class protocol::DcProtocol; /** for miSRCounter from DcProtocol::DC_SR */

public:

	unsigned mFeatures;         //< Features
	string msSupports;          //< Support cmd param
	string mVersion;           //< DC version
	string msData;              //< Some user's data
	int miProfile;              //< Profile
	bool mbSendNickList;        //< Sending user list when login
	bool mbIpRecv;              //< Permit on reception of the messages, sending on my ip
	bool mbNickListInProgress;  //< True while sending first nicklist
	DcUser * mDCUser;          //< User object

	struct Timers { /** Timers */

		Time mSearch;
		Time mSR;
		Time mMyINFO;
		Time mChat;
		Time mNickList;
		Time mTo;
		Time mCTM;
		Time mRCTM;
		Time mMCTo;
		Time mPing;
		Time mUnknown;

		unsigned miSearch;
		unsigned miSR;
		unsigned miMyINFO;
		unsigned miChat;
		unsigned miNickList;
		unsigned miTo;
		unsigned miCTM;
		unsigned miRCTM;
		unsigned miMCTo;
		unsigned miPing;
		unsigned miUnknown;

		Timers() : 
			mSearch(0l),
			mSR(0l),
			mMyINFO(0l),
			mChat(0l),
			mNickList(0l),
			mTo(0l),
			mCTM(0l),
			mRCTM(0l),
			mMCTo(0l),
			mPing(0l),
			mUnknown(0l),
			miSearch(0),
			miSR(0),
			miMyINFO(0),
			miChat(0),
			miNickList(0),
			miTo(0),
			miCTM(0),
			miRCTM(0),
			miMCTo(0),
			miPing(0),
			miUnknown(0)
		{
		}

	} mTimes1, mTimes2;

public:

	DcConn(int type, tSocket sock = 0, Server * server = NULL);
	virtual ~DcConn();

	//< Sending RAW command to the client
	virtual int send(const string & data, bool addSep = false, bool flush = true);

	virtual void disconnect();


	//< Get real port
	virtual int getPort() const;

	//< Get connection port
	virtual int getPortConn() const;

	//< Get numeric IP
	virtual unsigned long getNetIp() const;

	//< Get string of IP
	virtual const string & getIp() const;

	//< Get string of server IP (host)
	virtual const string & getIpConn() const;

	//< Get mac address
	virtual const string & getMacAddress() const;

	virtual long getEnterTime() const;

	//< Client's protocol version
	virtual const string & getVersion() const;

	virtual const string & getSupports() const;


	//< Get profile
	virtual int getProfile() const;

	virtual void setProfile(int iProfile);


	//< Get some user data
	virtual const string & getData() const;

	virtual void setData(const string & sData);



	
	/** Flush sending buffer */
	virtual void OnFlush();

	//< Setting entry status flag
	inline void SetLSFlag(unsigned int s) {
		mLoginStatus |= s;
	}
	
	//< Reset flag
	inline void ReSetLSFlag(unsigned int s) {
		mLoginStatus = s;
	}
	
	//< Get flag
	inline unsigned int GetLSFlag(unsigned int s) {
		return mLoginStatus & s;
	}

	//< Set timeout
	void SetTimeOut(HubTimeOut, double Sec, Time & now);
	
	//< Clear timeout
	void ClearTimeOut(HubTimeOut);
	
	// Check timeout
	int CheckTimeOut(HubTimeOut t, Time & now);

	//< Timer for current connection
	virtual int onTimer(Time & now);

	virtual void CloseNow(int iReason = 0);
	virtual void CloseNice(int msec, int iReason = 0);

	/** Pointer to the server */
	inline DcServer * server() {
		return (DcServer *) mServer;
	}
	
	/** Set user object for current connection */
	bool SetUser(DcUser * User);

protected:

	TimeOut mTimeOut[HUB_TIME_OUT_MAX];

	//< Counter search results
	unsigned miSRCounter;

	//< Timers
	struct Times {

		//< Time sending cmd $Key to the server
		Time mKey;

		//< Time last ping from server to client
		Time mPingServer;

	} mTimes;

private:

	unsigned int mLoginStatus; //< Login status


}; // class DcConn

}; // namespace dcserver

#endif // DC_CONN_H
