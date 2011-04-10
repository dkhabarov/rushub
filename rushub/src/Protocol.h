/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz

 * modified: 27 Aug 2009
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
	virtual int DoCmd(Parser *, Conn *) = 0; /** DoCmd */
	virtual Parser * createParser() = 0; /** createParser */
	virtual void deleteParser(Parser *) = 0; /** deleteParser */

}; // Protocol



/** Parser class
*/
class Parser : public Obj {

public:

	string mCommand; /** Main string with cmd */
	int miType; /** Type of cmd */

	typedef pair<size_t, size_t> tChunk; /** Pair for chunk (begin, end) */
	vector<tChunk> mChunks; /** List */

	size_t miLen; /** Cmd len */

public:

	Parser(int max);
	virtual ~Parser();

	virtual int Parse() = 0; /** Parse */
	virtual bool SplitChunks() = 0; /** SplitChunks */

	virtual void ReInit(); /** ReInit */

protected:

	size_t miKWSize; /** Key-word len */
	bool mbError; /** error */
	
	string * mStrings; /** String array for chunks */
	unsigned long mStrMap; /** Chunk already existed */
	bool mIsParsed;

protected:

	void SetChunk(int n, size_t start, size_t len);

	bool SplitOnTwo(size_t start, const string & lim, int cn1, int cn2, size_t len = 0, bool left = true);
	bool SplitOnTwo(size_t start, const char lim, int cn1, int cn2, size_t len = 0, bool left = true);

	bool SplitOnTwo(const string & lim, int chunk, int cn1, int cn2, bool left = true);
	bool SplitOnTwo(const char lim, int chunk, int cn1, int cn2, bool left = true);

	bool ChunkRedRight(int chunk, size_t amount);

	bool ChunkRedLeft(int chunk, size_t amount);

private:

	int mMaxChunks; /** Common (max) number of chunks */

}; // Parser

}; // server

#endif // PROTOCOL_H
