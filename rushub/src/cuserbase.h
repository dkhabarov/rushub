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

#ifndef CUSERBASE_H
#define CUSERBASE_H

#include <string>
using namespace std;

namespace nDCServer {

/** Base user class for quick list */
class cUserBase {

public:

	virtual bool CanSend() = 0;
	virtual void Send(const string &msg, bool sep = false, bool flush = true) = 0;
	virtual const string & Nick() const = 0;
	virtual const string & MyINFO() const = 0;
	virtual const string & GetIp() const = 0;
	virtual bool Hide() const = 0;
	virtual int GetProfile() const = 0;

}; // cUserBase

}; // nDCServer

#endif // CUSERBASE_H
