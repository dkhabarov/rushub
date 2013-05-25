/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz
 *
 * modified: 27 Aug 2009
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

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "Obj.h"

#include <vector>

using namespace ::std;
using namespace ::utils;

namespace server {

class Parser;
class Conn;

enum {
	TYPE_UNPARSED = -1
};


/// Main protocol class
class Protocol : public Obj {

public:

	Protocol();
	virtual ~Protocol();

	virtual const char * getSeparator() const = 0; ///< protocol separator
	virtual size_t getSeparatorLen() const = 0; ///< protocol separator length
	virtual unsigned int getMaxCommandLength() const = 0; ///< protocol max command length

	virtual int doCommand(Parser *, Conn *) = 0; ///< doCommand
	virtual Parser * createParser() = 0; ///< createParser
	virtual void deleteParser(Parser *) = 0; ///< deleteParser

	virtual Conn * getConnForUdpData(Conn *, Parser *) = 0;

	virtual int onNewConn(Conn *) = 0;

	virtual void onFlush(Conn *) {
	}

}; // Protocol



/// Parser class
class Parser : public Obj {

public:

	string mCommand; ///< Main string with cmd
	int mType; ///< Type of cmd

public:

	Parser(unsigned int max = 0);
	virtual ~Parser();

	virtual int parse() = 0; ///< Parse

	virtual void reInit(); ///< reInit

	/// Get string address for the chunk of command
	string & chunkString(unsigned int n);

	size_t getCommandLen();
	size_t getStartChunk(unsigned int n) const;

protected:

	size_t mLength; ///< Command len
	bool mIsParsed;	

protected:

	void setChunk(unsigned int n, size_t start, size_t len);
	size_t pushChunk(size_t start, size_t len);

	void splitAll(size_t start, const char lim, size_t len = 0, bool left = true);
	bool splitOnTwo(size_t start, const string & lim, unsigned int cn1, unsigned int cn2, size_t len = 0, bool left = true);
	bool splitOnTwo(size_t start, const char lim, unsigned int cn1, unsigned int cn2, size_t len = 0, bool left = true);

	bool splitOnTwo(const string & lim, unsigned int chunk, unsigned int cn1, unsigned int cn2, bool left = true);
	bool splitOnTwo(const char lim, unsigned int chunk, unsigned int cn1, unsigned int cn2, bool left = true);

	bool chunkRedRight(unsigned int cn, size_t amount);
	bool chunkRedLeft(unsigned int cn, size_t amount);

private:

	typedef pair<size_t, size_t> tChunk; ///< Pair for chunk (begin, end)
	vector<tChunk> mChunks; ///< List

	vector<string> mStrings; ///< String array for chunks
	unsigned int mMaxChunks; ///< Common (max) number of chunks (0 - resizeing)
	unsigned int mStrMap; ///< Chunk already existed

}; // class Parser

} // namespace server

#endif // PROTOCOL_H

/**
 * $Id$
 * $HeadURL$
 */
