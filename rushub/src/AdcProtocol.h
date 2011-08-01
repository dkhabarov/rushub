/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
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

#ifndef ADC_PROTOCOL_H
#define ADC_PROTOCOL_H

#include "Protocol.h"
#include "AdcParser.h"

#include <string>

using namespace ::server;
using namespace ::std;

namespace dcserver {

namespace protocol {

class AdcProtocol : public Protocol {

public:

	AdcProtocol();
	virtual ~AdcProtocol();
	
	virtual const char * getSeparator();
	virtual size_t getSeparatorLen();
	virtual unsigned long getMaxCommandLength();

	virtual Parser * createParser();
	virtual void deleteParser(Parser *);

	virtual int doCommand(Parser *, Conn *);
	virtual Conn * getConnForUdpData(Conn *, Parser *);

	virtual int onNewConn(Conn * conn);

}; // AdcProtocol

}; // namespace protocol

}; // namespace dcserver

#endif // ADC_PROTOCOL_H

/**
 * $Id$
 * $HeadURL$
 */
