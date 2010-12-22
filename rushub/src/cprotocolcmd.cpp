/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz

 * modified: 10 Dec 2009
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

#include "cprotocolcmd.h"
#include "stringutils.h"

using namespace nUtils;

namespace nServer {

cProtocolCmd::cProtocolCmd() {
}

cProtocolCmd::cProtocolCmd(string sKey) : mKey(sKey) { mLength = mKey.length(); }

cProtocolCmd::~cProtocolCmd() {
}

bool cProtocolCmd::Check(const string &sStr) { /** Checking that string contains command */
	return 0 == StrCompare(sStr, 0, mLength, mKey);
}

};
