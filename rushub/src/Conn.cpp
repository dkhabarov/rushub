/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz

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

#include "Conn.h"
#include "Server.h" // for mServer
#include "stringutils.h" // for ShrinkStringToFit, StrCutLeft

#include <iostream> // cout, endl

#ifdef _WIN32
	#include <Iphlpapi.h> // mac address
	#pragma comment(lib, "Iphlpapi.lib") // mac address
	#if defined(_MSC_VER) && (_MSC_VER >= 1400)
		#pragma warning(disable:4996) // Disable "This function or variable may be unsafe."
	#endif
#endif

using namespace ::utils; // ShrinkStringToFit, StrCutLeft

namespace server {

unsigned long Conn::iConnCounter = 0;
char Conn::msRecvBuf[MAX_RECV_SIZE + 1];

Conn::Conn(tSocket socket, Server * server, ConnType connType) :
	Obj("Conn"),
	mbOk(socket > 0),
	mbWritable(true),
	miCloseReason(0),
	mConnFactory(NULL),
	mListenFactory(NULL),
	mServer(server),
	mProtocol(NULL),
	mParser(NULL),
	mSocket(socket),
	miRecvBufEnd(0),
	miRecvBufRead(0),
	mbBlockInput(false),
	mbBlockOutput(true),
	mbClosed(false),
	mConnType(connType),
	miNetIp(0),
	miPort(0),
	miPortConn(0),
	miSendBufMax(MAX_SEND_SIZE)
{
	ClearStr(); /** Clear params */
	memset(&mCloseTime, 0, sizeof(mCloseTime));

	if (mSocket) {
		struct sockaddr_in saddr;
		socklen_t saddr_size = sizeof(saddr);
		if (getpeername(mSocket, (struct sockaddr *)&saddr, &saddr_size) < 0) {
			if (Log(2)) {
				LogStream() << "Error in getpeername, closing" << endl;
			}
			CloseNow(CLOSE_REASON_GETPEERNAME);
		}

		miNetIp = saddr.sin_addr.s_addr; /** Numeric ip */
		msIp = inet_ntoa(saddr.sin_addr); /** String ip */
		miPort = ntohs(saddr.sin_port); /** Port */

		if (mServer->mbMAC) {
			GetMac();
		}
	}
}

Conn::~Conn() {
	if (mParser) {
		this->DeleteParser(mParser);
	}
	mParser = NULL;
	mListenFactory = NULL;
	mConnFactory = NULL;
	mServer = NULL;
	mProtocol = NULL;
	close();
}



/** Get socket */
Conn::operator tSocket() const {
	return mSocket;
}

	

/** MakeSocket */
tSocket Conn::MakeSocket(int port, const char * ip, bool udp) {
	if (mSocket > 0) {
		return INVALID_SOCKET; /** Socket is already created */
	}
	mSocket = SocketCreate(udp); /** Create socket */
	mSocket = SocketBind(mSocket, port, ip); /** Bind */
	if (!udp) {
		mSocket = SocketListen(mSocket);
		mSocket = SocketNonBlock(mSocket);
	}
	miPort = port; /** Set port */
	msIp = ip; /** Set ip (host) */
	mbOk = mSocket > 0; /** Reg conn */
	return mSocket;
}

/** Create socket (default TCP) */
tSocket Conn::SocketCreate(bool bUDP) {
	tSocket sock;
	if (!bUDP) { /* Create socket TCP */
		if (SOCK_INVALID(sock = socket(AF_INET, SOCK_STREAM, 0))) {
			return INVALID_SOCKET;
		}
		sockoptval_t yes = 1;

		/* TIME_WAIT after close conn. Reuse address after disconn */
		if (SOCK_ERROR(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(sockoptval_t)))) {
			return INVALID_SOCKET;
		}
	} else {/* Create socket UDP */
		if (SOCK_INVALID(sock = socket(AF_INET, SOCK_DGRAM, 0))) {
			return INVALID_SOCKET;
		}
	}

