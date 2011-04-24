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



WebConnFactory::WebConnFactory(Protocol * protocol, Server * server) : 
	ConnFactory(protocol, server)
{
}



WebConnFactory::~WebConnFactory() {
}



Conn * WebConnFactory::createConn(tSocket sock) {

	if (!mServer) {
		return NULL;
	}

	WebConn * webConn = new WebConn(sock, mServer);
	webConn->setCreatedByFactory(true);
	webConn->mConnFactory = this; /** Fuctory current connection (WebConnFactory) */
	webConn->mProtocol = mProtocol; /** Protocol (WebProtocol) */

	return static_cast<Conn *> (webConn);

}



void WebConnFactory::deleteConn(Conn * &conn) {
	ConnFactory::deleteConn(conn);
}



void WebConnFactory::onNewData(Conn * conn, string * str) {

	if (conn->Log(1)) {
		conn->LogStream() << "WEB IN: " << (*str) << endl;
	}

	(*str).append(WEB_SEPARATOR);
	if (conn->remaining() < 0) {
		return;
	}

	DcServer * dcServer = static_cast<DcServer *> (mServer);
	WebConn * webConn = static_cast<WebConn *> (conn);
	if (!dcServer || !webConn) {
		return;
	}
#ifndef WITHOUT_PLUGINS
	WebParser * webParser = static_cast<WebParser *> (webConn->mParser);
	if (webParser && !dcServer->mCalls.mOnWebData.callAll(webConn, webParser))
#endif
	{
		conn->closeNice(9000, CLOSE_REASON_WEB);
	}
}



int WebConnFactory::onNewConn(Conn * conn) {
	mServer->inputData(conn);
	return 0;
}




WebConn::WebConn(tSocket sock, Server * server) : 
	DcConn(CLIENT_TYPE_WEB, sock, server)
{
	SetClassName("WebConn");
}



WebConn::~WebConn() {
}



/** Timer of the connection */
int WebConn::onTimer(Time &) {
	DcServer * dcServer = server();
	Time lastRecv(mLastRecv);
	if (dcServer->minDelay(lastRecv, dcServer->mDcConfig.mWebTimeout)) {
		if (Log(2)) {
			LogStream() << "Any action timeout..." << endl;
		}
		closeNice(9000, CLOSE_REASON_WEB);
		return 1;
	}
	return 0;
}



/** Get string of ip */
const string & WebConn::getIp() {
	return mIp;
}



/** Get string of server IP (host) */
const string & WebConn::getIpConn() const {
	return mIpConn;
}



/** Get mac address */
const string & WebConn::getMacAddress() {
	return mMac;
}



/** Get real port */
int WebConn::getPort() {
	return mPort;
}



/** Get conn port */
int WebConn::getPortConn() {
	return mPortConn;
}



unsigned long WebConn::getNetIp() {
	return mNetIp;
}



int WebConn::send(const string & data, bool flush /* = true */) {
	if (!mWritable) {
		return 0;
	}
	return writeData(data, flush);
}



void WebConn::disconnect() {
	closeNice(9000, CLOSE_REASON_WEB);
}



}; // namespace webserver
