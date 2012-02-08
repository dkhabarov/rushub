/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz
 *
 * modified: 27 Aug 2009
 * Copyright (C) 2009-2012 by Setuper
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

#include <set>

using namespace ::server;

namespace dcserver {

class DcUser;

namespace protocol {

enum Header {
	HEADER_BROADCAST = 0,
	HEADER_CLIENT = 1,
	HEADER_DIRECT = 2,
	HEADER_ECHO = 3,
	HEADER_FEATURE = 4,
	HEADER_HUB = 5,
	HEADER_INFO = 6,
	HEADER_UDP = 7,
	HEADER_UNKNOWN = 8
}; // enum Header


const char HEADER_SYMBOL_BROADCAST = 'B';
const char HEADER_SYMBOL_CLIENT = 'C';
const char HEADER_SYMBOL_DIRECT = 'D';
const char HEADER_SYMBOL_ECHO = 'E';
const char HEADER_SYMBOL_FEATURE = 'F';
const char HEADER_SYMBOL_HUB = 'H';
const char HEADER_SYMBOL_INFO = 'I';
const char HEADER_SYMBOL_UDP = 'U';


typedef enum { // Types of the commands
	ADC_TYPE_UNKNOWN = 0,
	ADC_TYPE_VOID,
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
	ADC_TYPE_INVALID // Always last
} AdcType;


enum ErrorCode {
	ERROR_CODE_GENERIC = 0,
	ERROR_CODE_GENERIC_HUB = 10,
	ERROR_CODE_HUB_FULL = 11,
	ERROR_CODE_HUB_DISABLED = 12,
	ERROR_CODE_GENERIC_LOGIN = 20,
	ERROR_CODE_NICK_INVALID = 21,
	ERROR_CODE_NICK_TAKEN = 22,
	ERROR_CODE_INVALID_PASSWORD = 23,
	ERROR_CODE_CID_TAKEN = 24,
	ERROR_CODE_ACCESS_DENIED = 25,
	ERROR_CODE_REGISTRED_ONLY = 26,
	ERROR_CODE_INVALID_PID = 27,
	ERROR_CODE_BAN_GENERIC = 30,
	ERROR_CODE_PERM_BAN = 31,
	ERROR_CODE_TEMP_BAN = 32,
	ERROR_CODE_PROTOCOL_ERROR = 40,
	ERROR_CODE_PROTOCOL_UNSUPPORTED = 41,
	ERROR_CODE_CONNECT_FAILED = 42,
	ERROR_CODE_INF_INVALID = 43,
	ERROR_CODE_INVALID_STATE = 44,
	ERROR_CODE_FEATURE_MISSING = 45,
	ERROR_CODE_INVALID_IP = 46,
	ERROR_CODE_NO_HUB_SUPPORT = 47,
	ERROR_CODE_TRANSFER_ERROR = 50,
	ERROR_CODE_FILE_NOT_AVAILABLE = 51,
	ERROR_CODE_FILE_PART_NOT_AVAILABLE = 52,
	ERROR_CODE_SLOTS_FULL = 53,
	ERROR_CODE_NO_CLIENT_HASH = 54
}; // enum Error


enum SeverityLevel {
	SEVERITY_LEVEL_SUCCESS = 0,
	SEVERITY_LEVEL_RECOVERABLE = 1,
	SEVERITY_LEVEL_FATAL = 2
};


enum {
	CHUNK_ADC_STA_CMD,
	CHUNK_ADC_STA_CODE,
	CHUNK_ADC_STA_DESC,
	CHUNK_ADC_STA_OTHER
};

enum {
	CHUNK_ADC_INF_CMD,
	CHUNK_ADC_INF_OTHER
};

enum {
	CHUNK_ADC_MSG_CMD,
	CHUNK_ADC_MSG_TEXT,
	CHUNK_ADC_MSG_ME,
	CHUNK_ADC_MSG_PM,
	CHUNK_ADC_MSG_OTHER
};


/// ADC commands parser
class AdcParser : public Parser {

public:

	AdcParser();
	virtual ~AdcParser();

	int getCommandType() const;
	int getHeader() const;
	int getErrorCode() const;
	const string & getErrorText() const;
	const string & getSidSource() const;
	const string & getSidTarget() const;
	const string & getCidSource() const;
	const vector<int> & getPositiveFeatures() const;
	const vector<int> & getNegativeFeatures() const;
	
	/// Do parse for command and return type of this command
	virtual int parse();
	
	virtual void reInit();

	bool splitChunks();

	static void parseInfo(DcUser *, const string & info);
	static void formingInfo(DcUser *, string & info);

private:

	int mHeader;
	int mErrorCode;
	string mErrorText;
	string mSidSource;
	string mSidTarget;
	string mCidSource;
	size_t mBodyPos;
	bool mError;

	vector<int> mPositiveFeature;
	vector<int> mNegativeFeature;

private:

	bool parseHeader();
	bool parseFeatures();
	void setError(int code, const char * text, size_t textLen);

	static void replaceParam(ParamBase * param, const char * oldValue, const string & newValue);
	static void parseFeatures(DcUser *);

}; // class AdcParser

}; // namespace protocol

}; // namespace dcserver

#endif // ADC_PARSER_H

/**
 * $Id$
 * $HeadURL$
 */
