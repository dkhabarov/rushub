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

#ifndef CSERVER_H
#define CSERVER_H

#include "cobj.h"
#include "cconnchoose.h"
#include "ctime.h" /** cTime */
#include "cmeanfrequency.h"

#include <string>
#include <list> /** tConnList */

#define INTERNALNAME "RusHub"
#define INTERNALVERSION "2.2.12[beta]" // without space!

#if USE_SELECT
	#include "cconnselect.h" /** Check sockets by method select */
#else
	#ifdef USE_EPOLL
		#include "cconnepoll.h" /** Check sockets by method epoll */
	#else
		#include "cconnpoll.h" /** Check sockets by method poll */
	#endif
#endif


using namespace std;
using namespace nUtils; // cTime

namespace nServer {

class cConn;
class cConnFactory;
class cServer;

/** Base listen factory class */
class cListenFactory {

public:
	cListenFactory(cServer *);
	virtual ~cListenFactory();
	virtual cConnFactory * ConnFactory();
	virtual int OnNewConn(cConn *);

protected:
	cServer * mServer;

}; // cListenFactory


class cServer : public cObj {

friend class cConn; /** for protected var on close socket (cConn::CloseNow) */
friend class cListenFactory; /* OnNewConn */
friend class cConnFactory; /* OnNewData */

public:

	string msAddresses; /** string "Ip1[:port1] Ip2[:port2] ... IpN[:portN]" - addresses of server */

	cConnFactory * mConnFactory; /** cConnFactory */

	unsigned long miStrSizeMax; /** (10240) Max msg size */
	string msSeparator; /** Proto separator */

	cTime mTime; /** Current time of main cycle on the server */
	cMeanFrequency<unsigned, 21> mMeanFrequency; /** Mean frequency */
	int mStepDelay; /** Step delay (for testing) */

	int miNumCloseConn; /** Strong close conn flag */

	int miTimerServPeriod; /** Serv period (msec) */
	int miTimerConnPeriod; /** Conn period (msec) */

	bool mbMAC; /** allow to define MAC address */

public:

	cServer(const string sSep);
	virtual ~cServer();

	/** Set and Listen port */
	virtual int Listening(cListenFactory *, const string & sIP, int iPort = 0);

	/** Listen port (TCP/UDP) */
	virtual cConn * Listen(const string & sIP, int iPort, bool bUDP = false);

	/** Create, bind and add connection for port */
	virtual cConn * AddListen(cConn *, const string & sIP, int iPort, bool bUDP = false);

	/** Stop listen conn */
	virtual bool StopListen(cConn *);

	/** Find conn by port */
	virtual cConn * FindConnByPort(int iPort);

	/** Main cycle */
	int Run();

	/** Main step in the cycle */
	void Step();

	/** OnClose conn */
	void OnClose(cConn *);

	/** Stop cycle */
	void Stop(int);

	/** Main base timer */
	int OnTimerBase(cTime &now);

	/** Main timer */
	virtual int OnTimer(cTime &now);

	/** InputData */
	int InputData(cConn *);

	/** OutputData */
	int OutputData(cConn *);

private:

	cConn *mNowConn; /** Current connection */

protected:

	#ifdef _WIN32
		static bool initWSA; /** Windows init flag for WSA (Windows Sockets API) */
	#endif

	typedef list<cConn *> tConnList; /** tConnList */
	typedef tConnList::iterator tCLIt;
	tConnList mConnList; /** ConnList */

	typedef list<cConn *> tListenList; /** tListenList */
	typedef tListenList::iterator tLLIt;
	tListenList mListenList; /** ListenList */

	/** select or poll object */
	#if USE_SELECT
		cConnSelect mConnChooser;
		typedef cConnSelect::iterator tChIt;
	#else
		cConnPoll mConnChooser;
		typedef cConnChoose::iterator tChIt;
	#endif

	bool mbRun; /** Run-flag */
	int miMainLoopCode; /** MainLoopCode (0) If 1 then restart hub! */

	struct sTimes { /** Timers */
		cTime mServ; /** (miTimerServPeriod) */
		cTime mConn; /** (miTimerConnPeriod) */
	} mTimes;

protected:

	/** AddConnection */
	virtual int AddConnection(cConn *);

	/** DelConnection */
	int DelConnection(cConn *);

	/** OnNewConn */
	virtual int OnNewConn(cConn *);

	/** GetPtrForStr */
	virtual string * GetPtrForStr(cConn *);

	/** OnNewData */
	virtual void OnNewData(cConn *, string *);

	/** Close server */
	virtual void Close(){}

}; // cServer

}; // nServer

#endif // CCONNSERVER_H
