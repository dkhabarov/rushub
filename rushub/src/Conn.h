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

	ConnFactory(Protocol *, Server *);
	virtual ~ConnFactory();
	virtual Conn * CreateConn(tSocket sock = 0);
	virtual void DelConn(Conn * &);

	virtual void OnNewData(Conn *, string *);

public:

	unsigned long mStrSizeMax;
	string msSeparator;
	Protocol * mProtocol; /** Protocal */

protected:

	Server * mServer; /** Pointer on server */

}; // class ConnFactory



/** Connections types */
enum ConnType {

	CONN_TYPE_LISTEN,    //< Listen TCP
	CONN_TYPE_CLIENTTCP, //< Client TCP
	CONN_TYPE_CLIENTUDP, //< Client UDP
	CONN_TYPE_SERVERTCP, //< Server TCP (hublist)
	CONN_TYPE_SERVERUDP  //< Server UDP (multihub)

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

	friend class Server; /* for ports */

public:

	static unsigned long iConnCounter; /** Conn counter */
	bool mbOk; /** Points that given connection is registered (socket of connection is created and bound) */
	bool mbWritable; /** Points that data can be read and be written */
	Time mLastRecv; /** Time of the last recv action from the client */

	int miCloseReason; /** Reason of close connection */

	ConnFactory * mConnFactory; /** Conn factory */
	ListenFactory * mListenFactory; /** Listen factory */
	Server * mServer; /** Server */
	Protocol * mProtocol; /** Protocol */
	Parser * mParser; /** Parser */

	list<Conn *>::iterator mIterator; /** Optimisation */

public:

	Conn(tSocket socket = 0, Server * server = NULL, ConnType connType = CONN_TYPE_CLIENTTCP);
	virtual ~Conn();

	/** Get socket */
	virtual operator tSocket() const;

	/** Get type connection */
	inline ConnType GetConnType() const {
		return mConnType;
	}

	/** Create, bind and listen socket */
	tSocket MakeSocket(int port, const char * ip = NULL, bool udp = false);

	void close(); /** Close connection (socket) */
	void CloseNice(int msec = 0, int reason = 0); /** Nice close conn (socket) */
	void CloseNow(int reason = 0); /** Now close conn */

	/** Creating the new object for enterring connection */
	virtual Conn * CreateNewConn();

	/** Reading all data from socket to buffer of the conn */
	virtual int Recv();

	/** Check empty recv buf */
	int RecvBufIsEmpty() const;

	/** Check empty recv buf */
	int SendBufIsEmpty() const;

	/** Remaining (for web-server) */
	virtual int Remaining();

	/** Clear params */
	void ClearStr();

	/** Get status */
	int StrStatus() const;

	/** Installing the string, in which will be recorded received data, 
	and installation main parameter */
	void SetStrToRead(string *, string separator, unsigned long max);

	/** Reading data from buffer and record in line of the protocol */
	int ReadFromRecvBuf();

	/** Get pointer for string */
	virtual string * GetPtrForStr();

	/** Create parser */
	virtual Parser * CreateParser();

	/** Remove parser */
	virtual void DeleteParser(Parser *);

	/** Get pointer for string with data */
	string * getCommand();

	/** Write data in sending buffer */
	virtual int WriteData(const string & data, bool flush);

	/** OnFlush */
	virtual void OnFlush();

	void Flush(); /** Flush buffer */

	static inline unsigned long Ip2Num(const char * ip) {
		return inet_addr(ip);
	}
	
	static inline char* Num2Ip(unsigned long ip) {
		struct in_addr in;
		in.s_addr = ip;
		return inet_ntoa(in);
	}
	
	/** Get string ip */
	const string & Ip() const;
	
	/** Get port */
	int Port() const;

	void GetMac();

	/** Get host */
	bool Host();
	
	/** Get ip by host */
	static unsigned long IpByHost(const string & host);
	
	/** Get host by ip */
	static bool HostByIp(const string & ip, string & host);

	/** Main base timer */
	int OnTimerBase(Time &now);

	/** Main timer */
	virtual int onTimer(Time &now);

	virtual bool StrLog();

	static bool CheckIp(const string &ip);

	inline bool IsClosed() {
		return mbClosed;
	}

private:

	tSocket mSocket; /** Socket descriptor */
	Time mCloseTime; /** Time before closing the conn */
	int miRecvBufEnd; /** Final position of the buffer msRecvBuf */
	int miRecvBufRead; /** Current position of the buffer msRecvBuf */
	int meStrStatus; /** Status of the line */

	bool mbBlockInput; /** Blocking enterring channel for given conn */
	bool mbBlockOutput; /** Blocking coming channel for given conn */

	string *mCommand; /** Pointer to line, in which will be written data from buffer */

	bool mbClosed; /** closed flag, for close counter */

protected:

	/** Socket type */
	ConnType mConnType;

	/** struct sockaddr_in */
	struct sockaddr_in mAddrIN;

	unsigned long miNetIp; /** Numeric ip */
	string msIp; /** String ip */
	string msIpConn; /** String ip (host) of server */
	int miPort; /** port */
	int miPortConn; /** listen-conn port */

	string msMAC; /** mac address */
	string msHost; /** DNS */

	static char msRecvBuf[MAX_RECV_SIZE + 1]; /** Recv buffer */
	string msSeparator; /** Separator */
	unsigned long mStrSizeMax; /** (10240) Max msg size */

	string msSendBuf; /** Buffer for sending */
	unsigned long miSendBufMax; /** Max size sending buf */

protected:

	/** Create socket (default TCP) */
	tSocket SocketCreate(bool udp = false);

	/** Bind */
	tSocket SocketBind(tSocket, int port, const char * ip = NULL);

	/** Listen TCP */
	tSocket SocketListen(tSocket);

	/** Set non-block socket */
	tSocket SocketNonBlock(tSocket);

	/** Accept new conn */
	tSocket Accept();

	/** Send len byte from buf */
	int send(const char * buf, size_t & len);

}; // class Conn

}; // namespace server

#endif // CONN_H
