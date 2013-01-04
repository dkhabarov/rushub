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
	mTime(true),
	mStepDelay(0),
	mTimerServPeriod(1000),
	mTimerConnPeriod(4000),
	mMaxSendSize(MAX_SEND_SIZE),
	mMac(true),
	mRun(true), // run by default
	mMainLoopCode(0),
	mNumCloseConn(0),
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

	LOG(LEVEL_DEBUG, endl << "Allocated objects: " << Obj::getCount()
		<< endl << "Unclosed sockets: " << Conn::getCount());
	if (mOfs.is_open()) {
		mOfs.close();
	}
}



void Server::sleep(int msec) {
	#ifdef _WIN32
		Sleep(static_cast<DWORD> (msec));
	#else
		usleep(static_cast<useconds_t> (msec * 1000));
	#endif
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
		return NULL;
	}

	// Set server listen factory
	conn->mCreatorConnFactory = connFactory;

	return conn;
}



/// Set and Connect to port
Conn * Server::connecting(ConnFactory * connFactory, const char * ip, const char * port, bool udp /*= false*/) {

	ConnType connType = udp ? CONN_TYPE_OUTGOING_UDP : CONN_TYPE_OUTGOING_TCP;
	Conn * conn = addSimpleConn(connType, ip, port);

	if (conn == NULL) {
		return NULL;
	}

	// Set server listen factory
	conn->mCreatorConnFactory = connFactory;

	return conn;
}



/// Add simple connection
Conn * Server::addSimpleConn(int connType, const char * ip, const char * port) {

	Conn * conn = new Conn(0, this, connType);

	if (conn->makeSocket(port, ip, connType) == INVALID_SOCKET) {
		ostringstream oss;
		if (connType == CONN_TYPE_LISTEN) {
			oss << "Fatal error: Can't listen on " << ip << ":" << port << " TCP";
		} else if (connType == CONN_TYPE_INCOMING_UDP) {
			oss << "Fatal error: Can't listen on " << ip << ":" << port << " UDP";
		} else if (connType == CONN_TYPE_OUTGOING_TCP) {
			oss << "Fatal error: Can't connect to " << ip << ":" << port << " TCP";
		} else if (connType == CONN_TYPE_OUTGOING_UDP) {
			oss << "Fatal error: Can't connect to " << ip << ":" << port << " UDP";
		} else {
			oss << "Fatal error: Unknown connection";
		}
		LOG(LEVEL_FATAL, oss.str());
		delete conn;
		return NULL;
	}

	if (addConnection(conn) < 0) {
		return NULL;
	}

	if (connType == CONN_TYPE_LISTEN) {
		LOG(LEVEL_INFO, "Listening on " << ip << ":" << port << " TCP");
	} else if (connType == CONN_TYPE_INCOMING_UDP) {
		LOG(LEVEL_INFO, "Listening on " << ip << ":" << port << " UDP");
	} else if (connType == CONN_TYPE_OUTGOING_TCP) {
		LOG(LEVEL_INFO, "Connected to " << ip << ":" << port << " TCP");
	} else if (connType == CONN_TYPE_OUTGOING_UDP) {
		LOG(LEVEL_INFO, "Connected to " << ip << ":" << port << " UDP");
	}
	return conn;
}



