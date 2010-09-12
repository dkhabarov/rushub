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

#ifndef CONNDEF_H
#define CONNDEF_H

//#if HAVE_ERRNO_H
  #include <errno.h>
//#endif

  

#ifdef _WIN32
  
  #ifndef FD_SETSIZE // For select
    #define FD_SETSIZE      10240//32768
  #endif /* FD_SETSIZE */
  
  #include <winsock2.h>
  #define socklen_t int /** unix type for socket (size_t) */
  #define sockoptval_t char /** for setsockopt */
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
  #define ECONNRESET WSAECONNRESET
  #define ETIMEDOUT WSAETIMEDOUT
  #define EHOSTUNREACH WSAEHOSTUNREACH
  #define EWOULDBLOCK WSAEWOULDBLOCK
  #define SockErr WSAGetLastError()
  #define SOCK_EAGAIN WSAEWOULDBLOCK
  #define SOCK_EINTR WSAEINTR
  typedef SOCKET tSocket;
#else
  #include <arpa/inet.h>
  #include <netinet/in.h> /** for sockaddr_in */
  #include <sys/socket.h> /** for AF_INET */
  #include <netdb.h>      /** for gethostbyaddr */
  #include <fcntl.h>      /** for nonblock flags F_GETFL & etc. */
  #define sockoptval_t int
  #define SockErr errno
  #define SOCK_EAGAIN EAGAIN
  #define SOCK_EINTR EINTR
  #define INVALID_SOCKET -1
  #define SOCKET_ERROR -1
  typedef int tSocket;
#endif

#define SOCK_BACKLOG 0x64 /** SOMAXCONN */

#define MAX_RECV_SIZE 0x02FFFF     /** Max buf size for recv (196607) */
#define MAX_SEND_SIZE 0x17FFFF     /** Max buf size for send (1572863) [0x0FFFFF (1048575)] */
#define MAX_SEND_UNBLOCK_SIZE 0x10FFFF /** Max size (send) unblock input chanel 0x10FFFF [0x07FFFF (524287)] */
#define MAX_SEND_BLOCK_SIZE   0x12FFFF /** Max size (send) block input chanel 0x12FFFF [0x0AFFFF (720895)] */
#define MAX_ATTEMPT_SEND 50 /** MAX_ATTEMPT_SEND */

#ifdef _WIN32
  #define SOCK_CLOSE(SOCK) ::closesocket(SOCK)
  #define SOCK_NON_BLOCK(SOCK) \
    static unsigned long one = 1; \
    if(ioctlsocket(SOCK, FIONBIO, &one) == SOCKET_ERROR) return INVALID_SOCKET;
#else
  #define SOCK_CLOSE(SOCK) ::close(SOCK)
  #define SOCK_NON_BLOCK(SOCK) \
    static int flags; \
    if((flags = fcntl(SOCK, F_GETFL, 0)) < 0) return INVALID_SOCKET; \
    if(fcntl(SOCK, F_SETFL, flags | O_NONBLOCK) < 0) return INVALID_SOCKET;
#endif


#ifndef TEMP_FAILURE_RETRY
  #define TEMP_FAILURE_RETRY(expression) expression
#endif
#ifndef TEMP_FAILURE_RETRY
  #ifdef _WIN32
    #ifndef __extension__
      #define __extension__
    #endif
    #define TEMP_FAILURE_RETRY(expression) \
      (__extension__ ({ long int __result; \
      while ((__result = (long int) (expression)) == -1L && SockErr == SOCK_EINTR){}; __result; }))
  #else
    #define TEMP_FAILURE_RETRY(expression) \
      while ((long int) (expression) == -1L && SockErr == SOCK_EINTR){}
  #endif
#endif

#endif // CONNDEF_H
