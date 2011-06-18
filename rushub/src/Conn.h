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

#ifndef CONN_H
#define CONN_H

#include "Obj.h"
#include "Times.h" // Time
#include "ConnBase.h"
#include "Protocol.h"
#include "Plugin.h" // for NMDC_SEPARATOR

#include <string>
#include <list>

using namespace ::std; // string, cout, endl
using namespace ::utils; // Time

namespace server {

class Server; // for mServer
class ListenFactory;
class Conn; // for ConnFactory
class Protocol; // *mProtocol
class Parser; // *mParser

/** Connection-factory for create and delete connection */
class ConnFactory {

public:

	Protocol * mProtocol; /** Protocal */

public:

	ConnFactory(Protocol *, Server *);
	virtual ~ConnFactory();

	virtual Conn * createConn(tSocket sock = 0);
	virtual void deleteConn(Conn * &);
	virtual void onNewData(Conn *, string *);
	virtual int onNewConn(Conn *);

protected:

	Server * mServer; /** Pointer on server */

}; // class ConnFactory



/** Connections types */
enum ConnType {

	CONN_TYPE_LISTEN,    //< Listen TCP
	CONN_TYPE_CLIENTTCP, //< Client TCP
	CONN_TYPE_CLIENTUDP, //< Client UDP
	CONN_TYPE_SERVERTCP, //< Server TCP
	CONN_TYPE_SERVERUDP  //< Server UDP

};



/** Status of string */
enum StringStatus {

	STRING_STATUS_NO_STR,   //< No str
	STRING_STATUS_PARTLY,   //< String is partly received
	STRING_STATUS_STR_DONE, //< String is completely received
	STRING_STATUS_ERROR     //< Error

};



/** Enumeration of reasons to closing connection (Close Reason) */
enum {

	CLOSE_REASON_CLIENT_DISCONNECT = 1,
	CLOSE_REASON_ERROR_RECV,
	CLOSE_REASON_ERROR_SEND,
	CLOSE_REASON_GETPEERNAME,
	CLOSE_REASON_MAXSIZE_RECV,
	CLOSE_REASON_MAXSIZE_SEND,
	CLOSE_REASON_MAXSIZE_REMAINING,
	CLOSE_REASON_MAX

};



/** Main connection class for server */
class Conn : public Obj, public ConnBase {

	friend class Server; /* for ports and mIterator */

public:

	static unsigned long mConnCounter; /** Conn counter */

	Time mLastRecv; /** Time of the last recv action from the client */

	ConnFactory * mConnFactory; /** Conn factory */
	Server * mServer; /** Server */
	Protocol * mProtocol; /** Protocol */
	Parser * mParser; /** Parser */

public:

	Conn(tSocket socket = 0, Server * server = NULL, ConnType connType = CONN_TYPE_CLIENTTCP);
	virtual ~Conn();

	/** Get socket */
	virtual operator tSocket() const;

	/** Get type connection */
	ConnType getConnType() const {
		return mConnType;
	}

	bool isOk() const {
		return mOk;
	}

	bool isWritable() const {
		return mWritable;
	}

	void setOk(bool);

	/** Create, bind and listen socket */
	tSocket makeSocket(const char * port, const char * ip = NULL, bool udp = false);

	void close(); /** Close connection (socket) */
	void closeNice(int msec = 0, int reason = 0); /** Nice close conn (socket) */
	void closeNow(int reason = 0); /** Now close conn */

	/** Creating the new object for enterring connection */
	virtual Conn * createNewConn();

	/** Reading all data from socket to buffer of the conn */
	virtual int recv();

	/** Check empty recv buf */
	int recvBufIsEmpty() const {
		return mRecvBufEnd == mRecvBufRead;
	}

	/** Check empty recv buf */
	int sendBufIsEmpty() const {
		return mSendBuf.length() == 0;
	}

	/** Get status */
	int strStatus() const {
		return mStrStatus;
	}

	void setCreatedByFactory(bool createdByFactory) {
		mCreatedByFactory = createdByFactory;
	}

	bool getCreatedByFactory() const {
		return mCreatedByFactory;
	}

