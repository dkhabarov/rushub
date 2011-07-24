
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
	virtual const char * getSeparator() {
		return "|";
	}
	virtual size_t getSeparatorLen() {
		return 1;
	}
	virtual unsigned long getMaxCommandLength() {
		return 102400;
	}
	virtual Conn * getConnForUdpData(Conn *, Parser *) {
		return NULL;
	}

}; // class Protocol



class NmdcClient : public Server {

public:
	ConnFactory * mConnFactory;
	int mConnCount;
	Time mChecker;

	const char * mIp;
	const char * mPort;
	int mMaxConn;
	int mBatch;

public:
	NmdcClient(const char * ip, const char * port, int maxConn, int batch, const char * logPath, int logLevel, int errLevel) : 
		Server(),
		mConnFactory(NULL),
		mConnCount(0),
		mIp(ip),
		mPort(port),
		mMaxConn(maxConn),
		mBatch(batch)
	{
		mLogsPath = new string(logPath);
		mMaxLevel = logLevel;
		mMaxErrLevel = errLevel;
	}

	~NmdcClient() {
		deleteAll();
	}

	void onNewData(Conn * conn, string * str) {
		const string & cmd(*str);

		//cout << cmd << endl;

		if (cmd.find("$Lock ", 0, 6) != cmd.npos) {
			string data("$Supports NoGetINFO NoHello UserIP2|$Key key|$ValidateNick ");
			data.append(static_cast<NmdcParser*> (conn->mParser)->mNick).append("|");
			size_t len = data.size();
			conn->writeData(data.c_str(), len, true);
		} else if (cmd.find("$Hello ", 0, 7) != cmd.npos) {
			string data("$Version 1,0091|$GetNickList|$MyINFO $ALL ");
			data.append(static_cast<NmdcParser*> (conn->mParser)->mNick).append(" $ $ $$0$|");
			size_t len = data.size();
			conn->writeData(data.c_str(), len, true);
		}
		str->clear();
	}

	int onTimer(Time & now) {
		__int64 diff = now.msec() - mChecker.msec();
		if (diff >= 0 ? diff >= mTimerServPeriod : -diff >= mTimerServPeriod) {
			mChecker = now;
			if (mConnCount < mMaxConn) {
				for (int i = 0; i < mBatch; ++i) {
					++mConnCount;
					Conn * conn = connecting(mConnFactory, mIp, mPort);
					if (conn != NULL) {
						ostringstream nick;
						nick << "test" << mConnCount;
						conn->mParser = conn->createParser();
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
		"  -errLevel <level>\tset error level" << endl <<
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
	int logLevel = 0;
	int errLevel = 2;

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
		} else if(!strcmp(last, "-errLevel")) {
			errLevel = atoi(argv[i]);
		}
		last = argv[i];
		++i;
	}
	
	if (batch > maxConn) {
		batch = maxConn;
	}
	if (batch > 100) {
		batch = 100;
	}

	NmdcClient client(ip, port, maxConn, batch, logPath, logLevel, errLevel);
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
