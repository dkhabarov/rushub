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

#ifndef WEB_PARSER_H
#define WEB_PARSER_H

#include "Protocol.h"
#include "Plugin.h"

using namespace ::server; // NMDC_TYPE_UNPARSED

namespace webserver {

namespace protocol {



class WebParser : public Parser, public WebParserBase {

public:

	WebParser();
	virtual ~WebParser();

	/** Parse cmd and return type */
	int parse();
	bool splitChunks();

}; // class WebParser


}; // namespace protocol

}; // namespace webserver

#endif // WEB_PARSER_H
