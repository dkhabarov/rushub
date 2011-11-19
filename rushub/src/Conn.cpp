/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz
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

#include "Conn.h"
#include "Server.h" // mServer

#include <iostream> // cout, endl
#include <stdlib.h> // atoi unix
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
	#include <Iphlpapi.h> // mac address
	#pragma comment(lib, "Iphlpapi.lib") // mac address
	#if defined(_MSC_VER) && (_MSC_VER >= 1400)
		#pragma warning(disable:4996) // Disable "This function or variable may be unsafe."
	#endif
#endif

#define NS_INADDRSZ 4
#define NS_IN6ADDRSZ 16
#define NS_INT16SZ 2



namespace server {


unsigned long Conn::mConnCounter = 0;
socklen_t Conn::mSockAddrInSize = sizeof(struct sockaddr_in);

char Conn::mRecvBuf[MAX_RECV_SIZE + 1];

Conn::Conn(tSocket socket, Server * server, int connType) :
	Obj("Conn"),
	mLastRecv(true),
	mSelfConnFactory(NULL),
	mCreatorConnFactory(NULL),
	mServer(server),
	mProtocol(NULL),
	mParser(NULL),
	mConnType(connType),
	mOk(socket > 0),
	mWritable(true),
	mPort(0),
	mPortConn(0),
	mSendBufMax(server->mMaxSendSize),
	mConnect(true),
	mSocket(socket),
	mRecvBufEnd(0),
	mRecvBufRead(0),
	mAddrInfo(NULL),
	mBlockInput(false),
	mBlockOutput(true),
	mCommand(NULL),
	mClosed(false),
	mCloseReason(0)
{
	clearCommandPtr();
	memset(&mCloseTime, 0, sizeof(mCloseTime));
}



Conn::~Conn() {
	if (mParser) {
		deleteParser(mParser);
		mParser = NULL;
	}
	mSelfConnFactory = NULL;
	mCreatorConnFactory = NULL;
	mServer = NULL;
	mProtocol = NULL;
	close();
}



/// Get socket
Conn::operator tSocket() const {
	return mSocket;
}



/// Get connection type
int Conn::getConnType() const {
	return mConnType;
}



/// Get status
int Conn::getStatus() const {
	return mStatus;
}



/// Is OK
bool Conn::isOk() const {
	return mOk;
}



/// Set OK
void Conn::setOk(bool ok) {
	mOk = ok;
	onOk(ok);
}



/// Is writable
bool Conn::isWritable() const {
	return mWritable;
}



/// Is closed
bool Conn::isClosed() const {
	return mClosed;
}



/// Get string of IP
const string & Conn::getIp() const {
	return mIp;
}



/// Get string of server IP (host)
const string & Conn::getIpConn() const {
	return mIpConn;
}



/// Get IP for UDP
const string & Conn::getIpUdp() const {
	return mIpUdp;
}



/// Get real port
int Conn::getPort() const {
	return mPort;
}



/// Get connection port
int Conn::getPortConn() const {
	return mPortConn;
}



/// Get mac-address
const string & Conn::getMacAddress() const {
	return mMacAddress;
}




void Conn::onOk(bool) {
}



/// makeSocket
tSocket Conn::makeSocket(const char * port, const char * address, int connType /*= CONN_TYPE_LISTEN*/) {
	if (mSocket > 0) {
		return INVALID_SOCKET; // Socket is already created
	}
	
	switch (connType) {
		case CONN_TYPE_LISTEN :
			mSocket = socketCreate(port, address, false);
			mSocket = socketBind(mSocket);
			mSocket = socketListen(mSocket);
			mSocket = socketNonBlock(mSocket);
			break;

		case CONN_TYPE_INCOMING_UDP :
			mSocket = socketCreate(port, address, true);
			mSocket = socketBind(mSocket);
			mSocket = socketNonBlock(mSocket);
			break;

		case CONN_TYPE_OUTGOING_TCP :
			mSocket = socketCreate(port, address, false);
			mSocket = socketConnect(mSocket);
			mSocket = socketNonBlock(mSocket);
			break;

		case CONN_TYPE_OUTGOING_UDP :
			mSocket = socketCreate(port, address, true);
			mSocket = socketNonBlock(mSocket);
			break;

		default :
			break;

	}

	mPort = atoi(port); // Set port
	mIp = address; // Set ip (host)
	setOk(mSocket > 0); // Reg conn
	return mSocket;
}



/// Create socket (default TCP)
tSocket Conn::socketCreate(const char * port, const char * address, bool udp) {
	tSocket sock;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;

	if (!udp) { // Create socket TCP
		hints.ai_socktype = SOCK_STREAM;
	} else {
		hints.ai_socktype = SOCK_DGRAM;
	}

	// getaddrinfo
	int ret = getaddrinfo(address, port, &hints, &mAddrInfo);
	if (ret != 0) {
		if (log(FATAL)) {
			logStream() << "Error in getaddrinfo: " << 
			#ifdef _WIN32
				SOCK_ERR
			#else
				gai_strerror(ret) << " (" << ret << ")"
			#endif
			<< endl;
		}
		return INVALID_SOCKET;
	}

	if (log(DEBUG)) {
		logStream() << "Using " << (mAddrInfo->ai_family == AF_INET6 ? "IPv6" : "IPv4") << " socket" << endl;
	}

	// socket
	if (SOCK_INVALID(sock = socket(mAddrInfo->ai_family, mAddrInfo->ai_socktype, 0))) {
		if (log(FATAL)) {
			logStream() << "Error in socket: " << SOCK_ERR_MSG << " [" << SOCK_ERR << "]" << endl;
		}
		return INVALID_SOCKET;
	}

	if (!udp) {
		sockoptval_t so_reuseaddr = 1;
		sockoptval_t tcp_nodelay = 1;

		// TIME_WAIT after close conn. Reuse address after disconn
		if (SOCK_ERROR(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof(sockoptval_t)))) {
			if (log(FATAL)) {
				logStream() << "Error in setsockopt: " << SOCK_ERR_MSG << " [" << SOCK_ERR << "]" << endl;
			}
			return INVALID_SOCKET;
		}

		#ifdef _WIN32
		if (SOCK_ERROR(setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &tcp_nodelay, sizeof(sockoptval_t)))) {
			if (log(FATAL)) {
				logStream() << "Error in setsockopt: " << SOCK_ERR_MSG << " [" << SOCK_ERR << "]" << endl;
			}
			return INVALID_SOCKET;
		}
		#endif
	}

	++mConnCounter;
	if (log(DEBUG)) {
		logStream() << "Created new socket: " << sock << endl;
	}
	return sock;
}



