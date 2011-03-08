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


namespace webserver {

class WebConn;

namespace protocol {



class WebProtocol : public Protocol {

public:

	WebProtocol();
	virtual ~WebProtocol();

	void SetServer(Server *);

	/** Execution of the command */
	virtual int DoCmd(Parser *, Conn *);

	/** Creating parser of the protocol */
	virtual Parser * createParser();

	/** Removing parser of the protocol */
	virtual void deleteParser(Parser *);

protected:

	Server * mServer;

}; // class WebProtocol


}; // namespace protocol

}; // namespace webserver

#endif // WEB_PROTOCOL_H
