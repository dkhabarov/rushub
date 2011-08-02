/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz
 *
 * modified: 27 Aug 2009
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

#include "AdcParser.h"

namespace dcserver {

namespace protocol {


enum {
	HEADER_HUB = 0,
	HEADER_DIRECT = 1,
	HEADER_ECHO = 2,
	HEADER_FEATURE = 3,
	HEADER_BROADCAST = 4,
	HEADER_CLIENT = 5,
	HEADER_UDP = 6,
	HEADER_INFO = 7,
	HEADER_UNKNOWN = 8
} Header;


AdcParser::AdcParser() :
	Parser(9) // Max number of chunks - 9 !!!
{ 
	setClassName("AdcParser");
}



AdcParser::~AdcParser() {
}



/// Do parse for command and return type of this command
int AdcParser::parse() {
	mLength = mCommand.size(); // Set cmd len
	if (mLength >= 4) { // ADC cmd key contain 4 symbols

		mHeader = getHeader(mCommand[0]);

	}
	return mType;
}



void AdcParser::reInit() {
	Parser::reInit();
}


int AdcParser::getHeader(char c) {

	switch (c) {

		case 'H': // Hub message
			return HEADER_HUB;

		case 'D': // Direct message
			return HEADER_DIRECT;

		case 'E': // Echo message
			return HEADER_ECHO;

		case 'F': // Feature broadcast
			return HEADER_FEATURE;

		case 'B': // Broadcast
			return HEADER_BROADCAST;

		case 'C': // Client message
			return HEADER_CLIENT;

		case 'U': // UDP message
			return HEADER_UDP;

		case 'I': // Info message
			return HEADER_INFO;

		default: // Unknown
			return HEADER_UNKNOWN;

	}

}

}; // namespace protocol

}; // namespace dcserver

/**
 * $Id$
 * $HeadURL$
 */