/// Bind
tSocket Conn::socketBind(tSocket sock) {
	if (sock == INVALID_SOCKET) {
		return INVALID_SOCKET;
	}

	// Bind
	if (SOCK_ERROR(bind(sock, mAddrInfo->ai_addr, static_cast<int> (mAddrInfo->ai_addrlen)))) {
		if (log(FATAL)) {
			logStream() << "Error bind: " << SOCK_ERR_MSG << " [" << SOCK_ERR << "]" << endl;
		}
		return INVALID_SOCKET;
	}

	return sock;
}



/// Listen TCP socket
tSocket Conn::socketListen(tSocket sock) {
	if (sock == INVALID_SOCKET) {
		return INVALID_SOCKET;
	}
	if (SOCK_ERROR(listen(sock, SOCK_BACKLOG))) {
		SOCK_CLOSE(sock);
		if (log(ERR)) {
			logStream() << "Error listening: " << SOCK_ERR_MSG << " [" << SOCK_ERR << "]" << endl;
		}
		return INVALID_SOCKET;
	}
	return sock;
}



/// Connect to TCP socket
tSocket Conn::socketConnect(tSocket sock) {
	if (sock == INVALID_SOCKET) {
		return INVALID_SOCKET;
	}
	if (SOCK_ERROR(connect(sock, mAddrInfo->ai_addr, static_cast<int> (mAddrInfo->ai_addrlen)))) {
		SOCK_CLOSE(sock);
		if (log(ERR)) {
			logStream() << "Error connecting: " << SOCK_ERR_MSG << " [" << SOCK_ERR << "]" << endl;
		}
		return INVALID_SOCKET;
	}
	return sock;
}



