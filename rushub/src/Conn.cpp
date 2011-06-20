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
#include "Server.h" // for mServer
#include "stringutils.h" // for atoi

#include <iostream> // cout, endl
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

Conn::Conn(tSocket socket, Server * server, ConnType connType) :
	Obj("Conn"),
	mConnFactory(NULL),
	mServer(server),
	mProtocol(NULL),
	mParser(NULL),
	mConnType(connType),
	mOk(socket > 0),
	mWritable(true),
	mPort(0),
	mPortConn(0),
	mSendBufMax(MAX_SEND_SIZE),
	mSocket(socket),
	mRecvBufEnd(0),
	mRecvBufRead(0),
	mAddrInfo(NULL),
	mBlockInput(false),
	mBlockOutput(true),
	mCommand(NULL),
	mClosed(false),
	mCloseReason(0),
	mCreatedByFactory(false)
{
	clearCommandPtr();
	memset(&mCloseTime, 0, sizeof(mCloseTime));
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
tSocket Conn::makeSocket(const char * port, const char * address, bool udp) {
	if (mSocket > 0) {
		return INVALID_SOCKET; /** Socket is already created */
	}
	mSocket = socketCreate(port, address, udp); /** Create socket */
	mSocket = socketBind(mSocket); /** Bind */
	if (!udp) {
		mSocket = socketListen(mSocket);
		mSocket = socketNonBlock(mSocket);
	}
	mPort = atoi(port); // Set port
	mIp = address; // Set ip (host)
	setOk(mSocket > 0); // Reg conn
	return mSocket;
}

/** Create socket (default TCP) */
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
		if (ErrLog(0)) {
			LogStream() << "Error in getaddrinfo: " << 
			#ifdef _WIN32
				SockErrMsg
			#else
				gai_strerror(ret) << " (" << ret << ")"
			#endif
			<< endl;
		}
		return INVALID_SOCKET;
	}

	if (Log(3)) {
		LogStream() << "Using " << (mAddrInfo->ai_family == AF_INET6 ? "IPv6" : "IPv4") << " socket" << endl;
	}

	// socket
	if (SOCK_INVALID(sock = socket(mAddrInfo->ai_family, mAddrInfo->ai_socktype, 0))) {
		if (ErrLog(0)) {
			LogStream() << "Error in socket: " << SockErrMsg << endl;
		}
		return INVALID_SOCKET;
	}

	if (!udp) {
		sockoptval_t yes = 1;

		// TIME_WAIT after close conn. Reuse address after disconn
		if (SOCK_ERROR(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(sockoptval_t)))) {
			if (ErrLog(0)) {
				LogStream() << "Error in setsockopt: " << SockErrMsg << endl;
			}
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
tSocket Conn::socketBind(tSocket sock) {
	if (sock == INVALID_SOCKET) {
		return INVALID_SOCKET;
	}

	// Bind
	if (SOCK_ERROR(bind(sock, mAddrInfo->ai_addr, mAddrInfo->ai_addrlen))) {
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

	// onClose
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

	freeaddrinfo(mAddrInfo);
	mSocket = 0;
}

/** closeNice */
void Conn::closeNice(int msec /* = 0 */, int reason /* = 0 */) {
	mWritable = false;
	if (reason) {
		mCloseReason = reason;
	}
	if ((msec <= 0) || (!mSendBuf.size())) {
		closeNow(reason);
		return;
	}
	mCloseTime.Get();
	mCloseTime += msec;
}

/** closeNow */
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
			if (Log(3)) {
				LogStream() << "closeNow (reason " << mCloseReason << ")" << endl;
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
			if (Log(3)) {
				LogStream() << "Re-closure (reason " << reason << ")" << endl;
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

	struct sockaddr_storage storage;

	tSocket sock = socketAccept(storage);
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

	if (new_conn->defineConnInfo(storage) == -1) {
		if (mConnFactory != NULL) {
			mConnFactory->deleteConn(new_conn);
		} else {
			delete new_conn;
		}
		return NULL;
	}
	return new_conn;
}

/** Accept new conn */
tSocket Conn::socketAccept(struct sockaddr_storage & storage) {
	int i = 0;
	socklen_t namelen = sizeof(storage);
	memset(&storage, 0, namelen);
	tSocket sock = accept(mSocket, (struct sockaddr *) &storage, (socklen_t*) &namelen);
	while (SOCK_INVALID(sock) && ((SockErr == SOCK_EAGAIN) || (SockErr == SOCK_EINTR)) && (++i <= 10)) {
		/** Try to accept connection not more than 10 once */
		sock = ::accept(mSocket, (struct sockaddr *) &storage, (socklen_t*) &namelen);
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

	sockoptval_t yes = 1;
	if (SOCK_ERROR(setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(int)))) {
		if (ErrLog(1)) {
			LogStream() << "Socket not SO_KEEPALIVE: " << SockErr << endl;
		}
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


int Conn::defineConnInfo(sockaddr_storage & storage) {
	if (mSocket) {
		char host[NI_MAXHOST] = { 0 };
		char port[NI_MAXSERV] = { 0 };
		if (getnameinfo((struct sockaddr *) &storage, sizeof(storage), host, NI_MAXHOST, port, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV) != 0) {
			if (Log(2)) {
				LogStream() << "Error in getnameinfo: " << SockErrMsg << endl;
			}
			closeNow(CLOSE_REASON_GETPEERNAME);
			return -1;
		}
		mIp = host;
		mPort = atoi(port);

/*
		struct sockaddr_in saddr;
		if (getpeername(mSocket, (struct sockaddr *)&saddr, &mSockAddrInSize) < 0) {
			if (Log(2)) {
				LogStream() << "Error in getpeername: " << SockErrMsg << endl;
			}
			closeNow(CLOSE_REASON_GETPEERNAME);
			return -1;
		}
		char ip[INET_ADDRSTRLEN];
		inetNtop(AF_INET, &(saddr.sin_addr), ip, INET_ADDRSTRLEN);
		mIp = ip;
		mPort = ntohs(saddr.sin_port);
*/

		if (mServer->mMac) {
			calcMacAddress();
		}
		return 0;
	}
	return -1;
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
				(struct sockaddr *) &mSockAddrIn,
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
					LogStream() << "Error in receive: " << SockErr << endl;
				}

				switch (SockErr) {

					case ECONNRESET : /** Connection reset by peer */
						if (Log(2)) {
							LogStream() << "(connection reset by peer)" << endl;
						}
						break;

					case ETIMEDOUT : /** Connection timed out */
						if (Log(2)) {
							LogStream() << "(connection timed out)" << endl;
						}
						break;

					case EHOSTUNREACH : /** No route to host */
						if (Log(2)) {
							LogStream() << "(no route to host)" << endl;
						}
						break;

					case EWOULDBLOCK : /** Non-blocking socket operation */
						return -1;

					default :
						if (Log(2)) {
							LogStream() << "(other reason)" << endl;
						}
						break;

				}
				closeNow(CLOSE_REASON_ERROR_RECV);
			}
			return -1;

		}

	} else {
		if (bUdp) {
			mIpUdp = inet_ntoa(mSockAddrIn.sin_addr);
		}
		mRecvBufEnd = iBufLen; /** End buf pos */
		mRecvBufRead = 0; /** Pos for reading from buf */
		mRecvBuf[mRecvBufEnd] = '\0'; /** Adding 0 symbol to end of str */
		mLastRecv.Get(); /** Write time last recv action */
	}
	return iBufLen;
}





bool Conn::isClosed() const {
	return mClosed;
}


//< Get string of IP
const string & Conn::getIp() const {
	return mIp;
}



//< Get string of server IP (host)
const string & Conn::getIpConn() const {
	return mIpConn;
}



//< Get IP for UDP
const string & Conn::getIpUdp() const {
	return mIpUdp;
}



//< Get real port
int Conn::getPort() const {
	return mPort;
}



//< Get connection port
int Conn::getPortConn() const {
	return mPortConn;
}



//< Get mac-address
const string & Conn::getMacAddress() const {
	return mMac;
}



//< Check IP
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



//< Calculate mac-address
void Conn::calcMacAddress() {
#ifdef _WIN32
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
					if (strcmp(mIp.c_str(), address) == 0) {
						char buf[18] = { '\0' };
						sprintf(buf, "%02x-%02x-%02x-%02x-%02x-%02x",
							curr->PhysicalAddress[0], curr->PhysicalAddress[1],
							curr->PhysicalAddress[2], curr->PhysicalAddress[3],
							curr->PhysicalAddress[4], curr->PhysicalAddress[5]);
						mMac = buf;
						free(addr);
						return;
					}
				}
			}
			curr = curr->Next;
		}
	}
	free(addr);
