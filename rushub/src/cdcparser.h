/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz

 * modified: 27 Aug 2009
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

#ifndef CDCPARSER_H
#define CDCPARSER_H

#include "cprotocol.h"
#include "cprotocolcmd.h" // for cProtocolCmd
#include "cplugin.h"

using namespace nServer;
using nDCServer::cDCParserBase;

namespace nDCServer {

namespace nProtocol {

using namespace nDCServer::protoenums;

class cDCParser : public cParser, public cDCParserBase {
public:
	cDCParser();
	virtual ~cDCParser();
	int Parse(); /** Do parse for command and return type of this command */
	string & ChunkString(unsigned int n); /** Get string address for the chunk of command */
	bool SplitChunks();
	int GetType() const { return miType; }
	static bool IsPassive(const string & sDesc);
}; // cDCParser

}; // nProtocol

}; // nDCServer

#endif // CDCPARSER