/// Set non-block socket
tSocket Conn::socketNonBlock(tSocket sock) {
	if (sock == INVALID_SOCKET) {
		return INVALID_SOCKET;
	}
	SOCK_NON_BLOCK(sock);
	return sock;
}





////////////////////////////////////////////////////////
/// Close socket
void Conn::close() {
	if (mSocket <= 0) {
		return;
	}
	mWritable = false;
	setOk(false);

	// onClose
	if (mServer) {
		mServer->onClose(this);
	}

#ifndef _WIN32
	SOCK_CLOSE(mSocket);
	if (SOCK_ERR != SOCK_EINTR || (mServer && !mServer->mRun)) { // Interrupted system call on exit
#else
	int err = SOCK_CLOSE(mSocket);
	if (!(SOCK_ERROR(err))) {
#endif
		--mConnCounter;
		if (log(DEBUG)) {
			logStream() << "Closing socket: " << mSocket << endl;
		}
	} else if (log(ERR)) {
		logStream() << "Socket not closed: " << SOCK_ERR_MSG << " [" << SOCK_ERR << "]" << endl;
	}

	if (mAddrInfo != NULL) {
		freeaddrinfo(mAddrInfo);
	}
	mSocket = 0;
}



/// closeNice
void Conn::closeNice(int msec /* = 0 */, int reason /* = 0 */) {
	mWritable = false;
	if (reason) {
		mCloseReason = reason;
	}
	if ((msec <= 0) || (!mSendBuf.size())) {
		closeNow(reason);
		return;
	}
	mCloseTime.get();
	mCloseTime += msec;
}



/// closeNow
void Conn::closeNow(int reason /* = 0 */) {
	mWritable = false;
	setOk(false);
	if (mServer) {
		if (!(mServer->mConnChooser.ConnChoose::optGet(static_cast<ConnBase *> (this)) & ConnChoose::eEF_CLOSE)) {
			++ mServer->miNumCloseConn;
			mClosed = true; // poll conflict

			if (reason) {
				mCloseReason = reason;
			}
			if (log(DEBUG)) {
				logStream() << "closeNow (reason " << mCloseReason << ")" << endl;
			}

#if USE_SELECT
			mServer->mConnChooser.ConnChoose::optIn(static_cast<ConnBase *> (this), ConnChoose::eEF_CLOSE);
			mServer->mConnChooser.ConnChoose::optOut(static_cast<ConnBase *> (this), ConnChoose::eEF_ALL);
#else
			// this sequence of flags for poll!
			mServer->mConnChooser.ConnChoose::optOut(static_cast<ConnBase *> (this), ConnChoose::eEF_ALL);
			mServer->mConnChooser.ConnChoose::optIn(static_cast<ConnBase *> (this), ConnChoose::eEF_CLOSE);
#endif
			
		} else {
			if (log(DEBUG)) {
				logStream() << "Re-closure (reason " << reason << ")" << endl;
			}
		}
	} else {
		if (log(FATAL)) {
			logStream() << "Close conn without Server" << endl;
		}
	}
}



void Conn::closeSelf() {
	if (log(DEBUG)) {
		logStream() << "User itself was disconnected" << endl;
	}
	closeNow(CLOSE_REASON_CLIENT_DISCONNECT);
}



/// createNewConn
Conn * Conn::createNewConn() {

	struct sockaddr_storage storage;

	tSocket sock = socketAccept(storage);
	if (sock == INVALID_SOCKET) {
		return NULL;
	}

	Conn * newConn = NULL;

	if (mCreatorConnFactory != NULL) {
		newConn = mCreatorConnFactory->createConn(sock); // Create connection object by factory
	} else {
		if (log(DEBUG)) {
			logStream() << "Create simple connection object for socket: " << sock << endl;
		}
		newConn = new Conn(sock, mServer, CONN_TYPE_INCOMING_TCP); // Create simple connection object
	}
	if (!newConn) {
		if (log(FATAL)) {
			logStream() << "Fatal error: Can't create new connection object" << endl;
		}
		throw "Fatal error: Can't create new connection object";
	}

	if (newConn->defineConnInfo(storage) == -1) {
		if (mCreatorConnFactory != NULL) {
			mCreatorConnFactory->deleteConn(newConn);
		} else {
			delete newConn;
		}
		return NULL;
	}
	return newConn;
}



/// Accept new conn
tSocket Conn::socketAccept(struct sockaddr_storage & storage) {
	int i = 0;
	socklen_t namelen = sizeof(storage);
	memset(&storage, 0, namelen);
	tSocket sock = ::accept(mSocket, (struct sockaddr *) &storage, (socklen_t*) &namelen);
	while (SOCK_INVALID(sock) && ((SOCK_ERR == SOCK_EAGAIN) || (SOCK_ERR == SOCK_EINTR)) && (++i <= 10)) {
		// Try to accept connection not more than 10 once
		sock = ::accept(mSocket, (struct sockaddr *) &storage, (socklen_t*) &namelen);
		#ifdef _WIN32
			Sleep(1);
		#else
			usleep(1000);
		#endif
	}
	if (SOCK_INVALID(sock)) {
		return INVALID_SOCKET;
	}

	sockoptval_t yes = 1;
	if (SOCK_ERROR(setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(int)))) {
		if (log(ERR)) {
			logStream() << "Socket not SO_KEEPALIVE: " << SOCK_ERR_MSG << " [" << SOCK_ERR << "]" << endl;
		}
#ifdef _WIN32
		int err = SOCK_CLOSE(sock);
		if (SOCK_ERROR(err))
#else
		SOCK_CLOSE(sock);
		if (SOCK_ERR != SOCK_EINTR)
#endif
		{
			if (log(WARN)) {
				logStream() << "Couldn't set keepalive flag for accepted socket" << endl;
			}
		} else if (log(ERR)) {
			logStream() << "Socket not closed" << endl;
		}
		return INVALID_SOCKET;
	}

	// Non-block socket
	if (socketNonBlock(sock) == INVALID_SOCKET) {
		if (log(ERR)) {
			logStream() << "Couldn't set non-block flag for accepted socket" << endl;
		}
		return INVALID_SOCKET;
	}

	// Accept new socket
	if (log(DEBUG)) {
		logStream() << "Accept new socket: " << sock << endl;
	}

	++mConnCounter;
	return sock;
}