	++iConnCounter;
	if (Log(3)) {
		LogStream() << "Created new socket: " << sock << endl;
	}
	return sock;
}


/* Bind
	mAddrIN.sin_family = AF_INET;
	mAddrIN.sin_addr.s_addr = inet_addr(ip);
	mAddrIN.sin_port = htons(port);
	mAddrIN.sin_zero = 0x00000000;
*/
tSocket Conn::SocketBind(tSocket sock, int iPort, const char *sIp) {
	if (sock == INVALID_SOCKET) {
		return INVALID_SOCKET;
	}
	string sIP(sIp);
	memset(&mAddrIN, 0, sizeof(struct sockaddr_in));
	mAddrIN.sin_family = AF_INET;
	mAddrIN.sin_port = htons((u_short)iPort);

	if (!CheckIp(sIP)) {
		struct hostent * host = gethostbyname(sIp);
		if (host) {
			mAddrIN.sin_addr = *((struct in_addr *)host->h_addr);
			host = NULL;
		}
	} else {
		mAddrIN.sin_addr.s_addr = INADDR_ANY; /* INADDR_ANY == 0 */
		if (sIp) {
			#ifdef _WIN32
				mAddrIN.sin_addr.s_addr = inet_addr(sIp);
			#else
				inet_aton(sIp, &mAddrIN.sin_addr);
			#endif
		}
	}
	memset(&(mAddrIN.sin_zero), '\0', 8);

	/** Bind */
	if (SOCK_ERROR(bind(sock, (struct sockaddr *)&mAddrIN, sizeof(mAddrIN)))) {
		if (ErrLog(0)) {
			LogStream() << "Error bind: " << SockErrMsg << endl;
		}
		return INVALID_SOCKET;
	}

	return sock;
}

/** Listen TCP socket */
tSocket Conn::SocketListen(tSocket sock) {
	if (sock == INVALID_SOCKET) {
		return INVALID_SOCKET;
	}
	if (SOCK_ERROR(listen(sock, SOCK_BACKLOG))) {
		SOCK_CLOSE(sock);
		if (ErrLog(1)) {
			LogStream() << "Error listening" << endl;
		}
		return INVALID_SOCKET;
	}
	return sock;
}

/** Set non-block socket */
tSocket Conn::SocketNonBlock(tSocket sock) {
	if (sock == INVALID_SOCKET) {
		return INVALID_SOCKET;
	}
	SOCK_NON_BLOCK(sock);
	return sock;
}


