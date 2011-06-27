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

#include "Server.h"
#include "Conn.h" // Conn, ConnFactory

#include <iostream> // cout, endl
#include <math.h> // abs

#ifdef _WIN32
	#pragma comment(lib, "ws2_32.lib")
#else
	#include <sys/socket.h>
	#include <unistd.h> // for usleep function
	#include <cstdlib> // for abs
#endif

namespace server {

#ifdef _WIN32
	/** Set flag WS */
	bool Server::initWSA = false;
#endif


//////////////////////////////////////////////constructor////////////////////////////////////////
Server::Server() :
	Obj("Server", true),
	mStepDelay(0),
	mTimerServPeriod(1000),
	mTimerConnPeriod(4000),
	mMac(true),
	mRun(true), // run by default
	mMainLoopCode(0),
	miNumCloseConn(0),
	mMeanFrequency(mTime, 90.0, 20),
	mServer(NULL),
	mNowConn(NULL)
{

#ifdef _WIN32
	/** WSA for windows */
	if (!initWSA) {
		WSADATA lpWSAData;
		//WORD wVersionRequested = MAKEWORD(2, 2);

		if (::WSAStartup(/*wVersionRequested*/ 0x0202, &lpWSAData)) {
			printf("Error WSAStartup in WinSock DLL %d\n", ::WSAGetLastError());
			return;
		}

		/** DLL support version 2.2 */
		if (HIBYTE(lpWSAData.wVersion) != 2 || LOBYTE(lpWSAData.wVersion) != 2) {
			printf("Error version WinSock DLL %d.%d != 2.2\n", HIBYTE(lpWSAData.wVersion), LOBYTE(lpWSAData.wVersion));
			::WSACleanup();
			return;
		}

		/** WinSock DLL was found */
		initWSA = true;
	}
#endif

	mServer = this;

}


////////////////////////////////////////////destructor////////////////////////////////////////////
Server::~Server() {
	mNowConn = NULL;
#ifdef _WIN32
	/** Close WSA DLL lib */
	::WSACleanup();
	initWSA = false;
#endif
	if (Log(1)) {
		LogStream() << endl << "Allocated objects: " << Obj::GetCount()
		<< endl << "Unclosed sockets: " << Conn::mConnCounter << endl;
	}
	if (mOfs.is_open()) {
		mOfs.close();
	}
}

/** Set and Listen port */
int Server::listening(ConnFactory * connFactory, const char * ip, const char * port, bool udp /*= false*/) {
	Conn * conn = listen(ip, port, udp);
	if (conn == NULL) {
		return -1;
	}

	// Set server listen factory
	conn->mConnFactory = connFactory;

	// Set protocol for UDP conn without factory
	conn->mProtocol = connFactory->mProtocol;

	return 0;
}



/** Listen port (TCP/UDP) */
Conn * Server::listen(const char * ip, const char * port, bool udp) {
	Conn * conn = NULL;

	if (!udp) { // Create socket listening server for TCP port
		conn = new Conn(0, this, CONN_TYPE_LISTEN);
	} else { // Create socket server for UDP port
		conn = new Conn(0, this, CONN_TYPE_CLIENTUDP);
	}

	if (!addListen(conn, ip, port, udp)) { // Listen conn
		delete conn;
		return NULL;
	}
	return conn;
}



/** Create, bind and add connection for port */
Conn * Server::addListen(Conn * conn, const char * ip, const char * port, bool udp) {
	// Socket object was created
	if (conn) {
		if (conn->makeSocket(port, ip, udp) == INVALID_SOCKET) {
			if (ErrLog(0)) {
				LogStream() << "Fatal error: Can't listen on " << ip << ":" << port << (udp ? " UDP" : " TCP") << endl;
			}
			return NULL;
		}

		mListenList.insert(mListenList.begin(), conn);
		mConnChooser.addConn(conn);

		if (!mConnChooser.ConnChoose::optIn(
			static_cast<ConnBase *> (conn),
			ConnChoose::tEventFlag(ConnChoose::eEF_INPUT | ConnChoose::eEF_ERROR))) {
			if (ErrLog(0)) {
				LogStream() << "Error: Can't add socket" << endl;
			}
			delete conn;
			return NULL;
		}

		if (Log(0)) {
			LogStream() << "Listening on " << string(ip) << ":" << string(port) << (udp ? " UDP" : " TCP") << endl;
		}
		return conn;
	} else {
		if (ErrLog(0)) {
			LogStream() << "Fatal error: Can't create conn object" << endl;
		}
	}
	return NULL;
}

/** StopListen */
bool Server::stopListen(Conn * conn) {
	if (conn) {
		mConnChooser.deleteConn(conn);
		return true;
	}
	return false;
}



// Main cycle
int Server::run() {
	// mRun = true; // by default server was run
	if (Log(1)) {
		LogStream() << "Main loop start" << endl;
	}

	while (mRun) { // Main cycle
		try {
			mTime.Get(); // Current time
			step(); // Server's step

			// Timers (100 msec)
			long msec = mTime.MiliSec();
			if (abs(msec - mTimes.mServ) >= 100) { // Transfer of time
				mTimes.mServ = msec;
				onTimerBase(mTime);
			}

			if (mStepDelay) {
				#ifdef _WIN32
					Sleep(mStepDelay); // (mStepDelay msec)
				#else
					usleep(mStepDelay * 1000);
				#endif
			}
			mMeanFrequency.insert(mTime); // MeanFrequency
		} catch(const char *str) {
			if (ErrLog(0)) {
				LogStream() << "Exception: " << str << endl;
			}
		} catch(...) {
			if (ErrLog(0)) {
				LogStream() << "Exception in Run function" << endl;
			}
			throw "Server::run()";
		}
	}
	if (Log(1)) {
		LogStream() << "Main loop stop(" << mMainLoopCode << ")" << endl;
	}
	return mMainLoopCode;
}

/** Stop main cycle */
void Server::stop(int code) {
	mRun = false;
	mMainLoopCode = code; // 1 - restart
}

/** step */
void Server::step() {
	int ret;
	static Time tmout(0, 1000l); // timeout 1 msec

	try {
		// Checking the arrival data in listen sockets
		ret = mConnChooser.choose(tmout);
		if (ret <= 0 && !miNumCloseConn) { 
			#ifdef _WIN32
				//Sleep(0);
			#else
				usleep(50); // 50 usec
			#endif
			return;
		}
	} catch(const char *str) {
		if (ErrLog(0)) {
			LogStream() << "Exception in choose: " << str << endl;
		}
		return;
	} catch(...) {
		if (ErrLog(0)) {
			LogStream() << "Exception in choose" << endl;
		}
		throw "Exception in choose";
	}

	if (Log(5)) {
		LogStream() << "<new actions>: " << ret << " [" << miNumCloseConn << "]" << endl;
	}

	ConnChoose::ChooseRes res;
	ConnType connType;
	bool ok = false;
	int activity = 0;
	int forDel = miNumCloseConn;

	for (tChIt it = mConnChooser.begin(); it != mConnChooser.end();) {
		res = (*it);
		++it;

		if ((mNowConn = static_cast<Conn *> (res.mConnBase)) == NULL) {
			continue;
		}
		activity = res.mRevents;
		ok = mNowConn->isOk();
		connType = mNowConn->getConnType();

		if (
			ok && 
			(activity & ConnChoose::eEF_INPUT) && 
			(connType == CONN_TYPE_LISTEN)
		) {

			if (mNowConn->Log(5)) {
				mNowConn->LogStream() << "::(s)NewConn" << endl;
			}

			// Create new connection:
			// 1. Accept new socket
			// 2. Create new connection object (using ConnFactory from ListenFactory else create simple Conn)
			Conn * new_conn = mNowConn->createNewConn(); /** CONN_TYPE_CLIENTTCP (Conn) */

			if (new_conn) {
				if (addConnection(new_conn) > 0) {

					//if (inputData(new_conn) >= 0) { // fix close conn if not recv data

						if (mNowConn->mConnFactory) {
							// On new connection using ListenFactory
							mNowConn->mConnFactory->onNewConn(new_conn);
						} else {
							if (Log(4)) {
								LogStream() << "ListenFactory is empty" << endl;
							}

							// On new connection by server
							onNewConn(new_conn);
						}

					//}

				}
			}
			if (mNowConn->Log(5)) {
				mNowConn->LogStream() << "::(e)NewConn. Number connections: " << mConnChooser.mConnBaseList.size() << endl;
			}

		} else { // not listening socket (optimization)

			if (ok && (activity & ConnChoose::eEF_INPUT) && ((connType == CONN_TYPE_CLIENTTCP) || (connType == CONN_TYPE_CLIENTUDP))) {
				try {
					if (mNowConn->Log(5)) {
						mNowConn->LogStream() << "::(s)inputData" << endl;
					}

					if (inputData(mNowConn) <= 0) {
						mNowConn->setOk(false);
					}

					if (mNowConn->Log(5)) {
						mNowConn->LogStream() << "::(e)inputData" << endl;
					}
				} catch (const char * str) {
					if (ErrLog(0)) {
						LogStream() << "Exception in inputData: " << str << endl;
					}
					throw "Exception in inputData";
				} catch (...) {
					if (ErrLog(0)) {
						LogStream() << "Exception in inputData" << endl;
					}
					throw "Exception in inputData";
				}
			}

			if (ok && (activity & ConnChoose::eEF_OUTPUT)) {
				try {
					if (mNowConn->Log(5)) {
						mNowConn->LogStream() << "::(s)outputData" << endl;
					}

					outputData(mNowConn);

					if (mNowConn->Log(5)) {
						mNowConn->LogStream() << "::(e)outputData" << endl;
					}
				} catch (const char * str) {
					if (ErrLog(0)) {
						LogStream() << "Exception in outputData: " << str << endl;
					}
					throw "Exception in outputData";
				} catch (...) {
					if (ErrLog(0)) {
						LogStream() << "Exception in outputData" << endl;
					}
					throw "Exception in outputData";
				}
			}

			if (!ok || (activity & (ConnChoose::eEF_ERROR | ConnChoose::eEF_CLOSE))) {

				forDel = 0; // tmp

				if (mNowConn->isClosed()) { // check close flag
					--miNumCloseConn;
				}

				try {
					if (Log(5)) {
						LogStream() << "::(s)delConnection" << endl;
					}

					delConnection(mNowConn);

					if (Log(5)) {
						LogStream() << "::(e)delConnection. Number connections: " << mConnChooser.mConnBaseList.size() << endl;
					}
				} catch (const char * str) {
					if (ErrLog(0)) {
						LogStream() << "Exception in delConnection: " << str << endl;
					}
					throw "Exception in delConnection";
				} catch (...) {
					if (ErrLog(0)) {
						LogStream() << "Exception in delConnection" << endl;
					}
					throw "Exception in delConnection";
				}
			}
		}
	}

	if (miNumCloseConn && forDel) {
		if (ErrLog(1)) {
			LogStream() << "Control not closed connections: " << miNumCloseConn << endl;
		}
		--miNumCloseConn;
	}

	if (Log(5)) {
		LogStream() << "<exit actions>" << endl;
	}
}

///////////////////////////////////add_connection/del_connection///////////////////////////////////

int Server::addConnection(Conn *conn) {

	if (!conn->isOk()) {
		if (conn->Log(2)) {
			conn->LogStream() << "Not reserved connection: " << conn->getIp() << endl;
		}
		if (conn->mConnFactory != NULL && conn->getCreatedByFactory()) {
			conn->mConnFactory->deleteConn(conn);
		} else {
			if (conn->Log(2)) {
				conn->LogStream() << "Connection without factory: " << conn->getIp() << endl;
			}
			delete conn;
		}
		return -1;
	}

	/*if (Log(4)) {
		LogStream() << "Num clients before add: " << mConnList.size() << ". Num socks: " << mConnChooser.mConnBaseList.size() << endl;
	}*/

	if (
		#if USE_SELECT
			(mConnChooser.size() == (FD_SETSIZE - 1)) || 
		#endif
		!mConnChooser.ConnChoose::optIn(static_cast<ConnBase *> (conn),
		ConnChoose::tEventFlag(ConnChoose::eEF_INPUT | ConnChoose::eEF_ERROR)))
	{
		if (conn->ErrLog(0)) {
			conn->LogStream() << "Error: Can't add socket!" << endl;
		}
		if (conn->mConnFactory != NULL && conn->getCreatedByFactory()) {
			conn->mConnFactory->deleteConn(conn);
		} else {
			delete conn;
		}
		return -2;
	}

	mConnChooser.addConn(conn);

	tCLIt it = mConnList.insert(mConnList.begin(), conn);
	conn->mIterator = it;

	/*if (Log(4)) {
		LogStream() << "Num clients after add: " << mConnList.size() << ". Num socks: " << mConnChooser.mConnBaseList.size() << endl;
	}*/

	conn->mPortConn = mNowConn->mPort;
	conn->mIpConn = mNowConn->mIp;
	return 1;
}

/** delConnection(Conn *old_conn) */
int Server::delConnection(Conn *old_conn) {
	if (!old_conn) {
		if (mNowConn && mNowConn->ErrLog(0)) {
			mNowConn->LogStream() << "Fatal error: delConnection null pointer" << endl;
		}
		throw "Fatal error: delConnection null pointer";
	}

	/*if (Log(4)) {
		LogStream() << "Num clients before del: " << mConnList.size() << ". Num socks: " << mConnChooser.mConnBaseList.size() << endl;
	}
	if (Log(4)) {
		LogStream() << "Delete connection on socket: " << (tSocket)(*old_conn) << endl;
	}*/

	tCLIt it = old_conn->mIterator;
	Conn *found = (*it);
	if ((it == mConnList.end()) || (found != old_conn)) {
		if (old_conn->ErrLog(0)) {
			old_conn->LogStream() << "Fatal error: Delete unknown connection: " << old_conn << endl;
		}
		throw "Fatal error: Delete unknown connection";
	}

	mConnList.erase(it);
	tCLIt empty_it;
	old_conn->mIterator = empty_it;

	mConnChooser.deleteConn(old_conn);

	if (old_conn->mConnFactory != NULL && old_conn->getCreatedByFactory()) {
		old_conn->mConnFactory->deleteConn(old_conn); 
	} else {
		if (old_conn->Log(0)) {
			old_conn->LogStream() << "Deleting conn without factory!" << endl;
		}
		delete old_conn;
	}

	/*if (Log(4)) {
		LogStream() << "Num clients after del: " << mConnList.size() << ". Num socks: " << mConnChooser.mConnBaseList.size() << endl;
	}*/
	return 1;
}

/** onNewConn */
int Server::onNewConn(Conn *conn) {
	if (!conn) {
		return -1;
	}
	return 0;
}

/** onClose */
void Server::onClose(Conn *conn) {
	if (!conn) {
		return;
	}
	mConnChooser.deleteConn(conn);
}

/** inputData */
int Server::inputData(Conn *conn) {
	try {
		if (conn->recv() <= 0) {
			return 0;
		}
	} catch(const char *str) {
		if (ErrLog(0)) {
			LogStream() << "Exception in recv: " << str << endl;
		}
		return 0;
	} catch(...) {
		if (ErrLog(0)) {
			LogStream() << "Exception in recv" << endl;
		}
		return 0;
	}

	int bytes = 0;
	while (conn->isOk() && conn->isWritable()) {
		if (conn->getStatus() == STRING_STATUS_NO_STR) {
			conn->setCommandPtr(createCommandPtr(conn));
		}

		bytes += conn->readFromRecvBuf();

		if (conn->getStatus() == STRING_STATUS_STR_DONE) {

			if (conn->mConnFactory != NULL) {
				// On new data using ListenFactory
				conn->mConnFactory->onNewData(conn, conn->getCommandPtr());
			} else {
				// On new data by server
				onNewData(conn, conn->getCommandPtr());
			}

			conn->clearCommandPtr();
		}

		if (conn->recvBufIsEmpty()) {
			break;
		}
	}
	return bytes;
}

/** createCommandPtr */
string * Server::createCommandPtr(Conn *) {
	return new string;
}

/** onNewData */
void Server::onNewData(Conn *, string * str) {
	delete str;
}

/** outputData */
int Server::outputData(Conn *conn) {
	conn->flush();
	return 0;
}

/** Main mase timer */
int Server::onTimerBase(Time & now) {
	onTimer(now);
	if (abs(mTimes.mServ - mTimes.mConn) >= mTimerConnPeriod) {
		mTimes.mConn = mTimes.mServ;
		tCLIt it_e = mConnList.end();
		Conn * conn = NULL;
		for (tCLIt it = mConnList.begin(); it != it_e; ++it) {
			conn = (*it);
			if (conn->isOk()) {
				conn->onTimerBase(now);
			}
		}
	}
	return 0;
}

/** Main timer */
int Server::onTimer(Time &) {
	return 0;
}


}; // server

/**
 * $Id$
 * $HeadURL$
 */