int Conn::defineConnInfo(sockaddr_storage & storage) {
	if (mSocket) {
		char host[NI_MAXHOST] = { 0 };
		char port[NI_MAXSERV] = { 0 };
		if (getnameinfo((struct sockaddr *) &storage, sizeof(struct sockaddr), host, NI_MAXHOST, port, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV) != 0) {
			if (log(WARN)) {
				logStream() << "Error in getnameinfo: " << SOCK_ERR_MSG << " [" << SOCK_ERR << "]" << endl;
			}
			closeNow(CLOSE_REASON_GETPEERNAME);
			return -1;
		}
		mIp = host;
		mPort = atoi(port);

		if (mServer->mMac) {
			calcMacAddress(mIp, mMacAddress);
		}
		return 0;
	}
	return -1;
}



/// Reading all data from socket to buffer of the conn
int Conn::recv() {
	if (!mOk || !mWritable) {
		return 0;
	}

	int iBufLen = 0, i = 0;


	bool udp = (this->mConnType == CONN_TYPE_INCOMING_UDP);
	if (!udp) { // TCP

		while (
			(SOCK_ERROR(iBufLen = ::recv(mSocket, mRecvBuf, MAX_RECV_SIZE, 0))) &&
			((SOCK_ERR == SOCK_EAGAIN) || (SOCK_ERR == SOCK_EINTR))
			&& (++i <= 100)
		) {
			#ifndef _WIN32
				usleep(100);
			#endif
		}

	} else { // UDP
		if (log(TRACE)) {
			logStream() << "Start read (UDP)" << endl;
		}
		while (
			(SOCK_ERROR(iBufLen = recvfrom(
				mSocket,
				mRecvBuf,
				MAX_RECV_SIZE,
				0,
				(struct sockaddr *) &mSockAddrIn,
				(socklen_t *) &mSockAddrInSize
			))) && (++i <= 100)
		) {
			#ifndef _WIN32
				usleep(100);
			#endif
		}
		if (log(TRACE)) {
			logStream() << "End read (UDP). Read bytes: " << iBufLen << endl;
		}
	}


	mRecvBufRead = mRecvBufEnd = 0;
	if (iBufLen <= 0) {
		if (!udp) {
			if (iBufLen == 0) {
				return -1;
			} else if (SOCK_ERR == EWOULDBLOCK) {
				if (log(DEBUG)) {
					logStream() << "Operation would block" << endl;
				}
				return -2;
			} else {
				if (log(DEBUG)) {
					logStream() << "Error in receive: " << SOCK_ERR_MSG << " [" << SOCK_ERR << "]" << endl;
				}
				closeNow(CLOSE_REASON_ERROR_RECV);
				return -3;
			}
		}
		return -4;
	}

	if (udp) {
		mIpUdp = inet_ntoa(mSockAddrIn.sin_addr);
	}
	mRecvBufEnd = iBufLen; // End buf pos
	mRecvBufRead = 0; // Pos for reading from buf
	mRecvBuf[mRecvBufEnd] = '\0'; // Adding 0 symbol to end of str
	mLastRecv.get(); // Write time last recv action
	return iBufLen;
}



