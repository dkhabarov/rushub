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

#ifndef CONN_BASE_H
#define CONN_BASE_H

#include <errno.h>

#ifdef _WIN32

	#ifndef FD_SETSIZE // For select
		#ifdef _WIN64
			#define FD_SETSIZE      16384 // also see Times.h
		#else
			#define FD_SETSIZE      32768 // also see Times.h
		#endif
	#endif /* FD_SETSIZE */

	#include <winsock2.h>
	#include <ws2tcpip.h>
	#define socklen_t int ///< unix type for socket (size_t)
	#define sockoptval_t char ///< for setsockopt

	#ifdef ECONNRESET
		#undef ECONNRESET
	#endif
	#ifdef ETIMEDOUT
		#undef ETIMEDOUT
	#endif
	#ifdef EHOSTUNREACH
		#undef EHOSTUNREACH
	#endif
	#ifdef EWOULDBLOCK
		#undef EWOULDBLOCK
	#endif
	#ifndef EAFNOSUPPORT
		#define EAFNOSUPPORT EINVAL
	#endif
	#define ECONNRESET WSAECONNRESET
	#define ETIMEDOUT WSAETIMEDOUT
	#define EHOSTUNREACH WSAEHOSTUNREACH
	#define EWOULDBLOCK WSAEWOULDBLOCK
	#define SOCK_ERR WSAGetLastError()
	#define SOCK_ERR_MSG ""
	#define SOCK_EAGAIN WSAEWOULDBLOCK
	#define SOCK_EINTR WSAEINTR
	#define SOCK_INVALID(SOCK) (SOCK) == INVALID_SOCKET
	#define SOCK_ERROR(SOCK) (SOCK) == SOCKET_ERROR
	typedef SOCKET tSocket;
#else
	#include <arpa/inet.h>
	#include <netinet/in.h> ///< for sockaddr_in
	#include <sys/socket.h> ///< for AF_INET
	#include <netdb.h>      ///< for gethostbyaddr
	#include <fcntl.h>      ///< for nonblock flags F_GETFL & etc
	#define sockoptval_t int
	#define SOCK_ERR errno
	#define SOCK_ERR_MSG strerror(errno)
	#define SOCK_EAGAIN EAGAIN
	#define SOCK_EINTR EINTR
	#define SOCK_INVALID(SOCK) (SOCK) < 0
	#define SOCK_ERROR(SOCK) (SOCK) < 0
	#define INVALID_SOCKET -1
	typedef int tSocket;
#endif

#define SOCK_BACKLOG          0x40     ///< SOMAXCONN
#define MAX_RECV_SIZE         0x02FFFF ///< Max buf size for recv
#define MAX_SEND_SIZE         0x2AFFFF ///< Max buf size for send (by default)
#define MAX_SEND_UNBLOCK_SIZE 0x25FFFF ///< Max size (send) unblock input chanel
#define MAX_SEND_BLOCK_SIZE   0x28FFFF ///< Max size (send) block input chanel

#ifndef TEMP_FAILURE_RETRY
	#define TEMP_FAILURE_RETRY
#endif
#ifndef TEMP_FAILURE_RETRY
	#ifdef _WIN32
		#ifndef __extension__
			#define __extension__
		#endif
		#define TEMP_FAILURE_RETRY(expression) \
		(__extension__ ({ \
			long int __result; \
			while ((__result = (long int) (expression)) == -1L && SOCK_ERR == SOCK_EINTR) { \
			}; \
			__result; \
		}))
	#else
		#define TEMP_FAILURE_RETRY(expression) \
			while ((long int) (expression) == -1L && SOCK_ERR == SOCK_EINTR){ \
			}
	#endif
#endif

#ifdef _WIN32
	#define SOCK_CLOSE(SOCK) \
		TEMP_FAILURE_RETRY(::closesocket(SOCK));
	#define SOCK_NON_BLOCK(SOCK) \
		static unsigned long one = 1; \
		if (ioctlsocket(SOCK, FIONBIO, &one) == SOCKET_ERROR) { \
			return INVALID_SOCKET; \
		}
#else
	#define SOCK_CLOSE(SOCK) \
		TEMP_FAILURE_RETRY(::close(SOCK));
	#define SOCK_NON_BLOCK(SOCK) \
		static int flags; \
		if ((flags = fcntl(SOCK, F_GETFL, 0)) < 0) { \
			return INVALID_SOCKET; \
		} \
		if (fcntl(SOCK, F_SETFL, flags | O_NONBLOCK) < 0) { \
			return INVALID_SOCKET; \
		}
#endif


namespace server {

/** Base connection class */
class ConnBase {

public:

	/** Get socket */
	virtual operator tSocket() const = 0;

}; // class ConnBase

}; // namespace server

#endif // CONN_BASE_H

/**
 * $Id$
 * $HeadURL$
 */
