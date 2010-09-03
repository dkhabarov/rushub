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

#include "cconn.h"
#include "cserver.h" /** for mServer */
#include "stringutils.h" /** for ShrinkStringToFit, StrCutLeft */

#include <iostream> /** cout, endl */

#ifdef _WIN32
  #include <Iphlpapi.h> /** mac address */
  #pragma comment(lib, "Iphlpapi.lib") /** mac address */
#endif

using namespace nUtils; /** ShrinkStringToFit, StrCutLeft */

namespace nServer {

unsigned long cConn::iConnCounter = 0;
char *cConn::msRecvBuf = new char[MAX_RECV_SIZE + 1];

cConn::cConn(tSocket sock, cServer *s, tConnType st) :
  cObj("cConn"),
  mSocket(sock),
  mbOk(sock > 0),
  mbWritable(true),
  mConnFactory(NULL),
  mListenFactory(NULL),
  mServer(s),
  mProtocol(NULL),
  mParser(NULL),
  miRecvBufEnd(0),
  miRecvBufRead(0),
  mbBlockInput(false),
  mbBlockOutput(true),
  miAttemptSend(0),
  mConnType(st),
  miNetIp(0),
  miPort(0),
  miPortConn(0),
  miSendBufMax(MAX_SEND_SIZE)
{
  ClearStr(); /** Clear params */
  memset(&mCloseTime, 0, sizeof(mCloseTime));
  if(mSocket) {
    static struct sockaddr saddr;
    static socklen_t saddr_size = sizeof(saddr);
    struct sockaddr_in *saddr_in;
    if(getpeername(mSocket, &saddr, &saddr_size) < 0) {
      if(Log(2)) LogStream() << "Error in getpeername, closing" << endl;
      CloseNow();
    }
    saddr_in = (struct sockaddr_in *)&saddr;

    miNetIp = saddr_in->sin_addr.s_addr; /** Numeric ip */
    msIp = inet_ntoa(saddr_in->sin_addr); /** String ip */
    miPort = ntohs(saddr_in->sin_port); /** Port */

    GetMac();
  }
}

cConn::~cConn() {
  if(mParser) this->DeleteParser(mParser);
  mParser = NULL;
  mListenFactory = NULL;
  mConnFactory = NULL;
  mServer = NULL;
  mProtocol = NULL;
  this->Close();
}

/** ListenPort */
tSocket cConn::ListenPort(int iPort, const char *sIp, bool bUDP) {
  if(mSocket) return -1; /** Socket is already created */
  mSocket = SocketCreate(bUDP); /** Create socket */
  mSocket = SocketBind(mSocket, iPort, sIp); /** Bind */
  if(!bUDP) {
      mSocket = SocketListen(mSocket);
      mSocket = SocketNonBlock(mSocket);
  }
  miPort = iPort; /** Set port */
  mbOk = mSocket > 0; /** Reg conn */
  return mSocket;
}

/** Create socket (default TCP) */
tSocket cConn::SocketCreate(bool bUDP) {
  tSocket sock;
  if(!bUDP) { /* Create socket TCP */
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
      return INVALID_SOCKET;
    sockoptval_t yes = 1;

    /* TIME_WAIT after close conn. Reuse address after disconn */
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(sockoptval_t)) == INVALID_SOCKET)
      return INVALID_SOCKET;
  }
  else /* Create socket UDP */
    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
      return INVALID_SOCKET;

  ++iConnCounter;
  if(Log(3)) LogStream() << "Created new socket: " << sock << endl;
  return sock;
}


