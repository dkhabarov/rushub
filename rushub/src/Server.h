/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz
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

#ifndef SERVER_H
#define SERVER_H

#define INTERNALNAME "RusHub"
#define INTERNALVERSION "2.3.3[beta]" // without space!

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

class Server : public Obj {

friend class Conn; /** for closeNow and miNumCloseConn */
friend class ConnFactory; /* onNewData */

public:

	Time mTime; /** Current time of main cycle on the server */
	int mStepDelay; /** Step delay (for testing) */

	int mTimerServPeriod; /** Serv period (msec) */
	int mTimerConnPeriod; /** Conn period (msec) */

	bool mMac; /** allow to define MAC address */

public:

	Server();
	virtual ~Server();

	/** Set and Listen port */
	virtual int listening(ConnFactory *, const string & ip, int port = 0, bool udp = false);

	/** Listen port (TCP/UDP) */
	virtual Conn * listen(const string & ip, int port, bool udp = false);

	/** Create, bind and add connection for port */
	virtual Conn * addListen(Conn *, const string & ip, int port, bool udp = false);

	/** Stop listen conn */
	virtual bool stopListen(Conn *);

	/** Main cycle */
	int run();

	/** Stop cycle */
	void stop(int);

	/** inputData */
	int inputData(Conn *);

	/** outputData */
	int outputData(Conn *);

	/** onNewConn */
	virtual int onNewConn(Conn *);

protected:

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
	bool mRun;

	/** MainLoopCode (0) If 1 then restart hub! */
	int mMainLoopCode;
	
	/** Strong close conn flag */
	int miNumCloseConn;
	
	/** Mean frequency */
	MeanFrequency<unsigned, 21> mMeanFrequency;

	/** Point to Server (for config) */
	Server * mServer;

	struct Times { /** Times */
		long mServ; /** (mTimerServPeriod) */
		long mConn; /** (mTimerConnPeriod) */
	} mTimes;

protected:

	/** addConnection */
	int addConnection(Conn *);

	/** delConnection */
	int delConnection(Conn *);

	/** main timer */
	virtual int onTimer(Time & now);
	
	/** createCommandPtr */
	virtual string * createCommandPtr(Conn *);

	/** onNewData */
	virtual void onNewData(Conn *, string *);

	/** close server */
	virtual void close() {
	}
	
	/** onClose conn */
	void onClose(Conn *);

private:

	/** Current connection */
	Conn * mNowConn;

	#ifdef _WIN32
		static bool initWSA; /** Windows init flag for WSA (Windows Sockets API) */
	#endif
	
private:

	/** Main step in the cycle */
	void step();

	/** Main base timer */
	int onTimerBase(Time & now);

}; // Server

}; // server

#endif // SERVER_H

/**
 * $Id$
 * $HeadURL$
 */
