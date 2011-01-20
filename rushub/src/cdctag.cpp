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

#include "cdctag.h"

namespace nDCServer {


DcTag::DcTag() :
	nil(TAGNIL_NO),
	tagSep(',')
{
}

DcTag::~DcTag() {
}

DcTag & DcTag::operator = (const DcTag &) {
	return *this;
}



void DcTag::parse(string & description) {

	unsigned int OldNil = nil;
	nil = TAGNIL_NO; // Set null value for all params

	size_t l = description.size();
	if (l) { // optimization
		size_t i = description.find_last_of('<');
		if (i != description.npos && description[--l] == '>') {
			nil |= TAGNIL_TAG;
			string sOldTag = tag;
			++i;
			tag.assign(description, i, l - i);
			description.assign(description, 0, --i);
			if (tag.compare(sOldTag) != 0) { // optimization
				parseTag();
			} else {
				nil |= OldNil;
			}
		}
	}
}



unsigned int DcTag::getNil() const {
	return nil;
}

void DcTag::setNil(unsigned int nil) {
	this->nil = nil;
}



const string & DcTag::getTag() const {
	return tag;
}

void DcTag::setTag (const string & tag) {
	this->tag = tag;
}



const string & DcTag::getClientName() const {
	return clientName;
}

void DcTag::setClientName (const string & clientName) {
	this->clientName = clientName;
}



const string & DcTag::getClientVersion() const {
	return clientVersion;
}

void DcTag::setClientVersion (const string & clientVersion) {
	this->clientVersion = clientVersion;
}



const string & DcTag::getMode() const {
	return mode;
}

void DcTag::setMode (const string & mode) {
	this->mode = mode;
}



bool DcTag::IsPassive() const {
	return passive;
}

void DcTag::setPassive (bool passive) {
	this->passive = passive;
}



int DcTag::getUnregHubs() const {
	return unregHubs;
}

void DcTag::setUnregHubs (int unregHubs) {
	this->unregHubs = unregHubs;
}



int DcTag::getRegHubs() const {
	return regHubs;
}

void DcTag::setRegHubs (int regHubs) {
	this->regHubs = regHubs;
}



int DcTag::getOpHubs() const {
	return opHubs;
}

void DcTag::setOpHubs (int opHubs) {
	this->opHubs = opHubs;
}



int DcTag::getSlots() const {
	return slots;
}

void DcTag::setSlots (int slots) {
	this->slots = slots;
}



int DcTag::getLimit() const {
	return limit;
}

void DcTag::setLimit (int limit) {
	this->limit = limit;
}



int DcTag::getOpen() const {
	return open;
}

void DcTag::setOpen (int open) {
	this->open = open;
}



int DcTag::getBandwidth() const {
	return bandwidth;
}

void DcTag::setBandwidth (int bandwidth) {
	this->bandwidth = bandwidth;
}



int DcTag::getDownload() const {
	return download;
}

void DcTag::setDownload (int download) {
	this->download = download;
}



const string & DcTag::getFraction() const {
	return fraction;
}

void DcTag::setFraction (const string & fraction) {
	this->fraction = fraction;
}



void DcTag::parseTag() {

	/* Get clientName and clientVersion */

	nil |= TAGNIL_CLIENT;

	size_t clientPos = tag.find(tagSep);
	size_t tagSize = tag.size();
	if (clientPos == tag.npos) {
		clientPos = tagSize;
	}

	size_t v = tag.find("V:");
	if (v != tag.npos) {
		nil |= TAGNIL_VERSION;
		clientVersion.assign(tag, v + 2, clientPos - v - 2);
		clientName.assign(tag, 0, v);
	} else {
		size_t s = tag.find(' ');
		if (s != tag.npos && s < clientPos) {
			++s;
			if (atof(tag.substr(s, clientPos - s).c_str())) {
				nil |= TAGNIL_VERSION;
				clientVersion.assign(tag, s, clientPos - s);
				clientName.assign(tag, 0, --s);
			} else {
				clientName.assign(tag, 0, clientPos);
			}
		} else {
			clientName.assign(tag, 0, clientPos);
		}
	}
	trim(clientName);
	trim(clientVersion);

	/* Get mode */
	size_t m = tag.find("M:");
	if (m != tag.npos) {
		nil |= TAGNIL_MODE;
		m += 2;
		size_t mPos = tag.find(tagSep, m);
		if (mPos == tag.npos) {
			mPos = tagSize;
		}
		mode.assign(tag, m, mPos - m);
		if (mPos > m) {
			unsigned p = mode[0];
			if(p == 80 || p == 53 || p == 83) {
				passive = true;
			}
		}
	}
	string tmp;

	/* hubs */
	size_t h = tag.find("H:");
	if (h != tag.npos) {
		h += 2;
		size_t unregPos = tag.find('/', h);
		if (unregPos == tag.npos) {
			unregPos = tag.find(tagSep, h);
			if (unregPos == tag.npos) {
				unregPos = tagSize;
			}
		} else {
			size_t regPos = tag.find('/', ++unregPos);
			if (regPos == tag.npos) {
				regPos = tag.find(tagSep, unregPos);
				if (regPos == tag.npos) {
					regPos = tagSize;
				}
			} else {
				size_t opPos = tag.find('/', ++regPos);
				if (opPos == tag.npos) {
					opPos = tag.find(tagSep, regPos);
					if (opPos == tag.npos) {
						opPos = tagSize;
					}
				}
				opHubs = atoi(tmp.assign(tag, regPos, opPos - regPos).c_str());
				if (tmp.size()) {
					nil |= TAGNIL_UNREG;
				}
			}
			regHubs = atoi(tmp.assign(tag, unregPos, regPos - unregPos - 1).c_str());
			if (tmp.size()) {
				nil |= TAGNIL_REG;
			}
		}
		unregHubs = atoi(tmp.assign(tag, h, unregPos - h - 1).c_str());
		if (tmp.size()) {
			nil |= TAGNIL_OP;
		}
	}

	/* slots and limits */
	findIntParam("S:", slots, TAGNIL_SLOT);
	findIntParam("L:", limit, TAGNIL_LIMIT);
	findIntParam("O:", open, TAGNIL_OPEN);
	findIntParam("B:", bandwidth, TAGNIL_BANDWIDTH);
	findIntParam("D:", download, TAGNIL_DOWNLOAD);

	size_t f = tag.find("F:");
	if (f != tag.npos) {
		nil |= TAGNIL_FRACTION;
		f += 2;
		size_t fPos = tag.find(tagSep, f);
		if(fPos == tag.npos) {
			fPos = tagSize;
		}
		fraction.assign(tag, f, fPos - f);
	}
}

void DcTag::findIntParam(const char * find, int & param, TagNil tagNil) {

	size_t pos = tag.find(find);
	if (pos != tag.npos) {

		pos += 2;
		size_t sepPos = tag.find(tagSep, pos);

		if (sepPos == tag.npos) {
			sepPos = tag.size();
		}

		nil |= tagNil;

		string tmp;
		param = atoi(tmp.assign(tag, pos, sepPos - pos).c_str());
	}
}


}; // nDCServer
