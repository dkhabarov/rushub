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

	if (log(1)) {
		logStream() << endl << "Allocated objects: " << Obj::getCount()
		<< endl << "Unclosed sockets: " << Conn::mConnCounter << endl;
	}
	if (mOfs.is_open()) {
		mOfs.close();
	}
}



void Server::deleteAll() {
	Conn * conn = NULL;

	tCLIt it = mClientList.begin();
	tCLIt it_e = mClientList.end();
	while (it != it_e) {
		conn = (*it++);
		delConnection(conn);
	}
	mClientList.clear();

	it = mListenList.begin();
	it_e = mListenList.end();
	while (it != it_e) {
		conn = (*it++);
		conn->mProtocol = NULL; // upd hack: fix me!
		delConnection(conn);
	}
	mListenList.clear();
}



/// Set and Listen port
Conn * Server::listening(ConnFactory * connFactory, const char * ip, const char * port, bool udp /*= false*/) {

	ConnType connType = udp ? CONN_TYPE_INCOMING_UDP : CONN_TYPE_LISTEN;
	Conn * conn = addSimpleConn(connType, ip, port);

	if (conn == NULL) {
		delete conn;
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

	ConnType connType = udp ? CONN_TYPE_OUTGOING_UDP : CONN_TYPE_OUTGOING_TCP;
	Conn * conn = addSimpleConn(connType, ip, port);

	if (conn == NULL) {
		delete conn;
		return NULL;
	}

	// Set server listen factory
	conn->mCreatorConnFactory = connFactory;

	// Set protocol for Conn without factory
	conn->mProtocol = connFactory->mProtocol;

	return conn;
}



/// Add simple connection
Conn * Server::addSimpleConn(int connType, const char * ip, const char * port) {

	Conn * conn = new Conn(0, this, connType);

	if (conn->makeSocket(port, ip, connType) == INVALID_SOCKET) {
		if (errLog(0)) {
			if (connType == CONN_TYPE_LISTEN) {
				logStream() << "Fatal error: Can't listen on " << ip << ":" << port << " TCP" << endl;
			} else if (connType == CONN_TYPE_INCOMING_UDP) {
				logStream() << "Fatal error: Can't listen on " << ip << ":" << port << " UDP" << endl;
			} else if (connType == CONN_TYPE_OUTGOING_TCP) {
				logStream() << "Fatal error: Can't connect to " << ip << ":" << port << " TCP" << endl;
			} else if (connType == CONN_TYPE_OUTGOING_UDP) {
				logStream() << "Fatal error: Can't connect to " << ip << ":" << port << " UDP" << endl;
			} else {
				logStream() << "Fatal error: Unknown connection" << endl;
			}
		}
		return NULL;
	}

	if (addConnection(conn) < 0) {
		return NULL;
	}

	if (connType == CONN_TYPE_LISTEN) {
		if (log(0)) {
			logStream() << "Listening on " << ip << ":" << port << " TCP" << endl;
		}
	} else if (connType == CONN_TYPE_INCOMING_UDP) {
		if (log(0)) {
			logStream() << "Listening on " << ip << ":" << port << " UDP" << endl;
		}
	} else if (connType == CONN_TYPE_OUTGOING_TCP) {
		if (log(4)) {
			logStream() << "Connected to " << ip << ":" << port << " TCP" << endl;
		}
	} else if (connType == CONN_TYPE_OUTGOING_UDP) {
		if (log(4)) {
			logStream() << "Connected to " << ip << ":" << port << " UDP" << endl;
		}
	}
	return conn;
}



/// Main cycle
int Server::run() {
	// mRun = true; // by default server was run
	if (log(1)) {
		logStream() << "Main loop start" << endl;
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
			if (errLog(0)) {
				logStream() << "Exception: " << exception << endl;
			}
		} catch(...) {
			if (errLog(0)) {
				logStream() << "Exception in Run function" << endl;
			}
			throw "Server::run()";
		}
	}
	if (log(1)) {
		logStream() << "Main loop stop(" << mMainLoopCode << ")" << endl;
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
		if (errLog(0)) {
			logStream() << "Exception in choose: " << exception << endl;
		}
		return;
	} catch(...) {
		if (errLog(0)) {
			logStream() << "Exception in choose" << endl;
		}
		throw "Exception in choose";
	}

	if (log(5)) {
		logStream() << "<new actions>: " << ret << " [" << miNumCloseConn << "]" << endl;
	}

	ConnChoose::ChooseRes res;
	int connType;
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

			if (mNowConn->log(5)) {
				mNowConn->logStream() << "::(s)NewConn" << endl;
			}

			int numAccept = 0;

			// accept before 64 connections for once
			while (++numAccept <= 64) {
				if (newAccept() == 0) {
					break;
				}
			}

			if (mNowConn->log(5)) {
				mNowConn->logStream() << "::(e)NewConn. Number connections: " << mConnChooser.mConnBaseList.size() << endl;
			}

		} else { // not listening socket (optimization)

			if (ok && (activity & ConnChoose::eEF_INPUT) && (
					connType == CONN_TYPE_INCOMING_TCP ||
					connType == CONN_TYPE_INCOMING_UDP ||
					connType == CONN_TYPE_OUTGOING_TCP ||
					connType == CONN_TYPE_OUTGOING_UDP
			)) {
				try {
					if (mNowConn->log(5)) {
						mNowConn->logStream() << "::(s)inputEvent" << endl;
					}

					if (inputEvent(mNowConn) == 0) {
						mNowConn->setOk(false);
					}

					if (mNowConn->log(5)) {
						mNowConn->logStream() << "::(e)inputEvent" << endl;
					}
				} catch (const char * exception) {
					if (errLog(0)) {
						logStream() << "Exception in inputEvent: " << exception << endl;
					}
					throw "Exception in inputEvent";
				} catch (...) {
					if (errLog(0)) {
						logStream() << "Exception in inputEvent" << endl;
					}
					throw "Exception in inputEvent";
				}
			}

			if (ok && (activity & ConnChoose::eEF_OUTPUT)) {
				try {
					if (mNowConn->log(5)) {
						mNowConn->logStream() << "::(s)outputEvent" << endl;
					}

					outputEvent(mNowConn);

					if (mNowConn->log(5)) {
						mNowConn->logStream() << "::(e)outputEvent" << endl;
					}
				} catch (const char * exception) {
					if (errLog(0)) {
						logStream() << "Exception in outputEvent: " << exception << endl;
					}
					throw "Exception in outputEvent";
				} catch (...) {
					if (errLog(0)) {
						logStream() << "Exception in outputEvent" << endl;
					}
					throw "Exception in outputEvent";
				}
			}

			if (!ok || (activity & (ConnChoose::eEF_ERROR | ConnChoose::eEF_CLOSE))) {

				forDel = 0; // tmp

				if (mNowConn->isClosed()) { // check close flag
					--miNumCloseConn;
				}

				try {
					if (log(5)) {
						logStream() << "::(s)delConnection" << endl;
					}

					delConnection(mNowConn);

					if (log(5)) {
						logStream() << "::(e)delConnection. Number connections: " << mConnChooser.mConnBaseList.size() << endl;
					}
				} catch (const char * exception) {
					if (errLog(0)) {
						logStream() << "Exception in delConnection: " << exception << endl;
					}
					throw "Exception in delConnection";
				} catch (...) {
					if (errLog(0)) {
						logStream() << "Exception in delConnection" << endl;
					}
					throw "Exception in delConnection";
				}
			}
		}
	}

	if (miNumCloseConn && forDel) {
		if (errLog(1)) {
			logStream() << "Control not closed connections: " << miNumCloseConn << endl;
		}
		--miNumCloseConn;
	}

	if (log(5)) {
		logStream() << "<exit actions>" << endl;
	}
}



