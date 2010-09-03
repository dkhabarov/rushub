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

#ifndef CWEBCONN_H
#define CWEBCONN_H

#include "cdcconn.h"
#include "cwebparserbase.h"
#include "cserver.h"

using namespace nServer;
using namespace nDCServer;

namespace nWebServer
{

class cWebConnFactory : public cConnFactory
{
public:
  cWebConnFactory(cProtocol *, cServer *, string sSep, int iMax);
  virtual ~cWebConnFactory();
  cConn * CreateConn(tSocket sock = 0);
  void DelConn(cConn * &);
  void OnNewData(cConn *, string *);
}; // cWebConnFactory

class cWebListenFactory : public cListenFactory
{
public:
  cWebListenFactory(cServer *);
  virtual ~cWebListenFactory();
  cConnFactory * ConnFactory();
  int OnNewConn(cConn *);
protected:
  cConnFactory * mWebConnFactory;
}; // cWebListenFactory


class cWebConn : public cDCConn
{
public:
  cWebConn(tSocket sock = 0, cServer *s = NULL);
  virtual ~cWebConn();

  virtual int OnTimer(cTime &now); /** Timer of the connection */

  virtual const string & GetIp(){ return msIp; } /** Get string of ip */
  virtual const string & GetMacAddr(){ return msMAC; } /** Get mac address */
  virtual int GetPort(){ return miPort; } /** Get real port */
  virtual int GetPortConn(){ return miPortConn; } /** Get conn port */
  virtual unsigned long GetNetIp(){ return miNetIp; }
  virtual int Send(const string & sData, bool bFlush = true);
  virtual void Disconnect(){ CloseNice(9000); }

}; // cWebConn

}; // nWebServer

#endif // CWEBCONN_H
