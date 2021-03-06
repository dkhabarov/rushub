/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz
 *
 * modified: 27 Aug 2009
 * Copyright (C) 2009-2013 by Setuper
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
#include "Thread.h"

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
#else
	#include <unistd.h> // for usleep function
#endif

#ifndef NS_INADDRSZ
  #define NS_INADDRSZ 4
#endif
#ifndef NS_IN6ADDRSZ
  #define NS_IN6ADDRSZ 16
#endif
#ifndef NS_INT16SZ
  #define NS_INT16SZ 2
#endif

using namespace ::utils;

namespace server {


volatile long Conn::mConnCounter = 0;
socklen_t Conn::mSockAddrInSize = sizeof(struct sockaddr_in);

char Conn::mRecvBuf[MAX_RECV_SIZE + 1];

Conn::Conn(tSocket socket, Server * server, int connType) :
	Obj("Conn"),
	mSelfConnFactory(NULL),
	mCreatorConnFactory(NULL),
	mServer(server),
	mProtocol(NULL),
	mParser(NULL),
	mPort(0),
	mPortConn(0),
	mLastRecv(true),
	mSendBufMax(server->mMaxSendSize),
	mOk(socket > 0),
	mWritable(true),
	mConnType(connType),
	mSocket(socket),
	mRecvBufEnd(0),
	mRecvBufRead(0),
	mStatus(STRING_STATUS_NO_STR),
	mAddrInfo(NULL),
	mBlockInput(false),
	mBlockOutput(true),
	mCommand(NULL),
	mClosed(false),
	mCloseReason(0)
{
	memset(&mCloseTime, 0, sizeof(mCloseTime));
}



Conn::~Conn() {

	if (mSocket > 0) {
	#ifndef _WIN32
		SOCK_CLOSE(mSocket);
		if (SOCK_ERR != SOCK_EINTR || (mServer && !mServer->mRun)) { // Interrupted system call on exit
	#else
		int err = SOCK_CLOSE(mSocket);
		if (!(SOCK_ERROR(err))) {
	#endif
			Thread::safeDec(mConnCounter);
			LOG(LEVEL_DEBUG, "Closing socket: " << mSocket);
		} else {
			LOG(LEVEL_ERROR, "Socket not closed: " << SOCK_ERR_MSG << " [" << SOCK_ERR << "]");
		}
		mSocket = 0;
	}

	if (mParser) {
		deleteParser(mParser);
		mParser = NULL;
	}

	if (mAddrInfo != NULL) {
		freeaddrinfo(mAddrInfo);
	}

	mSelfConnFactory = NULL;
	mCreatorConnFactory = NULL;
	mServer = NULL;
	mProtocol = NULL;
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
		#ifdef _WIN32
			LOG(LEVEL_FATAL, "Error in getaddrinfo: " << SOCK_ERR);
		#else
			LOG(LEVEL_FATAL, "Error in getaddrinfo: " << gai_strerror(ret) << " (" << ret << ")");
		#endif
		return INVALID_SOCKET;
	}

	LOG(LEVEL_DEBUG, "Using " << (mAddrInfo->ai_family == AF_INET6 ? "IPv6" : "IPv4") << " socket");

	// socket
	if (SOCK_INVALID(sock = socket(mAddrInfo->ai_family, mAddrInfo->ai_socktype, 0))) {
		LOG(LEVEL_FATAL, "Error in socket: " << SOCK_ERR_MSG << " [" << SOCK_ERR << "]");
		freeaddrinfo(mAddrInfo);
		return INVALID_SOCKET;
	}

	if (!udp) {
		sockoptval_t so_reuseaddr = 1;

		// TIME_WAIT after close conn. Reuse address after disconn
		if (SOCK_ERROR(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof(sockoptval_t)))) {
			LOG(LEVEL_FATAL, "Error in setsockopt: " << SOCK_ERR_MSG << " [" << SOCK_ERR << "]");
			return INVALID_SOCKET;
		}

		configSockSize(sock);

		// Nagle's algorithm
		sockoptval_t tcp_nodelay = mServer->tcpNodelay();
		if (SOCK_ERROR(setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &tcp_nodelay, sizeof(tcp_nodelay)))) {
			LOG(LEVEL_FATAL, "Error in setsockopt: " << SOCK_ERR_MSG << " [" << SOCK_ERR << "]");
		}

	}
	Thread::safeInc(mConnCounter);
	LOG(LEVEL_DEBUG, "Created new socket: " << sock);
	return sock;
}