/// Check IP
bool Conn::checkIp(const string & ip) {
	#ifndef _WIN32
		char dst[256];
		if (inet_pton(AF_INET, ip.c_str(), dst) > 0) {
			return true;
		} else if (inet_pton(AF_INET6, ip.c_str(), dst) > 0) {
			return true;
		}
		return false;
	#else
		struct sockaddr_in sockaddrIn;
		int size = sizeof(struct sockaddr_in);
		struct sockaddr * addr = (struct sockaddr*) &sockaddrIn;
		if (WSAStringToAddress((char*) ip.c_str(), AF_INET, NULL, addr, &size) != -1) {
			return true;
		}

		struct sockaddr_in6 sockaddrIn6;
		size = sizeof(struct sockaddr_in6);
		addr = (struct sockaddr*) &sockaddrIn6;
		if (WSAStringToAddress((char*) ip.c_str(), AF_INET6, NULL, addr, &size) != -1) {
			return true;
		}
		return false;
	#endif // _WIN32
}


/// Calculate mac-address
#ifdef _WIN32
void Conn::calcMacAddress(const string & ip, string & mac) {
	DWORD size;
	DWORD ret = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, NULL, &size);
	if (ret != ERROR_BUFFER_OVERFLOW) {
		return;
	}
	PIP_ADAPTER_ADDRESSES addr = (PIP_ADAPTER_ADDRESSES) malloc(size);
	if (addr == NULL) {
		return;
	}

	ret = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, addr, &size);
	if (ret == NO_ERROR) {
		PIP_ADAPTER_ADDRESSES curr = addr;
		while (curr) {
			if (curr->PhysicalAddressLength == 6) {
				int i;
				PIP_ADAPTER_UNICAST_ADDRESS pIpAdapterPrefix;
				for (i = 0, pIpAdapterPrefix= curr->FirstUnicastAddress;
					pIpAdapterPrefix;
					i++, pIpAdapterPrefix = pIpAdapterPrefix->Next
				) {
					char address[NI_MAXHOST];
					if (getnameinfo(pIpAdapterPrefix->Address.lpSockaddr,
						pIpAdapterPrefix->Address.iSockaddrLength,
						address, sizeof(address), NULL, 0,
						NI_NUMERICHOST)
					) {
						continue;
					}
					if (strcmp(ip.c_str(), address) == 0) {
						char buf[18] = { '\0' };
						sprintf(buf, "%02x-%02x-%02x-%02x-%02x-%02x",
							curr->PhysicalAddress[0], curr->PhysicalAddress[1],
							curr->PhysicalAddress[2], curr->PhysicalAddress[3],
							curr->PhysicalAddress[4], curr->PhysicalAddress[5]);
						mac = buf;
						free(addr);
						return;
					}
				}
			}
			curr = curr->Next;
		}
	}
	free(addr);
}
#else
void Conn::calcMacAddress(const string &, string &) {
}
#endif




/// Clear params
void Conn::clearCommandPtr() {
	mCommand = NULL;
	mStatus = STRING_STATUS_NO_STR;
}



/// Get pointer for string with data
string * Conn::getCommandPtr() {
	return mCommand;
}



