/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz
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

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "Obj.h"

#include <vector>

using namespace ::std;

namespace server {

class Parser;
class Conn;

enum {
	NMDC_TYPE_UNPARSED = -1
};


/**
  * Main proto class
  */
class Protocol : public Obj {

public:

	Protocol();
	virtual ~Protocol();
	virtual int doCommand(Parser *, Conn *) = 0; /** doCommand */
	virtual Parser * createParser() = 0; /** createParser */
	virtual void deleteParser(Parser *) = 0; /** deleteParser */

	virtual string getSeparator() = 0; /** protocol separator */
	virtual unsigned long getMaxCommandLength() = 0; /** protocol max command length */

	virtual Conn * getConnForUdpData(Conn *, Parser *) = 0;

}; // Protocol



/** Parser class
*/
class Parser : public Obj {

public:

	string mCommand; /** Main string with cmd */
	int mType; /** Type of cmd */

	typedef pair<size_t, size_t> tChunk; /** Pair for chunk (begin, end) */
	vector<tChunk> mChunks; /** List */

public:

	Parser(int max);
	virtual ~Parser();

	virtual int parse() = 0; /** Parse */

	virtual void reInit(); /** reInit */

	size_t getCommandLen();

protected:

	size_t mLength; //< Command len

	string * mStrings; //< String array for chunks
	unsigned long mStrMap; //< Chunk already existed
	bool mIsParsed;

protected:

	void setChunk(int n, size_t start, size_t len);

	bool splitOnTwo(size_t start, const string & lim, int cn1, int cn2, size_t len = 0, bool left = true);
	bool splitOnTwo(size_t start, const char lim, int cn1, int cn2, size_t len = 0, bool left = true);

	bool splitOnTwo(const string & lim, int chunk, int cn1, int cn2, bool left = true);
	bool splitOnTwo(const char lim, int chunk, int cn1, int cn2, bool left = true);

	bool chunkRedRight(int chunk, size_t amount);

	bool chunkRedLeft(int chunk, size_t amount);

private:

	int mMaxChunks; /** Common (max) number of chunks */

}; // Parser

}; // server

#endif // PROTOCOL_H

/**
 * $Id$
 * $HeadURL$
 */
