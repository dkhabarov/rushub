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

#include "WebProtocol.h"
#include "Server.h" // for mServer
#include "WebConn.h" // for WebConn

namespace webserver {

namespace protocol {



WebProtocol::WebProtocol(unsigned int & maxCommandLength) :
	mServer(NULL),
	mMaxCommandLength(maxCommandLength)
{
	setClassName("WebProtocol");
}



WebProtocol::~WebProtocol() {
}



void WebProtocol::setServer(Server * server) {
	mServer = server;
}



/** Execution of the command */
int WebProtocol::doCommand(Parser *, Conn *) {
	return 0;
}



int WebProtocol::onNewConn(Conn *) {
	return 0;
}



/** Creating parser of the protocol */
Parser * WebProtocol::createParser() {
	return new WebParser;
}



/** Removing parser of the protocol */
void WebProtocol::deleteParser(Parser * parser) {
	if (parser != NULL) {
		delete parser;
	}
}



}; // namespace protocol

}; // namespace webserver

/**
 * $Id$
 * $HeadURL$
 */
