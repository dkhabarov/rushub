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

#ifdef _WIN32
	#pragma comment(lib, "ws2_32.lib")
#else
	#include <sys/socket.h>
	#include <unistd.h> // for usleep function
#endif

namespace server {

#ifdef _WIN32
	// Set flag WS
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
	// WSA for windows
	if (!initWSA) {
		WSADATA lpWSAData;
		//WORD wVersionRequested = MAKEWORD(2, 2);

		if (::WSAStartup(/*wVersionRequested*/ 0x0202, &lpWSAData)) {
			printf("Error WSAStartup in WinSock DLL %d\n", ::WSAGetLastError());
			return;
		}

		// DLL support version 2.2
		if (HIBYTE(lpWSAData.wVersion) != 2 || LOBYTE(lpWSAData.wVersion) != 2) {
			printf("Error version WinSock DLL %d.%d != 2.2\n", HIBYTE(lpWSAData.wVersion), LOBYTE(lpWSAData.wVersion));
			::WSACleanup();
			return;
		}

		// WinSock DLL was found
		initWSA = true;
	}
#endif

	mServer = this;

}




////////////////////////////////////////////destructor////////////////////////////////////////////
Server::~Server() {
	mRun = false;
	mNowConn = NULL;

	deleteAll(); // fix me: go up on virtual funcs, that can be destroyed!

#ifdef _WIN32
	// Close WSA DLL lib
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



void Server::deleteAll() {
	for (tCLIt it = mConnList.begin(); it != mConnList.end(); ++it) {
		deleteConn(*it);
	}
	mConnList.clear();
	for (tLLIt it = mListenList.begin(); it != mListenList.end(); ++it) {
		(*it)->mProtocol = NULL; // upd hack: fix me!
		deleteConn(*it);
	}
	mListenList.clear();
}


void Server::deleteConn(Conn * conn) {
	if (conn != NULL) {
		mConnChooser.deleteConn(conn);

		ConnFactory * selfConnFactory = conn->mSelfConnFactory;
		if (selfConnFactory != NULL) {
			selfConnFactory->deleteConn(conn);
		} else {
			delete conn;
		}
		conn = NULL;
	}
}



/// Set and Listen port
Conn * Server::listening(ConnFactory * connFactory, const char * ip, const char * port, bool udp /*= false*/) {
	Conn * conn = listen(ip, port, udp);
	if (conn == NULL) {
		return NULL;
	}

	// Set server listen factory
	conn->mCreatorConnFactory = connFactory;

	// Set protocol for Conn without factory
	conn->mProtocol = connFactory->mProtocol;

	return conn;
}



/// Set and Connect to port
Conn * Server::connecting(ConnFactory * connFactory, const char * ip, const char * port, bool udp /*= false*/) {
	Conn * conn = connect(ip, port, udp);
	if (conn == NULL) {
		return NULL;
	}

	// Set server listen factory
	conn->mCreatorConnFactory = connFactory;

	// Set protocol for Conn without factory
	conn->mProtocol = connFactory->mProtocol;

	return conn;
}



/// Listen port (TCP/UDP)
Conn * Server::listen(const char * ip, const char * port, bool udp) {

	ConnType connType = udp ? CONN_TYPE_INCOMING_UDP : CONN_TYPE_LISTEN;
	Conn * conn = new Conn(0, this, connType);

	if (!addSimpleConn(conn, ip, port, connType)) { // Listen conn
		delete conn;
		conn = NULL;
	}
	return conn;
}



/// Connect to port (TCP/UDP)
Conn * Server::connect(const char * ip, const char * port, bool udp) {

	ConnType connType = udp ? CONN_TYPE_OUTGOING_UDP : CONN_TYPE_OUTGOING_TCP;
	Conn * conn = new Conn(0, this, connType);

	if (!addSimpleConn(conn, ip, port, connType)) { // Listen conn
		delete conn;
		conn = NULL;
	}
	return conn;
}



/// Add simple connection
Conn * Server::addSimpleConn(Conn * conn, const char * ip, const char * port, int connType) {
	if (conn) {
		if (conn->makeSocket(port, ip, connType) == INVALID_SOCKET) {
			if (ErrLog(0)) {
				if (connType == CONN_TYPE_LISTEN) {
					LogStream() << "Fatal error: Can't listen on " << ip << ":" << port << " TCP" << endl;
				} else if (connType == CONN_TYPE_INCOMING_UDP) {
					LogStream() << "Fatal error: Can't listen on " << ip << ":" << port << " UDP" << endl;
				} else if (connType == CONN_TYPE_OUTGOING_TCP) {
					LogStream() << "Fatal error: Can't connect to " << ip << ":" << port << " TCP" << endl;
				} else if (connType == CONN_TYPE_OUTGOING_UDP) {
					LogStream() << "Fatal error: Can't connect to " << ip << ":" << port << " UDP" << endl;
				} else {
					LogStream() << "Fatal error: Unknown connection" << endl;
				}
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
			if (connType == CONN_TYPE_LISTEN) {
				LogStream() << "Listening on " << ip << ":" << port << " TCP" << endl;
			} else if (connType == CONN_TYPE_INCOMING_UDP) {
				LogStream() << "Listening on " << ip << ":" << port << " UDP" << endl;
			} else if (connType == CONN_TYPE_OUTGOING_TCP) {
				LogStream() << "Connected to " << ip << ":" << port << " TCP" << endl;
			} else if (connType == CONN_TYPE_OUTGOING_UDP) {
				LogStream() << "Connected to " << ip << ":" << port << " UDP" << endl;
			}
		}
		return conn;
	} else {
		if (ErrLog(0)) {
			LogStream() << "Fatal error: Connection object is empty" << endl;
		}
	}
	return NULL;
}



/// Main cycle
int Server::run() {
	// mRun = true; // by default server was run
	if (Log(1)) {
		LogStream() << "Main loop start" << endl;
	}

	while (mRun) { // Main cycle
		try {
			mTime.get(); // Current time
			step(); // Server's step

			// Timers (100 msec)
			__int64 msec = mTime.msec();
			if (msec > mTimes.mServ ? msec - mTimes.mServ >= 100 : mTimes.mServ - msec >= 100) { // Transfer of time
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
		} catch(const char * exception) {
			if (ErrLog(0)) {
				LogStream() << "Exception: " << exception << endl;
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



/// Stop main cycle
void Server::stop(int code) {
	mRun = false;
	mMainLoopCode = code; // 1 - restart
}



/// Step
void Server::step() {
	int ret;
	static Time tmout(0, 3000l); // timeout 3 msec

	try {
		// Checking the arrival data in listen sockets
		ret = mConnChooser.choose(tmout);
		if (ret <= 0 && !miNumCloseConn) { 
			#ifdef _WIN32
				Sleep(1);
			#else
				usleep(1000);
			#endif
			return;
		}
	} catch(const char * exception) {
		if (ErrLog(0)) {
			LogStream() << "Exception in choose: " << exception << endl;
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

	for (ChooserIterator it = mConnChooser.begin(); it != mConnChooser.end();) {
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

			int numAccept = 0;
			Conn * new_conn = NULL;

			// accept before 64 connections for once
			while (++numAccept <= 64) {
			
				// Create new connection:
				// 1. Accept new socket
				// 2. Create new connection object (using ConnFactory from ListenFactory else create simple Conn)
				new_conn = mNowConn->createNewConn(); /** CONN_TYPE_INCOMING_TCP (Conn) */

				if (new_conn) {
					if (addConnection(new_conn) > 0) {

						//if (inputData(new_conn) >= 0) { // fix close conn if not recv data

							if (mNowConn->mCreatorConnFactory) {
								// On new connection using ListenFactory
								mNowConn->mCreatorConnFactory->onNewConn(new_conn);
							} else {
								if (Log(4)) {
									LogStream() << "ListenFactory is empty" << endl;
								}

								// On new connection by server
								onNewConn(new_conn);
							}

						//}

					}
				} else {
					break;
				}

			}

			if (mNowConn->Log(5)) {
				mNowConn->LogStream() << "::(e)NewConn. Number connections: " << mConnChooser.mConnBaseList.size() << endl;
			}

		} else { // not listening socket (optimization)

			if (ok && (activity & ConnChoose::eEF_INPUT) && (
					connType == CONN_TYPE_INCOMING_TCP ||
					connType == CONN_TYPE_INCOMING_UDP ||
					connType == CONN_TYPE_OUTGOING_TCP ||
					connType == CONN_TYPE_OUTGOING_UDP
			)) {
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
				} catch (const char * exception) {
					if (ErrLog(0)) {
						LogStream() << "Exception in inputData: " << exception << endl;
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
				} catch (const char * exception) {
					if (ErrLog(0)) {
						LogStream() << "Exception in outputData: " << exception << endl;
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
				} catch (const char * exception) {
					if (ErrLog(0)) {
						LogStream() << "Exception in delConnection: " << exception << endl;
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

int Server::addConnection(Conn * conn) {

	if (!conn->isOk()) {
		if (conn->Log(2)) {
			conn->LogStream() << "Not reserved connection: " << conn->getIp() << endl;
		}
		if (conn->mSelfConnFactory != NULL) {
			conn->mSelfConnFactory->deleteConn(conn);
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
		if (conn->mSelfConnFactory != NULL) {
			conn->mSelfConnFactory->deleteConn(conn);
		} else {
			delete conn;
		}
		return -2;
	}

	mConnChooser.addConn(conn);
	conn->mIterator = mConnList.insert(mConnList.begin(), conn);
	conn->mPortConn = mNowConn->mPort;
	conn->mIpConn = mNowConn->mIp;

	/*if (Log(4)) {
		LogStream() << "Num clients after add: " << mConnList.size() << ". Num socks: " << mConnChooser.mConnBaseList.size() << endl;
	}*/
	return 1;
}



/// delConnection
int Server::delConnection(Conn * conn) {
	if (conn == NULL) {
		if (mNowConn && mNowConn->ErrLog(0)) {
			mNowConn->LogStream() << "Fatal error: delConnection null pointer" << endl;
		}
		throw "Fatal error: delConnection null pointer";
	}

	/*if (Log(4)) {
		LogStream() << "Num clients before del: " << mConnList.size() << ". Num socks: " << mConnChooser.mConnBaseList.size() << endl;
	}
	if (Log(4)) {
		LogStream() << "Delete connection on socket: " << (tSocket)(*conn) << endl;
	}*/

	tCLIt it = conn->mIterator;
	Conn *found = (*it);
	if ((it == mConnList.end()) || (found != conn)) {
		if (conn->ErrLog(0)) {
			conn->LogStream() << "Fatal error: Delete unknown connection: " << conn << endl;
		}
		throw "Fatal error: Delete unknown connection";
	}

	mConnList.erase(it);
	tCLIt empty_it;
	conn->mIterator = empty_it;

	mConnChooser.deleteConn(conn);

	if (conn->mSelfConnFactory != NULL) {
		conn->mSelfConnFactory->deleteConn(conn); 
	} else {
		if (conn->Log(0)) {
			conn->LogStream() << "Deleting conn without factory!" << endl;
		}
		delete conn;
	}

	/*if (Log(4)) {
		LogStream() << "Num clients after del: " << mConnList.size() << ". Num socks: " << mConnChooser.mConnBaseList.size() << endl;
	}*/
	return 1;
}



/// onNewConn
int Server::onNewConn(Conn * conn) {
	return conn == NULL ? -1 : 0;
}



/// onClose
void Server::onClose(Conn * conn) {
	if (!conn) {
		return;
	}
	mConnChooser.deleteConn(conn);
}



/// inputData
size_t Server::inputData(Conn * conn) {
	try {
		if (conn->recv() <= 0) {
			return 0;
		}
	} catch(const char * exception) {
		if (ErrLog(0)) {
			LogStream() << "Exception in recv: " << exception << endl;
		}
		return 0;
	} catch(...) {
		if (ErrLog(0)) {
			LogStream() << "Exception in recv" << endl;
		}
		return 0;
	}

	size_t bytes = 0;
	while (conn->isOk() && conn->isWritable()) {
		if (conn->getStatus() == STRING_STATUS_NO_STR) {
			conn->setCommandPtr(createCommandPtr(conn));
		}

		bytes += conn->readFromRecvBuf();

		if (conn->getStatus() == STRING_STATUS_STR_DONE) {

			if (conn->mSelfConnFactory != NULL) {
				// On new data using selfConnFactory
				conn->mSelfConnFactory->onNewData(conn, conn->getCommandPtr());
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



/// createCommandPtr
string * Server::createCommandPtr(Conn *) {
	return new string;
}



/// onNewData
void Server::onNewData(Conn *, string * str) {
	delete str;
}



/// outputData
void Server::outputData(Conn * conn) {
	conn->flush();
}



/// Main mase timer
void Server::onTimerBase(Time & now) {
	onTimer(now);
	if (
		mTimes.mServ > mTimes.mConn ? 
		(mTimes.mServ - mTimes.mConn >= mTimerConnPeriod) :
		(mTimes.mConn - mTimes.mServ >= mTimerConnPeriod)
	) {
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
}



/// Main timer
int Server::onTimer(Time &) {
	return 0;
}


}; // server

/**
 * $Id$
 * $HeadURL$
 */