#endif
}




//< Clear params
void Conn::clearCommandPtr() {
	mCommand = NULL;
	mStrStatus = STRING_STATUS_NO_STR;
}



//< Get pointer for string with data
string * Conn::getCommandPtr() {
	return mCommand;
}



/** Installing the string, in which will be recorded received data, 
	and installation main parameter */
void Conn::setCommandPtr(string * pStr) {
	if (mStrStatus != STRING_STATUS_NO_STR) {
		if (ErrLog(0)) {
			LogStream() << "Fatal error: Bad setCommandPtr" << endl;
		}
		throw "Fatal error: Bad setCommandPtr";
	}
	if (!pStr) {
		if (ErrLog(0)) {
			LogStream() << "Fatal error: Bad setCommandPtr. Null string pointer" << endl;
		}
		throw "Fatal error: Bad setCommandPtr. Null string pointer";
	}
	mCommand = pStr;
	mStrStatus = STRING_STATUS_PARTLY;
}



//< Reading data from buffer and record in line of the protocol
int Conn::readFromRecvBuf() {
	if (!mCommand) {
		if (ErrLog(0)) {
			LogStream() << "Fatal error: ReadFromBuf with null string pointer" << endl;
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
	mStrStatus = STRING_STATUS_STR_DONE;
	return len;
}



//< Get pointer for string
string * Conn::getParserCommandPtr() {
	if (mParser == NULL) {
		mParser = createParser();
	}
	if (mParser == NULL) {
		return NULL;
	}
	mParser->reInit();
	return &(mParser->mCommand);
}



Parser * Conn::createParser() {
	if (mProtocol == NULL) {
		return NULL;
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



//< remaining (for web-server)
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



//< Write data in sending buffer and send to conn
int Conn::writeData(const char * data, size_t len, bool flush) {
	size_t bufLen = mSendBuf.size();
	if (bufLen + len >= mSendBufMax) {
		if (Log(0)) {
			LogStream() << "Sending buffer has big size, closing" << endl;
		}
		closeNow(CLOSE_REASON_MAXSIZE_SEND);
		return -1;
	}
	flush = flush || (bufLen > (mSendBufMax >> 1));

	if (!flush) { 
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
		size = len;
		if (size == 0) { // buff is empty and no data
			return 0;
		}
		send_buf = data;
	}

	// Sending
	if (send(send_buf, size) < 0) {

		if (SockErr != SOCK_EAGAIN) {
			if (Log(2)) {
				LogStream() << "Error in sending: " << SockErr << "(not EAGAIN), closing" << endl;
			}
			closeNow(CLOSE_REASON_ERROR_SEND);
			return -1;
		}

		if (Log(3)) {
			LogStream() << "Block sent. Was sent " << size << " bytes" << endl;
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
				if (Log(3)) {
					LogStream() << "Unblock output channel" << endl;
				}
			}

			bufLen = mSendBuf.size();
			if (mBlockInput && bufLen < MAX_SEND_UNBLOCK_SIZE) { // Unset block of input
				mServer->mConnChooser.ConnChoose::optIn(this, ConnChoose::eEF_INPUT);
				if (Log(3)) {
					LogStream() << "Unblock input channel" << endl;
				}
			} else if (!mBlockInput && bufLen >= MAX_SEND_BLOCK_SIZE) { // Set block of input
				mServer->mConnChooser.ConnChoose::optOut(this, ConnChoose::eEF_INPUT);
				if (Log(3)) {
					LogStream() << "Block input channel" << endl;
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
			if (Log(3)) {
				LogStream() << "Block output channel" << endl;
			}
		}
		onFlush();
	}
	return size;
}



//< onFlush
void Conn::onFlush() {
}



//< Flush
void Conn::flush() {
	if (mSendBuf.size() != 0) {
		writeData("", 0, true);
	}
}



//< Send len byte from buf
int Conn::send(const char *buf, size_t &len) {
#ifdef QUICK_SEND // Quick send
	if (mConnType != CONN_TYPE_CLIENTUDP) {
		len = ::send(mSocket, buf, len, 0);
	} else {
		len = ::sendto(mSocket, buf, len, 0, (struct sockaddr *) &mSockAddrIn, mSockAddrInSize);
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
			n = ::sendto(mSocket, buf + total, bytesleft, 0, (struct sockaddr *) &mSockAddrIn, mSockAddrInSize);
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
	len = total; // Number sending bytes
	return SOCK_ERROR(n) ? -1 : 0; // return -1 - fail, 0 - ok
#endif
}



//< Main base timer
int Conn::onTimerBase(Time &now) {
	if (bool(mCloseTime) && mCloseTime > now) {
		closeNow();
		return 0;
	}
	flush();
	onTimer(now);
	return 0;
}



//< Main timer
int Conn::onTimer(Time &) {
	return 0;
}



bool Conn::strLog() {
	Obj::strLog();
	LogStream() << "(sock " << mSocket << ") ";
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

		if (WSAAddressToString(addr, size, NULL, dst, &len) == 0) {
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

/**
 * $Id$
 * $HeadURL$
 */
