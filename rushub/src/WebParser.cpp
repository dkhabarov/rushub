/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * Copyright (C) 2009-2011 by Setuper
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

#include "WebParser.h"

namespace webserver {

namespace protocol {



WebParser::WebParser() : 
	Parser(5)
{
	setClassName("WebParser");
}



WebParser::~WebParser() {
}



/** Parse cmd and return type */
int WebParser::parse() {
	return NMDC_TYPE_UNPARSED;
}



/** Split cmd to chunks */
bool WebParser::splitChunks() {
	return false;
}

}; // namespace protocol

}; // namespace webserver

/**
 * $Id$
 * $HeadURL$
 */