/// Bind
tSocket Conn::socketBind(tSocket sock) {
	if (sock == INVALID_SOCKET) {
		return INVALID_SOCKET;
	}

	// Bind
	if (SOCK_ERROR(bind(sock, mAddrInfo->ai_addr, static_cast<int> (mAddrInfo->ai_addrlen)))) {
		LOG(LEVEL_FATAL, "Error bind: " << SOCK_ERR_MSG << " [" << SOCK_ERR << "]");
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
		LOG(LEVEL_ERROR, "Error listening: " << SOCK_ERR_MSG << " [" << SOCK_ERR << "]");
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
		LOG(LEVEL_ERROR, "Error connecting: " << SOCK_ERR_MSG << " [" << SOCK_ERR << "]");
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



void Conn::configSockSize(tSocket sock) {
	if (sock == INVALID_SOCKET) {
		return;
	}

	int old_so_sndbuf;
	int new_so_sndbuf = SOCK_SEND_BUFF;
	socklen_t old_so_sndbuf_size = sizeof(old_so_sndbuf);

	if (SOCK_ERROR(getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (sockoptval_t *) &old_so_sndbuf, &old_so_sndbuf_size))) {
		LOG(LEVEL_ERROR, "Error in getsockopt SO_SNDBUF: " << SOCK_ERR_MSG << " [" << SOCK_ERR << "]");
		return;
	} else if (SOCK_ERROR(setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (sockoptval_t *) &new_so_sndbuf, sizeof(new_so_sndbuf)))) {
		LOG(LEVEL_FATAL, "Error in setsockopt SO_SNDBUF: " << SOCK_ERR_MSG << " [" << SOCK_ERR << "]");
		return;
	}

	LOG(LEVEL_TRACE, "SO_SNDBUF: " << old_so_sndbuf << "->" << new_so_sndbuf);


	int old_so_rcvbuf;
	int new_so_rcvbuf = SOCK_RECV_BUFF;
	socklen_t old_so_rcvbuf_size = sizeof(old_so_sndbuf);

	if (SOCK_ERROR(getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (sockoptval_t *) &old_so_rcvbuf, &old_so_rcvbuf_size))) {
		LOG(LEVEL_ERROR, "Error in getsockopt SO_RCVBUF: " << SOCK_ERR_MSG << " [" << SOCK_ERR << "]");
		return;
	} else if (SOCK_ERROR(setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (sockoptval_t *) &new_so_rcvbuf, sizeof(new_so_rcvbuf)))) {
		LOG(LEVEL_FATAL, "Error in setsockopt SO_RCVBUF: " << SOCK_ERR_MSG << " [" << SOCK_ERR << "]");
		return;
	}

	LOG(LEVEL_TRACE, "SO_RCVBUF: " << old_so_rcvbuf << "->" << new_so_rcvbuf);

}




////////////////////////////////////////////////////////
/// closeNice
void Conn::closeNice(int msec /* = 0 */, int reason /* = 0 */) {
	mWritable = false;
	if (reason) {
		mCloseReason = reason;
	}
	if (msec <= 0 || mSendBuf.empty()) {
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
		if (!(mServer->mConnChooser.ConnChoose::optGet(this) & ConnChoose::EF_CLOSE)) {
			++ mServer->mNumCloseConn;
			mClosed = true; // poll conflict

			if (reason) {
				mCloseReason = reason;
			}
			LOG(LEVEL_DEBUG, "closeNow (reason " << mCloseReason << ")");

#if USE_SELECT
			mServer->mConnChooser.ConnChoose::optIn(this, ConnChoose::EF_CLOSE);
			mServer->mConnChooser.ConnChoose::optOut(this, ConnChoose::EF_ALL);
#else
			// this sequence of flags for poll!
			mServer->mConnChooser.ConnChoose::optOut(this, ConnChoose::EF_ALL);
			mServer->mConnChooser.ConnChoose::optIn(this, ConnChoose::EF_CLOSE);
#endif
			
		} else {
			LOG(LEVEL_DEBUG, "Re-closure (reason " << reason << ")");
		}
	} else {
		LOG(LEVEL_FATAL, "Close conn without Server");
	}
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
		LOG(LEVEL_DEBUG, "Create simple connection object for socket: " << sock);
		newConn = new Conn(sock, mServer, CONN_TYPE_INCOMING_TCP); // Create simple connection object
	}
	if (!newConn) {
		LOG(LEVEL_FATAL, "Fatal error: Can't create new connection object");
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
	memset(&storage, 0, sizeof(storage));
	tSocket sock = ::accept(mSocket, reinterpret_cast<struct sockaddr *> (&storage), static_cast<socklen_t*> (&namelen));
	while (SOCK_INVALID(sock) && ((SOCK_ERR == SOCK_EAGAIN) || (SOCK_ERR == SOCK_EINTR)) && (++i <= 10)) {
		// Try to accept connection not more than 10 once
		sock = ::accept(mSocket, reinterpret_cast<struct sockaddr *> (&storage), static_cast<socklen_t*> (&namelen));
		Server::sleep(1);
	}
	if (SOCK_INVALID(sock)) {
		return INVALID_SOCKET;
	}

	sockoptval_t yes = 1;
	if (SOCK_ERROR(setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(int)))) {
		LOG(LEVEL_ERROR, "Socket not SO_KEEPALIVE: " << SOCK_ERR_MSG << " [" << SOCK_ERR << "]");
#ifdef _WIN32
		int err = SOCK_CLOSE(sock);
		if (SOCK_ERROR(err))
#else
		SOCK_CLOSE(sock);
		if (SOCK_ERR != SOCK_EINTR)
#endif
		{
			LOG(LEVEL_WARN, "Couldn't set keepalive flag for accepted socket");
		} else {
			LOG(LEVEL_ERROR, "Socket not closed");
		}
		return INVALID_SOCKET;
	}

	// Non-block socket
	if (socketNonBlock(sock) == INVALID_SOCKET) {
		LOG(LEVEL_ERROR, "Couldn't set non-block flag for accepted socket");
		return INVALID_SOCKET;
	}

	// Accept new socket
	LOG(LEVEL_DEBUG, "Accept new socket: " << sock);

	Thread::safeInc(mConnCounter);
	return sock;
}



