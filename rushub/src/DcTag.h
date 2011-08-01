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

#ifndef DC_TAG_H
#define DC_TAG_H

#include "NmdcParser.h" // TagNil
#include "stringutils.h"

#include <string>

using namespace ::std;
using namespace ::utils;


namespace dcserver {

class MyInfo;

class DcTag {

public:

	DcTag();
	~DcTag();
	DcTag & operator = (const DcTag &);

	void parse(string & description);

	inline unsigned int getNil() const {
		return nil;
	}

	inline void setNil(unsigned int nil) {
		this->nil = nil;
	}

	inline const string & getTag() const {
		return tag;
	}

	inline void setTag(const string & tag) {
		this->tag = tag;
	}

	inline const string & getClientName() const {
		return clientName;
	}

	inline void setClientName(const string & clientName) {
		this->clientName = clientName;
	}

	inline const string & getClientVersion() const {
		return clientVersion;
	}

	inline void setClientVersion(const string & clientVersion) {
		this->clientVersion = clientVersion;
	}

	inline const string & getMode() const {
		return mode;
	}

	inline void setMode(const string & mode) {
		this->mode = mode;
	}

	inline bool isPassive() const {
		return passive;
	}

	inline void setPassive(bool passive) {
		this->passive = passive;
	}

	inline int getUnregHubs() const {
		return unregHubs;
	}

	inline void setUnregHubs(int unregHubs) {
		this->unregHubs = unregHubs;
	}

	inline int getRegHubs() const {
		return regHubs;
	}

	inline void setRegHubs(int regHubs) {
		this->regHubs = regHubs;
	}

	inline int getOpHubs() const {
		return opHubs;
	}

	inline void setOpHubs(int opHubs) {
		this->opHubs = opHubs;
	}

	inline int getSlots() const {
		return slots;
	}

	inline void setSlots(int slots) {
		this->slots = slots;
	}

	inline int getLimit() const {
		return limit;
	}

	inline void setLimit(int limit) {
		this->limit = limit;
	}

	inline int getOpen() const {
		return open;
	}

	inline void setOpen(int open) {
		this->open = open;
	}

	inline int getBandwidth() const {
		return bandwidth;
	}

	inline void setBandwidth(int bandwidth) {
		this->bandwidth = bandwidth;
	}

	inline int getDownload() const {
		return download;
	}

	inline void setDownload(int download) {
		this->download = download;
	}

	inline const string & getFraction() const {
		return fraction;
	}

	inline void setFraction(const string & fraction) {
		this->fraction = fraction;
	}

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

}; // namespace dcserver

#endif // DC_TAG_H

/**
 * $Id$
 * $HeadURL$
 */
