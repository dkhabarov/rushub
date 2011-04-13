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

unsigned long Conn::mConnCounter = 0;
socklen_t Conn::mSockAddrInSize = sizeof(struct sockaddr_in);

char Conn::mRecvBuf[MAX_RECV_SIZE + 1];

Conn::Conn(tSocket socket, Server * server, ConnType connType) :
	Obj("Conn"),
	mConnFactory(NULL),
	mServer(server),
	mProtocol(NULL),
	mParser(NULL),
	mConnType(connType),
	mOk(socket > 0),
	mWritable(true),
	mNetIp(0),
	mPort(0),
	mPortConn(0),
	mSendBufMax(MAX_SEND_SIZE),
	mSocket(socket),
	mRecvBufEnd(0),
	mRecvBufRead(0),
	mBlockInput(false),
	mBlockOutput(true),
	mClosed(false),
	mCloseReason(0),
	mCreatedByFactory(false)
{
	clearStr(); /** Clear params */
	memset(&mCloseTime, 0, sizeof(mCloseTime));

	if (mSocket) {
		struct sockaddr_in saddr;
		if (getpeername(mSocket, (struct sockaddr *)&saddr, &mSockAddrInSize) < 0) {
			if (Log(2)) {
				LogStream() << "Error in getpeername, closing" << endl;
			}
			closeNow(CLOSE_REASON_GETPEERNAME);
		}

		#ifdef _WIN32
			mIp = inet_ntoa(saddr.sin_addr);
		#else
			char ip[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &(saddr.sin_addr), ip, INET_ADDRSTRLEN);
			mIp = ip; /** String ip */
		#endif
		mNetIp = saddr.sin_addr.s_addr; /** Numeric ip */
		mPort = ntohs(saddr.sin_port); /** port */

		if (mServer->mMac) {
			getMac();
		}
	}
}

Conn::~Conn() {
	if (mParser) {
		deleteParser(mParser);
		mParser = NULL;
	}
	mConnFactory = NULL;
	mServer = NULL;
	mProtocol = NULL;
	close();
}



/** Get socket */
Conn::operator tSocket() const {
	return mSocket;
}



void Conn::setOk(bool ok) {
	mOk = ok;
	onOk(ok);
}



void Conn::onOk(bool) {
}

	

/** makeSocket */
tSocket Conn::makeSocket(int port, const char * ip, bool udp) {
	if (mSocket > 0) {
		return INVALID_SOCKET; /** Socket is already created */
	}
	mSocket = socketCreate(udp); /** Create socket */
	mSocket = socketBind(mSocket, port, ip); /** Bind */
	if (!udp) {
		mSocket = socketListen(mSocket);
		mSocket = socketNonBlock(mSocket);
	}
	mPort = port; /** Set port */
	mIp = ip; /** Set ip (host) */
	setOk(mSocket > 0); /** Reg conn */
	return mSocket;
}

/** Create socket (default TCP) */
tSocket Conn::socketCreate(bool bUDP) {
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

	++mConnCounter;
	if (Log(3)) {
		LogStream() << "Created new socket: " << sock << endl;
	}
	return sock;
}


/** Bind */
tSocket Conn::socketBind(tSocket sock, int iPort, const char *sIp) {
	if (sock == INVALID_SOCKET) {
		return INVALID_SOCKET;
	}
	string sIP(sIp);
	memset(&mSockAddrIn, 0, sizeof(struct sockaddr_in));
	mSockAddrIn.sin_family = AF_INET;
	mSockAddrIn.sin_port = htons((u_short)iPort);

	if (!checkIp(sIP)) {
		struct hostent * host = gethostbyname(sIp);
		if (host) {
			mSockAddrIn.sin_addr = *((struct in_addr *)host->h_addr);
			host = NULL;
		}
	} else {
		mSockAddrIn.sin_addr.s_addr = INADDR_ANY; /* INADDR_ANY == 0 */
		if (sIp) {
			#ifdef _WIN32
				mSockAddrIn.sin_addr.s_addr = inet_addr(sIp);
			#else
				inet_aton(sIp, &mSockAddrIn.sin_addr);
			#endif
		}
	}
	memset(&(mSockAddrIn.sin_zero), '\0', 8);

	/** Bind */
	if (SOCK_ERROR(bind(sock, (struct sockaddr *)&mSockAddrIn, mSockAddrInSize))) {
		if (ErrLog(0)) {
			LogStream() << "Error bind: " << SockErrMsg << endl;
		}
		return INVALID_SOCKET;
	}

	return sock;
}