/** Installing the string, in which will be recorded received data, 
	and installation main parameter */
void Conn::setCommandPtr(string * pStr) {
	if (mStatus != STRING_STATUS_NO_STR) {
		if (log(FATAL)) {
			logStream() << "Fatal error: Bad setCommandPtr" << endl;
		}
		throw "Fatal error: Bad setCommandPtr";
	}
	if (!pStr) {
		if (log(FATAL)) {
			logStream() << "Fatal error: Bad setCommandPtr. Null string pointer" << endl;
		}
		throw "Fatal error: Bad setCommandPtr. Null string pointer";
	}
	mCommand = pStr;
	mStatus = STRING_STATUS_PARTLY;
}



/// Reading data from buffer and record in line of the protocol
size_t Conn::readFromRecvBuf() {
	if (!mCommand) {
		if (log(FATAL)) {
			logStream() << "Fatal error: ReadFromBuf with null string pointer" << endl;
		}
		throw "Fatal error: ReadFromBuf with null string pointer";
	}

	char * buf = mRecvBuf + mRecvBufRead;
	const char * sep = "\0";
	size_t sep_len = 1, len = (mRecvBufEnd - mRecvBufRead);
	unsigned long maxCommandLength = 10240;
	
	if (mProtocol != NULL) {
		sep = mProtocol->getSeparator();
		sep_len = mProtocol->getSeparatorLen();
		maxCommandLength = mProtocol->getMaxCommandLength();
	}

	const char * pos_sep = NULL;
	if ((pos_sep = strstr(buf, sep)) == NULL) {
		if (mCommand->size() + len > maxCommandLength) {
			closeNow(CLOSE_REASON_MAXSIZE_RECV);
			return 0;
		}
		mCommand->append(buf, len);
		mRecvBufRead = mRecvBufEnd = 0;
		return len;
	}
	size_t pos = pos_sep - buf;
	len = pos + sep_len;
	mCommand->append(buf, pos);
	mRecvBufRead += len;
	mStatus = STRING_STATUS_STR_DONE;
	return len;
}



/// Get pointer for string
string * Conn::getParserCommandPtr() {
	if (mParser == NULL) {
		if ((mParser = createParser()) == NULL) {
			return NULL;
		}
	} else {
		mParser->reInit();
	}
	return &(mParser->mCommand);
}



Parser * Conn::createParser() {
	if (mProtocol == NULL) {
		// ToDo remove!
		if (mCreatorConnFactory == NULL) {
			return NULL;
		}
		// ToDo remove!
		mProtocol = mCreatorConnFactory->getProtocol();
	}
	return mProtocol->createParser();
}



void Conn::deleteParser(Parser * parser) {
	if (parser != NULL) {
		if (mProtocol != NULL) {
			mProtocol->deleteParser(parser);
		} else {
			delete parser;
		}
	}
}



/// Remaining (for web-server)
size_t Conn::remaining() {

	unsigned long maxCommandLength = (mProtocol != NULL ? mProtocol->getMaxCommandLength() : 10240);

	char * buf = mRecvBuf + mRecvBufRead;
	size_t len = mRecvBufEnd - mRecvBufRead;
	if (mCommand->size() + len > maxCommandLength) {
		closeNow(CLOSE_REASON_MAXSIZE_REMAINING);
		return 0;
	}
	mCommand->append(buf, len);
	mRecvBufRead = mRecvBufEnd = 0;
	return len;
}



const char * Conn::getSeparator() {
	return mProtocol != NULL ? mProtocol->getSeparator() : "\0";
}



size_t Conn::getSeparatorLen() {
	return mProtocol != NULL ? mProtocol->getSeparatorLen() : 1;
}



