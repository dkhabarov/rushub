/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz

 * modified: 10 Dec 2009
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

#ifndef CCONN_H
#define CCONN_H

#include "cobj.h"
#include "ctime.h" /** cTime */
#include "cconnbase.h"
#include "cprotocol.h"
#include "cplugin.h" /** for DC_SEPARATOR */

#include <string>
#include <list>

using namespace std; /** string, cout, endl */
using namespace nUtils; /** cTime */

namespace nServer {

class cServer; /** for mServer */
class cListenFactory;
class cConn; /** for cConnFactory */
class cProtocol; /** *mProtocol */
class cParser; /** *mParser */

/** Connection-factory for create and delete connection */
class cConnFactory {

public:
	cConnFactory(cProtocol *, cServer *);
	virtual ~cConnFactory();
	virtual cConn * CreateConn(tSocket sock = 0);
	virtual void DelConn(cConn * &);

	virtual void OnNewData(cConn *, string *);

public:
	unsigned long miStrSizeMax;
	string msSeparator;
	cProtocol * mProtocol; /** Protocal */

protected:
	cServer * mServer; /** Pointer on server */

}; // cConnFactory

/** Connections types */
enum tConnType {
	eCT_LISTEN,    //< Listen TCP
	eCT_CLIENTTCP, //< Client TCP
	eCT_CLIENTUDP, //< Client UDP
	eCT_SERVERTCP, //< Server TCP (hublist)
	eCT_SERVERUDP  //< Server UDP (multihub)
};

/** Status of string */
enum {
	eSS_NO_STR,   //< No str
	eSS_PARTLY,   //< String is partly received
	eSS_STR_DONE, //< String is completely received
	eSS_ERROR     //< Error
};

/** Enumeration of reasons to closing connection (Close Reason) */
enum {
	eCR_CLIENT_DISCONNECT = 1,
	eCR_ERROR_RECV,
	eCR_ERROR_SEND,
	eCR_GETPEERNAME,
	eCR_MAXSIZE_RECV,
	eCR_MAXSIZE_SEND,
	eCR_MAXSIZE_REMAINING,
	eCR_MAX
};

/** Main connection class for server */
class cConn : public cObj, public cConnBase {

	friend class cServer; /* for ports */

public:

	static unsigned long iConnCounter; /** Conn counter */
	bool mbOk; /** Points that given connection is registered (socket of connection is created and bound) */
	bool mbWritable; /** Points that data can be read and be written */
	cTime mLastRecv; /** Time of the last recv action from the client */

	int miCloseReason; /** Reason of close connection */

	cConnFactory * mConnFactory; /** Conn factory */
	cListenFactory * mListenFactory; /** Listen factory */
	cServer * mServer; /** Server */
	cProtocol * mProtocol; /** Protocol */
	cParser * mParser; /** Parser */

	list<cConn *>::iterator mIterator; /** Optimisation */

public:

	cConn(tSocket socket = 0, cServer * server = NULL, tConnType connType = eCT_CLIENTTCP);
	virtual ~cConn();

	/** Get socket */
	virtual operator tSocket() const { return mSocket; }

	/** Get type connection */
	inline tConnType GetConnType() const { return mConnType; }

	/** Create, bind and listen socket */
	tSocket MakeSocket(int port, const char * ip = NULL, bool udp = false);

	void Close(); /** Close connection (socket) */
	void CloseNice(int msec = 0, int reason = 0); /** Nice close conn (socket) */
	void CloseNow(int reason = 0); /** Now close conn */

	/** Creating the new object for enterring connection */
	virtual cConn * CreateNewConn();

	/** Reading all data from socket to buffer of the conn */
	virtual int Recv();

	/** Check empty recv buf */
	int RecvBufIsEmpty() const { return miRecvBufEnd == miRecvBufRead; }

	/** Check empty recv buf */
	int SendBufIsEmpty() const { return msSendBuf.length() == 0; }

	/** Remaining (for web-server) */
	virtual int Remaining();

	/** Clear params */
	void ClearStr();

	/** Get status */
	int StrStatus() const { return meStrStatus; }

	/** Installing the string, in which will be recorded received data, 
	and installation main parameter */
	void SetStrToRead(string *, string separator, unsigned long max);

	/** Reading data from buffer and record in line of the protocol */
	int ReadFromRecvBuf();

	/** Get pointer for string */
	virtual string * GetPtrForStr();

	/** Create parser */
	virtual cParser * CreateParser();

	/** Remove parser */
	virtual void DeleteParser(cParser *);

	/** Get pointer for string with data */
	string * GetStr(){ return msStr; }

	/** Write data in sending buffer */
	virtual int WriteData(const string & data, bool flush);

	/** OnFlush */
	virtual void OnFlush();

	void Flush(); /** Flush buffer */

	static inline unsigned long Ip2Num(const char * ip) { return inet_addr(ip); }
	static inline char* Num2Ip(unsigned long ip) { struct in_addr in; in.s_addr = ip; return inet_ntoa(in); }
	const string & Ip() const { return msIp; } /** Get string ip */
	int Port() const { return miPort; } /** Get port */

	void GetMac();

	bool Host(); /** Get host */
	static unsigned long IpByHost(const string & host); /** Get ip by host */
	static bool HostByIp(const string & ip, string & host); /** Get host by ip */

	/** Main base timer */
	int OnTimerBase(cTime &now);

	/** Main timer */
	virtual int OnTimer(cTime &now);

	virtual int StrLog(ostream & ostr, int level, int maxLevel, bool isError = false);

	static bool CheckIp(const string &ip);

	bool IsClosed() { return mbClosed; }

private:

	tSocket mSocket; /** Socket descriptor */
	cTime mCloseTime; /** Time before closing the conn */
	int miRecvBufEnd; /** Final position of the buffer msRecvBuf */
	int miRecvBufRead; /** Current position of the buffer msRecvBuf */
	int meStrStatus; /** Status of the line */

	bool mbBlockInput; /** Blocking enterring channel for given conn */
	bool mbBlockOutput; /** Blocking coming channel for given conn */

	string *msStr; /** Pointer to line, in which will be written data from buffer */

	bool mbClosed; /** closed flag, for close counter */

protected:

	/** Socket type */
	tConnType mConnType;

	/** struct sockaddr_in */
	struct sockaddr_in mAddrIN;

	unsigned long miNetIp; /** Numeric ip */
	string msIp; /** String ip */
	string msIpConn; /** String ip (host) of server */
	int miPort; /** port */
	int miPortConn; /** listen-conn port */

	string msMAC; /** mac address */
	string msHost; /** DNS */

	static char * msRecvBuf; /** Recv buffer */
	string msSeparator; /** Separator */
	unsigned long miStrSizeMax; /** (10240) Max msg size */

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
	int Send(const char * buf, size_t & len);

}; // cConn

}; // nServer

#endif // CCONN_H