////////////////////////////////////////////////////////
/** Close socket */
void Conn::close() {
	if (mSocket <= 0) {
		return;
	}
	mbWritable = mbOk = false;

	/** OnClose */
	if (mServer) {
		mServer->OnClose(this);
	}

#ifndef _WIN32
	TEMP_FAILURE_RETRY(SOCK_CLOSE(mSocket));
	if (SockErr != SOCK_EINTR || (mServer && !mServer->mbRun)) { // Interrupted system call on exit
#else
	int err = TEMP_FAILURE_RETRY(SOCK_CLOSE(mSocket));
	if (!(SOCK_ERROR(err))) {
#endif
		--iConnCounter;
		if (Log(3)) {
			LogStream() << "Closing socket: " << mSocket << endl;
		}
	} else if (ErrLog(1)) {
		LogStream() << "Socket not closed: " << SockErr << endl;
	}

	mSocket = 0;
}

/** CloseNice */
void Conn::CloseNice(int imsec /* = 0 */, int iReason /* = 0 */) {
	mbWritable = false;
	if (iReason) {
		miCloseReason = iReason;
	}
	if ((imsec <= 0) || (!msSendBuf.size())) {
		CloseNow(iReason);
		return;
	}
	mCloseTime.Get();
	mCloseTime += int(imsec);
}

/** CloseNow */
void Conn::CloseNow(int iReason /* = 0 */) {
	mbWritable = mbOk = false;
	if (mServer) {
		if (!(mServer->mConnChooser.ConnChoose::OptGet((ConnBase*)this) & ConnChoose::eEF_CLOSE)) {
			++ mServer->miNumCloseConn;
			mbClosed = true; // poll conflict

			if (iReason) {
				miCloseReason = iReason;
			}
			if (Log(3)) {
				LogStream() << "CloseNow (reason " << miCloseReason << ")" << endl;
			}

#if USE_SELECT
			mServer->mConnChooser.ConnChoose::OptIn((ConnBase*)this, ConnChoose::eEF_CLOSE);
			mServer->mConnChooser.ConnChoose::OptOut((ConnBase*)this, ConnChoose::eEF_ALL);
#else
			// this sequence of flags for poll!
			mServer->mConnChooser.ConnChoose::OptOut((ConnBase*)this, ConnChoose::eEF_ALL);
			mServer->mConnChooser.ConnChoose::OptIn((ConnBase*)this, ConnChoose::eEF_CLOSE);
#endif
			
		} else {
			if (Log(3)) {
				LogStream() << "Re-closure (reason " << iReason << ")" << endl;
			}
		}
	} else {
		if (ErrLog(0)) {
			LogStream() << "Close conn without Server" << endl;
		}
	}
}

/** CreateNewConn */
Conn * Conn::CreateNewConn() {
	tSocket sock;
	if ((sock = Accept()) == INVALID_SOCKET) {
		return NULL;
	}

	ConnFactory *Factory = NULL;
	Conn *new_conn = NULL;

	/** Presence of the factory for listen socket without fall! */
	if (mListenFactory) {
		Factory = mListenFactory->connFactory();
	}
	/*if (mServer && mServer->mConnFactory) {
		Factory = mServer->mConnFactory;
	}*/

	if (Factory != NULL) {
		new_conn = Factory->CreateConn(sock); /** Create CONN_TYPE_CLIENTTCP conn */
	}
	if (!new_conn) {
		if (ErrLog(0)) {
			LogStream() << "Fatal error: Can't create new connection object" << endl;
		}
		throw "Fatal error: Can't create new connection object";
	}

	return new_conn;
}

/** Accept new conn */
tSocket Conn::Accept() {
	#ifdef _WIN32
		static struct sockaddr client;
	#else
		static struct sockaddr_in client;
	#endif
	static socklen_t namelen = sizeof(client);
	tSocket sock;
	int i = 0;
	memset(&client, 0, namelen);
	sock = accept(mSocket, (struct sockaddr *)&client, (socklen_t*)&namelen);
	while (SOCK_INVALID(sock) && ((SockErr == SOCK_EAGAIN) || (SockErr == SOCK_EINTR)) && (++i <= 10)) {
		/** Try to accept connection not more than 10 once */
		sock = ::accept(mSocket, (struct sockaddr *)&client, (socklen_t*)&namelen);
		#ifdef _WIN32
			Sleep(1);
		#else
			usleep(50);
		#endif
	}
	if (SOCK_INVALID(sock)) {
		if (ErrLog(1)) {
			LogStream() << "Socket not accept: " << SockErr << endl;
		}
		return INVALID_SOCKET;
	}

	static sockoptval_t yes = 1;
	static socklen_t yeslen = sizeof(yes);
	if (SOCK_ERROR(setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &yes, yeslen))) {
#ifdef _WIN32
		int err = TEMP_FAILURE_RETRY(SOCK_CLOSE(sock));
		if (SOCK_ERROR(err))
#else
		TEMP_FAILURE_RETRY(SOCK_CLOSE(sock));
		if (SockErr != SOCK_EINTR)
#endif
		{
			if (Log(2)) {
				LogStream() << "Couldn't set keepalive flag for accepted socket" << endl;
			}
		} else if (ErrLog(1)) {
			LogStream() << "Socket not closed" << endl;
		}
		return INVALID_SOCKET;
	}

	/** Non-block socket */
	if (SocketNonBlock(sock) == INVALID_SOCKET) {
		if (ErrLog(1)) {
			LogStream() << "Couldn't set non-block flag for accepted socket" << endl;
		}
		return INVALID_SOCKET;
	}

	/** Accept new socket */
	if (Log(3)) {
		LogStream() << "Accept new socket: " << sock << endl;
	}

	++iConnCounter;
	return sock;
}