/* Bind
  mAddrIN.sin_family = AF_INET;
  mAddrIN.sin_addr.s_addr = inet_addr(ip);
  mAddrIN.sin_port = htons(port);
  mAddrIN.sin_zero = 0x00000000;
*/
tSocket cConn::SocketBind(tSocket sock, int iPort, const char *sIp) {
  if(sock < 0) return SOCKET_ERROR;
  string sIP(sIp);
  memset(&mAddrIN, 0, sizeof(struct sockaddr_in));
  mAddrIN.sin_family = AF_INET;
  mAddrIN.sin_port = htons(iPort);

  if(!CheckIp(sIP)) {
    struct hostent *host = gethostbyname(sIp);
    if(host)
		  mAddrIN.sin_addr = *((struct in_addr *)host->h_addr);
  } else {
    mAddrIN.sin_addr.s_addr = INADDR_ANY; /* INADDR_ANY == 0 */
    if(sIp)
      #ifdef _WIN32
        mAddrIN.sin_addr.s_addr = inet_addr(sIp);
      #else
        inet_aton(sIp, &mAddrIN.sin_addr);
      #endif
  }
  memset(&(mAddrIN.sin_zero), '\0', 8);

  /** Bind */
  if(bind(sock, (struct sockaddr *)&mAddrIN, sizeof(mAddrIN)) == SOCKET_ERROR)
    return SOCKET_ERROR;

  return sock;
}

/** Listen TCP socket */
tSocket cConn::SocketListen(tSocket sock) {
  if(sock < 0) return SOCKET_ERROR;
  if(listen(sock, SOCK_BACKLOG) == SOCKET_ERROR) {
    SOCK_CLOSE(sock);
    if(ErrLog(1)) LogStream() << "Error listening" << endl;
    return SOCKET_ERROR;
  }
  return sock;
}

/** Set non-block socket */
tSocket cConn::SocketNonBlock(tSocket sock) {
  if(sock < 0) return SOCKET_ERROR;
  SOCK_NON_BLOCK(sock);
  return sock;
}


////////////////////////////////////////////////////////
/** Close socket */
void cConn::Close() {
  if(mSocket <= 0) return;
  mbWritable = mbOk = false;

  /** OnClose */
  if(mServer) mServer->OnClose(this);

#ifndef _WIN32
  TEMP_FAILURE_RETRY(SOCK_CLOSE(mSocket));
  if(SockErr != SOCK_EINTR){
#else
  static int err;
  err = TEMP_FAILURE_RETRY(SOCK_CLOSE(mSocket));
  if(err != SOCKET_ERROR){
#endif
    --iConnCounter;
    if(Log(3)) LogStream() << "Closing socket: " << mSocket << endl;
  }
  else if(ErrLog(1)) LogStream() << "Socket not closed" << endl;

  mSocket = 0;
}

/** OnCloseNice */
int cConn::OnCloseNice(void) {
  return 0;
}

/** CloseNice */
void cConn::CloseNice(int imsec) {
  OnCloseNice();
  mbWritable = false;
  if((imsec <= 0) || (!msSendBuf.size())){CloseNow(); return;}
  mCloseTime.Get();
  mCloseTime += int(imsec);
}

/** CloseNow */
void cConn::CloseNow() {
  mbWritable = mbOk = false;
  if(mServer) {
    mServer->mConnChooser.cConnChoose::OptIn((cConnBase*)this, cConnChoose::eEF_CLOSE);
    mServer->mConnChooser.cConnChoose::OptOut((cConnBase*)this, cConnChoose::eEF_ALL);
    mServer->miNumCloseConn ++;
  }
}

/** CreateNewConn */
cConn * cConn::CreateNewConn() {
  static tSocket sock;
  if((sock = Accept()) == INVALID_SOCKET) return NULL;
  cConnFactory *Factory = NULL;
  cConn *new_conn = NULL;

  /** Presence of the factory for listen socket without fall! */
  if(mListenFactory) Factory = mListenFactory->ConnFactory();
  //if(mServer && mServer->mConnFactory) Factory = mServer->mConnFactory;

  if(Factory != NULL) new_conn = Factory->CreateConn(sock); /** Create eCT_CLIENTTCP conn */
  if(!new_conn) {
    if(ErrLog(0)) LogStream() << "Fatal error: Can't create new connection object" << endl;
    throw "Fatal error: Can't create new connection object";
  }

  mTimeLastIOAction.Get();
  return new_conn;
}

/** Accept new conn */
tSocket cConn::Accept() {
  #ifdef _WIN32
    static struct sockaddr client;
  #else
    static struct sockaddr_in client;
  #endif
  static socklen_t namelen = sizeof(client);
  static tSocket sock;
  static int i;

  i = 0;
  sock = INVALID_SOCKET;
  memset(&client, 0, namelen);
  sock = accept(mSocket, (struct sockaddr *)&client, (socklen_t*)&namelen);
  while((sock == INVALID_SOCKET) && ((SockErr == SOCK_EAGAIN) || (SockErr == SOCK_EINTR)) && (i++ < 10))
  { /** Try to accept connection not more than 10 once */
    sock = ::accept(mSocket, (struct sockaddr *)&client, (socklen_t*)&namelen);
    #ifdef _WIN32
      Sleep(1);
    #else
      usleep(50);
    #endif
  }
  if(sock == INVALID_SOCKET) {
    if(ErrLog(1)) LogStream() << "Socket not accept: " << SockErr << "  " << sizeof(fd_set) << endl;
    return INVALID_SOCKET;
  }

  static sockoptval_t yes = 1;
  static socklen_t yeslen = sizeof(yes);
  if(setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &yes, yeslen) == SOCKET_ERROR) {
#ifdef _WIN32
    static int err;
    err = TEMP_FAILURE_RETRY(SOCK_CLOSE(sock));
    if(err != SOCKET_ERROR)
#else
    TEMP_FAILURE_RETRY(SOCK_CLOSE(sock));
    if(SockErr != SOCK_EINTR)
#endif
    { if(Log(3)) LogStream() << "Closing socket: " << sock << endl; }
    else if(ErrLog(1)) LogStream() << "Socket not closed" << endl;
    return INVALID_SOCKET;
  }

  /** Non-block socket */
  if((sock = SocketNonBlock(sock)) == INVALID_SOCKET) return INVALID_SOCKET;

  /** Accept new socket */
  if(Log(3)) LogStream() << "Accept new socket: " << sock << endl;
  ++iConnCounter;
  return sock;
}

