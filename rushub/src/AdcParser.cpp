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



AdcParser::AdcParser() :
	Parser(9) // Max number of chunks - 9 !!!
{ 
	setClassName("AdcParser");
}



AdcParser::~AdcParser() {
}



/// Do parse for command and return type of this command
int AdcParser::parse() {
	return mType;
}



void AdcParser::reInit() {
	Parser::reInit();
}


}; // namespace protocol

}; // namespace dcserver

/**
 * $Id$
 * $HeadURL$
 */
