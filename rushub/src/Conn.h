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

#include "ConnBase.h" // first (def winsock2.h)
#include "Obj.h"
#include "Times.h"
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

/// Connection-factory for create and delete connection
class ConnFactory {

public:

	ConnFactory(Protocol *, Server *);
	virtual ~ConnFactory();

	virtual Conn * createConn(tSocket sock = 0);
	virtual void deleteConn(Conn * &);
	virtual void onNewData(Conn *, string *);
	virtual int onNewConn(Conn *);

protected:

	Server * mServer; ///< Pointer on server
	Protocol * mProtocol; ///< Protocal

}; // class ConnFactory



/// Connections types
enum ConnType {

	CONN_TYPE_LISTEN,       ///< Listen TCP connection
	CONN_TYPE_INCOMING_TCP, ///< Incoming TCP connection
	CONN_TYPE_INCOMING_UDP, ///< Incoming UDP connection
	CONN_TYPE_OUTGOING_TCP, ///< Outgoing TCP connection
	CONN_TYPE_OUTGOING_UDP  ///< Outgoing UDP connection

};



/// Status of string
enum StringStatus {

	STRING_STATUS_NO_STR,   ///< No str
	STRING_STATUS_PARTLY,   ///< String is partly received
	STRING_STATUS_STR_DONE, ///< String is completely received
	STRING_STATUS_ERROR     ///< Error

};



/// Enumeration of reasons to closing connection (Close Reason)
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



/// Main connection class for server
class Conn : public Obj, public ConnBase {

	friend class Server; // for mIterator and ports

public:

	static unsigned long mConnCounter; ///< Conn counter

	Time mLastRecv; ///< Time of the last recv action from the client

	ConnFactory * mSelfConnFactory; ///< Self Conn Factory
	ConnFactory * mCreatorConnFactory; ///< Conn Factory Creator
	Server * mServer; ///< Server
	Protocol * mProtocol; ///< Protocol
	Parser * mParser; ///< Parser

public:

	Conn(tSocket socket, Server * server, int connType);
	virtual ~Conn();

	// === static functions ===

	static const char * inetNtop(int af, const void * src, char * dst, socklen_t cnt);
	static int inetPton(int af, const char * src, void * dst);
	static bool checkIp(const string & ip);


	virtual operator tSocket() const;


	// === getter / setter ===

	/// Get connection type
	int getConnType() const;

	/// Get status
	int getStatus() const;

	/// Is OK
	bool isOk() const;

	/// Set OK
	void setOk(bool);

	/// Is writable
	bool isWritable() const;

	/// Is closed
	bool isClosed() const;

	/// Get string of IP
	const string & getIp() const;

	/// Get string of server IP (host)
	const string & getIpConn() const;

	/// Get IP for UDP
	const string & getIpUdp() const;

	/// Get real port
	int getPort() const;

	/// Get connection port
	int getPortConn() const;

	/// Get mac-address
	const string & getMacAddress() const;

	/// Server first init conn
	bool isServerInit() const;


	/// Create, bind and listen socket
	tSocket makeSocket(const char * port, const char * ip = NULL, int connType = CONN_TYPE_LISTEN);

	/// Close connection (socket)
	void close();

	/// Nice close conn (socket)
	void closeNice(int msec = 0, int reason = 0);

	/// Now close conn
	void closeNow(int reason = 0);

	/// Client close conn
	void closeSelf();

	/// Creating the new object for enterring connection
	virtual Conn * createNewConn();

	/// Reading all data from socket to buffer of the conn
	virtual int recv();

	/// Check empty recv buf
	int recvBufIsEmpty() const {
		return mRecvBufEnd == mRecvBufRead;
	}

	/// Check empty recv buf
	int sendBufIsEmpty() const {
		return mSendBuf.size() == 0;
	}

	/// Remaining (for web-server)
	virtual size_t remaining();

	/// Clear params
	void clearCommandPtr();

	/** Installing the string, in which will be recorded received data, 
	and installation main parameter */
	void setCommandPtr(string *);

	/// Reading data from buffer and record in line of the protocol
	size_t readFromRecvBuf();

	/// Get pointer for string
	virtual string * getParserCommandPtr();

	/// Get parser
	virtual Parser * createParser();

	/// Remove parser
	virtual void deleteParser(Parser *);

	/// Get pointer for string with data
	string * getCommandPtr();

	/// Flush buffer
	void flush();


	virtual bool strLog();

	/// Main base timer
	void onTimerBase(Time &now);

	/// Main timer
	virtual int onTimer(Time &now);

	/// Write data in sending buffer
	size_t writeData(const char * data, size_t len, bool flush);

protected:

	/// Socket type
	int mConnType;

	bool mOk; ///< Points that given connection is registered (socket of connection is created and bound)
	bool mWritable; ///< Points that data can be read and be written
	
	string mIp; ///< String ip
	string mIpConn; ///< String ip (host) of server
	string mIpUdp;
	int mPort; ///< port
	int mPortConn; ///< listen-conn port

	string mMacAddress; ///< mac address
	string mHost; ///< DNS

	static char mRecvBuf[MAX_RECV_SIZE + 1]; ///< Recv buffer
	string mSendBuf; ///< Buffer for sending
	unsigned long mSendBufMax; ///< Max size sending buf

	list<Conn *>::iterator mIterator; ///< Optimisation

	/// Time entering into the hub
	Time mConnect;

protected:

	/// onFlush
	virtual void onFlush();

	virtual void onOk(bool);

	/// Server first initiated the connection
	void setServerInit();

private:

	tSocket mSocket; ///< Socket descriptor
	Time mCloseTime; ///< Time before closing the conn
	size_t mRecvBufEnd; ///< Final position of the buffer mRecvBuf
	size_t mRecvBufRead; ///< Current position of the buffer mRecvBuf
	int mStatus; ///< Status of the line

	/// struct sockaddr_in
	struct sockaddr_in mSockAddrIn;
	static socklen_t mSockAddrInSize;

	/// ipv6
	struct addrinfo * mAddrInfo;

	bool mBlockInput; ///< Blocking enterring channel for given conn
	bool mBlockOutput; ///< Blocking coming channel for given conn

	string * mCommand; ///< Pointer to line, in which will be written data from buffer

	bool mClosed; ///< closed flag, for close counter
	int mCloseReason; ///< Reason of close connection

	bool mServerInit;

private:

	/// Create socket (default TCP)
	tSocket socketCreate(const char * port, const char * address, bool udp = false);

	/// Bind
	tSocket socketBind(tSocket);

	/// Listen TCP
	tSocket socketListen(tSocket);

	/// Connect to TCP socket
	tSocket socketConnect(tSocket);

	/// Set non-block socket
	tSocket socketNonBlock(tSocket);

	/// Accept new conn
	tSocket socketAccept(struct sockaddr_storage &);

	int defineConnInfo(struct sockaddr_storage &);

	/// Calculate mac-address
	static void calcMacAddress(const string & ip, string & mac);

	/// Send len byte from buf
	int send(const char * buf, size_t & len);


}; // class Conn

}; // namespace server

#endif // CONN_H

/**
 * $Id$
 * $HeadURL$
 */
