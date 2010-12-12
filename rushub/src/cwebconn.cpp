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

#include "cwebconn.h"
#include "cwebprotocol.h"
#include "cdcserver.h" /** for plugin func */

using namespace nDCServer;

namespace nWebServer {

using namespace nProtocol;

cWebConn::cWebConn(tSocket sock, cServer *s) : cDCConn(eT_WEB_CLIENT, sock, s) {
	SetClassName("cWebConn");
}

cWebConn::~cWebConn() {
}

/** Timer of the connection */
int cWebConn::OnTimer(cTime &now) {
	cDCServer * dcserver = Server();
	cTime lastRecv(mLastRecv);
	if(dcserver->MinDelay(lastRecv, dcserver->mDCConfig.miWebTimeout)) {
		if(Log(2)) LogStream() << "Any action timeout..." << endl;
		CloseNice(9000, eCR_WEB);
		return 1;
	}
	return 0;
}

int cWebConn::Send(const string & sData, bool bFlush) {
	if(!mbWritable) return 0;
	return WriteData(sData, bFlush);
}



cWebConnFactory::cWebConnFactory(cProtocol *protocol, cServer *s, string sSep, int iMax) : 
	cConnFactory(protocol, s)
{
	msSeparator = sSep;
	miStrSizeMax = iMax;
}

cWebConnFactory::~cWebConnFactory() {
	if(mProtocol) delete mProtocol; // only for WebProtocol!
	mProtocol = NULL;
}


cConn *cWebConnFactory::CreateConn(tSocket sock) {
	if(!mServer) return NULL;

	cWebConn * webconn = new cWebConn(sock, mServer);
	webconn->mConnFactory = this; /** Fuctory current connection (cWebConnFactory) */
	webconn->mProtocol = mProtocol; /** Protocol (cWebProtocol) */

	return (cConn *)webconn;
}

void cWebConnFactory::DelConn(cConn * &conn) {
	cConnFactory::DelConn(conn);
}

void cWebConnFactory::OnNewData(cConn * conn, string * str) {

	if(conn->Log(1)) conn->LogStream() << "WEB IN: " << (*str) << endl;

	(*str).append(WEB_SEPARATOR);
	if(conn->Remaining() < 0) return;

	cDCServer * Server = (cDCServer*)mServer;
	cWebConn * Conn = (cWebConn*) conn;
	if(!Server || !Conn) return;
#ifndef WITHOUT_PLUGINS
	cWebParser * Parser = (cWebParser*)Conn->mParser;
	if(!Server->mCalls.mOnWebData.CallAll(Conn, Parser))
#endif
	{
		conn->CloseNice(9000, eCR_WEB);
	}
}


cWebListenFactory::cWebListenFactory(cServer * Server) : cListenFactory(Server) {
	cWebProtocol * WebProtocol = new cWebProtocol;
	mWebConnFactory = new cWebConnFactory(WebProtocol, Server, "\r\n\r\n", ((cDCServer*)Server)->mDCConfig.miWebStrSizeMax);
}

cWebListenFactory::~cWebListenFactory() {
	if(mWebConnFactory) delete mWebConnFactory; // only for WebConnFactory!
	mWebConnFactory = NULL;
}

cConnFactory * cWebListenFactory::ConnFactory() {
	return mWebConnFactory;
}

int cWebListenFactory::OnNewConn(cConn * conn) {
	mServer->InputData(conn);
	return 0;//cListenFactory::OnNewConn(conn);
}

}; // nWebServer
