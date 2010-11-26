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

#ifndef CDCCONNBASE_H
#define CDCCONNBASE_H

#include "cdcuserbase.h"
#include <string>

using ::std::string;

namespace nDCServer {

class cDCConnBase {

public:
	cDCUserBase * mDCUserBase; //< User
	int _miConnType; //< Connection type (for protection and compatibility)

public:
	cDCConnBase() : mDCUserBase(NULL), _miConnType(1){}
	~cDCConnBase(){}
	virtual int Send(const string & sData, bool bAddSep = false, bool bFlush = true) = 0; //< Sending RAW cmd to the client
	virtual const string & GetVersion() const = 0; //< Client's protocol version
	virtual const string & GetIp() const = 0; //< Get string of IP
	virtual const string & GetData() const = 0; //< Get some user data
	virtual const string & GetMacAddr() const = 0; //< Get mac address
	virtual const string & GetSupports() const = 0;
	virtual int GetPort() const = 0; //< Get real port
	virtual int GetPortConn() const = 0; //< Get connection port
	virtual int GetProfile() const = 0; //< Get profile
	virtual unsigned long GetNetIp() const = 0; //< Get numeric IP
	virtual void SetProfile(int) = 0;
	virtual void SetData(const string &) = 0;
	virtual void Disconnect() = 0;
	virtual long GetEnterTime() const = 0; //< Get enter time

}; // cDCConnBase

}; // nDCServer

#endif // CDCCONNBASE_H