/// Main cycle
int Server::run() {
	// mRun = true; // by default server was run
	LOG(LEVEL_INFO, "Main loop start");

	while (mRun) { // Main cycle
		try {
			mTime.get(); // Current time
			step(); // Server's step

			// Timers (100 msec)
			int64_t msec = mTime.msec();
			if (msec > mTimes.mServ ? msec - mTimes.mServ >= 100 : mTimes.mServ - msec >= 100) { // Transfer of time
				mTimes.mServ = msec;
				onTimerBase(mTime);
			}

			if (mStepDelay) {
				sleep(mStepDelay); // (mStepDelay msec)
			}
			mMeanFrequency.insert(mTime); // MeanFrequency
		} catch(const char * exception) {
			LOG(LEVEL_FATAL, "Exception: " << exception);
		} catch(...) {
			LOG(LEVEL_FATAL, "Exception in Run function");
			throw "Server::run()";
		}
	}
	LOG(LEVEL_INFO, "Main loop stop(" << mMainLoopCode << ")");
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
		if (ret <= 0 && !mNumCloseConn) { 
			sleep(1);
			return;
		}
	} catch (const char * exception) {
		LOG(LEVEL_FATAL, "Exception in choose: " << exception);
		return;
	} catch (...) {
		LOG(LEVEL_FATAL, "Exception in choose");
		throw "Exception in choose";
	}

	LOG(LEVEL_TRACE, "<new actions>: " << ret << " [" << mNumCloseConn << "]");

	bool ok, input;
	int connType, activity = 0, forDel = mNumCloseConn;
	ConnChoose::ChooseRes res;

	for (ChooserIterator it = mConnChooser.begin(); it != mConnChooser.end();) {
		res = (*it);
		++it;

		if ((mNowConn = static_cast<Conn *> (res.mConnBase)) == NULL) {
			continue;
		}
		activity = res.mRevents;
		ok = mNowConn->isOk();
		connType = mNowConn->getConnType();
		input = ok && (activity & ConnChoose::EF_INPUT);

		if (input && connType == CONN_TYPE_LISTEN) {

			LOG_CLASS(mNowConn, LEVEL_TRACE, "::(s)NewConn");

			int numAccept = 0;

			// accept before 64 connections for once
			while (++numAccept <= 64) {
				if (newAccept() == 0) {
					break;
				}
			}

			LOG_CLASS(mNowConn, LEVEL_TRACE, "::(e)NewConn. Number connections: " << mConnChooser.mConnBaseList.size());

		} else { // not listening socket (other conn type)

			if (input) {
				try {
					LOG_CLASS(mNowConn, LEVEL_TRACE, "::(s)inputEvent");

					if (inputEvent(mNowConn) == 0) {
						mNowConn->setOk(false);
					}

					LOG_CLASS(mNowConn, LEVEL_TRACE, "::(e)inputEvent");
				} catch (const char * exception) {
					LOG(LEVEL_FATAL, "Exception in inputEvent: " << exception);
					throw "Exception in inputEvent";
				} catch (...) {
					LOG(LEVEL_FATAL, "Exception in inputEvent");
					throw "Exception in inputEvent";
				}
			}

			if (ok && (activity & ConnChoose::EF_OUTPUT)) {
				try {
					LOG_CLASS(mNowConn, LEVEL_TRACE, "::(s)outputEvent");

					outputEvent(mNowConn);

					LOG_CLASS(mNowConn, LEVEL_TRACE, "::(e)outputEvent");
				} catch (const char * exception) {
					LOG(LEVEL_FATAL, "Exception in outputEvent: " << exception);
					throw "Exception in outputEvent";
				} catch (...) {
					LOG(LEVEL_FATAL, "Exception in outputEvent");
					throw "Exception in outputEvent";
				}
			}

			if (!ok || (activity & (ConnChoose::EF_ERROR | ConnChoose::EF_CLOSE))) {

				forDel = 0; // tmp

				if (mNowConn->isClosed()) { // check close flag
					--mNumCloseConn;
				}

				try {
					LOG(LEVEL_TRACE, "::(s)delConnection");

					delConnection(mNowConn);

					LOG(LEVEL_TRACE, "::(e)delConnection. Number connections: " << mConnChooser.mConnBaseList.size());
				} catch (const char * exception) {
					LOG(LEVEL_FATAL, "Exception in delConnection: " << exception);
					throw "Exception in delConnection";
				} catch (...) {
					LOG(LEVEL_FATAL, "Exception in delConnection");
					throw "Exception in delConnection";
				}
			}
		}
	}

	if (mNumCloseConn && forDel) {
		LOG(LEVEL_ERROR, "Control not closed connections: " << mNumCloseConn);
		--mNumCloseConn;
	}

	LOG(LEVEL_TRACE, "<exit actions>");
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

	newConn->mPortConn = mNowConn->mPort;
	newConn->mIpConn = mNowConn->mIp;


	// Set protocol point and may send hello msg to the client
	if (mNowConn->mCreatorConnFactory) {
		mNowConn->mCreatorConnFactory->onNewConn(newConn);
	} else {
		onNewConn(newConn); // Now it was not setting protocol!
	}

	if (newConn->mProtocol == NULL) {
		LOG(LEVEL_FATAL, "Protocol was not set");
	}

	return 1;
}



///////////////////////////////////add_connection/del_connection///////////////////////////////////

int Server::addConnection(Conn * conn) {

	if (!conn->isOk()) {
		LOG_CLASS(conn, LEVEL_DEBUG, "Not reserved connection: " << conn->getIp());
		if (conn->mSelfConnFactory != NULL) {
			conn->mSelfConnFactory->deleteConn(conn);
		} else {
			LOG_CLASS(conn, LEVEL_DEBUG, "Connection without factory: " << conn->getIp());
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
		!mConnChooser.ConnChoose::optIn(conn,
		ConnChoose::EventFlag(ConnChoose::EF_INPUT | ConnChoose::EF_ERROR)))
	{
		LOG_CLASS(conn, LEVEL_FATAL, "Error: Can't add socket!");
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
		if (mNowConn) {
			LOG_CLASS(mNowConn, LEVEL_FATAL, "Fatal error: delConnection null pointer");
		}
		throw "Fatal error: delConnection null pointer";
	}

	// Removing from client or listen list
	tCLIt it = conn->mIterator;
	tConnList * connList = (conn->getConnType() == CONN_TYPE_INCOMING_TCP) ? &mClientList : &mListenList;
	if (it == connList->end() || (*it) != conn) {
		LOG_CLASS(conn, LEVEL_FATAL, "Fatal error: Delete unknown connection: " << conn);
		throw "Fatal error: Delete unknown connection";
	}
	connList->erase(it);
	conn->mIterator = connList->end();

	// Removing from common list
	mConnChooser.deleteConn(conn);

	// Removing self connection
	if (conn->mSelfConnFactory != NULL) {
		conn->mSelfConnFactory->deleteConn(conn); 
	} else {
		delete conn;
	}

	return 1;
}



/// inputEvent
size_t Server::inputEvent(Conn * conn) {
	if (conn->recv() > 0) {
		return onRecv(conn);
	}
	return 0;
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
	return conn == NULL ? -1 : 0;
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


} // namespace server

/**
 * $Id$
 * $HeadURL$
 */
