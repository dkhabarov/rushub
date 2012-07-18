/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
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

#include "stdinc.h"
#include "Server.h"
#include "Conn.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <signal.h>

using namespace ::std;
using namespace ::server;


#ifdef _WIN32
	#define SIGQUIT 1000
	#define SIGHUP 1001
	#define SIGTSTP 1002
#endif

static Server * curServer = NULL;


static void sigHandler(int sig) {

	switch (sig) {

		case SIGINT :
			// Fallthrough

		case SIGTERM :
			// Fallthrough

		case SIGQUIT :
			// Fallthrough

		case SIGTSTP :
			// Fallthrough

		case SIGHUP :
			cout << "Received a " << sig << " signal, quiting" << endl;
			curServer->stop(0);
			signal(sig, sigHandler);
			break;

		default :
			cout << "Received a " << sig << " signal, ignoring it" << endl;
			signal(sig, sigHandler);
			break;
	}
}




class NmdcParser : public Parser {
public:
	string mNick;
	NmdcParser() : Parser(9) {}
	virtual ~NmdcParser() {}
	virtual int parse() {
		return 1;
	}
}; // class NmdcParser



class NmdcProtocol : public Protocol {

public:

	NmdcProtocol() {
	}
	virtual ~NmdcProtocol() {
	}
	virtual int doCommand(Parser *, Conn *) {
		return 0;
	}
	virtual Parser * createParser() {
		return new NmdcParser();
	}
	virtual void deleteParser(Parser * parser) {
		delete parser;
	}
	virtual const char * getSeparator() const {
		return "|";
	}
	virtual size_t getSeparatorLen() const {
		return 1;
	}
	virtual unsigned int getMaxCommandLength() const {
		return 102400;
	}
	virtual Conn * getConnForUdpData(Conn *, Parser *) {
		return NULL;
	}
	virtual int onNewConn(Conn *) {
		return 0;
	}

}; // class Protocol



class NmdcClient : public Server {

public:

	ConnFactory * mConnFactory;

public:

	NmdcClient(const char * ip, const char * port, int maxConn, int batch, const char * logPath, int logLevel) : 
		Server(),
		mConnFactory(NULL),
		mIp(ip),
		mPort(port),
		mConnCount(0),
		mMaxConn(maxConn),
		mBatch(batch)
	{
		mLogsPath = new string(logPath);
		mMaxLevel = logLevel;
	}

	~NmdcClient() {
		deleteAll();
	}

private:

	const char * mIp;
	const char * mPort;
	int mConnCount;
	int mMaxConn;
	int mBatch;
	
	Time mChecker;

private:

	void onNewData(Conn * conn, string * str) {
		const string & cmd(*str);

		//cout << cmd << endl;

		if (cmd.find("$Lock ", 0, 6) != cmd.npos) {
			string data;
			data.reserve(150);
			data.append(STR_LEN("$Supports NoGetINFO NoHello UserIP2|$Key key|$ValidateNick "));
			data.append(static_cast<NmdcParser*> (conn->mParser)->mNick);
			data.append(STR_LEN("|"));
			conn->writeData(data.c_str(), data.size(), true);
		} else if (cmd.find("$Hello ", 0, 7) != cmd.npos) {
			string data;
			data.reserve(150);
			data.append(STR_LEN("$Version 1,0091|$GetNickList|$MyINFO $ALL "));
			data.append(static_cast<NmdcParser*> (conn->mParser)->mNick);
			data.append(STR_LEN(" any description<TestDC++ 1.00,M:A,H:1/0/0,S:32>$ $1000$some@email.ru$1234554321$|"));
			conn->writeData(data.c_str(), data.size(), true);
		}
		str->clear();
	}

	int onTimer(Time & now) {
		int64_t diff = now.msec() - mChecker.msec();
		if (diff >= 0 ? diff >= mTimerServPeriod : -diff >= mTimerServPeriod) {
			mChecker = now;
			if (mConnCount < mMaxConn) {
				for (int i = 0; i < mBatch; ++i) {
					++mConnCount;
					Conn * conn = connecting(mConnFactory, mIp, mPort);
					if (conn != NULL) {
						ostringstream nick;
						nick << "test" << mConnCount;
						conn->getParserCommandPtr();
						static_cast<NmdcParser*> (conn->mParser)->mNick = nick.str();
					} else {
						--mConnCount;
						return 0;
					}
				}
			}
		}
		return 0;
	}

}; // class NmdcClient


void printHelp() {
	cout << "stress-test client build on "__DATE__" "__TIME__ << endl << endl <<
		"Usage: client [OPTIONS] ..." << endl << endl <<
		"Options:" << endl <<
		"  -ip <ip>\t\tset ip address to connect" << endl <<
		"  -port <port>\t\tset port to connect" << endl <<
		"  -max <max>\t\tset max connections" << endl <<
		"  -batch <batch>\tset entering batch" << endl <<
		"  -logPath <path>\tset log path" << endl <<
		"  -logLevel <level>\tset log level" << endl <<
		"  -help\t\t\tshow this help" << endl;
}

int main(int argc, char ** argv) {

	signal(SIGINT, sigHandler);
	signal(SIGTERM, sigHandler);

	const char * last = "";
	const char * ip = "127.0.0.1";
	const char * port = "411";
	const char * logPath = "";
	int maxConn = 50;
	int batch = 25;
	int logLevel = LEVEL_INFO;

	if (argc == 1) {
		printHelp();
		return 1;
	}

	int i = 0;
	while (i < argc) {
		if(!strcmp(argv[i], "-help")) {
			printHelp();
			return 1;
		} else if(!strcmp(last, "-ip")) {
			ip = argv[i];
		} else if(!strcmp(last, "-port")) {
			port = argv[i];
		} else if(!strcmp(last, "-max")) {
			maxConn = atoi(argv[i]);
		} else if(!strcmp(last, "-batch")) {
			batch = atoi(argv[i]);
		} else if(!strcmp(last, "-logPath")) {
			logPath = argv[i];
		} else if(!strcmp(last, "-logLevel")) {
			logLevel = atoi(argv[i]);
		}
		last = argv[i];
		++i;
	}
	
	if (batch > maxConn) {
		batch = maxConn;
	}
	if (batch > 100) { // max 100
		batch = 100;
	}

	NmdcClient client(ip, port, maxConn, batch, logPath, logLevel);
	curServer = &client;

	NmdcProtocol nmdcProtocol;
	client.mConnFactory = new ConnFactory(&nmdcProtocol, &client);
	client.run();

	return 0;
}

/**
 * $Id$
 * $HeadURL$
 */