/// Write data in sending buffer and send to conn
size_t Conn::writeData(const char * data, size_t len, bool flush) {
	size_t bufLen = mSendBuf.size();
	if (bufLen + len >= mSendBufMax) {
		if (log(WARN)) {
			logStream() << "Sending buffer has big size, closing" << endl;
		}
		closeNow(CLOSE_REASON_MAXSIZE_SEND);
		return 0;
	}

	if (!flush && (bufLen < (mSendBufMax >> 1))) { 
		mSendBuf.append(data, len);
		return 0;
	}

	const char * send_buf = NULL; 
	size_t size;

	if (bufLen != 0) {
		mSendBuf.append(data, len);
		size = mSendBuf.size();
		send_buf = mSendBuf.c_str();
	} else { 
		if (len == 0) { // buff is empty and no new data
			return 0;
		}
		size = len;
		send_buf = data;
	}

	// Sending
	if (send(send_buf, size) < 0) {

		if (SOCK_ERR != SOCK_EAGAIN) {
			if (log(DEBUG)) {
				logStream() << "Error in sending: " << SOCK_ERR_MSG << " [" << SOCK_ERR << "]" << "(not EAGAIN), closing" << endl;
			}
			closeNow(CLOSE_REASON_ERROR_SEND);
			return 0;
		}

		if (log(DEBUG)) {
			logStream() << "Block sent. Was sent " << size << " bytes" << endl;
		}
		if (bufLen == 0) {
			mSendBuf.append(data + size, len - size); // Now sData.size() != size
		} else {
			string(mSendBuf, size, mSendBuf.size() - size).swap(mSendBuf); // Del from buf sent data
		}

		if (bool(mCloseTime)) {
			closeNow();
			return size;
		}

		if (mServer && mOk) {
			if (mBlockOutput) {
				mBlockOutput = false;
				mServer->mConnChooser.ConnChoose::optIn(this, ConnChoose::eEF_OUTPUT);
				if (log(DEBUG)) {
					logStream() << "Unblock output channel" << endl;
				}
			}

			bufLen = mSendBuf.size();
			if (mBlockInput && bufLen < MAX_SEND_UNBLOCK_SIZE) { // Unset block of input
				mServer->mConnChooser.ConnChoose::optIn(this, ConnChoose::eEF_INPUT);
				if (log(DEBUG)) {
					logStream() << "Unblock input channel" << endl;
				}
			} else if (!mBlockInput && bufLen >= MAX_SEND_BLOCK_SIZE) { // Set block of input
				mServer->mConnChooser.ConnChoose::optOut(this, ConnChoose::eEF_INPUT);
				if (log(DEBUG)) {
					logStream() << "Block input channel" << endl;
				}
			}
		}
	} else {
		if (bufLen != 0) {
			mSendBuf.erase();
			mSendBuf.reserve();
		}

		if (bool(mCloseTime)) {
			closeNow();
			return size;
		}

		if (mServer && mOk && !mBlockOutput) {
			mBlockOutput = true;
			mServer->mConnChooser.ConnChoose::optOut(this, ConnChoose::eEF_OUTPUT);
			if (log(DEBUG)) {
				logStream() << "Block output channel" << endl;
			}
		}
		onFlush();
	}
	return size;
}



/// onFlush
void Conn::onFlush() {
	if (mProtocol != NULL) {
		mProtocol->onFlush(this);
	}
}



/// Flush
void Conn::flush() {
	if (mSendBuf.size() != 0) {
		writeData("", 0, true);
	}
}



