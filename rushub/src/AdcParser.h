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

enum Header {
	HEADER_HUB = 0,
	HEADER_DIRECT = 1,
	HEADER_ECHO = 2,
	HEADER_FEATURE = 3,
	HEADER_BROADCAST = 4,
	HEADER_CLIENT = 5,
	HEADER_UDP = 6,
	HEADER_INFO = 7,
	HEADER_UNKNOWN = 8
};

typedef enum { // Types of the commands
	ADC_TYPE_NO = -1,
	ADC_TYPE_SUP,
	ADC_TYPE_STA,
	ADC_TYPE_INF,
	ADC_TYPE_MSG,
	ADC_TYPE_SCH,
	ADC_TYPE_RES,
	ADC_TYPE_CTM,
	ADC_TYPE_RCM,
	ADC_TYPE_GPA,
	ADC_TYPE_PAS,
	ADC_TYPE_QUI,
	ADC_TYPE_GET,
	ADC_TYPE_GFI,
	ADC_TYPE_SND,
	ADC_TYPE_SID,
	ADC_TYPE_CMD,
	ADC_TYPE_NAT,
	ADC_TYPE_RNT,
	ADC_TYPE_PSR,
	ADC_TYPE_PUB,
	ADC_TYPE_VOID,
	ADC_TYPE_UNKNOWN,
	ADC_TYPE_INVALID
} AdcType;


enum {
	CHUNK_ADC_ALL
};

enum {
	CHUNK_ADC_STA_ALL,
	CHUNK_ADC_STA_CMD,
	CHUNK_ADC_STA_TO_SID,
	CHUNK_ADC_STA_CODE,
	CHUNK_ADC_STA_DESC,
	CHUNK_ADC_STA_OTHER
};

enum {
	CHUNK_ADC_INF_ALL,
	CHUNK_ADC_INF_CMD,
	CHUNK_ADC_INF_SID,
	CHUNK_ADC_INF_OTHER
};

enum {
	CHUNK_ADC_MSG_ALL,
	CHUNK_ADC_MSG_CMD,
	CHUNK_ADC_MSG_SID,
	CHUNK_ADC_MSG_TO_SID,
	CHUNK_ADC_MSG_TEXT,
	CHUNK_ADC_MSG_ME,
	CHUNK_ADC_MSG_PM,
	CHUNK_ADC_MSG_OTHER
};

enum {
	CHUNK_ADC_PARAMS_ALL,
	CHUNK_ADC_PARAMS_CMD,
	CHUNK_ADC_PARAMS_SID,
	CHUNK_ADC_PARAMS_OTHER
};

enum {
	CHUNK_ADC_SINGLE_ALL,
	CHUNK_ADC_SINGLE_CMD,
	CHUNK_ADC_SINGLE_DATA
};

enum {
	CHUNK_ADC_QUI_ALL,
	CHUNK_ADC_QUI_CMD,
	CHUNK_ADC_QUI_SID
};

class AdcParser : public Parser {

public:

	int mHeader;

public:

	AdcParser();
	virtual ~AdcParser();

	int getCommandType() const {
		return mType;
	}
	
	/// Do parse for command and return type of this command
	virtual int parse();
	
	virtual void reInit();

	bool splitChunks();

private:

	bool mError;

private:

	bool checkHeaderSyntax();

}; // class AdcParser

}; // namespace protocol

}; // namespace dcserver

#endif // ADC_PARSER_H

/**
 * $Id$
 * $HeadURL$
 */