/** Reading all data from socket to buffer of the conn */
int cConn::Recv() {
  if(!mbOk || !mbWritable) return -1;

  static int iBufLen, i;
  static bool bUdp;
  
  iBufLen = 0; i = 0;
  bUdp = (this->mConnType == eCT_CLIENTUDP);
  if(!bUdp) { /** TCP */
    while(
      ((iBufLen = recv(mSocket, msRecvBuf, MAX_RECV_SIZE, 0)) == SOCKET_ERROR) &&
      ((SockErr == SOCK_EAGAIN) || (SockErr == SOCK_EINTR))
      && (i++ <= 100)
    )  {
      #ifndef _WIN32
        usleep(5);
      #endif
    }
  } else { /** bUdp */
    if(Log(4)) LogStream() << "Start read (UDP)" << endl;
    static int iAddrLen = sizeof(struct sockaddr);
    while(
      ((iBufLen = recvfrom(mSocket, msRecvBuf, MAX_RECV_SIZE, 0, (struct sockaddr *)&mAddrIN, (socklen_t *)&iAddrLen)) == SOCKET_ERROR) &&
      (i++ <= 100)
    ) {
      #ifndef _WIN32
        usleep(5);
      #endif
    }
    if(Log(4)) LogStream() << "End read (UDP). Read bytes: " << iBufLen << endl;
  }

  miRecvBufRead = miRecvBufEnd = 0;
  if(iBufLen <= 0) {
    if(!bUdp) {
      if(iBufLen == 0) {
        if(Log(3)) LogStream() << "User itself was disconnected" << endl;
      } else {
        if(Log(2)) {
          LogStream() << "Error in receive: " << SockErr;
          switch(SockErr) {
            case ECONNRESET: /** Connection reset by peer */
              LogStream() << "(connection reset by peer)" << endl;
              break;
            case ETIMEDOUT: /** Connection timed out */
              LogStream() << "(connection timed out)" << endl;
              break;
            case EHOSTUNREACH: /** No route to host */
              LogStream() << "(no route to host)" << endl;
              break;
            case EWOULDBLOCK: /** Non-blocking socket operation */
              return -1;
            default:
              LogStream() << "(other reason)" << endl;
              break;
          }
        }
      }
      CloseNow();
      return -1;
    }
  } else {
    miRecvBufEnd = iBufLen; /** End buf pos */
    miRecvBufRead = 0; /** Pos for reading from buf */
    msRecvBuf[miRecvBufEnd] = '\0'; /** Adding 0 symbol to end of str */
    mTimeLastIOAction.Get(); /** Write time last action */
  }
  return iBufLen;
}