/** Listen TCP socket */
tSocket Conn::socketListen(tSocket sock) {
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
tSocket Conn::socketNonBlock(tSocket sock) {
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
	mWritable = false;
	setOk(false);

	/** onClose */
	if (mServer) {
		mServer->onClose(this);
	}

#ifndef _WIN32
	TEMP_FAILURE_RETRY(SOCK_CLOSE(mSocket));
	if (SockErr != SOCK_EINTR || (mServer && !mServer->mRun)) { // Interrupted system call on exit
#else
	int err = TEMP_FAILURE_RETRY(SOCK_CLOSE(mSocket));
	if (!(SOCK_ERROR(err))) {
#endif
		--mConnCounter;
		if (Log(3)) {
			LogStream() << "Closing socket: " << mSocket << endl;
		}
	} else if (ErrLog(1)) {
		LogStream() << "Socket not closed: " << SockErr << endl;
	}

	mSocket = 0;
}

/** closeNice */
void Conn::closeNice(int imsec /* = 0 */, int iReason /* = 0 */) {
	mWritable = false;
	if (iReason) {
		mCloseReason = iReason;
	}
	if ((imsec <= 0) || (!mSendBuf.size())) {
		closeNow(iReason);
		return;
	}
	mCloseTime.Get();
	mCloseTime += int(imsec);
}

/** closeNow */
void Conn::closeNow(int iReason /* = 0 */) {
	mWritable = false;
	setOk(false);
	if (mServer) {
		if (!(mServer->mConnChooser.ConnChoose::OptGet(static_cast<ConnBase *> (this)) & ConnChoose::eEF_CLOSE)) {
			++ mServer->miNumCloseConn;
			mClosed = true; // poll conflict

			if (iReason) {
				mCloseReason = iReason;
			}
			if (Log(3)) {
				LogStream() << "closeNow (reason " << mCloseReason << ")" << endl;
			}

#if USE_SELECT
			mServer->mConnChooser.ConnChoose::OptIn(static_cast<ConnBase *> (this), ConnChoose::eEF_CLOSE);
			mServer->mConnChooser.ConnChoose::OptOut(static_cast<ConnBase *> (this), ConnChoose::eEF_ALL);
#else
			// this sequence of flags for poll!
			mServer->mConnChooser.ConnChoose::OptOut(static_cast<ConnBase *> (this), ConnChoose::eEF_ALL);
			mServer->mConnChooser.ConnChoose::OptIn(static_cast<ConnBase *> (this), ConnChoose::eEF_CLOSE);
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

/** createNewConn */
Conn * Conn::createNewConn() {
	tSocket sock = socketAccept();
	if (sock == INVALID_SOCKET) {
		return NULL;
	}

	Conn * new_conn = NULL;

	if (mConnFactory != NULL) {
		new_conn = mConnFactory->createConn(sock); // Create connection object by factory (CONN_TYPE_CLIENTTCP)
	} else {
		if (Log(3)) {
			LogStream() << "Create simple connection object for socket: " << sock << endl;
		}
		new_conn = new Conn(sock, mServer); // Create simple connection object (CONN_TYPE_CLIENTTCP)
		new_conn->mProtocol = mProtocol;
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
tSocket Conn::socketAccept() {
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
	if (socketNonBlock(sock) == INVALID_SOCKET) {
		if (ErrLog(1)) {
			LogStream() << "Couldn't set non-block flag for accepted socket" << endl;
		}
		return INVALID_SOCKET;
	}

	/** Accept new socket */
	if (Log(3)) {
		LogStream() << "Accept new socket: " << sock << endl;
	}

	++mConnCounter;
	return sock;
}

/** Reading all data from socket to buffer of the conn */
int Conn::recv() {
	if (!mOk || !mWritable) {
		return -1;
	}

	int iBufLen = 0, i = 0;


	bool bUdp = (this->mConnType == CONN_TYPE_CLIENTUDP);
	if (!bUdp) { /** TCP */

		while (
			(SOCK_ERROR(iBufLen = ::recv(mSocket, mRecvBuf, MAX_RECV_SIZE, 0))) &&
			((SockErr == SOCK_EAGAIN) || (SockErr == SOCK_EINTR))
			&& (++i <= 100)
		) {
			#ifndef _WIN32
				usleep(5);
			#endif
		}

	} else { /** bUdp */
		if (Log(4)) {
			LogStream() << "Start read (UDP)" << endl;
		}
		while (
			(SOCK_ERROR(iBufLen = recvfrom(
				mSocket,
				mRecvBuf,
				MAX_RECV_SIZE,
				0,
				(struct sockaddr *) &mSockAddrInUdp,
				(socklen_t *) &mSockAddrInSize
			))) && (++i <= 100)
		) {
			#ifndef _WIN32
				usleep(5);
			#endif
		}
		if (Log(4)) {
			LogStream() << "End read (UDP). Read bytes: " << iBufLen << endl;
		}
	}


	mRecvBufRead = mRecvBufEnd = 0;
	if (iBufLen <= 0) {

		if (!bUdp) {

			if (iBufLen == 0) {
				if (Log(3)) {
					LogStream() << "User itself was disconnected" << endl;
				}
				closeNow(CLOSE_REASON_CLIENT_DISCONNECT);
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
				closeNow(CLOSE_REASON_ERROR_RECV);
			}
			return -1;

		}

	} else {
		if (bUdp) {
			mIpUdp = inet_ntoa(mSockAddrInUdp.sin_addr);
		}
		mRecvBufEnd = iBufLen; /** End buf pos */
		mRecvBufRead = 0; /** Pos for reading from buf */
		mRecvBuf[mRecvBufEnd] = '\0'; /** Adding 0 symbol to end of str */
		mLastRecv.Get(); /** Write time last recv action */
	}
	return iBufLen;
}



/** Clear params */
void Conn::clearStr() {
	mCommand = NULL;
	mStrStatus = STRING_STATUS_NO_STR;
}



/** Get pointer for string with data */
string * Conn::getCommand() {
	return &mParser->mCommand;
}



/** Get string ip */
const string & Conn::ip() const {
	return mIp;
}



const string & Conn::ipUdp() const {
	return mIpUdp;
}



/** Installing the string, in which will be recorded received data, 
	and installation main parameter */
void Conn::setStrToRead(string * pStr) {
	if (mStrStatus != STRING_STATUS_NO_STR) {
		if (ErrLog(0)) {
			LogStream() << "Fatal error: Bad setStrToRead" << endl;
		}
		throw "Fatal error: Bad setStrToRead";
	}
	if (!pStr) {
		if (ErrLog(0)) {
			LogStream() << "Fatal error: Bad setStrToRead. Null string pointer" << endl;
		}
		throw "Fatal error: Bad setStrToRead. Null string pointer";
	}
	mCommand = pStr;
	mStrStatus = STRING_STATUS_PARTLY;
}

/** Reading data from buffer and record in line of the protocol */
int Conn::readFromRecvBuf() {
	if (!mCommand) {
		if (ErrLog(0)) {
			LogStream() << "Fatal error: ReadFromBuf with null string pointer" << endl;
		}
		throw "Fatal error: ReadFromBuf with null string pointer";
	}

	char * buf = mRecvBuf + mRecvBufRead;
	size_t pos, len = (mRecvBufEnd - mRecvBufRead);

	string separator("\0");
	unsigned long maxCommandLength = 10240;
	
	if (mProtocol != NULL) {
		separator = mProtocol->getSeparator();
		maxCommandLength = mProtocol->getMaxCommandLength();
	}

	if ((pos = string(buf).find(separator)) == string::npos) {
		if (mCommand->size() + len > maxCommandLength) {
			closeNow(CLOSE_REASON_MAXSIZE_RECV);
			return 0;
		}
		mCommand->append(buf, len);
		mRecvBufRead = mRecvBufEnd = 0;
		return len;
	}
	len = pos + separator.length();
	mCommand->append(buf, pos);
	mRecvBufRead += len;
	mStrStatus = STRING_STATUS_STR_DONE;
	return len;
}

/** Get pointer for string */
string * Conn::getParserStringPtr() {
	if (mParser == NULL) {
		mParser = getParser();
	}
	mParser->ReInit();
	return &(mParser->mCommand);
}

Parser * Conn::getParser() {
	if (mProtocol != NULL) {
		return mProtocol->createParser();
	} else {
		throw "Protocol is NULL";
		// return NULL;
	}
}

void Conn::deleteParser(Parser * OldParser) {
	if (this->mProtocol != NULL) {
		this->mProtocol->deleteParser(OldParser);
	} else {
		delete OldParser;
	}
}

/** remaining (for web-server) */
int Conn::remaining() {

	unsigned long maxCommandLength = (mProtocol != NULL ? mProtocol->getMaxCommandLength() : 10240);

	char * buf = mRecvBuf + mRecvBufRead;
	int len = mRecvBufEnd - mRecvBufRead;
	if (mCommand->size() + len > maxCommandLength) {
		closeNow(CLOSE_REASON_MAXSIZE_REMAINING);
		return -1;
	}
	mCommand->append(buf, len);
	mRecvBufRead = mRecvBufEnd = 0;
	return len;
}

/** Write data in sending buffer and send to conn */
int Conn::writeData(const string & sData, bool bFlush) {
	size_t bufLen = mSendBuf.size();
	if (bufLen + sData.size() >= mSendBufMax) {
		if (Log(0)) {
			LogStream() << "Sending buffer has big size, closing" << endl;
		}
		closeNow(CLOSE_REASON_MAXSIZE_SEND);
		return -1;
	}
	bFlush = bFlush || (bufLen > (mSendBufMax >> 1));

	if (!bFlush) { 
		mSendBuf.append(sData.c_str(), sData.size());
		return 0;
	}

	const char * send_buf = NULL; 
	size_t size; 
	bool appended = false;

	if (bufLen) {
		mSendBuf.append(sData.c_str(), sData.size());
		size = mSendBuf.size();
		send_buf = mSendBuf.c_str();
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
			closeNow(CLOSE_REASON_ERROR_SEND);
			return -1;
		}

		if (Log(3)) {
			LogStream() << "Block sent. Was sent " << size << " bytes" << endl;
		}
		if (!appended) {
			StrCutLeft(sData, mSendBuf, size);
		} else {
			StrCutLeft(mSendBuf, size); /** del from buf sent data */
		}

		if (bool(mCloseTime)) {
			closeNow();
			return size;
		}

		if (mServer && mOk) {
			if (mBlockOutput) {
				mBlockOutput = false;
				mServer->mConnChooser.ConnChoose::OptIn(this, ConnChoose::eEF_OUTPUT);
				if (Log(3)) {
					LogStream() << "Unblock output channel" << endl;
				}
			}

			bufLen = mSendBuf.size();
			if (mBlockInput && bufLen < MAX_SEND_UNBLOCK_SIZE) { /** Unset block of input */
				mServer->mConnChooser.ConnChoose::OptIn(this, ConnChoose::eEF_INPUT);
				if (Log(3)) {
					LogStream() << "Unblock input channel" << endl;
				}
			} else if (!mBlockInput && bufLen >= MAX_SEND_BLOCK_SIZE) { /** Set block of input */
				mServer->mConnChooser.ConnChoose::OptOut(this, ConnChoose::eEF_INPUT);
				if (Log(3)) {
					LogStream() << "Block input channel" << endl;
				}
			}
		}
	} else {
		if (appended) {
			mSendBuf.erase(0, mSendBuf.size());
		}
		ShrinkStringToFit(mSendBuf);

		if (bool(mCloseTime)) {
			closeNow();
			return size;
		}

		if (mServer && mOk && !mBlockOutput) {
			mBlockOutput = true;
			mServer->mConnChooser.ConnChoose::OptOut(this, ConnChoose::eEF_OUTPUT);
			if (Log(3)) {
				LogStream() << "Block output channel" << endl;
			}
		}
		onFlush();
	}
	return size;
}

/** onFlush */
void Conn::onFlush() {
}

/** Flush */
void Conn::flush() {
	static string empty("");
	if (mSendBuf.length()) {
		writeData(empty, true);
	}
}

/** Send len byte from buf */
int Conn::send(const char *buf, size_t &len) {
#ifdef QUICK_SEND /** Quick send */
	if (mConnType != CONN_TYPE_CLIENTUDP) {
		len = ::send(mSocket, buf, len, 0);
	} else {
		len = ::sendto(mSocket, buf, len, 0, (struct sockaddr *) &mSockAddrInUdp, mSockAddrInSize);
	}
	return SOCK_ERROR(len) ? -1 : 0; /* return -1 - fail, 0 - ok */
#else
	int n = -1;
	size_t total = 0, bytesleft = len;

	bool bUDP = (this->mConnType == CONN_TYPE_CLIENTUDP);

	while (total < len) { // EMSGSIZE (WSAEMSGSIZE)

		if (!bUDP) {

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

		} else {
			n = ::sendto(mSocket, buf + total, bytesleft, 0, (struct sockaddr *) &mSockAddrInUdp, mSockAddrInSize);
		}

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

/** Main base timer */
int Conn::onTimerBase(Time &now) {
	if (bool(mCloseTime) && mCloseTime > now) {
		closeNow();
		return 0;
	}
	flush();
	onTimer(now);
	return 0;
}

/** Main timer */
int Conn::onTimer(Time &) {
	return 0;
}

bool Conn::strLog() {
	Obj::strLog();
	LogStream() << "(sock " << mSocket << ") ";
	return true;
}

bool Conn::checkIp(const string &ip) {
	int i = -1;
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

void Conn::getMac() {
#ifdef _WIN32
	char buf[18] = { '\0' };
	unsigned long ip = ip2Num(mIp.c_str());
	unsigned long size = 0x0;
	::GetIpNetTable(NULL, &size, false);
	MIB_IPNETTABLE * pT = (MIB_IPNETTABLE *) new char[size];
	if (0L == ::GetIpNetTable(pT, &size, true)) {
		for (unsigned long i = 0; i < pT->dwNumEntries; ++i) {
			if ((pT->table[i].dwAddr == ip) && (pT->table[i].dwType != 2)) {
				sprintf(buf, "%02x-%02x-%02x-%02x-%02x-%02x",
					pT->table[i].bPhysAddr[0], pT->table[i].bPhysAddr[1],
					pT->table[i].bPhysAddr[2], pT->table[i].bPhysAddr[3],
					pT->table[i].bPhysAddr[4], pT->table[i].bPhysAddr[5]);
				mMac = string(buf);
				break;
			}
		}
		delete[] pT;
	}
#endif
}

bool Conn::getHost() {
	if (mHost.size()) {
		return true;
	}
	struct hostent * h;
	if ((h = gethostbyaddr(mIp.c_str(), sizeof(mIp), AF_INET)) != NULL) {
		mHost = h->h_name;
	}
	return h != NULL;
}

unsigned long Conn::ipByHost(const string &sHost) {
	struct sockaddr_in saddr;
	memset(&saddr, 0, sizeof(sockaddr_in));
	struct hostent * h;
	if ((h = gethostbyname(sHost.c_str())) != NULL) {
		saddr.sin_addr = *((struct in_addr *)h->h_addr);
	}
	return saddr.sin_addr.s_addr;
}

bool Conn::hostByIp(const string & sIp, string &sHost) {
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
ConnFactory::ConnFactory(Protocol * protocol, Server * server) : 
	mProtocol(protocol),
	mServer(server)
{
}

ConnFactory::~ConnFactory() {
}

Conn * ConnFactory::createConn(tSocket sock) {
	Conn * conn = new Conn(sock, mServer); /** CONN_TYPE_CLIENTTCP */
	conn->setCreatedByFactory(true);
	conn->mConnFactory = this;
	conn->mProtocol = mProtocol; /** proto */
	return conn;
}

void ConnFactory::deleteConn(Conn * &conn) {
	conn->close(); /** close socket */
	delete conn;
	conn = NULL;
}

void ConnFactory::onNewData(Conn * conn, string * str) {
	mServer->onNewData(conn, str);
}

int ConnFactory::onNewConn(Conn * conn) {
	return mServer->onNewConn(conn);
}

}; // server