int Server::newAccept() {

	// Create new connection:
	// 1. Accept new socket
	// 2. Create new connection object (using mCreatorConnFactory else create simple Conn)
	Conn * newConn = NULL;
	if ((newConn = mNowConn->createNewConn()) == NULL) {
		return 0;
	}

	if (addConnection(newConn) < 0) {
		return -1;
	}

	if (newConn->recv() > 0) { // Client initialization (ADC behaviour)

		if (mNowConn->mCreatorConnFactory) {
			mNowConn->mCreatorConnFactory->onNewConnClient(newConn, mNowConn); // Set protocol point
		} else {
			newConn->mProtocol = mNowConn->mProtocol; // Set protocol by this Conn
			onNewConn(newConn);
		}
		if (newConn->isOk()) {
			onRecv(newConn); // Parse recv data
		}

	} else if (newConn->isOk()) { // Server initialization (NMDC behaviour)
			
		if (mNowConn->mCreatorConnFactory) {
			mNowConn->mCreatorConnFactory->onNewConnServer(newConn, mNowConn); // Set protocol point
		} else {
			newConn->mProtocol = mNowConn->mProtocol; // Set protocol by this Conn
			onNewConn(newConn);
		}

	}

	return 1;
}



///////////////////////////////////add_connection/del_connection///////////////////////////////////