/** Reading all data from socket to buffer of the conn */
int Conn::Recv() {
	if (!mbOk || !mbWritable) {
		return -1;
	}

	int iBufLen = 0, i = 0;

#ifdef USE_UDP
	bool bUdp = (this->mConnType == CONN_TYPE_CLIENTUDP);
	if (!bUdp) { /** TCP */
#endif
		while (
			(SOCK_ERROR(iBufLen = recv(mSocket, msRecvBuf, MAX_RECV_SIZE, 0))) &&
			((SockErr == SOCK_EAGAIN) || (SockErr == SOCK_EINTR))
			&& (++i <= 100)
		) {
			#ifndef _WIN32
				usleep(5);
			#endif
		}
#ifdef USE_UDP
	} else { /** bUdp */
		if (Log(4)) {
			LogStream() << "Start read (UDP)" << endl;
		}
		static int iAddrLen = sizeof(struct sockaddr);
		while (
			(SOCK_ERROR(iBufLen = recvfrom(mSocket, msRecvBuf, MAX_RECV_SIZE, 0, (struct sockaddr *)&mAddrIN, (socklen_t *)&iAddrLen))) &&
			(++i <= 100)
		) {
			#ifndef _WIN32
				usleep(5);
			#endif
		}
		if (Log(4)) {
			LogStream() << "End read (UDP). Read bytes: " << iBufLen << endl;
		}
	}
#endif

	miRecvBufRead = miRecvBufEnd = 0;
	if (iBufLen <= 0) {
		#ifdef USE_UDP
		if (!bUdp) {
		#endif
			if (iBufLen == 0) {
				if (Log(3)) {
					LogStream() << "User itself was disconnected" << endl;
				}
				CloseNow(CLOSE_REASON_CLIENT_DISCONNECT);
			} else {
				if (Log(2)) {
					LogStream() << "Error in receive: " << SockErr;
					switch (SockErr) {
						case ECONNRESET : /** Connection reset by peer */
							LogStream() << "(connection reset by peer)" << endl;
							break;
						case ETIMEDOUT : /** Connection timed out */
							LogStream() << "(connection timed out)" << endl;
							break;
						case EHOSTUNREACH : /** No route to host */
							LogStream() << "(no route to host)" << endl;
							break;
						case EWOULDBLOCK : /** Non-blocking socket operation */
							return -1;
						default :
							LogStream() << "(other reason)" << endl;
							break;
					}
				}
				CloseNow(CLOSE_REASON_ERROR_RECV);
			}
			return -1;
		#ifdef USE_UDP
		}
		#endif
	} else {
		miRecvBufEnd = iBufLen; /** End buf pos */
		miRecvBufRead = 0; /** Pos for reading from buf */
		msRecvBuf[miRecvBufEnd] = '\0'; /** Adding 0 symbol to end of str */
		mLastRecv.Get(); /** Write time last recv action */
	}
	return iBufLen;
}



/** Clear params */
void Conn::ClearStr() {
	mCommand = NULL;
	msSeparator = "\0";
	mStrSizeMax = 0;
	meStrStatus = STRING_STATUS_NO_STR;
}



/** Get pointer for string with data */
string * Conn::getCommand() {
	return mCommand;
}



/** Get string ip */
const string & Conn::Ip() const {
	return msIp;
}



/** Installing the string, in which will be recorded received data, 
	and installation main parameter */
void Conn::SetStrToRead(string * pStr, string separator, unsigned long iMax) {
	if (meStrStatus != STRING_STATUS_NO_STR) {
		if (ErrLog(0)) {
			LogStream() << "Fatal error: Bad SetStrToRead" << endl;
		}
		throw "Fatal error: Bad SetStrToRead";
	}
	if (!pStr) {
		if (ErrLog(0)) {
			LogStream() << "Fatal error: Bad SetStrToRead. Null string pointer" << endl;
		}
		throw "Fatal error: Bad SetStrToRead. Null string pointer";
	}
	mCommand = pStr;
	msSeparator = separator;
	mStrSizeMax = iMax;
	meStrStatus = STRING_STATUS_PARTLY;
}

