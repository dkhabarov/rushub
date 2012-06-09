/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz
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

#include "AdcParser.h"
#include "DcUser.h"
#include "stringutils.h"

using namespace utils;

namespace dcserver {

namespace protocol {

#define FOURCC(c) (*(uint32_t *)c)
#define CMD(c) (_LITTLE_ENDIAN ? (FOURCC(c) >> 8) : (FOURCC(c) << 8))

/// ADC command
class AdcCommand {

public:

	static const int mLen = 3;

public:

	AdcCommand(uint32_t command, AdcType adcType) :
		mCommand(command),
		mAdcType(adcType)
	{
	}

	virtual ~AdcCommand() {
	}

	bool check(uint32_t command) const {
		return mCommand == command;
	}

	AdcType getAdcType() const {
		return mAdcType;
	}

private:

	//hash = *reinterpret_cast<const unsigned int*>(cmd);
	const uint32_t mCommand;
	const AdcType mAdcType;

	const AdcCommand & operator = (const AdcCommand &); // for gcc

}; // AdcCommand



/// Main ADC commands
AdcCommand AdcCommands[] = {
	AdcCommand(FOURCC("SCH"), ADC_TYPE_SCH), // SCH
	AdcCommand(FOURCC("RES"), ADC_TYPE_RES), // RES
	AdcCommand(FOURCC("CTM"), ADC_TYPE_CTM), // CTM
	AdcCommand(FOURCC("RCM"), ADC_TYPE_RCM), // RCM
	AdcCommand(FOURCC("SUP"), ADC_TYPE_SUP), // SUP
	AdcCommand(FOURCC("STA"), ADC_TYPE_STA), // STA
	AdcCommand(FOURCC("INF"), ADC_TYPE_INF), // INF
	AdcCommand(FOURCC("MSG"), ADC_TYPE_MSG), // MSG
	AdcCommand(FOURCC("GPA"), ADC_TYPE_GPA), // GPA
	AdcCommand(FOURCC("PAS"), ADC_TYPE_PAS), // PAS
	AdcCommand(FOURCC("QUI"), ADC_TYPE_QUI), // QUI
	AdcCommand(FOURCC("SID"), ADC_TYPE_SID), // SID
	AdcCommand(FOURCC("CMD"), ADC_TYPE_CMD), // CMD
	AdcCommand(FOURCC("GFI"), ADC_TYPE_GFI), // GFI
	AdcCommand(FOURCC("NAT"), ADC_TYPE_NAT), // NAT
	AdcCommand(FOURCC("RNT"), ADC_TYPE_RNT), // RNT
	AdcCommand(FOURCC("PSR"), ADC_TYPE_PSR), // PSR
	AdcCommand(FOURCC("PUB"), ADC_TYPE_PUB), // PUB
	AdcCommand(FOURCC("GET"), ADC_TYPE_GET), // GET
	AdcCommand(FOURCC("SND"), ADC_TYPE_SND), // SND
};



AdcParser::AdcParser() :
	Parser(), // Parser with free number of chunks
	mHeader(HEADER_UNKNOWN),
	mErrorCode(ERROR_CODE_GENERIC),
	mBodyPos(0),
	mError(false)
{
	setClassName("AdcParser");
}



AdcParser::~AdcParser() {
}



int AdcParser::getCommandType() const {
	return mType;
}



int AdcParser::getHeader() const {
	return mHeader;
}



const string & AdcParser::getSidSource() const {
	return mSidSource;
}



const string & AdcParser::getSidTarget() const {
	return mSidTarget;
}



const string & AdcParser::getCidSource() const {
	return mCidSource;
}



int AdcParser::getErrorCode() const {
	return mErrorCode;
}



const string & AdcParser::getErrorText() const {
	return mErrorText;
}



const vector<int> & AdcParser::getPositiveFeatures() const {
	return mPositiveFeature;
}



const vector<int> & AdcParser::getNegativeFeatures() const {
	return mNegativeFeature;
}



/// Do parse for command and return type of this command
int AdcParser::parse() {

	mLength = mCommand.size(); // Set command len

	if (mLength >= 4) { // ADC cmd key must contain 4 symbols, else it's invalid cmd

		if (parseHeader()) {
			uint32_t cmd = CMD(mCommand.c_str());
			for (unsigned int i = 0; i < sizeof(AdcCommands) / sizeof(AdcCommands[0]); ++i) {
				AdcCommand & adcCommand = AdcCommands[i];
				if (adcCommand.check(cmd)) { // Check command from mCommand
					return mType = adcCommand.getAdcType(); // Set cmd type
				}
			}
			return mType = ADC_TYPE_UNKNOWN; // Unknown command
		}

	} else if (mLength == 0) {
		return mType = ADC_TYPE_VOID; // Void command (ping)
	}

	return mType = ADC_TYPE_INVALID; // Invalid command
}



bool AdcParser::parseHeader() {

	// Check command name ([A-Z] [A-Z0-9] [A-Z0-9])
	if (!isUpperAlpha(mCommand[1]) || !isUpperAlphaNum(mCommand[2]) || !isUpperAlphaNum(mCommand[3])) {
		// Invalid command name
		setError(ERROR_CODE_PROTOCOL_ERROR, STR_LEN("Invalid command name"));
		return false;
	}

	unsigned int counter = 0;

	switch (mCommand[0]) {

		case HEADER_SYMBOL_BROADCAST: // Broadcast
			// 'B' command_name ' ' base32_character{4} ...
			mHeader = HEADER_BROADCAST;
			if (mLength < 9 || mCommand[4] != ' ') {
				setError(ERROR_CODE_PROTOCOL_ERROR, STR_LEN("Invalid source SID length"));
				return false;
			} else if (!isBase32(mCommand[5]) || !isBase32(mCommand[6]) || !isBase32(mCommand[7]) || !isBase32(mCommand[8])) {
				setError(ERROR_CODE_PROTOCOL_ERROR, STR_LEN("Invalid source SID syntax"));
				return false;
			}
			mSidSource.assign(mCommand, 5, 4);
			mBodyPos = 9;
			break;

		case HEADER_SYMBOL_HUB: // Hub message
			// 'H' command_name ...
			mHeader = HEADER_HUB;
			mBodyPos = 4;
			break;

		case HEADER_SYMBOL_INFO: // Info message
			// 'I' command_name ...
			mHeader = HEADER_INFO;
			mBodyPos = 4;
			break;

		case HEADER_SYMBOL_CLIENT: // Client message
			// 'C' command_name ...
			mHeader =  HEADER_CLIENT;
			mBodyPos = 4;
			break;

		case HEADER_SYMBOL_DIRECT: // Direct message
			// 'D' command_name ' ' base32_character{4} ' ' base32_character{4} ...
			mHeader = HEADER_DIRECT;
			// Fallthrough

		case HEADER_SYMBOL_ECHO: // Echo message
			// 'E' command_name ' ' base32_character{4} ' ' base32_character{4} ...
			if (mHeader == HEADER_UNKNOWN) {
				mHeader = HEADER_ECHO;
			}
			if (mLength < 14 || mCommand[4] != ' ' || mCommand[9] != ' ') {
				setError(ERROR_CODE_PROTOCOL_ERROR, STR_LEN("Invalid source SID or target SID length"));
				return false;
			} else if (!isBase32(mCommand[5]) || !isBase32(mCommand[6]) || !isBase32(mCommand[7]) || !isBase32(mCommand[8])) {
				setError(ERROR_CODE_PROTOCOL_ERROR, STR_LEN("Invalid source SID syntax"));
				return false;
			} else if (!isBase32(mCommand[10]) || !isBase32(mCommand[11]) || !isBase32(mCommand[12]) || !isBase32(mCommand[13])) {
				setError(ERROR_CODE_PROTOCOL_ERROR, STR_LEN("Invalid target SID syntax"));
				return false;
			}
			mSidSource.assign(mCommand, 5, 4);
			mSidTarget.assign(mCommand, 10, 4);
			mBodyPos = 14;
			break;

		case HEADER_SYMBOL_FEATURE: // Feature broadcast
			// 'F' command_name ' ' base32_character{4} ' ' (('+'|'-') [A-Z] [A-Z0-9]{3})+ ...
			// example: FSCH AA7V +TCP4-NAT0 TOauto TRZSIJM5OH6FCOIC6Y6LR5FUA2TXG5N3ZS7P6M5DQ
			mHeader = HEADER_FEATURE;
			if (mLength < 15 || mCommand[4] != ' ' || mCommand[9] != ' ') {
				setError(ERROR_CODE_PROTOCOL_ERROR, STR_LEN("Invalid source SID or feature length"));
				return false;
			} else if (!isBase32(mCommand[5]) || !isBase32(mCommand[6]) || !isBase32(mCommand[7]) || !isBase32(mCommand[8])) {
				setError(ERROR_CODE_PROTOCOL_ERROR, STR_LEN("Invalid source SID syntax"));
				return false;
			} else if (!parseFeatures()) {
				setError(ERROR_CODE_PROTOCOL_ERROR, STR_LEN("Invalid feature syntax"));
				return false;
			}

			mSidSource.assign(mCommand, 5, 4);
			mBodyPos = 14;
			break;

		case HEADER_SYMBOL_UDP: // UDP message
			// 'U' command_name ' ' base32_character+ ...
			mHeader = HEADER_UDP;
			if (mLength < 6 || mCommand[4] != ' ' || !isBase32(mCommand[5])) {
				setError(ERROR_CODE_PROTOCOL_ERROR, STR_LEN("Invalid source CID length"));
				return false;
			}
			counter = 6;
			while (counter < mLength && isBase32(mCommand[counter++])) {
			}
			if (counter != mLength && mCommand[counter] != ' ') {
				setError(ERROR_CODE_PROTOCOL_ERROR, STR_LEN("Invalid source CID syntax"));
				return false;
			}
			mCidSource.assign(mCommand, 5, counter - 5);
			mBodyPos = counter;
			break;

		default: // Unknown command
			if (!isUpperAlpha(mCommand[0])) { // Is it ADC message_header?
				// Invalid command name
				setError(ERROR_CODE_PROTOCOL_ERROR, STR_LEN("Invalid command name"));
				return false;
			}

	}

	return true;
}



bool AdcParser::parseFeatures() {
	size_t pos = 10;
	size_t endPos = mCommand.find(' ', 15);
	if (endPos == mCommand.npos) {
		endPos = mLength;
	}
	mPositiveFeature.clear();
	mNegativeFeature.clear();
	while (pos < endPos) {
		if (pos + 4 > endPos || 
			(mCommand[pos] != '+' && mCommand[pos] != '-') ||
			!isUpperAlpha(mCommand[pos + 1]) || 
			!isUpperAlphaNum(mCommand[pos + 2]) || 
			!isUpperAlphaNum(mCommand[pos + 3]) || 
			!isUpperAlphaNum(mCommand[pos + 4])
		) {
				mPositiveFeature.clear();
				mNegativeFeature.clear();
				return false;
		}
		if (mCommand[pos] == '+') {
			mPositiveFeature.push_back(FOURCC(&mCommand[pos + 1]));
		} else {
			mNegativeFeature.push_back(FOURCC(&mCommand[pos + 1]));
		}
		pos += 5;
	}
	return true;
}



void AdcParser::setError(int code, const char * text, size_t textLen) {
	mErrorText.reserve(textLen);
	mErrorCode = code;
	mErrorText.assign(text, textLen);
}



void AdcParser::reInit() {
	Parser::reInit();

	mHeader = HEADER_UNKNOWN;
	mSidSource.clear();
	mSidTarget.clear();
	mError = false;
}



/// Split command to chunks
bool AdcParser::splitChunks() {

	if (mIsParsed) {
		return mError;
	}
	mIsParsed = true;

	pushChunk(0, mCommand.size()); // Zero part - always whole command
	splitAll(mBodyPos + 1, ' ');

	return mError;
}



void AdcParser::parseInfo(DcUser * dcUser, const string & info) {
	size_t s = info.find(' ', 9);
	if (s != info.npos) {
		size_t e;
		bool last = true;
		set<string> & names = dcUser->getInfoNames();
		while ((e = info.find(' ', ++s)) != info.npos || last) {
			if (e == info.npos) {
				e = info.size();
				last = false;
			}
			if (s + 2 <= e) { // max 2 (for name)
				string name;
				name.assign(info, s, 2);
				s += 2;
				if (e != s) {
					string value;
					value.assign(info, s, e - s);
					names.insert(name); // TODO refactoring
					dcUser->getParamForce(name.c_str())->setString(value);
				} else {
					names.erase(name); // TODO refactoring
					dcUser->removeParam(name.c_str());
				}
			}
			s = e;
		}
	}

	// Raplace IP
	replaceParam(dcUser->getParam("I4"), "0.0.0.0", dcUser->getIp());
	replaceParam(dcUser->getParam("I6"), "::", dcUser->getIp());

	// Parse features
	AdcParser::parseFeatures(dcUser);
}



void AdcParser::replaceParam(ParamBase * param, const char * oldValue, const string & newValue) {
	if (param != NULL) {
		if (param->toString() == oldValue) {
			param->setString(newValue);
		}
	}
}



void AdcParser::parseFeatures(DcUser * dcUser) {

	ParamBase * param = dcUser->getParam("SU"); // TODO replace name to macros
	if (param) {
		const string & value = param->toString();
		set<int> & features = dcUser->getFeatures();

		features.clear();
		size_t i, j = 0;
		while ((i = value.find(',', j)) != value.npos) {
			if (i - j == 4) {
				if (isUpperAlpha(value[j]) && isUpperAlphaNum(value[j + 1]) && 
					isUpperAlphaNum(value[j + 2]) && isUpperAlphaNum(value[j + 3]))
				{
					features.insert(FOURCC(&value[j]));
				}
			}
			j = i + 1;
		}
		if (value.size() - j == 4) {
			if (isUpperAlpha(value[j]) && isUpperAlphaNum(value[j + 1]) && 
				isUpperAlphaNum(value[j + 2]) && isUpperAlphaNum(value[j + 3]))
			{
				features.insert(FOURCC(&value[j]));
			}
		}
	}
}



void AdcParser::formingInfo(DcUser * dcUser, string & info) {
	info.reserve(0xFF); // usual length of command
	info.assign(STR_LEN("BINF "));
	info.append(dcUser->getUid());
	const set<string> & names = dcUser->getInfoNames();
	for (set<string>::const_iterator it = names.begin(); it != names.end(); ++it) {
		const string & name = (*it);
		ParamBase * param = dcUser->getParam(name.c_str());
		if (param != NULL) {
			info.append(STR_LEN(" ")).append(name).append(param->toString());
		}
	}
}


}; // namespace protocol

}; // namespace dcserver

/**
 * $Id$
 * $HeadURL$
 */
