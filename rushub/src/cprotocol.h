/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * Copyright (C) 2009-2010 by Setuper
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

#ifndef CPARSER_H
#define CPARSER_H

#include "cobj.h"
#include <vector>

using namespace std;

namespace nServer
{

class cParser;
class cConn;

enum
{
  eUNPARSED = -1
};


/**
  * Main proto class
  */
class cProtocol : public cObj
{
public:
  cProtocol();
  virtual ~cProtocol();
  virtual int DoCmd(cParser *, cConn *) = 0; /** DoCmd */
  virtual cParser * CreateParser() = 0; /** CreateParser */
  virtual void DeleteParser(cParser *) = 0; /** DeleteParser */
};


/** Parser class
*/
class cParser : public cObj
{

public:

  string mStr; /** Main string with cmd */
  int miType; /** Type of cmd */
  unsigned miLen; /** Cmd len */
  unsigned miKWSize; /** Key-word len */
  bool mbError; /** error */

  typedef pair<int, int> tChunk; /** Pair for chunk (begin, end) */
  typedef vector<tChunk> tChunkList; /** Chunks list */
  typedef tChunkList::iterator tCLIt; /** Iterator */
  tChunkList mChunks; /** List */
  string *mStrings; /** String array for chunks */
  unsigned long mStrMap; /** Chunk already existed */
  
public:

  cParser(int max);
  virtual ~cParser();

  virtual int Parse() = 0; /** Parse */
  virtual bool SplitChunks() = 0; /** SplitChunks */

  string & GetStr(); /** GetStr */
  virtual void ReInit(); /** ReInit */
  
protected:

  int mMaxChunks; /** Common (max) number of chunks */

  void SetChunk(int n, int start, int len);

  bool SplitOnTwo(size_t start, const string & lim, int cn1, int cn2, size_t len = 0, bool left = true);
  bool SplitOnTwo(size_t start, const char lim, int cn1, int cn2, size_t len = 0, bool left = true);

  bool SplitOnTwo(const string & lim, int ch, int cn1, int cn2, bool left = true);
  bool SplitOnTwo(const char lim, int ch, int cn1, int cn2, bool left = true);

  bool ChunkRedRight(int cn, int amount);
  
  bool ChunkRedLeft(int cn, int amount);

}; // cParser

}; // nServer

#endif // CPARSER_H
