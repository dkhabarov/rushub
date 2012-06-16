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

#ifdef _WIN32
	#pragma warning(disable:4127) // Disable "conditional expression is constant"
#endif

namespace server {


Protocol::Protocol() : Obj("Protocol") {
}



Protocol::~Protocol(){
}



Parser::Parser(unsigned int max) : 
	Obj("Parser"),
	mType(TYPE_UNPARSED),
	mLength(0),
	mIsParsed(false),
	mChunks(max),
	mMaxChunks(max),
	mStrMap(0)
{
	if (mMaxChunks) {
		if (mMaxChunks > 31) {
			if (log(LEVEL_WARN)) {
				logStream() << "Max number of chunks more than 31" << endl;
			}
		}
		for(unsigned int i = 0; i < mMaxChunks; ++i) {
			tChunk & p = mChunks[i];
			p.first = p.second = 0;
		}
	}
}



Parser::~Parser() {
}



/// reInit()
void Parser::reInit() {
	mType = TYPE_UNPARSED;
	mLength = 0;
	mIsParsed = false;
	mStrMap = 0;
	string().swap(mCommand); // erase & free memory
	vector<string>().swap(mStrings); // erase & free memory
	if (mMaxChunks) {
		for(unsigned int i = 0; i < mMaxChunks; ++i) {
			tChunk & p = mChunks[i];
			p.first = p.second = 0;
		}
	} else {
		mChunks.clear();
	}
}



size_t Parser::getCommandLen() {
	return mLength;
}



size_t Parser::getChunks() const {
	return mChunks.size();
}



size_t Parser::getStartChunk(unsigned int n) const {
	if (n < mChunks.size()) {
		return mChunks[n].first;
	}
	return 0;
}



void Parser::setChunk(unsigned int n, size_t start, size_t len) {
	if (mMaxChunks) {
		if (n <= mMaxChunks) {
			tChunk & chunk = mChunks[n];
			chunk.first = start;
			chunk.second = len;
		} else if (log(LEVEL_ERROR)) {
			logStream() << "Error number of chunks" << endl;
		}
	}
}



size_t Parser::pushChunk(size_t start, size_t len) {
	if (!mMaxChunks) {
		mChunks.push_back(tChunk(start, len));
	} else if (log(LEVEL_ERROR)) {
		logStream() << "mMaxChunks not equal 0" << endl;
	}
	return mChunks.size() - 1;
}



/// Get string address for the chunk of command
string & Parser::chunkString(unsigned int n) {
	if (!n) {
		return mCommand; // Empty line always full, and this pointer for empty line
	} else if (n > mChunks.size()) { // This must not never happen, but if this happens, we are prepared
		if (log(LEVEL_ERROR)) {
			logStream() << "Error number of chunks" << endl;
		}
		return mCommand;
	}

	unsigned int flag = (1u << n);
	if (n > 31u || !(mStrMap & flag)) {
		tChunk & c = mChunks[n];
		size_t size = mCommand.size();
		if (mStrMap == 0u) {
			mStrings.resize(mChunks.size());
		}
		if (n > 31u) {
			n = 0u; // Big number of chunks (using mStrings[0])
		} else {
			mStrMap |= flag;
		}
		if (c.first < size && c.second < size) {
			mStrings[n].assign(mCommand, c.first, c.second); // Record n part in n element of the array of the lines
		} else if (log(LEVEL_ERROR)) {
			logStream() << "Badly parsed message : " << mCommand.substr(0, 25) << endl;
		}
	}
	return mStrings[n];
}



void Parser::splitAll(size_t start, const char lim, size_t len, bool left) {
	size_t i = 0;
	if (len == 0) {
		len = mLength;
	}
	while (true) {
		if (left) {
			i = mCommand.find_first_of(lim, start); 
			if (i == mCommand.npos || i - start >= len) {
				break;
			}
			pushChunk(start, i - start);
			start = i + 1;
		} else { 
			i = mCommand.find_last_of(lim, len - 1);
			if (i == mCommand.npos || i < start) {
				break;
			}
			pushChunk(i + 1, len - i);
			len = i - 1;
		}
	}
	if (left) {
		pushChunk(start, len - start);
	} else {
		if (i == mCommand.npos || i < start) {
			pushChunk(start, len - start + 1);
		} else {
			pushChunk(start, len - i);
		}
	}
}



bool Parser::splitOnTwo(size_t start, const string & lim, unsigned int cn1, unsigned int cn2, size_t len, bool left) {
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



bool Parser::splitOnTwo(size_t start, const char lim, unsigned int cn1, unsigned int cn2, size_t len, bool left) {
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



bool Parser::splitOnTwo(const string & lim, unsigned int ch, unsigned int cn1, unsigned int cn2, bool left) {
	if (mMaxChunks) {
		tChunk & chunk = mChunks[ch];
		return splitOnTwo(chunk.first, lim, cn1, cn2, chunk.second, left);
	}
	return false;
}



bool Parser::splitOnTwo(const char lim, unsigned int ch, unsigned int cn1, unsigned int cn2, bool left) {
	if (mMaxChunks) {
		tChunk & chunk = mChunks[ch];
		return splitOnTwo(chunk.first, lim, cn1, cn2, chunk.second, left);
	}
	return false;
}



bool Parser::chunkRedRight(unsigned int cn, size_t amount) {
	if (mMaxChunks) {
		mChunks[cn].second -= amount;
		return true;
	}
	return false;
}



bool Parser::chunkRedLeft(unsigned int cn, size_t amount) {
	if (mMaxChunks) {
		tChunk & chunk = mChunks[cn];
		if (chunk.first + amount < mLength) {
			chunk.first += amount;
			chunk.second -= amount;
			return true;
		}
	}
	return false;
}


} // namespace server

/**
 * $Id$
 * $HeadURL$
 */
