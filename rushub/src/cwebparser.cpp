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

#include "cwebparser.h"

namespace nWebServer {

namespace nProtocol {

cWebParser::cWebParser() : cParser(5), cWebParserBase(mStr) {
	SetClassName("cWebParser");
}

cWebParser::~cWebParser() {
}

/** Parse cmd and return type */
int cWebParser::Parse() {
	return eUNPARSED;
}

/** Split cmd to chunks */
bool cWebParser::SplitChunks() {
	return false;
}

}; // nProtocol

}; // nWebServer
