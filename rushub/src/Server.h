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

#ifndef SERVER_H
#define SERVER_H

#define INTERNALNAME "RusHub"
#define INTERNALVERSION "2.3.10[beta]" // without space!

#include "ConnChoose.h" // first (def winsock2.h)
#include "Obj.h"
#include "Times.h"
#include "MeanFrequency.h"

#if USE_SELECT
	#include "ConnSelect.h" // Check sockets by method select
#else
	#ifdef USE_EPOLL
		#include "ConnEpoll.h" // Check sockets by method epoll
	#else
		#include "ConnPoll.h" // Check sockets by method poll
	#endif
#endif

#include <string>
#include <list> // tConnList


using namespace ::std;
using namespace ::utils; // Time, Obj


/// Server realization namespace
namespace server {

class Conn;
class ConnFactory;


/// Server realization
class Server : public Obj {

friend class Conn; // for closeNow and mNumCloseConn
friend class ConnFactory; // onNewData

public:

	Time mTime; ///< Current time of main cycle on the server

	int mStepDelay; ///< Step delay (for testing)
	unsigned int mTimerServPeriod; ///< Serv period (msec)
	unsigned int mTimerConnPeriod; ///< Conn period (msec)
	unsigned int mMaxSendSize;
	bool mMac; ///< allow to define MAC address

public:

	Server();
	virtual ~Server();

	/// Main cycle
	int run();

	/// Stop cycle
	void stop(int);

	/// inputEvent
	size_t inputEvent(Conn *);

	/// outputEvent
	void outputEvent(Conn *);

	/// onNewConn
	virtual int onNewConn(Conn *);

protected:

	typedef list<Conn *> tConnList; ///< tConnList
	typedef tConnList::iterator tCLIt;

	tConnList mClientList; ///< ClientList
	tConnList mListenList; ///< ListenList

	// select/poll/epoll/kqueue objects
	#if USE_SELECT
		ConnSelect mConnChooser;
		typedef ConnSelect::iterator ChooserIterator;
	#else
		#ifdef USE_EPOLL
			ConnEpoll mConnChooser;
		#else
			ConnPoll mConnChooser;
		#endif
		typedef ConnChoose::iterator ChooserIterator;
	#endif

	/// Run-flag
	bool mRun;

	/// MainLoopCode (0) If 1 then restart hub!
	int mMainLoopCode;

	/// Strong close conn flag
	int mNumCloseConn;

	/// Mean frequency
	MeanFrequency<unsigned, 21> mMeanFrequency;

	/// Point to Server (for config)
	Server * mServer;

	/// Time of server & connection periods
	struct Times {
		int64_t mServ; ///< Timer Serv Period
		int64_t mConn; ///< Timer Conn Period
	} mTimes;

protected:

	/// Set and Listen port
	virtual Conn * listening(ConnFactory *, const char * ip, const char * port = 0, bool udp = false);

	/// Set and Connect to port
	virtual Conn * connecting(ConnFactory *, const char * ip, const char * port = 0, bool udp = false);

	/// addConnection
	int addConnection(Conn *);

	/// delConnection
	int delConnection(Conn *);
	
	/// createCommandPtr
	virtual string * createCommandPtr(Conn *);

	void deleteAll();

private:

	/// Current connection
	Conn * mNowConn;
	string mCommand;

	#ifdef _WIN32
		static bool initWSA; ///< Windows init flag for WSA (Windows Sockets API)
	#endif
	
private:

	/// onNewData
	virtual void onNewData(Conn *, string *);

	/// Main timer
	virtual int onTimer(Time & now);

	/// Main step in the cycle
	void step();

	/// Main base timer
	void onTimerBase(Time & now);

	size_t onRecv(Conn *);

	int newAccept();

	/// Add simple connection
	Conn * addSimpleConn(int connType, const char * ip, const char * port);

}; // class Server

} // server namespace

#endif // SERVER_H

/**
 * $Id$
 * $HeadURL$
 */
