/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz

 * modified: 27 Aug 2009
 * Copyright (C) 2009-2011 by Setuper
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

#ifndef SERVER_H
#define SERVER_H

#define INTERNALNAME "RusHub"
#define INTERNALVERSION "2.3.0[beta]" // without space!

#include "Obj.h"
#include "ConnChoose.h"
#include "Times.h" // Time
#include "MeanFrequency.h"

#if USE_SELECT
	#include "ConnSelect.h" /** Check sockets by method select */
#else
	#ifdef USE_EPOLL
		#include "ConnEpoll.h" /** Check sockets by method epoll */
	#else
		#include "ConnPoll.h" /** Check sockets by method poll */
	#endif
#endif

#include <string>
#include <list> // tConnList


using namespace ::std;
using namespace ::utils; // Time

namespace server {

class Conn;
class ConnFactory;
class Server;

/** Base listen factory class */
class ListenFactory {

public:
	ListenFactory(Server *);
	virtual ~ListenFactory();
	virtual ConnFactory * connFactory();
	virtual int OnNewConn(Conn *);

protected:
	Server * mServer;

}; // ListenFactory


class Server : public Obj {

friend class Conn; /** for protected var on close socket (Conn::CloseNow) */
friend class ListenFactory; /* OnNewConn */
friend class ConnFactory; /* OnNewData */

public:

	string msAddresses; /** string "Ip1[:port1] Ip2[:port2] ... IpN[:portN]" - addresses of server */

	ConnFactory * mConnFactory; /** ConnFactory */

	unsigned long mStrSizeMax; /** (10240) Max msg size */
	string msSeparator; /** Proto separator */

	Time mTime; /** Current time of main cycle on the server */
	MeanFrequency<unsigned, 21> mMeanFrequency; /** Mean frequency */
	int mStepDelay; /** Step delay (for testing) */

	int miNumCloseConn; /** Strong close conn flag */

	int mTimerServPeriod; /** Serv period (msec) */
	int mTimerConnPeriod; /** Conn period (msec) */

	bool mbMAC; /** allow to define MAC address */

public:

	Server(const string sSep);
	virtual ~Server();

	/** Set and Listen port */
	virtual int Listening(ListenFactory *, const string & sIP, int iPort = 0);

	/** Listen port (TCP/UDP) */
	virtual Conn * Listen(const string & sIP, int iPort, bool bUDP = false);

	/** Create, bind and add connection for port */
	virtual Conn * AddListen(Conn *, const string & sIP, int iPort, bool bUDP = false);

	/** Stop listen conn */
	virtual bool StopListen(Conn *);

	/** Find conn by port */
	virtual Conn * FindConnByPort(int iPort);

	/** Main cycle */
	int Run();

	/** Main step in the cycle */
	void Step();

	/** OnClose conn */
	void OnClose(Conn *);

	/** Stop cycle */
	void Stop(int);

	/** Main base timer */
	int OnTimerBase(Time &now);

	/** Main timer */
	virtual int onTimer(Time &now);

	/** InputData */
	int InputData(Conn *);

	/** OutputData */
	int OutputData(Conn *);

protected:

	#ifdef _WIN32
		static bool initWSA; /** Windows init flag for WSA (Windows Sockets API) */
	#endif

	typedef list<Conn *> tConnList; /** tConnList */
	typedef tConnList::iterator tCLIt;
	tConnList mConnList; /** ConnList */

	typedef list<Conn *> tListenList; /** tListenList */
	typedef tListenList::iterator tLLIt;
	tListenList mListenList; /** ListenList */

	/** select or poll object */
	#if USE_SELECT
		ConnSelect mConnChooser;
		typedef ConnSelect::iterator tChIt;
	#else
		ConnPoll mConnChooser;
		typedef ConnChoose::iterator tChIt;
	#endif

	/** Run-flag */
	bool mbRun;

	/** MainLoopCode (0) If 1 then restart hub! */
	int miMainLoopCode;

	/** Point to Server (for config) */
	Server * mServer;

	struct Times { /** Times */
		Time mServ; /** (mTimerServPeriod) */
		Time mConn; /** (mTimerConnPeriod) */
	} mTimes;

protected:

	/** AddConnection */
	virtual int AddConnection(Conn *);

	/** DelConnection */
	int DelConnection(Conn *);

	/** OnNewConn */
	virtual int OnNewConn(Conn *);

	/** GetPtrForStr */
	virtual string * GetPtrForStr(Conn *);

	/** OnNewData */
	virtual void OnNewData(Conn *, string *);

	/** Close server */
	virtual void close() {
	}

private:

	/** Current connection */
	Conn * mNowConn;


}; // Server

}; // server

#endif // SERVER_H
