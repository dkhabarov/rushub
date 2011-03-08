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

#ifndef PROTOCOL_CMD_H
#define PROTOCOL_CMD_H

#include <string>

using namespace ::std;

namespace server {

/** Protocol command */
class ProtocolCommand {

public:
	/** Key-word of cmd */
	string mKey;

	/** Cmd len */
	size_t mLength;

public:
	ProtocolCommand();
	ProtocolCommand(string sKey);
	virtual ~ProtocolCommand();
	bool Check(const string & str);

}; // ProtocolCommand

}; // server

#endif // PROTOCOL_CMD_H
