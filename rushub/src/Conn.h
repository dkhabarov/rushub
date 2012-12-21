/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz
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

#ifndef CONN_H
#define CONN_H

#include "ConnBase.h" // first (def winsock2.h)
#include "Obj.h"
#include "Times.h"
#include "Protocol.h"
#include "Mutex.h"

#include <string>
#include <list>

using namespace ::std; // string, cout, endl
using namespace ::utils; // Time, Obj

namespace server {

class Server; // for mServer
class ListenFactory;
class Conn; // for ConnFactory
class Protocol; // *mProtocol
class Parser; // *mParser

/// Connection-factory for create and delete connection
class ConnFactory {

	friend class Conn; // for mProtocol

public:

	ConnFactory(Protocol *, Server *);
	virtual ~ConnFactory();

	virtual Conn * createConn(tSocket sock = 0);
	virtual void deleteConn(Conn * &);
	virtual void onNewData(Conn *, string *);
	virtual int onNewConn(Conn *);

protected:

	Protocol * mProtocol; ///< Protocol
	Server * mServer; ///< Pointer on server

private:

	ConnFactory(const ConnFactory &);
	ConnFactory & operator = (const ConnFactory &);

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

	CLOSE_REASON_OTHER_SIDE = 1,
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

	ConnFactory * mSelfConnFactory; ///< Self Conn Factory
	ConnFactory * mCreatorConnFactory; ///< Conn Factory Creator
	Server * mServer; ///< Server
	Protocol * mProtocol; ///< Protocol
	Parser * mParser; ///< Parser

public:

	Conn(tSocket socket, Server * server, int connType);
	virtual ~Conn();

	virtual operator tSocket() const;

	static bool checkIp(const string & ip);
	static long getCount();
	//static const char * inetNtop(int af, const void * src, char * dst, socklen_t cnt);
	//static int inetPton(int af, const char * src, void * dst);


	/// Get client IP address
	const string & getIp() const;

	/// Get server IP address (host)
	const string & getIpConn() const;

	/// Get IP address for UDP
	const string & getIpUdp() const;

	/// Get client mac-address
	const string & getMacAddress() const;

	/// Get client port
	int getPort() const;

	/// Get server port
	int getPortConn() const;


	/// Is writable
	bool isWritable() const;

	/// Is OK
	bool isOk() const;

	/// Set OK (for Server)
	void setOk(bool);

	/// Get connection type
	int getConnType() const;

	/// Get status (for Server)
	int getStatus() const;

	/// Is closed (for Server)
	bool isClosed() const;


	/// Nice close conn
	void closeNice(int msec = 0, int reason = 0);

	/// Now close conn
	void closeNow(int reason = 0);

	/// Flush buffer
	void flush();

	/// Create, bind and listen socket (for Server)
	tSocket makeSocket(const char * port, const char * ip = NULL, int connType = CONN_TYPE_LISTEN);

	/// Creating the new object for enterring connection (for Server)
	virtual Conn * createNewConn();

	/// Reading all data from socket to buffer of the conn (for Server)
	virtual int recv();

	/// Reading data from buffer and record in line of the protocol (for Server)
	size_t readFromRecvBuf();

	/// Check empty recv buf (for Server)
	bool recvBufIsEmpty() const;

	/// Remaining (for web-server)
	virtual size_t remaining();

	/// Clear params
	void clearCommandPtr();

	/** Installing the string, in which will be recorded received data, 
	and installation main parameter */
	void setCommandPtr(string *);

	/// Get pointer for string (for DcServer and Stress-test client)
	virtual string * getParserCommandPtr();

	/// Get pointer for string with data
	string * getCommandPtr();


	/// Call timer (for Server)
	void onTimerBase(Time & now);

	/// Write data in sending buffer
	size_t writeData(const char * data, size_t len, bool flush);

	/// Reserve space in send buffer
	void reserve(size_t len);

protected:

	string mIp; ///< Client IP address
	string mIpConn; ///< Server IP address (host)
	string mIpUdp; /// UDP IP address
	string mMacAddress; ///< MAC-address

	int mPort; ///< Client port
	int mPortConn; ///< Server port

	Time mLastRecv; ///< Time of the last recv action from the client

	unsigned int & mSendBufMax; ///< Max size sending buf

	list<Conn *>::iterator mIterator; ///< Optimisation

	Mutex mutex;

protected:

	virtual bool strLog();

	virtual void onOk(bool);

	const char * getSeparator() const;

	size_t getSeparatorLen() const;

private:

	bool mOk; ///< Points that given connection is registered (socket of connection is created and bound)
	bool mWritable; ///< Points that data can be read and be written

	int mConnType; ///< Socket type

	tSocket mSocket; ///< Socket descriptor
	Time mCloseTime; ///< Time before closing the conn

	static char mRecvBuf[MAX_RECV_SIZE + 1]; ///< Recv buffer
	size_t mRecvBufEnd; ///< Final position of the buffer mRecvBuf
	size_t mRecvBufRead; ///< Current position of the buffer mRecvBuf
	string mSendBuf; ///< Buffer for sending

	int mStatus; ///< Status of the line

	/// struct sockaddr_in
	struct sockaddr_in mSockAddrIn;
	static socklen_t mSockAddrInSize;

	static volatile long mConnCounter; ///< Conn counter

	/// ipv6
	struct addrinfo * mAddrInfo;

	bool mBlockInput; ///< Blocking enterring channel for given conn
	bool mBlockOutput; ///< Blocking coming channel for given conn

	string * mCommand; ///< Pointer to line, in which will be written data from buffer

	bool mClosed; ///< closed flag, for close counter
	int mCloseReason; ///< Reason of close connection

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

	/// Get parser
	virtual Parser * createParser();

	/// Remove parser
	virtual void deleteParser(Parser *);

	/// onFlush
	virtual void onFlush();

	/// Main timer
	virtual int onTimer(Time & now);

	/// Send len byte from buf
	int send(const char * buf, size_t & len);

	// Define conn info
	int defineConnInfo(struct sockaddr_storage &);

	/// Calculate mac-address
	void calcMacAddress();

	Conn(const Conn &);
	Conn & operator = (const Conn &);

}; // class Conn

} // namespace server

#endif // CONN_H

/**
 * $Id$
 * $HeadURL$
 */