/** Reading data from buffer and record in line of the protocol */
int Conn::ReadFromRecvBuf() {
	if (!mCommand) {
		if (ErrLog(0)) {
			LogStream() << "Fatal error: ReadFromBuf with null string pointer" << endl;
		}
		throw "Fatal error: ReadFromBuf with null string pointer";
	}
	char * buf = msRecvBuf + miRecvBufRead;
	size_t pos, len = (miRecvBufEnd - miRecvBufRead);
	if ((pos = string(buf).find(msSeparator)) == string::npos) {
		if (mCommand->size() + len > mStrSizeMax) {
			CloseNow(CLOSE_REASON_MAXSIZE_RECV);
			return 0;
		}
		mCommand->append((char *)buf, len);
		miRecvBufRead = miRecvBufEnd = 0;
		return len;
	}
	len = pos + msSeparator.size();
	mCommand->append((char *)buf, pos);
	miRecvBufRead += len;
	meStrStatus = STRING_STATUS_STR_DONE;
	return len;
}

/** Remaining (for web-server) */
int Conn::Remaining() {
	char * buf = msRecvBuf + miRecvBufRead;
	int len = miRecvBufEnd - miRecvBufRead;
	if (mCommand->size() + len > mStrSizeMax) {
		CloseNow(CLOSE_REASON_MAXSIZE_REMAINING);
		return -1;
	}
	mCommand->append((char *)buf, len);
	miRecvBufRead = miRecvBufEnd = 0;
	return len;
}

/** Write data in sending buffer and send to conn */
int Conn::WriteData(const string & sData, bool bFlush) {
	size_t bufLen = msSendBuf.size();
	if (bufLen + sData.size() >= miSendBufMax) {
		if (Log(0)) {
			LogStream() << "Sending buffer has big size, closing" << endl;
		}
		CloseNow(CLOSE_REASON_MAXSIZE_SEND);
		return -1;
	}
	bFlush = bFlush || (bufLen > (miSendBufMax >> 1));

	if (!bFlush) { 
		msSendBuf.append(sData.c_str(), sData.size());
		return 0;
	}

	const char * send_buf = NULL; 
	size_t size; 
	bool appended = false;

	if (bufLen) {
		msSendBuf.append(sData.c_str(), sData.size());
		size = msSendBuf.size();
		send_buf = msSendBuf.c_str();
		appended = true;
	} else { 
		size = sData.size();
		if (!size) {
			return 0;
		}
		send_buf = sData.c_str();
	}

	/** Sending */
	if (send(send_buf, size) < 0) {

		if ((SockErr != SOCK_EAGAIN) /*&& (SockErr != SOCK_EINTR)*/) {
			if (Log(2)) {
				LogStream() << "Error in sending: " << SockErr << "(not EAGAIN), closing" << endl;
			}
			CloseNow(CLOSE_REASON_ERROR_SEND);
			return -1;
		}

		if (Log(3)) {
			LogStream() << "Block sent. Was sent " << size << " bytes" << endl;
		}
		if (!appended) {
			StrCutLeft(sData, msSendBuf, size);
		} else {
			StrCutLeft(msSendBuf, size); /** del from buf sent data */
		}

		if (bool(mCloseTime)) {
			CloseNow();
			return size;
		}

		if (mServer && mbOk) {
			if (mbBlockOutput) {
				mbBlockOutput = false;
				mServer->mConnChooser.ConnChoose::OptIn(this, ConnChoose::eEF_OUTPUT);
				if (Log(3)) {
					LogStream() << "Unblock output channel" << endl;
				}
			}

			bufLen = msSendBuf.size();
			if (mbBlockInput && bufLen < MAX_SEND_UNBLOCK_SIZE) { /** Unset block of input */
				mServer->mConnChooser.ConnChoose::OptIn(this, ConnChoose::eEF_INPUT);
				if (Log(3)) {
					LogStream() << "Unblock input channel" << endl;
				}
			} else if (!mbBlockInput && bufLen >= MAX_SEND_BLOCK_SIZE) { /** Set block of input */
				mServer->mConnChooser.ConnChoose::OptOut(this, ConnChoose::eEF_INPUT);
				if (Log(3)) {
					LogStream() << "Block input channel" << endl;
				}
			}
		}
	} else {
		if (appended) {
			msSendBuf.erase(0, msSendBuf.size());
		}
		ShrinkStringToFit(msSendBuf);

		if (bool(mCloseTime)) {
			CloseNow();
			return size;
		}

		if (mServer && mbOk && !mbBlockOutput) {
			mbBlockOutput = true;
			mServer->mConnChooser.ConnChoose::OptOut(this, ConnChoose::eEF_OUTPUT);
			if (Log(3)) {
				LogStream() << "Block output channel" << endl;
			}
		}
		OnFlush();
	}
	return size;
}

