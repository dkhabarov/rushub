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
#include "stringutils.h"

using namespace utils;

namespace dcserver {

namespace protocol {


/// ADC command
class AdcCommand {

public:

	AdcCommand() {
	}

	AdcCommand(const char * cmd) {
		mCmd = cmd;
	}

	virtual ~AdcCommand() {
	}

	bool check(const string & cmd) const {
		return 0 == cmd.compare(1, 3, mCmd);
	}

private:

	//hash = *reinterpret_cast<const unsigned int*>(cmd);
	const char * mCmd;

}; // AdcCommand



/// Main ADC commands
AdcCommand AdcCommands[] = {
	AdcCommand("SUP"), // SUP
	AdcCommand("STA"), // STA
	AdcCommand("INF"), // INF
	AdcCommand("MSG"), // MSG
	AdcCommand("SCH"), // SCH
	AdcCommand("RES"), // RES
	AdcCommand("CTM"), // CTM
	AdcCommand("RCM"), // RCM
	AdcCommand("GPA"), // GPA
	AdcCommand("PAS"), // PAS
	AdcCommand("QUI"), // QUI
	AdcCommand("GET"), // GET
	AdcCommand("GFI"), // GFI
	AdcCommand("SND"), // SND
	AdcCommand("SID"), // SID
	AdcCommand("CMD"), // CMD
	AdcCommand("NAT"), // NAT
	AdcCommand("RNT"), // RNT
	AdcCommand("PSR"), // PSR
	AdcCommand("PUB")  // PUB
};



AdcParser::AdcParser() :
	Parser(9), // Max number of chunks - 9 !!!
	mHeader(HEADER_UNKNOWN),
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



const string & AdcParser::getErrorCode() const {
	return mErrorCode;
}



const string & AdcParser::getErrorText() const {
	return mErrorText;
}



const vector<string> & AdcParser::getPositiveFeatures() const {
	return mPositiveFeature;
}



const vector<string> & AdcParser::getNegativeFeatures() const {
	return mNegativeFeature;
}



/// Do parse for command and return type of this command
int AdcParser::parse() {

	mLength = mCommand.size(); // Set cmd len

	if (mLength >= 4) { // ADC cmd key must contain 4 symbols

		if (checkHeaderSyntax()) {

			for (unsigned int i = 0; i < ADC_TYPE_INVALID; ++i) {
				if (AdcCommands[i].check(mCommand)) { // Check cmd from mCommand
					return mType = AdcType(i); // Set cmd type
				}
			}
			return mType = ADC_TYPE_UNKNOWN; // Unknown cmd
		}

	} else if (mLength == 0) {
		return mType = ADC_TYPE_VOID; // Void cmd
	}

	return mType = ADC_TYPE_INVALID; // Invalid cmd
}



bool AdcParser::checkHeaderSyntax() {

	// Check cmd name ([A-Z] [A-Z0-9] [A-Z0-9])
	if (!isUpperAlpha(mCommand[1]) || !isUpperAlphaNum(mCommand[2]) || !isUpperAlphaNum(mCommand[3])) {
		return false;
	}

	unsigned int counter = 0;

	switch (mCommand[0]) {

		case 'B': // Broadcast
			// 'B' command_name ' ' base32_character{4}
			mHeader = HEADER_BROADCAST;
			if (mLength < 9 || mCommand[4] != ' ') {
				setError("40", "Protocol syntax error");
				return false;
			} else if (!isBase32(mCommand[5]) || !isBase32(mCommand[6]) || !isBase32(mCommand[7]) || !isBase32(mCommand[8])) {
				setError("40", "Invalid source SID");
				return false;
			}
			mSidSource.assign(mCommand, 5, 4);
			mBodyPos = 9;
			break;

		case 'H': // Hub message
			// 'H' command_name
			mHeader = HEADER_HUB;
			mBodyPos = 4;
			break;

		case 'I': // Info message
			// 'I' command_name
			mHeader = HEADER_INFO;
			mBodyPos = 4;
			break;

		case 'C': // Client message
			// 'C' command_name
			mHeader =  HEADER_CLIENT;
			mBodyPos = 4;
			break;

		case 'D': // Direct message
			// 'D' command_name ' ' base32_character{4} ' ' base32_character{4}
			mHeader = HEADER_DIRECT;
			// Fallthrough

		case 'E': // Echo message
			// 'E' command_name ' ' base32_character{4} ' ' base32_character{4}
			if (mHeader == HEADER_UNKNOWN) {
				mHeader = HEADER_ECHO;
			}
			if (mLength < 14 || mCommand[4] != ' ' || mCommand[9] != ' ') {
				setError("40", "Protocol syntax error");
				return false;
			} else if (!isBase32(mCommand[5]) || !isBase32(mCommand[6]) || !isBase32(mCommand[7]) || !isBase32(mCommand[8])) {
				setError("40", "Invalid source SID");
				return false;
			} else if (!isBase32(mCommand[10]) || !isBase32(mCommand[11]) || !isBase32(mCommand[12]) || !isBase32(mCommand[13])) {
				setError("40", "Invalid target SID");
				return false;
			}
			mSidSource.assign(mCommand, 5, 4);
			mSidTarget.assign(mCommand, 10, 4);
			mBodyPos = 14;
			break;

		case 'F': // Feature broadcast
			// 'F' command_name ' ' base32_character{4} ' ' (('+'|'-') [A-Z] [A-Z0-9]{3})+
			// example: FSCH AA7V +TCP4-NAT0 TOauto TRZSIJM5OH6FCOIC6Y6LR5FUA2TXG5N3ZS7P6M5DQ
			mHeader = HEADER_FEATURE;
			if (mLength < 15 || mCommand[4] != ' ' || mCommand[9] != ' ') {
				setError("40", "Protocol syntax error");
				return false;
			} else if (!isBase32(mCommand[5]) || !isBase32(mCommand[6]) || !isBase32(mCommand[7]) || !isBase32(mCommand[8])) {
				setError("40", "Invalid source SID");
				return false;
			} else if (!parseFeatures()) {
				setError("40", "Invalid feature");
				return false;
			}

			mSidSource.assign(mCommand, 5, 4);
			mBodyPos = 14;
			break;

		case 'U': // UDP message
			// 'U' command_name ' ' base32_character+
			mHeader = HEADER_UDP;
			if (mLength < 6 || mCommand[4] != ' ' || !isBase32(mCommand[5])) {
				setError("40", "Protocol syntax error");
				return false;
			}
			counter = 6;
			while (counter < mLength && isBase32(mCommand[counter++])) {
			}
			if (counter != mLength && mCommand[counter] != ' ') {
				setError("40", "Invalid source SID");
				return false;
			}
			mCidSource.assign(mCommand, 5, counter - 5);
			mBodyPos = counter;
			break;

		default: // Unknown
			return false;

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
			mPositiveFeature.push_back(mCommand.substr(pos + 1, 4));
		} else {
			mNegativeFeature.push_back(mCommand.substr(pos + 1, 4));
		}
		pos += 5;
	}
	return true;
}



void AdcParser::setError(const char * code, const char * text) {
	mErrorCode = code;
	mErrorText = text;
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

	//setChunk(0, 0, mCommand.size()); // Zero part - always whole command

	switch (mType) {

		case ADC_TYPE_SUP : // This command has a different number parameters
			break;

		case ADC_TYPE_STA : // STA code [desc]
			if (!splitOnTwo(mBodyPos, ' ', CHUNK_ADC_STA_CMD, CHUNK_ADC_STA_CODE)) {
				mError = true;
			}
			if (splitOnTwo(' ', CHUNK_ADC_STA_CODE, CHUNK_ADC_STA_CODE, CHUNK_ADC_STA_DESC)) {
				splitOnTwo(' ', CHUNK_ADC_STA_DESC, CHUNK_ADC_STA_DESC, CHUNK_ADC_STA_OTHER);
			}
			break;

		case ADC_TYPE_INF : // INF sid params
			if (!splitOnTwo(mBodyPos, ' ', CHUNK_ADC_INF_CMD, CHUNK_ADC_INF_OTHER)) {
				mError = true;
			}
			break;
			
		case ADC_TYPE_MSG : // MSG sid [to_sid] text [me] [pm]
			if (!splitOnTwo(mBodyPos, ' ', CHUNK_ADC_MSG_CMD, CHUNK_ADC_MSG_TEXT)) {
				mError = true;
			}
			if (!splitOnTwo(" ME", CHUNK_ADC_MSG_TEXT, CHUNK_ADC_MSG_TEXT, CHUNK_ADC_MSG_ME)) {
				if (!splitOnTwo(" PM", CHUNK_ADC_MSG_TEXT, CHUNK_ADC_MSG_TEXT, CHUNK_ADC_MSG_PM)) {
					splitOnTwo(' ', CHUNK_ADC_MSG_TEXT, CHUNK_ADC_MSG_TEXT, CHUNK_ADC_MSG_OTHER);
				} else {
					splitOnTwo(' ', CHUNK_ADC_MSG_PM, CHUNK_ADC_MSG_PM, CHUNK_ADC_MSG_OTHER);
				}
			} else {
				if (!splitOnTwo(" PM", CHUNK_ADC_MSG_ME, CHUNK_ADC_MSG_ME, CHUNK_ADC_MSG_PM)) {
					splitOnTwo(' ', CHUNK_ADC_MSG_ME, CHUNK_ADC_MSG_ME, CHUNK_ADC_MSG_OTHER);
				} else {
					splitOnTwo(' ', CHUNK_ADC_MSG_PM, CHUNK_ADC_MSG_PM, CHUNK_ADC_MSG_OTHER);
				}
			}
			break;

		case ADC_TYPE_SCH : // SCH sid params
			break;

		case ADC_TYPE_RES : // RES sid params
			break;

		case ADC_TYPE_CTM : // CTM sid params
			break;

		case ADC_TYPE_RCM : // RCM sid params
			break;

		case ADC_TYPE_GPA : // GPA data
			break;

		case ADC_TYPE_PAS : // PAS password
			break;

		case ADC_TYPE_QUI : // QUI sid
			break;

		case ADC_TYPE_GET : //
			break;

		case ADC_TYPE_GFI : //
			break;

		case ADC_TYPE_SND : //
			break;

		case ADC_TYPE_SID : //
			break;

		case ADC_TYPE_CMD : //
			break;

		case ADC_TYPE_NAT : //
			break;

		case ADC_TYPE_RNT : //
			break;

		case ADC_TYPE_PSR : //
			break;

		case ADC_TYPE_PUB : //
			break;

		case ADC_TYPE_UNKNOWN : //
			break;

		default :
			break;

	}
	return mError;
}



void AdcParser::parseInf(DcUserBase * dcUserBase, const string & info) {
	size_t s = info.find(' ', 9);
	if (s != info.npos) {
		size_t e;
		while ((e = info.find(' ', ++s)) != info.npos) {
			if (s + 2 <= e) { // max 2 (for name)
				string name;
				name.assign(info, s, 2);
				s += 2;
				if (e != s) {
					dcUserBase->getParamForce(name.c_str())->setString(string().assign(info, s, e - s));
				} else {
					dcUserBase->removeParam(name.c_str());
				}
			}
			s = e;
		}
	}
}

void AdcParser::getInf(DcUserBase *, string &) {
	// TODO
}


}; // namespace protocol

}; // namespace dcserver

/**
 * $Id$
 * $HeadURL$
 */
