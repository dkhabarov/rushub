/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz
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

#ifndef ADC_PARSER_H
#define ADC_PARSER_H

#include "Protocol.h"
#include "Plugin.h"

using namespace ::server;

namespace dcserver {

namespace protocol {

typedef enum { // Types of the commands
	ADC_TYPE_NO = -1,
	ADC_TYPE_SUP = 248,
	ADC_TYPE_STA = 232,
	ADC_TYPE_INF = 221,
	ADC_TYPE_MSG = 231,
	ADC_TYPE_SCH = 222,
	ADC_TYPE_RES = 234,
	ADC_TYPE_CTM = 228,
	ADC_TYPE_RCM = 226,
	ADC_TYPE_GPA = 216,
	ADC_TYPE_PAS = 228,
	ADC_TYPE_QUI = 239,
	ADC_TYPE_GET = 224,
	ADC_TYPE_GFI = 214,
	ADC_TYPE_SND = 229,
	ADC_TYPE_SID = 224,
	ADC_TYPE_CMD = 212,
	ADC_TYPE_NAT = 227,
	ADC_TYPE_RNT = 244,
	ADC_TYPE_PSR = 245,
	ADC_TYPE_PUB = 231,
	ADC_TYPE_UNKNOWN,
} AdcType;

class AdcParser : public Parser {

public:

	AdcParser();
	virtual ~AdcParser();

	int getCommandType() const {
		return mType;
	}
	
	/// Do parse for command and return type of this command
	virtual int parse();
	
	virtual void reInit();
	

}; // class AdcParser

}; // namespace protocol

}; // namespace dcserver

#endif // ADC_PARSER_H

/**
 * $Id$
 * $HeadURL$
 */