/// Send len byte from buf
int Conn::send(const char * buf, size_t & len) {
#ifdef QUICK_SEND // Quick send
	if (mConnType != CONN_TYPE_INCOMING_UDP) {
		len = ::send(mSocket, buf, len, 0);
	} else {
		len = ::sendto(mSocket, buf, len, 0, (struct sockaddr *) &mSockAddrIn, mSockAddrInSize);
	}
	return SOCK_ERROR(len) ? -1 : 0; /* return -1 - fail, 0 - ok */
#else
	int n = -1;
	size_t total = 0, bytesleft = len;

	bool tcp = (mConnType != CONN_TYPE_INCOMING_UDP);

	while (total < len) { // EMSGSIZE (WSAEMSGSIZE)

		if (tcp) {

			n = ::send(
				mSocket,
				buf + total,
				static_cast<int> (bytesleft), // fix me: protection to very long msg
				#ifndef _WIN32
					MSG_NOSIGNAL | MSG_DONTWAIT
				#else
					0
				#endif
			);

		} else {

			n = ::sendto(
				mSocket,
				buf + total,
				static_cast<int> (bytesleft), // fix me: protection to very long msg
				0,
				(struct sockaddr *) &mSockAddrIn,
				mSockAddrInSize
			);

		}

/*		if (log(TRACE)) {
				logStream() << "len = " << len
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
	len = total; // Number sent bytes
	return SOCK_ERROR(n) ? -1 : 0; // return -1 - fail, 0 - ok
#endif
}



/// Main base timer
void Conn::onTimerBase(Time & now) {
	if (bool(mCloseTime) && mCloseTime > now) {
		closeNow();
	} else {
		flush();
		onTimer(now);
	}
}



/// Main timer
int Conn::onTimer(Time &) {
	return 0;
}



bool Conn::strLog() {
	Obj::strLog();
	simpleLogStream() << "[sock:" << mSocket << "] ";
	return true;
}



const char * Conn::inetNtop(int af, const void * src, char * dst, socklen_t cnt) {
	#ifndef _WIN32
	if (inet_ntop(af, src, dst, cnt)) {
		if (af == AF_INET6 && strncmp(dst, "::ffff:", 7) == 0) {
			memmove(dst, dst + 7, cnt - 7);
		}
		return dst;
	}
	return NULL;
	#else
		struct sockaddr * addr = NULL;
		size_t size;
		DWORD len = cnt;

		if (af == AF_INET) {
			struct in_addr * inAddr = (struct in_addr*) src;
			struct sockaddr_in sockaddrIn;
			sockaddrIn.sin_family = AF_INET;
			sockaddrIn.sin_port = 0;
			sockaddrIn.sin_addr = *inAddr;
			addr = (struct sockaddr *) &sockaddrIn;
			size = sizeof(sockaddrIn);
		} else if (af == AF_INET6) {
			struct in6_addr * in6Addr = (struct in6_addr*) src;
			struct sockaddr_in6 sockaddrIn6;
			sockaddrIn6.sin6_family = AF_INET6;
			sockaddrIn6.sin6_port = 0;
			sockaddrIn6.sin6_addr = *in6Addr;
			addr = (struct sockaddr *) &sockaddrIn6;
			size = sizeof(sockaddrIn6);
		} else {
			return NULL;
		}

		if (WSAAddressToString(addr, static_cast<DWORD> (size), NULL, dst, &len) == 0) {
			return dst;
		}
		return NULL;
	#endif // _WIN32
}



int Conn::inetPton(int af, const char * src, void * dst) {
	#ifndef _WIN32
		return inet_pton(af, src, dst);
	#else
		if (af == AF_INET) {
			struct sockaddr_in sockaddrIn;
			int size = sizeof(struct sockaddr_in);
			struct sockaddr * addr = (struct sockaddr*) &sockaddrIn;
			if (WSAStringToAddress((char*) src, af, NULL, addr, &size) == -1) {
				return -1;
			}
			memcpy(dst, &sockaddrIn.sin_addr, sizeof(sockaddrIn.sin_addr));
		} else if (af == AF_INET6) {
			struct sockaddr_in6 sockaddrIn6;
			int size = sizeof(struct sockaddr_in6);
			struct sockaddr * addr = (struct sockaddr*) &sockaddrIn6;
			if (WSAStringToAddress((char*) src, af, NULL, addr, &size) == -1) {
				return -1;
			}
			memcpy(dst, &sockaddrIn6.sin6_addr, sizeof(sockaddrIn6.sin6_addr));
		} else {
			return -1;
		}
		return 1;
	#endif // _WIN32
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
	Conn * conn = new Conn(sock, mServer, CONN_TYPE_INCOMING_TCP);
	conn->mSelfConnFactory = this;
	return conn;
}



void ConnFactory::deleteConn(Conn * & conn) {
	conn->close(); // close socket
	delete conn;
	conn = NULL;
}



void ConnFactory::onNewData(Conn * conn, string * str) {
	mServer->onNewData(conn, str);
}



int ConnFactory::onNewConn(Conn * conn) {
	if (conn->mProtocol == NULL) {
		conn->mProtocol = mProtocol; // Set protocol
	}
	return mServer->onNewConn(conn);
}

// ToDo remove!
Protocol * ConnFactory::getProtocol() {
	return mProtocol;
}


}; // server

/**
 * $Id$
 * $HeadURL$
 */