int Server::addConnection(Conn * conn) {

	if (!conn->isOk()) {
		if (conn->log(2)) {
			conn->logStream() << "Not reserved connection: " << conn->getIp() << endl;
		}
		if (conn->mSelfConnFactory != NULL) {
			conn->mSelfConnFactory->deleteConn(conn);
		} else {
			if (conn->log(2)) {
				conn->logStream() << "Connection without factory: " << conn->getIp() << endl;
			}
			delete conn;
		}
		return -1;
	}

	// Adding in common list
	mConnChooser.addConn(conn);

	if (
		#if USE_SELECT
			(mConnChooser.size() == (FD_SETSIZE - 1)) || 
		#endif
		!mConnChooser.ConnChoose::optIn(static_cast<ConnBase *> (conn),
		ConnChoose::tEventFlag(ConnChoose::eEF_INPUT | ConnChoose::eEF_ERROR)))
	{
		if (conn->errLog(0)) {
			conn->logStream() << "Error: Can't add socket!" << endl;
		}
		mConnChooser.deleteConn(conn);
		if (conn->mSelfConnFactory != NULL) {
			conn->mSelfConnFactory->deleteConn(conn);
		} else {
			delete conn;
		}
		return -2;
	}
	
	// Adding in client or listen list
	if (conn->getConnType() == CONN_TYPE_INCOMING_TCP) {
		conn->mIterator = mClientList.insert(mClientList.begin(), conn);
	} else {
		conn->mIterator = mListenList.insert(mListenList.begin(), conn);
	}

	return 0;
}



/// delConnection
int Server::delConnection(Conn * conn) {
	if (conn == NULL) {
		if (mNowConn && mNowConn->errLog(0)) {
			mNowConn->logStream() << "Fatal error: delConnection null pointer" << endl;
		}
		throw "Fatal error: delConnection null pointer";
	}

	tCLIt it = conn->mIterator;
	Conn * found = (*it);

	// Removing from client or listen list
	if (conn->getConnType() == CONN_TYPE_INCOMING_TCP) {
		if (it == mClientList.end() || found != conn) {
			if (conn->errLog(0)) {
				conn->logStream() << "Fatal error: Delete unknown connection: " << conn << endl;
			}
			throw "Fatal error: Delete unknown connection";
		}
		mClientList.erase(it);
		conn->mIterator = mClientList.end();
	} else {
		if (it == mListenList.end() || found != conn) {
			if (conn->errLog(0)) {
				conn->logStream() << "Fatal error: Delete unknown connection: " << conn << endl;
			}
			throw "Fatal error: Delete unknown connection";
		}
		mListenList.erase(it);
		conn->mIterator = mListenList.end();
	}

	// Removing from common list
	mConnChooser.deleteConn(conn);


	// Removing self connection
	if (conn->mSelfConnFactory != NULL) {
		conn->mSelfConnFactory->deleteConn(conn); 
	} else {
		if (conn->log(3)) {
			conn->logStream() << "Deleting conn without factory!" << endl;
		}
		delete conn;
	}

	return 1;
}



/// inputEvent
size_t Server::inputEvent(Conn * conn) {
	int ret = conn->recv();
	if (ret <= 0) {
		if (ret == -1) {
			conn->closeSelf();
		}
		return 0;
	}
	return onRecv(conn);
}



// onRecv
size_t Server::onRecv(Conn * conn) {
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



/// outputEvent
void Server::outputEvent(Conn * conn) {
	conn->flush();
}



/// createCommandPtr
string * Server::createCommandPtr(Conn *) {
	return &mCommand;
}



/// onNewConn
int Server::onNewConn(Conn * conn) {
	if (conn == NULL) {
		return -1;
	}
	conn->mPortConn = mNowConn->mPort;
	conn->mIpConn = mNowConn->mIp;
	return 0;
}



/// onClose
void Server::onClose(Conn * conn) {
	if (!conn) {
		return;
	}
	mConnChooser.deleteConn(conn);
}



/// onNewData
void Server::onNewData(Conn *, string *) {
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
		Conn * conn = NULL;
		
		tCLIt it = mClientList.begin();
		tCLIt it_e = mClientList.end();
		while (it != it_e) {
			conn = (*it++);
			if (conn->isOk()) {
				conn->onTimerBase(now);
			}
		}
		
		it = mListenList.begin();
		it_e = mListenList.end();
		while (it != it_e) {
			conn = (*it++);
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
