/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * Copyright (C) 2009-2011 by Setuper
 * E-Mail: setuper at gmail dot com (setuper@gmail.com)

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CDCTAG_H
#define CDCTAG_H

#include <string>

#include "cplugin.h" // TagNil
#include "stringutils.h"

using namespace std;
using namespace nUtils;


namespace nDCServer {

class MyInfo;

class DcTag {

public:

	DcTag();
	~DcTag();
	DcTag & operator = (const DcTag &);

	void parse(string & description);

	unsigned int getNil() const;
	void setNil(unsigned int nil);

	const string & getTag() const;
	void setTag (const string & tag);

	const string & getClientName() const;
	void setClientName (const string & clientName);

	const string & getClientVersion() const;
	void setClientVersion (const string & clientVersion);

	const string & getMode() const;
	void setMode (const string & mode);

	bool IsPassive() const;
	void setPassive (bool passive);

	int getUnregHubs() const;
	void setUnregHubs (int unregHubs);

	int getRegHubs() const;
	void setRegHubs (int regHubs);

	int getOpHubs() const;
	void setOpHubs (int opHubs);

	int getSlots() const;
	void setSlots (int slots);

	int getLimit() const;
	void setLimit (int limit);

	int getOpen() const;
	void setOpen (int open);

	int getBandwidth() const;
	void setBandwidth (int bandwidth);

	int getDownload() const;
	void setDownload (int download);

	const string & getFraction() const;
	void setFraction (const string & fraction);

private:

	unsigned int nil;

	/// Tag string
	string tag;

	/// Client name
	string clientName;

	/// Client version
	string clientVersion;

	/// Mode
	string mode;

	/// Passive mode
	bool passive;

	/// Count of hubs where client unregistered
	int unregHubs;

	/// Count of hubs where client registered
	int regHubs;

	/// Count of hubs where client is operator
	int opHubs;

	/// Count of slots
	int slots;

	/// Limit L:x
	int limit;

	/// Limit O:x
	int open;

	/// Limit B:x
	int bandwidth;

	/// Limit D:x
	int download;

	/// Limit F:x/y
	string fraction;

	/// Tag separator
	const char tagSep;

private:

	void parseTag();
	void findIntParam(const char * find, int & param, TagNil tagNil);

}; // DcTag

}; // nDCServer

#endif // CDCTAG_H
