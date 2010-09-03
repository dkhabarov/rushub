/*
 * RusHub - hub server for Direct Connect peer to peer network.

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
#include "cdcparserbase.h" /** for DC_SEPARATOR */

#include <string>
#include <list>

using namespace std; /** string, cout, endl */
using namespace nUtils; /** cTime */

namespace nServer
{

class cServer; /** for mServer */
class cListenFactory;
class cConn; /** for cConnFactory */
class cProtocol; /** *mProtocol */
class cParser; /** *mParser */

/** Connection-factory for create and delete connection */
class cConnFactory
{

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

};

/** Типы соединений */
enum tConnType
{
  eCT_LISTEN,    //< Listen TCP
  eCT_CLIENTTCP, //< Client TCP
  eCT_CLIENTUDP, //< Client UDP
  eCT_SERVERTCP, //< Server TCP (hublist)
  eCT_SERVERUDP  //< Server UDP (multihub)
};

/** Статусы строки */
enum
{
  eSS_NO_STR,   //< No str
  eSS_PARTLY,   //< String is partly received
  eSS_STR_DONE, //< String is completely received
  eSS_ERROR     //< Error
};

/** Main connection class for server */
class cConn : public cObj, public cConnBase
{
  friend class cServer; /* for ports */
public:
  
  tSocket mSocket; /** Socket descriptor */
  static unsigned long iConnCounter; /** Conn counter */
  bool mbOk; /** Points that given connection is registered (socket of connection is created and bound) */
  bool mbWritable; /** Points that data can be read and be written */
  cTime mTimeLastIOAction; /** Time of the last action of the client */

  cConnFactory * mConnFactory; /** Conn factory */
  cListenFactory * mListenFactory; /** Listen factory */
  cServer * mServer; /** Server */
  cProtocol * mProtocol; /** Protocol */
  cParser * mParser; /** Parser */

  cTime mTimePing; /** Last ping time from client side */

public:

  cConn(tSocket sock = 0, cServer *s = NULL, tConnType st = eCT_CLIENTTCP);
  virtual ~cConn();

  /** Get socket */
  virtual operator tSocket() const { return mSocket; }

  /** Get type connection */
  inline tConnType GetConnType() const { return mConnType; }

  /** Create, bind and listen connection (socket) */
  tSocket ListenPort(int iPort, const char *sIp = NULL, bool bUDP = false);

  void Close(); /** Close connection (socket) */
  void CloseNice(int msec=0); /** Nice close conn (socket) */
  void CloseNow(); /** Now close conn */

  /** Creating the new object for enterring connection */
  virtual cConn * CreateNewConn();

  /** Reading all data from socket to buffer of the conn */
  int Recv();

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
  void SetStrToRead(string *, string separator, unsigned long iMax);

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
  virtual int WriteData(const string &sData, bool bFlush);

  /** OnFlush */
  virtual void OnFlush();

  void Flush(); /** Flush buffer */

  static inline unsigned long Ip2Num(const char* sIP) { return inet_addr(sIP); }
  static inline char* Num2Ip(unsigned long iIP) { struct in_addr in; in.s_addr = iIP; return inet_ntoa(in); }
  const string & Ip() const { return msIp; } /** Get string ip */
  int Port() const { return miPort; } /** Get port */

  void GetMac();

  bool Host(); /** Get host */
  static unsigned long IpByHost(const string &sHost); /** Get ip by host */
  static bool HostByIp(const string &sIp, string &sHost); /** Get host by ip */

  /** Main base timer */
  int OnTimerBase(cTime &now);

  /** Main timer */
  virtual int OnTimer(cTime &now);

  virtual int StrLog(ostream & ostr, int iLevel, int iMaxLevel);

  static bool CheckIp(const string &ip);

private:

  cTime mCloseTime; /** Time before closing the conn */
  int miRecvBufEnd; /** Final position of the buffer msRecvBuf */
  int miRecvBufRead; /** Current position of the buffer msRecvBuf */
  int meStrStatus; /** Status of the line */

  bool mbBlockInput; /** Blocking enterring channel for given conn */
  bool mbBlockOutput; /** Blocking coming channel for given conn */

  string *msStr; /** Pointer to line, in which will be written data from buffer */

  typedef list<cConn *> tConnList;
  typedef tConnList::iterator tCLIt;

  unsigned miAttemptSend;

public:
  tCLIt mIterator; /** Optimisation */

protected:

  /** Socket type */
  tConnType mConnType;

  /** struct sockaddr_in */
  struct sockaddr_in mAddrIN;

  unsigned long miNetIp; /** Numeric ip */
  string msIp; /** String ip */
  int miPort; /** port */
  int miPortConn; /** listen-conn port */

  string msMAC; /** mac address */
  string msHost; /** DNS */

  static char * msRecvBuf; /** Recv buffer */
  string msSeparator; /** Separator */
  unsigned long miStrSizeMax; /** (10240) max size of msg */

  string msSendBuf; /** Buffer for sending */
  unsigned long miSendBufMax; /** Max size sending buf */

protected:
  /** Create socket (default TCP) */
  tSocket SocketCreate(bool bUDP = false);

  /** Bind */
  tSocket SocketBind(tSocket, int iPort, const char *sIp = NULL);

  /** Listen TCP */
  tSocket SocketListen(tSocket);

  /** Set non-block socket */
  tSocket SocketNonBlock(tSocket);

  /** OnCloseNice event */
  virtual int OnCloseNice(void);

  /** Accept new conn */
  tSocket Accept();

  /** Send len byte from buf */
  int Send(const char *buf, size_t &len);

}; // cConn

}; // nServer

#endif // CCONN_H
