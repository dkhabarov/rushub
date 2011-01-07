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

#ifndef CWEBPROTOCOL_H
#define CWEBPROTOCOL_H

#include "cprotocol.h"
#include "cwebparser.h"

#include <string>

using namespace nServer;
using namespace std;

namespace nServer { class cServer; };

namespace nWebServer {

class cWebConn;

namespace nProtocol {

class cWebProtocol : public cProtocol {

public:
	cWebProtocol();
	virtual ~cWebProtocol();
	void SetServer(cServer *);
	virtual int DoCmd(cParser *, cConn *); /** Execution of the command */
	virtual cParser * CreateParser() { return new cWebParser; } /** Creating parser of the protocol */
	virtual void DeleteParser(cParser *parser) { if(parser != NULL) delete parser; } /** Removing parser of the protocol */

protected:
	cServer * mServer;

}; // cWebProtocol

}; // nProtocol

}; // nWebServer

#endif // CWEBPROTOCOL_H
