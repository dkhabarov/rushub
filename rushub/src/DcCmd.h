/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * Copyright (C) 2009-2013 by Setuper
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

#ifndef DC_CMD_H
#define DC_CMD_H

#include "Plugin.h"
#include "AdcProtocol.h"
#include "NmdcProtocol.h"

#include <string>

using namespace ::std;
using namespace ::dcserver::protocol;

namespace dcserver {

namespace protocol {


class DcCmd {

public:

	DcCmd(int protocolType = DC_PROTOCOL_TYPE_ALL) :
		mProtocolType(protocolType)
	{
	}

	~DcCmd() {
	}

	void parse(const string & data) {
	}

	void buildMsg(const string & data, const char * nick = NULL, const char * from = NULL, bool toAll = false) {
		string adcCmd, nmdcCmd;
		string adcCmd2, nmdcCmd2;
		bool typeAll = false;
		switch (mProtocolType) {
			case DC_PROTOCOL_TYPE_ALL:
				typeAll = true;
			case DC_PROTOCOL_TYPE_ADC:
				if (nick != NULL && from != NULL) {
					AdcProtocol::appendPm(adcCmd, adcCmd2, data, nick, from);
				} else {
					AdcProtocol::appendChat(adcCmd, data, nick, toAll);
				}
				if (!typeAll) {
					break;
				}
			case DC_PROTOCOL_TYPE_NMDC:
				if (nick != NULL && from != NULL) {
					NmdcProtocol::appendPm(nmdcCmd, nmdcCmd2, data, nick, from);
				} else {
					if (nick != NULL) {
						NmdcProtocol::appendChat(nmdcCmd, data, nick);
					} else {
						NmdcProtocol::appendChat(nmdcCmd, data);
					}
				}
				if (!typeAll) {
					break;
				}
		}
		if (!adcCmd.empty()) { // cmd can be empty
			mAdcParts.push_back(adcCmd);
		}
		if (!adcCmd2.empty()) { // cmd can be empty
			mAdcParts.push_back(adcCmd2);
		}
		if (!nmdcCmd.empty()) { // cmd can be empty
			mNmdcParts.push_back(nmdcCmd);
		}
		if (!nmdcCmd2.empty()) { // cmd can be empty
			mNmdcParts.push_back(nmdcCmd2);
		}
	}

	const vector<string> & getParts(int protocolType) {
		switch (protocolType) {
			case DC_PROTOCOL_TYPE_ADC:
				return mAdcParts;
			case DC_PROTOCOL_TYPE_NMDC:
				return mNmdcParts;
			default:
				throw "never";
		}
	}

private:

	int mProtocolType;

	vector<string> mNmdcParts;
	vector<string> mAdcParts;

}; // class DcCmd


} // namespace protocol

} // namespace dcserver

#endif // DC_CMD_H

/**
 * $Id$
 * $HeadURL$
 */
