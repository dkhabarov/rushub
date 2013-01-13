/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * Copyright (C) 2009-2013 by Setuper
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

#ifndef DC_CMD_H
#define DC_CMD_H

#include "Plugin.h"
#include "AdcProtocol.h"
#include "NmdcProtocol.h"
#include "stdinc.h"

#include <string>

using namespace ::std;
using namespace ::dcserver::protocol;

namespace dcserver {

namespace protocol {


class DcCmd {

public:

	DcCmd(int protocolType = DC_PROTOCOL_TYPE_ALL);
	~DcCmd();

	void parse(const string &);

	/// If nick is empty, then build simple char msg (without nick), else with this nick
	void buildChat(const string & data, const string & nick, bool toAll);
	void buildPm(const string & data, const string & nick, const string & from);

	void appendChat(int protocolType, string & str) const;
	void appendPm(int protocolType, string & str, const string & nick) const;

	const string & getChunk1(int protocolType) const;
	const string & getChunk2(int protocolType) const;

private:

	int mProtocolType;
	string mChunk1[DC_PROTOCOL_TYPE_SIZE];
	string mChunk2[DC_PROTOCOL_TYPE_SIZE];

}; // class DcCmd


} // namespace protocol

} // namespace dcserver

#endif // DC_CMD_H

/**
 * $Id$
 * $HeadURL$
 */
