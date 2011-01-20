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

#ifndef CWEBCONN_H
#define CWEBCONN_H

#include "cdcconn.h"
#include "cplugin.h"
#include "cserver.h"

using namespace nServer;
using namespace nDCServer;

namespace nWebServer {

class cWebConnFactory : public cConnFactory {
public:
	cWebConnFactory(cProtocol *, cServer *, string sSep, int iMax);
	virtual ~cWebConnFactory();
	cConn * CreateConn(tSocket sock = 0);
	void DelConn(cConn * &);
	void OnNewData(cConn *, string *);
}; // cWebConnFactory

class cWebListenFactory : public cListenFactory {
public:
	cWebListenFactory(cServer *);
	virtual ~cWebListenFactory();
	cConnFactory * ConnFactory();
	int OnNewConn(cConn *);
protected:
	cConnFactory * mWebConnFactory;
}; // cWebListenFactory


class cWebConn : public cDCConn {
public:
	cWebConn(tSocket sock = 0, cServer *s = NULL);
	virtual ~cWebConn();

	virtual int onTimer(cTime &now); /** Timer of the connection */

	virtual const string & getIp() { return msIp; } /** Get string of ip */
	virtual const string & getIpConn() const { return msIpConn; } /** Get string of server IP (host) */
	virtual const string & getMacAddress() { return msMAC; } /** Get mac address */
	virtual int getPort() { return miPort; } /** Get real port */
	virtual int getPortConn() { return miPortConn; } /** Get conn port */
	virtual unsigned long getNetIp() { return miNetIp; }
	virtual int Send(const string & sData, bool bFlush = true);
	virtual void disconnect() { CloseNice(9000, eCR_WEB); }

}; // cWebConn

}; // nWebServer

#endif // CWEBCONN_H
