/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
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
	webConn->mSelfConnFactory = this; /** Fuctory current connection (WebConnFactory) */

	WebUser * webUser = new WebUser(CLIENT_TYPE_WEB);
	webConn->mWebUser = webUser;
	webUser->mWebConn = webConn;

	return static_cast<Conn *> (webConn);

}



void WebConnFactory::deleteConn(Conn * &conn) {

	WebConn * webConn = static_cast<WebConn *> (conn);
	delete webConn->mWebUser;
	webConn->mWebUser = NULL;

	ConnFactory::deleteConn(conn);
}



void WebConnFactory::onNewData(Conn * conn, string * str) {

	if (conn->log(DEBUG)) {
		conn->logStream() << "WEB IN: " << (*str) << endl;
	}

	(*str).append(WEB_SEPARATOR);
	conn->remaining();
	if (!conn->isOk()) {
		return;
	}

	DcServer * dcServer = static_cast<DcServer *> (mServer);
	WebConn * webConn = static_cast<WebConn *> (conn);
	if (!dcServer || !webConn) {
		return;
	}

#ifndef WITHOUT_PLUGINS
	if (!dcServer->mCalls.mOnWebData.callAll(webConn->mWebUser))
#endif
	{
		conn->closeNice(9000, CLOSE_REASON_WEB);
	}
}



int WebConnFactory::onNewConn(Conn * conn) {
	conn->mProtocol = mProtocol; // Set protocol
	mServer->inputEvent(conn);
	return 0;
}



const char * WebUser::getCommand() {
	WebParser * webParser = static_cast<WebParser *> (mWebConn->mParser);
	return webParser->mCommand.c_str();
}



void WebUser::disconnect() {
	mWebConn->closeNice(9000, CLOSE_REASON_WEB);
}




WebConn::WebConn(tSocket sock, Server * server) : 
	Conn(sock, server, CONN_TYPE_INCOMING_TCP)
{
	setClassName("WebConn");
}



WebConn::~WebConn() {
}



DcServer * WebConn::server() {
	return static_cast<DcServer *> (mServer);
}



/** Timer of the connection */
int WebConn::onTimer(Time &) {
	DcServer * dcServer = server();
	Time lastRecv(mLastRecv);
	if (dcServer->minDelay(lastRecv, dcServer->mDcConfig.mWebTimeout)) {
		if (log(DEBUG)) {
			logStream() << "Any action timeout..." << endl;
		}
		closeNice(9000, CLOSE_REASON_WEB);
		return 1;
	}
	return 0;
}



/** Get string of ip */
const string & WebUser::getIp() {
	return mWebConn->getIp();
}



/** Get string of server IP (host) */
const string & WebUser::getIpConn() const {
	return mWebConn->getIpConn();
}



/** Get mac address */
const string & WebUser::getMacAddress() {
	return mWebConn->getMacAddress();
}



/** Get real port */
int WebUser::getPort() {
	return mWebConn->getPort();
}



/** Get conn port */
int WebUser::getPortConn() {
	return mWebConn->getPortConn();
}



size_t WebConn::send(const string & data, bool flush /* = true */) {
	if (!mWritable) {
		return 0;
	}
	return writeData(data.c_str(), data.size(), flush);
}


}; // namespace webserver

/**
 * $Id$
 * $HeadURL$
 */