/** Clear params */
void cConn::ClearStr() {
  msStr = NULL;
  msSeparator = "\0";
  miStrSizeMax = 0;
  meStrStatus = eSS_NO_STR;
}

/** Installing the string, in which will be recorded received data, 
  and installation main parameter */
void cConn::SetStrToRead(string *pStr, string separator, unsigned long iMax) {
  if(meStrStatus != eSS_NO_STR) {
    if(ErrLog(0)) LogStream() << "Fatal error: Bad SetStrToRead" << endl;
    throw "Fatal error: Bad SetStrToRead";
  }
  if(!pStr) {
    if(ErrLog(0)) LogStream() << "Fatal error: Bad SetStrToRead. Null string pointer" << endl;
    throw "Fatal error: Bad SetStrToRead. Null string pointer";
  }
  msStr = pStr;
  msSeparator = separator;
  miStrSizeMax = iMax;
  meStrStatus = eSS_PARTLY;
}

/** Reading data from buffer and record in line of the protocol */
int cConn::ReadFromRecvBuf() {
  if(!msStr) {
    if(ErrLog(0)) LogStream() << "Fatal error: ReadFromBuf with null string pointer" << endl;
    throw "Fatal error: ReadFromBuf with null string pointer";
  }
  char *buf = msRecvBuf + miRecvBufRead;
  unsigned pos, len = (miRecvBufEnd - miRecvBufRead);
  if((pos = string(buf).find(msSeparator)) == string::npos) {
    if(msStr->size() + len > miStrSizeMax) {
      CloseNow();
      return 0;
    }
    msStr->append((char*)buf, len);
    miRecvBufRead = miRecvBufEnd = 0;
    return len;
  }
  len = pos + msSeparator.size();
  msStr->append((char*)buf, pos);
  miRecvBufRead += len;
  meStrStatus = eSS_STR_DONE;
  return len;
}

/** Remaining (for web-server) */
int cConn::Remaining() {
  char *buf = msRecvBuf + miRecvBufRead;
  int len = miRecvBufEnd - miRecvBufRead;  
  if(msStr->size() + len > miStrSizeMax) {
    CloseNow();
    return -1;
  }
  msStr->append((char*)buf, len);
  miRecvBufRead = miRecvBufEnd = 0;
  return len;
}

/** Write data in sending buffer and send to conn */
int cConn::WriteData(const string &sData, bool bFlush) {
  if(msSendBuf.size() + sData.size() >= miSendBufMax) {
    if(Log(2)) LogStream() << "Sending buffer has big size, closing" << endl;
    CloseNow();
    return -1;
  }
  bFlush = bFlush || (msSendBuf.size() > (miSendBufMax >> 1));

  if(!bFlush) { 
    msSendBuf.append(sData.c_str(), sData.size());
    return 0;
  }

  const char *send_buf; 
  size_t size; 
  static bool appended; 

  if(msSendBuf.size()) { 
    msSendBuf.append(sData.c_str(), sData.size());
    size = msSendBuf.size();
    send_buf = msSendBuf.c_str();
    appended = true;
  } else { 
    size = sData.size();
    if(!size) return 0; 
    send_buf = sData.c_str();
    appended = false;
  }

  /** Sending */
  if(Send(send_buf, size) == SOCKET_ERROR) {

    if((SockErr != SOCK_EAGAIN) /*&& (SockErr != SOCK_EINTR)*/) {
      if(Log(2)) LogStream() << "Error in sending: " << SockErr << "(not EAGAIN), closing" << endl;
      CloseNow();
      return -1;
    } //else if(miAttemptSend++ > MAX_ATTEMPT_SEND) {
      //if(ErrLog(1)) LogStream() << "Error in sending: Attempt send more than " << MAX_ATTEMPT_SEND << ", closing" << endl;
      //CloseNow();
      //return -1;
    //}

    if(Log(3)) LogStream() << "Block sent: " << size << endl;
    if(!appended)
      StrCutLeft(sData, msSendBuf, size);
    else
      StrCutLeft(msSendBuf, size); /** del from buf sent data */
    mTimeLastIOAction.Get();

    if(bool(mCloseTime)) {
      CloseNow();
      return size;
    }

    if(mServer && mbOk) {
      if(mbBlockOutput) {
        mbBlockOutput = false;
        mServer->mConnChooser.cConnChoose::OptIn(this, cConnChoose::eEF_OUTPUT);
        if(Log(3)) LogStream() << "Unblock output channel" << endl;
      }

      if(mbBlockInput && msSendBuf.size() < MAX_SEND_UNBLOCK_SIZE) { /** Снятие блокировки с входящего канала */
        mServer->mConnChooser.cConnChoose::OptIn(this, cConnChoose::eEF_INPUT);
        if(Log(3)) LogStream() << "Unblock input channel" << endl;
      } else if(!mbBlockInput && msSendBuf.size() >= MAX_SEND_BLOCK_SIZE) { /** Установка блокировки на входящий канал */
        mServer->mConnChooser.cConnChoose::OptOut(this, cConnChoose::eEF_INPUT);
        if(Log(3)) LogStream() << "Block input channel" << endl;
      }
    }
  } else {
    if(appended) msSendBuf.erase(0, msSendBuf.size());
    ShrinkStringToFit(msSendBuf);

    if(bool(mCloseTime)) {
      CloseNow();
      return size;
    }

    if(mServer && mbOk && !mbBlockOutput) {
      mbBlockOutput = true;
      mServer->mConnChooser.cConnChoose::OptOut(this, cConnChoose::eEF_OUTPUT);
      if(Log(3)) LogStream() << "Block output channel" << endl;
    }
    miAttemptSend = 0;
    mTimeLastIOAction.Get();

    OnFlush();
  }
  return size;
}