/** OnFlush */
void Conn::OnFlush() {
}

/** Flush */
void Conn::Flush() {
	static string empty("");
	if (msSendBuf.length()) {
		WriteData(empty, true);
	}
}

/** Send len byte from buf */
int Conn::send(const char *buf, size_t &len) {
#ifdef QUICK_SEND /** Quick send */
	if (this->mConnType != CONN_TYPE_SERVERUDP) {
		len = ::send(mSocket, buf, len, 0);
	} else {
		len = ::sendto(mSocket, buf, len, 0, (struct sockaddr *)&mAddrIN, sizeof(struct sockaddr));
	}
	return SOCK_ERROR(len) ? -1 : 0; /* return -1 - fail, 0 - ok */
#else
	int n = -1;
	size_t total = 0, bytesleft = len;
	#ifdef USE_UDP
	bool bUDP = (this->mConnType == CONN_TYPE_SERVERUDP);
	#endif
	while (total < len) { // EMSGSIZE (WSAEMSGSIZE)
		#ifdef USE_UDP
		if (!bUDP) {
		#endif
			//int count = 0;
			//do {
				n = ::send(mSocket, buf + total, bytesleft, 
				#ifndef _WIN32
					MSG_NOSIGNAL | MSG_DONTWAIT);
				#else
					0);
				#endif
			/*	if (SockErr == SOCK_EAGAIN) {
					#ifdef _WIN32
						Sleep(5);
					#else
						usleep(5000);
					#endif
				} else {
					break;
				}
			} while (++count < 5);
			*/
		#ifdef USE_UDP
		} else {
			static int tolen = sizeof(struct sockaddr);
			n = ::sendto(mSocket, buf + total, bytesleft, 0, (struct sockaddr *)&mAddrIN, tolen);
		}
		#endif
/*		if (Log(5)) {
				LogStream() << "len = " << len
					<< " total=" << total
					<< " left=" << bytesleft
					<< " n=" << n << endl;
			}
*/
		if (SOCK_ERROR(n)) {
			break;
		}
		total += n;
		bytesleft -= n;
	}
	len = total; /* Number sending bytes */
	return SOCK_ERROR(n) ? -1 : 0; /* return -1 - fail, 0 - ok */
#endif
}

/** Get pointer for string */
string * Conn::GetPtrForStr() {
	if (mParser == NULL) {
		mParser = this->CreateParser();
	}
	if (mParser == NULL) {
		return NULL;
	}
	mParser->ReInit();
	return &(mParser->getCommand());
}

Parser * Conn::CreateParser() {
	return this->mProtocol != NULL ? this->mProtocol->CreateParser(): NULL;
}

void Conn::DeleteParser(Parser * OldParser) {
	if (this->mProtocol != NULL) {
		this->mProtocol->DeleteParser(OldParser);
	} else {
		delete OldParser;
	}
}

