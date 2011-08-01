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

#ifndef WEB_CONN_H
#define WEB_CONN_H

#include "Conn.h"
#include "Plugin.h"
#include "Server.h"

using namespace ::server;
using namespace ::dcserver;

namespace dcserver {
	class DcServer;
};

namespace webserver {

class WebConnFactory : public ConnFactory {

public:

	WebConnFactory(Protocol *, Server *);
	virtual ~WebConnFactory();

	Conn * createConn(tSocket sock = 0);
	void deleteConn(Conn * &);
	void onNewData(Conn *, string *);
	int onNewConnClient(Conn * newConn, Conn * builderConn);
	int onNewConnServer(Conn * newConn, Conn * builderConn);

}; // class WebConnFactory



class WebConn;



class WebUser : public WebUserBase {

public:

	WebConn * mWebConn;

public:

	WebUser(int type) : WebUserBase(type) {}
	virtual ~WebUser() {}

	virtual const char * getCommand();

	/// Disconnect this client
	virtual void disconnect();

	/** Get string of ip */
	virtual const string & getIp();

	/** Get string of server IP (host) */
	virtual const string & getIpConn() const;

	/** Get mac address */
	virtual const string & getMacAddress();

	/** Get real port */
	virtual int getPort();

	/** Get conn port */
	virtual int getPortConn();

}; // class WebUser



class WebConn : public Conn {

public:

	WebUser * mWebUser;

public:

	WebConn(tSocket sock = 0, Server * server = NULL);
	virtual ~WebConn();

	/** Timer of the connection */
	virtual int onTimer(Time & now);

	virtual size_t send(const string & data, bool flush = true);

	DcServer * server();


}; // class WebConn

}; // namespace webserver

#endif // WEB_CONN_H

/**
 * $Id$
 * $HeadURL$
 */