/** OnFlush */
void cConn::OnFlush() {
}

/** Flush */
void cConn::Flush() {
  static string empty("");
  if(msSendBuf.length()) WriteData(empty, true);
}

/** Send len byte from buf */
int cConn::Send(const char *buf, size_t &len) {
#ifdef QUICK_SEND /** Quick send */
  if(this->mConnType != eCT_SERVERUDP)
    len = send(mSocket, buf, len, 0);
  else
    len = sendto(mSocket, buf, len, 0, (struct sockaddr *)&mAddrIN, sizeof(struct sockaddr));
  return len == SOCKET_ERROR ? SOCKET_ERROR : 0; /* return SOCKET_ERROR - fail, 0 - ok */
#else
  static size_t total, bytesleft;
  static int n;
  static bool bUDP;
  total = 0; bytesleft = len; n = 0;
  bUDP = (this->mConnType == eCT_SERVERUDP);
  while(total < len) { // EMSGSIZE (WSAEMSGSIZE)
    try {
      if(!bUDP) {
        n = send(mSocket, buf + total, bytesleft, 
  #ifndef _WIN32
        MSG_NOSIGNAL|MSG_DONTWAIT);
  #else
        0);
  #endif
      } else {
        static int tolen = sizeof(struct sockaddr);
        n = sendto(mSocket, buf + total, bytesleft, 0, (struct sockaddr *)&mAddrIN, tolen);
      }
    } catch(...) {
      if(ErrLog(1))
        LogStream() << "exception in Send(buf," << len
          << ") total=" << total
          << " left=" << bytesleft
          << " n=" << n << endl;
      return SOCKET_ERROR;
    }
    if(Log(5))
        LogStream() << "len = " << len
          << " total=" << total
          << " left=" << bytesleft
          << " n=" << n << endl;
    if(n == SOCKET_ERROR) break;
    total += n;
    bytesleft -= n;
  }
  len = total; /* Number sending bytes */
  return n == SOCKET_ERROR ? SOCKET_ERROR : 0; /* return -1 - fail, 0 - ok */
#endif
}

/** Get pointer for string */
string * cConn::GetPtrForStr() {
  if(mParser == NULL) mParser = this->CreateParser();
  if(mParser == NULL) return NULL;
  mParser->ReInit();
  return &(mParser->GetStr());
}

cParser * cConn::CreateParser() {
  if(this->mProtocol != NULL)
    return this->mProtocol->CreateParser();
  else return NULL;
}

