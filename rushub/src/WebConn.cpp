/*
 * RusHub - hub server for Direct Connect peer to peer network.

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

#include "WebConn.h"
#include "WebProtocol.h"
#include "DcServer.h" /** for plugin func */

using namespace ::dcserver;

namespace webserver {

using namespace ::webserver::protocol;



WebConnFactory::WebConnFactory(Protocol * protocol, Server * server, string separator, int max) : 
	ConnFactory(protocol, server)
{
	msSeparator = separator;
	mStrSizeMax = max;
}



WebConnFactory::~WebConnFactory() {
	if (mProtocol) {
		delete mProtocol; // only for WebProtocol!
	}
	mProtocol = NULL;
}



Conn *WebConnFactory::CreateConn(tSocket sock) {

	if (!mServer) {
		return NULL;
	}

	WebConn * webconn = new WebConn(sock, mServer);
	webconn->mConnFactory = this; /** Fuctory current connection (WebConnFactory) */
	webconn->mProtocol = mProtocol; /** Protocol (WebProtocol) */

	return (Conn *)webconn;

}



void WebConnFactory::DelConn(Conn * &conn) {
	ConnFactory::DelConn(conn);
}



void WebConnFactory::OnNewData(Conn * conn, string * str) {

	if (conn->Log(1)) {
		conn->LogStream() << "WEB IN: " << (*str) << endl;
	}

	(*str).append(WEB_SEPARATOR);
	if (conn->Remaining() < 0) {
		return;
	}

	DcServer * dcServer = (DcServer*)mServer;
	WebConn * webConn = (WebConn*) conn;
	if (!dcServer || !webConn) {
		return;
	}
#ifndef WITHOUT_PLUGINS
	WebParser * webParser = (WebParser *)webConn->mParser;
	if (!dcServer->mCalls.mOnWebData.CallAll(webConn, webParser))
#endif
	{
		conn->CloseNice(9000, CLOSE_REASON_WEB);
	}
}





WebListenFactory::WebListenFactory(Server * server) : ListenFactory(server) {
	WebProtocol * webProtocol = new WebProtocol;
	mWebConnFactory = new WebConnFactory(webProtocol, server, "\r\n\r\n", ((DcServer*)server)->mDcConfig.miWebStrSizeMax);
}



WebListenFactory::~WebListenFactory() {
	if(mWebConnFactory) delete mWebConnFactory; // only for WebConnFactory!
	mWebConnFactory = NULL;
}



ConnFactory * WebListenFactory::connFactory() {
	return mWebConnFactory;
}



int WebListenFactory::OnNewConn(Conn * conn) {
	mServer->InputData(conn);
	return 0;//ListenFactory::OnNewConn(conn);
}





WebConn::WebConn(tSocket sock, Server * server) : DcConn(CLIENT_TYPE_WEB, sock, server) {
	SetClassName("WebConn");
}



WebConn::~WebConn() {
}



/** Timer of the connection */
int WebConn::onTimer(Time &) {
	DcServer * dcServer = server();
	Time lastRecv(mLastRecv);
	if (dcServer->MinDelay(lastRecv, dcServer->mDcConfig.miWebTimeout)) {
		if (Log(2)) {
			LogStream() << "Any action timeout..." << endl;
		}
		CloseNice(9000, CLOSE_REASON_WEB);
		return 1;
	}
	return 0;
}



/** Get string of ip */
const string & WebConn::getIp() {
	return msIp;
}



/** Get string of server IP (host) */
const string & WebConn::getIpConn() const {
	return msIpConn;
}



/** Get mac address */
const string & WebConn::getMacAddress() {
	return msMAC;
}



/** Get real port */
int WebConn::getPort() {
	return miPort;
}



/** Get conn port */
int WebConn::getPortConn() {
	return miPortConn;
}



unsigned long WebConn::getNetIp() {
	return miNetIp;
}



int WebConn::send(const string & data, bool flush /* = true */) {
	if (!mbWritable) {
		return 0;
	}
	return WriteData(data, flush);
}



void WebConn::disconnect() {
	CloseNice(9000, CLOSE_REASON_WEB);
}



}; // namespace webserver