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

#include <set>

using namespace ::server;

namespace dcserver {

class DcUser;

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
	const string & getSidSource() const;
	const string & getSidTarget() const;
	const string & getCidSource() const;
	const string & getErrorCode() const;
	const string & getErrorText() const;
	const vector<int> & getPositiveFeatures() const;
	const vector<int> & getNegativeFeatures() const;
	
	/// Do parse for command and return type of this command
	virtual int parse();
	
	virtual void reInit();

	bool splitChunks();

	static void parseFeatures(DcUser *, set<int> & features);
	static void parseInfo(DcUser *, const string & info, set<string> & names);
	static void formingInfo(DcUser *, string & info, const set<string> & names);

private:

	int mHeader;
	string mSidSource;
	string mSidTarget;
	string mCidSource;
	string mErrorCode;
	string mErrorText;
	size_t mBodyPos;
	bool mError;

	vector<int> mPositiveFeature;
	vector<int> mNegativeFeature;

private:

	bool checkHeaderSyntax();
	bool parseFeatures();
	void setError(const char * code, const char * text);

	static void replaceParam(ParamBase * param, const char * oldValue, const string & newValue);

}; // class AdcParser

}; // namespace protocol

}; // namespace dcserver

#endif // ADC_PARSER_H

/**
 * $Id$
 * $HeadURL$
 */