void cConn::DeleteParser(cParser *OldParser) {
  if(this->mProtocol != NULL)
    this->mProtocol->DeleteParser(OldParser);
  else delete OldParser;
}

/** Main base timer */
int cConn::OnTimerBase(cTime &now) {
  if(bool(mCloseTime) && mCloseTime > now) {
    CloseNow();
    return 0;
  }
  Flush();
  OnTimer(now);
  return 0;
}

/** Main timer */
int cConn::OnTimer(cTime &) {
  return 0;
}

int cConn::StrLog(ostream & ostr, int iLevel, int iMaxLevel) {
  if(cObj::StrLog(ostr, iLevel, iMaxLevel)) {
    LogStream() << "(sock " << this->mSocket << ") ";
    return 1;
  }
  return 0;
}

bool cConn::CheckIp(const string &ip) {
  int i;
	char c;
	istringstream is(ip);
  is >> i >> c; if(i < 0 || i > 255 || c != '.') return false;
  is >> i >> c; if(i < 0 || i > 255 || c != '.') return false;
  is >> i >> c; if(i < 0 || i > 255 || c != '.') return false;
  is >> i >> (c = '\0'); if(i < 0 || i > 255 || c) return false;
  return true;
}

void cConn::GetMac() {
#ifdef _WIN32
  static char buf[17];
  unsigned long ip = Ip2Num(msIp.c_str()), iSize = 0xFFFF;
  MIB_IPNETTABLE * pT = (MIB_IPNETTABLE *) new char[0xFFFF];
  if(0L == ::GetIpNetTable(pT, &iSize, 1)) {
    for(unsigned long i = 0; i < pT->dwNumEntries; ++i)
      if((pT->table[i].dwAddr == ip) && (pT->table[i].dwType != 2)) {
        sprintf(buf, "%02x-%02x-%02x-%02x-%02x-%02x",
          pT->table[i].bPhysAddr[0], pT->table[i].bPhysAddr[1],
          pT->table[i].bPhysAddr[2], pT->table[i].bPhysAddr[3],
          pT->table[i].bPhysAddr[4], pT->table[i].bPhysAddr[5]);
        msMAC = string(buf);
        break;
      }
    delete[] pT;
  }
#endif
}

bool cConn::Host() {
	if(msHost.size()) return true;
  struct hostent *h;
  if((h = gethostbyaddr(msIp.c_str(), sizeof(msIp), AF_INET)) != NULL)
		msHost = h->h_name;
	return h != NULL;
}

unsigned long cConn::IpByHost(const string &sHost) {
	struct sockaddr_in saddr;
	memset(&saddr, 0, sizeof(sockaddr_in));
	struct hostent *h;
  if((h = gethostbyname(sHost.c_str())) != NULL) saddr.sin_addr = *((struct in_addr *)h->h_addr);
	return saddr.sin_addr.s_addr;
}

bool cConn::HostByIp(const string &sIp, string &sHost) {
	struct hostent *h;
	struct in_addr addr;
#ifndef _WIN32
  if(!inet_aton(sIp.c_str(), &addr)) return false;
#else
  addr.s_addr = inet_addr(sIp.c_str());
#endif
  if((h = gethostbyaddr((char *)&addr, sizeof(addr), AF_INET)) != NULL)
		sHost = h->h_name;
	return h != NULL;
}


/////////////////////cConnFactory/////////////////////
cConnFactory::cConnFactory(cProtocol *protocol, cServer *s) : 
  miStrSizeMax(s->miStrSizeMax),
  msSeparator(s->msSeparator),
  mProtocol(protocol),
  mServer(s)
{
}

cConnFactory::~cConnFactory() {
}

cConn * cConnFactory::CreateConn(tSocket sock) {
  cConn *conn = new cConn(sock, mServer); /** eCT_CLIENTTCP */
  conn->mConnFactory = this;
  conn->mProtocol = mProtocol; /** proto */
  return conn;
}

void cConnFactory::DelConn(cConn * &conn) {
  conn->Close(); /** close socket */
  delete conn;
  conn = NULL;
}

void cConnFactory::OnNewData(cConn * conn, string * str) {
  mServer->OnNewData(conn, str);
}

}; // nServer
