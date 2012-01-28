/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * Copyright (C) 2009-2012 by Setuper
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

#ifndef DC_PROTOCOL_H
#define DC_PROTOCOL_H

#include "Protocol.h"

#include <string>

using namespace ::server;
using namespace ::std;

namespace dcserver {

class DcConn;
class DcServer;

namespace protocol {


/// DC protocol
class DcProtocol : public Protocol {

public:

	DcProtocol();
	virtual ~DcProtocol();

	void setServer(DcServer * dcServer) {
		mDcServer = dcServer;
	}

	virtual string & appendChat(string & str, const string & uid, const string & msg) = 0;
	virtual string & appendPm(string & str, const string & to, const string & from, const string & uid, const string & msg) = 0;

	virtual void forceMove(DcConn * dcConn, const char * address, const char * reason = NULL) = 0;
	virtual int sendNickList(DcConn *) = 0;

protected:

	DcServer * mDcServer;


}; // DcProtocol

}; // namespace protocol

}; // namespace dcserver

#endif // DC_PROTOCOL_H

/**
 * $Id$
 * $HeadURL$
 */
