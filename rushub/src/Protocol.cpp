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

#include "Protocol.h"

namespace server {


Protocol::Protocol() : Obj("Protocol") {
}



Protocol::~Protocol(){
}



Parser::Parser(int max) : 
	Obj("Parser"),
	mType(TYPE_UNPARSED),
	mChunks(max),
	mLength(0),
	mIsParsed(false),
	mStrings(new string[max]),
	mStrMap(0l),
	mMaxChunks(max)
{
	for(int i = 0; i < mMaxChunks; ++i) {
		tChunk & p = mChunks[i];
		p.first = p.second = 0;
	}
}



Parser::~Parser() {
	delete[] mStrings;
}



/// reInit()
void Parser::reInit() {
	mType = TYPE_UNPARSED;
	mLength = 0;
	mStrMap = 0l;
	mIsParsed = false;
	string().swap(mCommand); // erase & free memory
	for(int i = 0; i < mMaxChunks; ++i) {
		tChunk & p = mChunks[i];
		p.first = p.second = 0;
	}
}



size_t Parser::getCommandLen() {
	return mLength;
}



void Parser::setChunk(int n, size_t start, size_t len) {
	tChunk & chunk = mChunks[n];
	chunk.first = start;
	chunk.second = len;
}



/// Get string address for the chunk of command
string & Parser::chunkString(unsigned int n) {
	if (!n) {
		return mCommand; // Empty line always full, and this pointer for empty line
	}
	if (n > mChunks.size()) { // This must not never happen, but if this happens, we are prepared
		if (log(LEVEL_FATAL)) {
			logStream() << "Error number of chunks" << endl;
		}
		return mStrings[0];
	}

	unsigned long flag = 1 << n;
	if (!(mStrMap & flag)) {
		mStrMap |= flag;
		tChunk & c = mChunks[n];
		size_t size = mCommand.size();
		if (c.first < size && c.second < size) {
			mStrings[n].assign(mCommand, c.first, c.second); // Record n part in n element of the array of the lines
		} else if (log(LEVEL_ERROR)) {
			logStream() << "Badly parsed message : " << mCommand << endl;
		}
	}
	return mStrings[n];
}



bool Parser::splitOnTwo(size_t start, const string & lim, int cn1, int cn2, size_t len, bool left) {
	size_t i = 0;
	size_t lim_len = lim.size();
	if (len == 0) {
		len = mLength;
	}
	if (left) { 
		i = mCommand.find(lim, start); 
		if (i == mCommand.npos || i - start >= len) {
			return false;
		}
	} else { 
		i = mCommand.rfind(lim, start + len - lim_len); 
		if (i == mCommand.npos || i < start) {
			return false;
		}
	}
	lim_len += i;
	setChunk(cn1, start, i - start);
	setChunk(cn2, lim_len, mLength - lim_len);
	return true;
}



bool Parser::splitOnTwo(size_t start, const char lim, int cn1, int cn2, size_t len, bool left) {
	size_t i = 0;
	if (len == 0) {
		len = mLength;
	}
	if (left) { 
		i = mCommand.find_first_of(lim, start); 
		if (i == mCommand.npos || i - start >= len) {
			return false;
		}
	} else { 
		i = mCommand.find_last_of(lim, start + len - 1);
		if (i == mCommand.npos || i < start) {
			return false;
		}
	}
	setChunk(cn1, start, i - start);
	setChunk(cn2, i + 1, mLength - i - 1);
	return true;
}



bool Parser::splitOnTwo(const string & lim, int ch, int cn1, int cn2, bool left) {
	tChunk & chunk = mChunks[ch];
	return splitOnTwo(chunk.first, lim, cn1, cn2, chunk.second, left);
}



bool Parser::splitOnTwo(const char lim, int ch, int cn1, int cn2, bool left) {
	tChunk & chunk = mChunks[ch];
	return splitOnTwo(chunk.first, lim, cn1, cn2, chunk.second, left);
}



bool Parser::chunkRedRight(int cn, size_t amount) {
	mChunks[cn].second -= amount;
	return true;
}



bool Parser::chunkRedLeft(int cn, size_t amount) {
	tChunk & chunk = mChunks[cn];
	if (chunk.first + amount < mLength) {
		chunk.first += amount;
		chunk.second -= amount;
		return true;
	}
	return false;
}


};

/**
 * $Id$
 * $HeadURL$
 */
