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

#include "cserver.h"
#include "cconn.h" /** cConn, cConnFactory */

#include <iostream> /** cout, endl */
#include <math.h> /** abs */

#ifdef _WIN32
	#pragma comment(lib, "ws2_32.lib")
#else
	#include <sys/socket.h>
	#include <unistd.h> // for usleep function
	#include <cstdlib> // for abs
#endif

namespace nServer {

#ifdef _WIN32
	/** Set flag WS */
	bool cServer::initWSA = false;
#endif

//////////////////////////////////////////////constructor////////////////////////////////////////
cServer::cServer(const string sSep) :
	cObj("cServer"),
	msAddresses("0.0.0.0"),
	mConnFactory(NULL),
	miStrSizeMax(10240),
	msSeparator(sSep),
	mMeanFrequency(mTime, 90.0, 20),
	mStepDelay(0),
	miNumCloseConn(0),
	miTimerServPeriod(100),
	miTimerConnPeriod(4000),
	mbMAC(true),
	mNowConn(NULL),
	miMainLoopCode(0)
{
	//mConnFactory = new cConnFactory(NULL, this);

#ifdef _WIN32
	/** WSA for windows */
	if(!initWSA) {
		WSADATA lpWSAData;
		//WORD wVersionRequested = MAKEWORD(2, 2);

		if(::WSAStartup(/*wVersionRequested*/ 0x0202, &lpWSAData)) {
			printf("Error WSAStartup in WinSock DLL %d\n", ::WSAGetLastError());
			return;
		}

		/** DLL support version 2.2 */
		if(HIBYTE(lpWSAData.wVersion) != 2 || LOBYTE(lpWSAData.wVersion) != 2) {
			printf("Error version WinSock DLL %d.%d != 2.2\n", HIBYTE(lpWSAData.wVersion), LOBYTE(lpWSAData.wVersion));
			::WSACleanup();
			return;
		}

		/** WinSock DLL was found */
		initWSA = true;
	}
#endif
}


////////////////////////////////////////////destructor////////////////////////////////////////////
cServer::~cServer() {
	mNowConn = NULL;
#ifdef _WIN32
	/** Close WSA DLL lib */
	::WSACleanup();
	initWSA = false;
#endif
	if(Log(1)) {
		LogStream() << endl << "Allocated objects: " << 
		#ifdef _WIN32
			cObj::GetCount() - 2 // except cDCServer and cService
		#else
			cObj::GetCount() - 1 // except cDCServer
		#endif
		<< endl << "Unclosed sockets: " << cConn::iConnCounter << endl;
	}
	if(mOfs.is_open()) mOfs.close();
}

/** Set and Listen port */
int cServer::Listening(cListenFactory * ListenFactory, const string & sIP, int iPort) {
	cConn * conn;
	if((conn = Listen(sIP, iPort)) != NULL) { /** Listen port (TCP) */
		conn->mListenFactory = ListenFactory;
		return 0;
	}
	return -1;
}



/** Listen port (TCP/UDP) */
cConn *cServer::Listen(const string & sIP, int iPort, bool bUDP) {
	cConn *conn = NULL;

	if(!bUDP) /** Create socket object for TCP port */
		conn = new cConn(0, this, eCT_LISTEN);

	else /** Create socket object for UDP port */
		conn = new cConn(0, this, eCT_CLIENTUDP);

	if(!AddListen(conn, sIP, iPort, bUDP)) { /** Listen conn */
		delete conn;
		return NULL;
	}
	return conn;
}



/** Create, bind and add connection for port */
cConn *cServer::AddListen(cConn *conn, const string & sIP, int iPort, bool bUDP) {
	/** Socket object was created */
	if(conn) {
		if(conn->ListenPort(iPort, sIP.c_str(), bUDP) < 0) {
			if(ErrLog(0))
				LogStream() << "Fatal error: Can't listen on " << sIP << ":" << iPort << (bUDP ? " UDP" : " TCP") << endl;
			return NULL;
		}

		mListenList.insert(mListenList.begin(), conn);
		mConnChooser.AddConn(conn);

		if(!mConnChooser.cConnChoose::OptIn(
			(cConnBase *)conn,
			cConnChoose::tEventFlag(cConnChoose::eEF_INPUT | cConnChoose::eEF_ERROR))) {
			if(ErrLog(0)) LogStream() << "Error: Can't add socket" << endl;
			delete conn;
			return NULL;
		}

		if(Log(0))
			LogStream() << "Listening on " << sIP << ":" << iPort << (bUDP ? " UDP" : " TCP") << endl;
		return conn;
	} else {
		if(ErrLog(0)) LogStream() << "Fatal error: Can't create conn object" << endl;
	}
	return NULL;
}

/** StopListen */
bool cServer::StopListen(cConn *conn) {
	if(conn) {
		mConnChooser.DelConn(conn);
		return true;
	}
	return false;
}

cConn * cServer::FindConnByPort(int iPort) {
	cConnChoose::tConnBaseList::iterator it;
	cConn * conn;
	for(it = mConnChooser.mConnBaseList.begin(); it != mConnChooser.mConnBaseList.end(); ++it) {
		conn = (cConn *)(*it);
		if(conn && conn->Port() == iPort)
			return conn;
	}
	return NULL;
}

/** Main cycle */
int cServer::Run() {
	cTime now;
	mbRun = true;
	if(Log(1)) LogStream() << "Main loop start" << endl;

	while(mbRun) { /** Main cycle */
		try {
			mTime.Get(); /** Current time */
			Step(); /** Server's step */

			/** Timers (100 msec) */
			if(abs(int(now.Get() - mTimes.mServ)) >= 100) { /** transfer of time */
				mTimes.mServ = now;
				OnTimerBase(now);
			}

			if(mStepDelay)
			#ifdef _WIN32
				Sleep(mStepDelay); /** Testing (mStepDelay msec) */
			#else
				usleep(mStepDelay * 1000);
			#endif
			mMeanFrequency.Insert(mTime); /** MeanFrequency */
		} catch(const char *str) {
			if(ErrLog(0)) LogStream() << "Exception: " << str << endl;
		} catch(...) {
			if(ErrLog(0)) LogStream() << "Exception in Run function" << endl;
			throw "cServer::Run()";
		}
	}
	if(Log(1)) LogStream() << "Main loop stop(" << miMainLoopCode << ")" << endl;
	return miMainLoopCode;
}

/** Stop main cycle */
void cServer::Stop(int iCode) {
	mbRun = false;
	miMainLoopCode = iCode; // 1 - restart
}

/** Step */
void cServer::Step() {
	static int ret;
	static cTime tmout(0, 1000l); /** timeout 1 msec */

	try {
		/** Checking the arrival data in listen sockets */
		ret = mConnChooser.Choose(tmout);
		if(ret <= 0 && !miNumCloseConn) { 
			#ifdef _WIN32
				//Sleep(0);
			#else
				usleep(50); // 50 usec
			#endif
			return;
		}
	} catch(const char *str) {
		if(ErrLog(0)) LogStream() << "Exception in Choose: " << str << endl;
		return;
	} catch(...) {
		if(ErrLog(0)) LogStream() << "Exception in Choose" << endl;
		throw "Exception in Choose";
	}

	static cConnChoose::sChooseRes res;

	if(Log(5)) LogStream() << "<new actions>: " << ret << " [" << miNumCloseConn << "]" << endl;

	for(tChIt it = mConnChooser.begin(); it != mConnChooser.end();) {
		res = (*it);
		++it;
		bool &bOk = mNowConn->mbOk;
		if((res.mRevents == 0 && bOk) || ((mNowConn = (cConn* )res.mConnBase) == NULL)) continue; // optimization cycle
		int activity = res.mRevents;

		if(bOk && (activity & cConnChoose::eEF_INPUT) && (mNowConn->GetConnType() == eCT_LISTEN)) {

			if(mNowConn->Log(5)) mNowConn->LogStream() << "::(s)NewConn" << endl;

			cConn * new_conn = mNowConn->CreateNewConn(); /** eCT_CLIENTTCP (cConn) */
			if(new_conn) {
				if(!mNowConn->mListenFactory) {
					if(ErrLog(0)) LogStream() << "ListenFactory is empty" << endl;
					throw "Exception in InputData";
				}
				if(AddConnection(new_conn) > 0)
					mNowConn->mListenFactory->OnNewConn(new_conn);
			}
			if(mNowConn->Log(5)) mNowConn->LogStream() << "::(e)NewConn. Number connections: " << mConnChooser.mConnBaseList.Size() << endl;

		} else { // not listening socket (optimization)

			if(bOk && (activity & cConnChoose::eEF_INPUT) && ((mNowConn->GetConnType() == eCT_CLIENTTCP) || (mNowConn->GetConnType() == eCT_CLIENTUDP))) {
				try {
					if(mNowConn->Log(5)) mNowConn->LogStream() << "::(s)InputData" << endl;

					if(InputData(mNowConn) <= 0) bOk = false;

					if(mNowConn->Log(5)) mNowConn->LogStream() << "::(e)InputData" << endl;
				} catch(const char *str) {
					if(ErrLog(0)) LogStream() << "Exception in InputData: " << str << endl;
					throw "Exception in InputData";
				} catch(...) {
					if(ErrLog(0)) LogStream() << "Exception in InputData" << endl;
					throw "Exception in InputData";
				}
			}

			if(bOk && (activity & cConnChoose::eEF_OUTPUT)) {
				try {
					if(mNowConn->Log(5)) mNowConn->LogStream() << "::(s)OutputData" << endl;

					OutputData(mNowConn);

					if(mNowConn->Log(5)) mNowConn->LogStream() << "::(e)OutputData" << endl;
				} catch(const char *str) {
					if(ErrLog(0)) LogStream() << "Exception in OutputData: " << str << endl;
					throw "Exception in OutputData";
				} catch(...) {
					if(ErrLog(0)) LogStream() << "Exception in OutputData" << endl;
					throw "Exception in OutputData";
				}
			}

			if(!bOk || (activity & (cConnChoose::eEF_ERROR | cConnChoose::eEF_CLOSE))) {

				if(mNowConn->IsClosed()) // check close flag
					--miNumCloseConn;

				try {
					if(Log(5)) LogStream() << "::(s)DelConnection" << endl;

					DelConnection(mNowConn);

					if(Log(5)) LogStream() << "::(e)DelConnection. Number connections: " << mConnChooser.mConnBaseList.Size() << endl;
				} catch(const char *str) {
					if(ErrLog(0)) LogStream() << "Exception in DelConnection: " << str << endl;
					throw "Exception in DelConnection";
				} catch(...) {
					if(ErrLog(0)) LogStream() << "Exception in DelConnection" << endl;
					throw "Exception in DelConnection";
				}
			}
		}

		//if(activity - cConnChoose::eEF_CLOSE)
		//	--ret;
		//if(ret < 0) { // this will not happen, but we must be reinsure
		//	if(ErrLog(0)) LogStream() << "Faild error in return value" << endl;
		//	break;
		//}
		//if(!(ret + miNumCloseConn))
		//	break;
	}

	if(miNumCloseConn) {
		if(Log(0)) LogStream() << "Control not closed connections: " << miNumCloseConn << endl;
		//miNumCloseConn = 0;
	}
}

///////////////////////////////////add_connection/del_connection///////////////////////////////////

int cServer::AddConnection(cConn *conn) {
	if(!conn) {
		if(conn->ErrLog(0)) LogStream() << "Fatal error: AddConnection null pointer" << endl;
		throw "Fatal error: AddConnection null pointer";
	}
	if(!conn->mbOk) { 
		if(conn->Log(2)) conn->LogStream() << "Not reserved connection: " << conn->Ip() << endl;
		if(conn->mConnFactory != NULL) conn->mConnFactory->DelConn(conn);
		else {
			if(conn->Log(2)) conn->LogStream() << "Connection without factory: " << conn->Ip() << endl;
			delete conn;
		}
		return -1;
	}

	//if(Log(4)) LogStream() << "Num clients before add: " << mConnList.size() << ". Num socks: " << mConnChooser.mConnBaseList.Size() << endl;
	//if(Log(4)) LogStream() << "Add new connection on socket: " << conn->mSocket << " - " << conn->Ip() << ":" << conn->Port() << endl;

	if(
		#if USE_SELECT
			(mConnChooser.Size() == (FD_SETSIZE - 1)) || 
		#endif
		!mConnChooser.cConnChoose::OptIn((cConnBase *)conn,
		cConnChoose::tEventFlag(cConnChoose::eEF_INPUT | cConnChoose::eEF_ERROR)))
	{
		if(conn->ErrLog(1)) conn->LogStream() << "Error: Can't add socket!" << endl;
		if(conn->mConnFactory != NULL) 
			conn->mConnFactory->DelConn(conn); 
		else delete conn; 
		return -2;
	}

	mConnChooser.AddConn(conn);

	tCLIt it = mConnList.insert(mConnList.begin(), conn);
	conn->mIterator = it;

	//if(Log(4)) LogStream() << "Num clients after add: " << mConnList.size() << ". Num socks: " << mConnChooser.mConnBaseList.Size() << endl;

	conn->miPortConn = mNowConn->miPort;
	return 1;
}

/** DelConnection(cConn *old_conn) */
int cServer::DelConnection(cConn *old_conn) {
	if(!old_conn) {
		if(mNowConn && mNowConn->ErrLog(0)) mNowConn->LogStream() << "Fatal error: DelConnection null pointer" << endl;
		throw "Fatal error: DelConnection null pointer";
	}

	//if(Log(4)) LogStream() << "Num clients before del: " << mConnList.size() << ". Num socks: " << mConnChooser.mConnBaseList.Size() << endl;
	//if(Log(4)) LogStream() << "Delete connection on socket: " << old_conn->mSocket << endl;

	tCLIt it = old_conn->mIterator;//find(mConnList.begin(), mConnList.end(), old_conn);
	cConn *found = (*it);
	if((it == mConnList.end()) || (found != old_conn)) {
		if(old_conn->ErrLog(0)) old_conn->LogStream() << "Fatal error: Delete unknown connection: " << old_conn << endl;
		throw "Fatal error: Delete unknown connection";
	}

	mConnList.erase(it);
	tCLIt empty_it;
	old_conn->mIterator = empty_it;

	mConnChooser.DelConn(old_conn);

	if(old_conn->mConnFactory != NULL) 
		old_conn->mConnFactory->DelConn(old_conn); 
	else {
		if(old_conn->Log(0)) old_conn->LogStream() << "Deleting conn without factory!" << endl;
		delete old_conn;
	}

	//if(Log(4)) LogStream() << "Num clients after del: " << mConnList.size() << ". Num socks: " << mConnChooser.mConnBaseList.Size() << endl;
	return 1;
}

/** OnNewConn */
int cServer::OnNewConn(cConn *conn) {
	if(!conn) return -1;
	return 0;
}

/** OnClose */
void cServer::OnClose(cConn *conn) {
	if(!conn) return;
	mConnChooser.DelConn(conn);
}

/** InputData */
int cServer::InputData(cConn *conn) {
	try {
		if(conn->Recv() <= 0) return 0;
	} catch(const char *str) {
		if(ErrLog(0)) LogStream() << "Exception in Recv: " << str << endl;
		return 0;
	} catch(...) {
		if(ErrLog(0)) LogStream() << "Exception in Recv" << endl;
		return 0;
	}

	int iRead = 0;
	while(conn->mbOk && conn->mbWritable) {
		if(conn->StrStatus() == eSS_NO_STR) {
			conn->SetStrToRead(GetPtrForStr(conn),
				(conn->mConnFactory != NULL) ? conn->mConnFactory->msSeparator : msSeparator,
				(conn->mConnFactory != NULL) ? conn->mConnFactory->miStrSizeMax: miStrSizeMax
			);
		}

		iRead += conn->ReadFromRecvBuf();

		if(conn->StrStatus() == eSS_STR_DONE) { 
			if(conn->mConnFactory != NULL)
				conn->mConnFactory->OnNewData(conn, conn->GetStr());
			else
				OnNewData(conn, conn->GetStr());
			conn->ClearStr();
		}
		if(conn->RecvBufIsEmpty()) break;
	}
	return iRead;
}

/** GetPtrForStr */
string * cServer::GetPtrForStr(cConn *) {
	return new string;
}

/** OnNewData */
void cServer::OnNewData(cConn *, string *str) {
	delete str;
}

/** OutputData */
int cServer::OutputData(cConn *conn) {
	conn->Flush();
	return 0;
}

/** Main mase timer */
int cServer::OnTimerBase(cTime &now) {
	static tCLIt it, it_e;
	OnTimer(now);
	if(abs(int(now - mTimes.mConn)) >= miTimerConnPeriod) {
		mTimes.mConn = now;
		it_e = mConnList.end();
		for(it = mConnList.begin(); it != it_e; ++it)
			if((*it)->mbOk) (*it)->OnTimerBase(now); 
	}
	return 0;
}

/** Main timer */
int cServer::OnTimer(cTime &) {
	return 0;
}

cListenFactory::cListenFactory(cServer * Server) : mServer(Server) {
}

cListenFactory::~cListenFactory() {
}

cConnFactory * cListenFactory::ConnFactory() {
	return mServer->mConnFactory;
}

int cListenFactory::OnNewConn(cConn * conn) {
	return mServer->OnNewConn(conn);
}

}; // nServer
