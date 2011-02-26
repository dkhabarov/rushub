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

#ifndef WEB_CONN_H
#define WEB_CONN_H

#include "DcConn.h"
#include "Plugin.h"
#include "Server.h"

using namespace ::server;
using namespace ::dcserver;

namespace webserver {

class WebConnFactory : public ConnFactory {

public:

	WebConnFactory(Protocol *, Server *, string separator, int max);

	virtual ~WebConnFactory();

	Conn * CreateConn(tSocket sock = 0);

	void DelConn(Conn * &);

	void OnNewData(Conn *, string *);

}; // class WebConnFactory



class WebListenFactory : public ListenFactory {

public:

	WebListenFactory(Server *);

	virtual ~WebListenFactory();

	ConnFactory * connFactory();

	int OnNewConn(Conn *);

protected:

	ConnFactory * mWebConnFactory;

}; // class WebListenFactory



class WebConn : public DcConn {

public:

	WebConn(tSocket sock = 0, Server * server = NULL);

	virtual ~WebConn();

	/** Timer of the connection */
	virtual int onTimer(Time & now);

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

	virtual unsigned long getNetIp();

	virtual int send(const string & data, bool flush = true);

	virtual void disconnect();

}; // class WebConn

}; // namespace webserver

#endif // WEB_CONN_H
