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

#include "DcTag.h"

namespace dcserver {



DcTag::DcTag() :
	nil(TAGNIL_NO),
	passive(false),
	unregHubs(0),
	regHubs(0),
	opHubs(0),
	slots(0),
	limit(0),
	open(0),
	bandwidth(0),
	download(0),
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


}; // namespace dcserver

/**
* $Id$
* $HeadURL$
*/
