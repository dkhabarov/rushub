/*
 * RusHub - hub server for Direct Connect peer to peer network.

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

#ifndef CWEBPARSERBASE_H
#define CWEBPARSERBASE_H

#include <string>

using namespace std;

#ifndef WEB_SEPARATOR
	#define WEB_SEPARATOR "\r\n\r\n" /** Protocol separator */
#endif

namespace nWebServer {

class cWebParserBase {
public:
	string & msStr; /** Ref to string with cmd */
public:
	cWebParserBase(string & sStr):msStr(sStr) {}

}; // cWebParserBase

}; // nWebServer

#endif // CWEBPARSERBASE_H
