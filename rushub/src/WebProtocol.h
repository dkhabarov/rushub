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

#ifndef WEB_PROTOCOL_H
#define WEB_PROTOCOL_H

#include "Protocol.h"
#include "WebParser.h"

#include <string>

using namespace ::server;
using namespace ::std;

namespace server {
	class Server;
}; // namespace server


/// Web Server namespace
namespace webserver {

class WebConn;

/// Web Server Protocol namespace
namespace protocol {


/// Web protocol
class WebProtocol : public Protocol {

public:

	WebProtocol(unsigned long & maxCommandLength);
	virtual ~WebProtocol();

	void setServer(Server *);

	/** Execution of the command */
	virtual int doCommand(Parser *, Conn *);

	virtual Conn * getConnForUdpData(Conn *, Parser *) {
		return NULL;
	}

	virtual int onNewConn(Conn *);

	/** Creating parser of the protocol */
	virtual Parser * createParser();

	/** Removing parser of the protocol */
	virtual void deleteParser(Parser *);

	virtual const char * getSeparator() const {
		return "\r\n\r\n";
	}

	/** protocol separator length */
	virtual size_t getSeparatorLen() const {
		return 4;
	}

	virtual unsigned long getMaxCommandLength() const {
		return mMaxCommandLength;
	}

protected:

	Server * mServer;
	unsigned long & mMaxCommandLength;

private:

	WebProtocol & operator = (const WebProtocol &);

}; // class WebProtocol


}; // namespace protocol

}; // namespace webserver

#endif // WEB_PROTOCOL_H

/**
 * $Id$
 * $HeadURL$
 */