/** Main base timer */
int Conn::OnTimerBase(Time &now) {
	if (bool(mCloseTime) && mCloseTime > now) {
		CloseNow();
		return 0;
	}
	Flush();
	onTimer(now);
	return 0;
}

/** Main timer */
int Conn::onTimer(Time &) {
	return 0;
}

bool Conn::StrLog() {
	Obj::StrLog();
	LogStream() << "(sock " << mSocket << ") ";
	return true;
}

bool Conn::CheckIp(const string &ip) {
	int i;
	char c;
	istringstream is(ip);
	is >> i >> c;
	if (i < 0 || i > 255 || c != '.') {
		return false;
	}
	is >> i >> c;
	if (i < 0 || i > 255 || c != '.') {
		return false;
	}
	is >> i >> c;
	if (i < 0 || i > 255 || c != '.') {
		return false;
	}
	is >> i >> (c = '\0');
	if (i < 0 || i > 255 || c) {
		return false;
	}
	return true;
}

void Conn::GetMac() {
#ifdef _WIN32
	char buf[17] = { '\0' };
	unsigned long ip = Ip2Num(msIp.c_str()), iSize = 0xFFFF;
	MIB_IPNETTABLE * pT = (MIB_IPNETTABLE *) new char[0xFFFF];
	if (0L == ::GetIpNetTable(pT, &iSize, 1)) {
		for (unsigned long i = 0; i < pT->dwNumEntries; ++i) {
			if ((pT->table[i].dwAddr == ip) && (pT->table[i].dwType != 2)) {
				sprintf(buf, "%02x-%02x-%02x-%02x-%02x-%02x",
					pT->table[i].bPhysAddr[0], pT->table[i].bPhysAddr[1],
					pT->table[i].bPhysAddr[2], pT->table[i].bPhysAddr[3],
					pT->table[i].bPhysAddr[4], pT->table[i].bPhysAddr[5]);
				msMAC = string(buf);
				break;
			}
		}
		delete[] pT;
	}
#endif
}

bool Conn::Host() {
	if (msHost.size()) {
		return true;
	}
	struct hostent * h;
	if ((h = gethostbyaddr(msIp.c_str(), sizeof(msIp), AF_INET)) != NULL) {
		msHost = h->h_name;
	}
	return h != NULL;
}

unsigned long Conn::IpByHost(const string &sHost) {
	struct sockaddr_in saddr;
	memset(&saddr, 0, sizeof(sockaddr_in));
	struct hostent * h;
	if ((h = gethostbyname(sHost.c_str())) != NULL) {
		saddr.sin_addr = *((struct in_addr *)h->h_addr);
	}
	return saddr.sin_addr.s_addr;
}

bool Conn::HostByIp(const string & sIp, string &sHost) {
	struct hostent * h;
	struct in_addr addr;
#ifndef _WIN32
	if (!inet_aton(sIp.c_str(), &addr)) {
		return false;
	}
#else
	addr.s_addr = inet_addr(sIp.c_str());
#endif
	if ((h = gethostbyaddr((char *)&addr, sizeof(addr), AF_INET)) != NULL) {
		sHost = h->h_name;
	}
	return h != NULL;
}


/////////////////////ConnFactory/////////////////////
ConnFactory::ConnFactory(Protocol * protocol, Server * s) : 
	mStrSizeMax(s->mStrSizeMax),
	msSeparator(s->msSeparator),
	mProtocol(protocol),
	mServer(s)
{
}

ConnFactory::~ConnFactory() {
}

Conn * ConnFactory::CreateConn(tSocket sock) {
	Conn *conn = new Conn(sock, mServer); /** CONN_TYPE_CLIENTTCP */
	conn->mConnFactory = this;
	conn->mProtocol = mProtocol; /** proto */
	return conn;
}

void ConnFactory::DelConn(Conn * &conn) {
	conn->close(); /** close socket */
	delete conn;
	conn = NULL;
}

void ConnFactory::OnNewData(Conn * conn, string * str) {
	mServer->OnNewData(conn, str);
}

}; // server