	/** remaining (for web-server) */
	virtual int remaining();

	/** Clear params */
	void clearCommandPtr();

	/** Installing the string, in which will be recorded received data, 
	and installation main parameter */
	void setCommandPtr(string *);

	/** Reading data from buffer and record in line of the protocol */
	int readFromRecvBuf();

	/** Get pointer for string */
	virtual string * getParserCommandPtr();

	/** Get parser */
	virtual Parser * createParser();

	/** Remove parser */
	virtual void deleteParser(Parser *);

	/** Get pointer for string with data */
	string * getCommandPtr();

	void flush(); /** Flush buffer */


	virtual bool strLog();

	/** Main base timer */
	int onTimerBase(Time &now);

	/** Main timer */
	virtual int onTimer(Time &now);


	static const char * inetNtop(int af, const void * src, char * dst, socklen_t cnt);
	static int inetPton(int af, const char * src, void * dst);

	
	//< Is closed
	bool isClosed() const;

	//< Get string of IP
	const string & getIp() const;

	//< Get string of server IP (host)
	const string & getIpConn() const;

	//< Get IP for UDP
	const string & getIpUdp() const;

	//< Get enter time (in unix time sec)
	long getConnectTime() const;

	//< Get real port
	int getPort() const;

	//< Get connection port
	int getPortConn() const;

	//< Get mac-address
	const string & getMacAddress() const;

	//< Check IP
	static bool checkIp(const string &ip);

protected:

	/** Socket type */
	ConnType mConnType;

	bool mOk; /** Points that given connection is registered (socket of connection is created and bound) */
	bool mWritable; /** Points that data can be read and be written */
	
	string mIp; /** String ip */
	string mIpConn; /** String ip (host) of server */
	string mIpUdp;
	int mPort; /** port */
	int mPortConn; /** listen-conn port */

	string mMac; /** mac address */
	string mHost; /** DNS */

	static char mRecvBuf[MAX_RECV_SIZE + 1]; /** Recv buffer */
	string mSendBuf; /** Buffer for sending */
	unsigned long mSendBufMax; /** Max size sending buf */

	list<Conn *>::iterator mIterator; /** Optimisation */

	//< Time entering into the hub
	Time mConnect;

protected:

	//< Write data in sending buffer
	int writeData(const char * data, size_t len, bool flush);

	//< onFlush
	virtual void onFlush();

	virtual void onOk(bool);

private:

	tSocket mSocket; /** Socket descriptor */
	Time mCloseTime; /** Time before closing the conn */
	int mRecvBufEnd; /** Final position of the buffer mRecvBuf */
	int mRecvBufRead; /** Current position of the buffer mRecvBuf */
	int mStrStatus; /** Status of the line */

	/** struct sockaddr_in */
	struct sockaddr_in mSockAddrIn;
	static socklen_t mSockAddrInSize;

	// ipv6
	struct addrinfo * mAddrInfo;
	//struct sockaddr_storage mSockaddr;

	bool mBlockInput; /** Blocking enterring channel for given conn */
	bool mBlockOutput; /** Blocking coming channel for given conn */

	string * mCommand; /** Pointer to line, in which will be written data from buffer */

	bool mClosed; /** closed flag, for close counter */
	int mCloseReason; /** Reason of close connection */

	//< Created by ConnFactory
	bool mCreatedByFactory;

private:

	//< Create socket (default TCP)
	tSocket socketCreate(const char * port, const char * address, bool udp = false);

	//< Bind
	tSocket socketBind(tSocket);

	//< Listen TCP
	tSocket socketListen(tSocket);

	//< Set non-block socket
	tSocket socketNonBlock(tSocket);

	//< Accept new conn
	tSocket socketAccept(struct sockaddr_storage &);

	int defineConnInfo(struct sockaddr_storage &);

	//< Calculate mac-address
	void calcMacAddress();

	//< Send len byte from buf
	int send(const char * buf, size_t & len);


}; // class Conn

}; // namespace server

#endif // CONN_H

/**
 * $Id$
 * $HeadURL$
 */