int Conn::defineConnInfo(sockaddr_storage & storage) {
	if (mSocket) {
		socklen_t len = storage.ss_family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
		char host[NI_MAXHOST] = { 0 };
		char port[NI_MAXSERV] = { 0 };
		int ret = getnameinfo(reinterpret_cast<struct sockaddr *> (&storage), len, host, NI_MAXHOST, port, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
		if (ret != 0) {
			LOG(LEVEL_WARN, "Error in getnameinfo: " << SOCK_ERR_GAI_MSG(ret) << " [" << ret << "]");
			closeNow(CLOSE_REASON_GETPEERNAME);
			return -1;
		}
		mIp = host;
		mPort = atoi(port);

		if (mServer->mMac) {
			calcMacAddress();
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

	int bufLen = 0, i = 0;

	bool udp = (this->mConnType == CONN_TYPE_INCOMING_UDP);
	if (!udp) { // TCP

		while (
			(SOCK_ERROR(bufLen = ::recv(mSocket, mRecvBuf, MAX_RECV_SIZE, 0))) &&
			((SOCK_ERR == SOCK_EAGAIN) || (SOCK_ERR == SOCK_EINTR))
			&& (++i <= 100)
		) {
			#ifndef _WIN32
				usleep(100u);
			#endif
		}

	} else { // UDP
		LOG(LEVEL_TRACE, "Start read (UDP)");
		while (
			(SOCK_ERROR(bufLen = recvfrom(
				mSocket,
				mRecvBuf,
				MAX_RECV_SIZE,
				0,
				reinterpret_cast<struct sockaddr *> (&mSockAddrIn),
				static_cast<socklen_t *> (&mSockAddrInSize)
			))) && (++i <= 100)
		) {
			#ifndef _WIN32
				usleep(100u);
			#endif
		}
		LOG(LEVEL_TRACE, "End read (UDP). Read bytes: " << bufLen);
	}

	mRecvBufRead = mRecvBufEnd = 0;
	if (bufLen <= 0) {
		if (!udp) {
			if (bufLen == 0) {
				LOG(LEVEL_DEBUG, "Other side has closed connection");
				closeNow(CLOSE_REASON_OTHER_SIDE);
				return -1;
			} else if (SOCK_ERR == EWOULDBLOCK) {
				LOG(LEVEL_DEBUG, "Operation would block");
				return -2;
			} else {
				LOG(LEVEL_DEBUG, "Error in receive: " << SOCK_ERR_MSG << " [" << SOCK_ERR << "]");
				closeNow(CLOSE_REASON_ERROR_RECV);
				return -3;
			}
		}
		return -4;
	}

	if (udp) {
		mIpUdp = inet_ntoa(mSockAddrIn.sin_addr);
	}
	mRecvBufEnd = static_cast<size_t> (bufLen); // End buf pos
	mRecvBufRead = 0; // Pos for reading from buf
	mRecvBuf[mRecvBufEnd] = '\0'; // Adding 0 symbol to end of str
	mLastRecv.get(); // Write time last recv action
	return bufLen;
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



long Conn::getCount() {
	return mConnCounter;
}



/// Calculate mac-address
#ifdef _WIN32
void Conn::calcMacAddress() {
	DWORD size = 0;
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
					if (strcmp(mIp.c_str(), address) == 0) {
						char buf[18] = { '\0' };
						sprintf(buf, "%02x-%02x-%02x-%02x-%02x-%02x",
							curr->PhysicalAddress[0], curr->PhysicalAddress[1],
							curr->PhysicalAddress[2], curr->PhysicalAddress[3],
							curr->PhysicalAddress[4], curr->PhysicalAddress[5]);
						mMacAddress = buf;
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
void Conn::calcMacAddress() {
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
		LOG(LEVEL_FATAL, "Fatal error: Bad setCommandPtr");
		throw "Fatal error: Bad setCommandPtr";
	}
	if (!pStr) {
		LOG(LEVEL_FATAL, "Fatal error: Bad setCommandPtr. Null string pointer");
		throw "Fatal error: Bad setCommandPtr. Null string pointer";
	}
	mCommand = pStr;
	mStatus = STRING_STATUS_PARTLY;
}



/// Reading data from buffer and record in line of the protocol
size_t Conn::readFromRecvBuf() {
	if (!mCommand) {
		LOG(LEVEL_FATAL, "Fatal error: ReadFromBuf with null string pointer");
		throw "Fatal error: ReadFromBuf with null string pointer";
	}

	char * buf = mRecvBuf + mRecvBufRead;
	const char * pos_sep = strstr(buf, getSeparator());
	size_t len = 0u;

	if (pos_sep) { // separator was found
		len = static_cast<size_t> (pos_sep - buf);
		mRecvBufRead += (len + getSeparatorLen());
		mStatus = STRING_STATUS_STR_DONE;
	} else { // separator was not found
		len = mRecvBufEnd - mRecvBufRead;
		mRecvBufRead = mRecvBufEnd = 0u;
	}

	size_t size = mCommand->size() + len;
	if (size > (mProtocol ? mProtocol->getMaxCommandLength() : 10240u)) {
		closeNow(CLOSE_REASON_MAXSIZE_RECV);
		return 0u;
	}
	mCommand->reserve(size);
	mCommand->append(buf, len);
	return pos_sep ? len + getSeparatorLen() : len; // fix read len
}



/// Check empty recv buf
bool Conn::recvBufIsEmpty() const {
	return mRecvBufEnd == mRecvBufRead;
}



/// Get pointer for string
string * Conn::getParserCommandPtr() {
	if (!mParser) {
		if ((mParser = createParser()) == NULL) {
			return NULL;
		}
	} else {
		mParser->reInit();
	}
	return &(mParser->mCommand);
}



Parser * Conn::createParser() {
	if (!mProtocol) {
		if (!mCreatorConnFactory) {
			return NULL;
		}
		// For UDP connection
		mProtocol = mCreatorConnFactory->mProtocol;
	}
	return mProtocol->createParser();
}



void Conn::deleteParser(Parser * parser) {
	if (parser) {
		if (mProtocol) {
			mProtocol->deleteParser(parser);
		} else {
			delete parser;
		}
	}
}



/// Remaining (for web-server)
size_t Conn::remaining() {
	char * buf = mRecvBuf + mRecvBufRead;
	size_t len = mRecvBufEnd - mRecvBufRead;
	size_t size = mCommand->size() + len;
	if (size > (mProtocol ? mProtocol->getMaxCommandLength() : 10240)) {
		closeNow(CLOSE_REASON_MAXSIZE_REMAINING);
		return 0;
	}
	mCommand->reserve(size);
	mCommand->append(buf, len);
	mRecvBufRead = mRecvBufEnd = 0;
	return len;
}



const char * Conn::getSeparator() const {
	return mProtocol ? mProtocol->getSeparator() : "\0";
}



size_t Conn::getSeparatorLen() const {
	return mProtocol ? mProtocol->getSeparatorLen() : 1;
}



/// Write data in sending buffer and send to conn
size_t Conn::writeData(const char * data, size_t len, bool flush) {

	if (!isWritable()) {
		return 0;
	}

	size_t size;
	bool doFlush = false;

	{ // for lock
		Mutex::Lock l(mMutex);

		size_t bufLen = mSendBuf.size();
		if (bufLen + len >= mSendBufMax) {
			LOG(LEVEL_WARN, "Sending buffer has big size, closing");
			closeNow(CLOSE_REASON_MAXSIZE_SEND);
			return 0;
		}

		if (!flush && (bufLen < (mSendBufMax >> 1))) {
			mSendBuf.reserve(bufLen + len);
			mSendBuf.append(data, len);
			return 0;
		}

		const char * send_buf = NULL;

		if (bufLen != 0) {
			mSendBuf.reserve(bufLen + len);
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
				LOG(LEVEL_DEBUG, "Error in sending: " << SOCK_ERR_MSG << " [" << SOCK_ERR << "]" << "(not EAGAIN), closing");
				closeNow(CLOSE_REASON_ERROR_SEND);
				return 0;
			}

			LOG(LEVEL_DEBUG, "Block sent. Was sent " << size << " bytes");
			if (bufLen == 0) {
				size_t s = len - size;
				mSendBuf.reserve(s);
				mSendBuf.append(data + size, s); // Now data.size() != size
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
					mServer->mConnChooser.ConnChoose::optIn(this, ConnChoose::EF_OUTPUT);
					LOG(LEVEL_DEBUG, "Unblock output channel");
				}

				bufLen = mSendBuf.size();
				if (mBlockInput && bufLen < MAX_SEND_UNBLOCK_SIZE) { // Unset block of input
					mServer->mConnChooser.ConnChoose::optIn(this, ConnChoose::EF_INPUT);
					LOG(LEVEL_DEBUG, "Unblock input channel");
				} else if (!mBlockInput && bufLen >= MAX_SEND_BLOCK_SIZE) { // Set block of input
					mServer->mConnChooser.ConnChoose::optOut(this, ConnChoose::EF_INPUT);
					LOG(LEVEL_DEBUG, "Block input channel");
				}
			}
		} else {
			if (bufLen != 0) {
				string().swap(mSendBuf); // erase & free memory
			}

			if (bool(mCloseTime)) {
				closeNow();
				return size;
			}

			if (mServer && mOk && !mBlockOutput) {
				mBlockOutput = true;
				mServer->mConnChooser.ConnChoose::optOut(this, ConnChoose::EF_OUTPUT);
				LOG(LEVEL_DEBUG, "Block output channel");
			}
			doFlush = true;
		}
	}
	
	if (doFlush) {
		onFlush();
	}
	return size;
}



/// Reserve space in send buffer
void Conn::reserve(size_t len) {
	mSendBuf.reserve(mSendBuf.size() + len);
}



/// onFlush
void Conn::onFlush() {
	if (mProtocol && mOk && mWritable) {
		mProtocol->onFlush(this);
	}
}



/// Flush
void Conn::flush() {
	if (!mSendBuf.empty()) {
		writeData("", 0, true);
	}
}



/// Send len byte from buf
int Conn::send(const char * buf, size_t & len) {
	int n = -1;
	size_t total = 0, bytesleft = len;
	bool tcp = (mConnType != CONN_TYPE_INCOMING_UDP);

	while (total < len) { // EMSGSIZE (WSAEMSGSIZE)

		if (tcp) {

			n = ::send(
				mSocket,
				buf + total,
				#ifndef _WIN32
					bytesleft,
					MSG_NOSIGNAL | MSG_DONTWAIT
				#else
					static_cast<int> (bytesleft), // Attention! Max len: 2147483647 (0x7FFFFFFF)
					0
				#endif
			);

		} else {

			n = ::sendto(
				mSocket,
				buf + total,
				#ifndef _WIN32
					bytesleft,
				#else
					static_cast<int> (bytesleft), // Attention! Max len: 2147483647 (0x7FFFFFFF)
				#endif
				0,
				reinterpret_cast<struct sockaddr *> (&mSockAddrIn),
				mSockAddrInSize
			);

		}
		if (SOCK_ERROR(n)) {
			break;
		} else if (static_cast<size_t> (n) == len) { // small optimization
			return 0; // ok
		}
		total += static_cast<size_t> (n);
		bytesleft -= static_cast<size_t> (n);
		LOG(LEVEL_TRACE, n << "/" << total << "/" << len);
	}
	len = total; // Number sent bytes
	return SOCK_ERROR(n) ? -1 : 0; // return -1 - fail, 0 - ok
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



bool Conn::strLog(int level, ostream & os) {
	Obj::strLog(level, os);
	os << "[sock:" << mSocket << "] ";
	return true;
}


/* // It is not using now
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
*/



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


} // namespace server

/**
 * $Id$
 * $HeadURL$
 */
