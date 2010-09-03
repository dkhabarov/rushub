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

#include "cprotocol.h"

namespace nServer
{

cProtocol::cProtocol() : cObj("cProtocol"){}
cProtocol::~cProtocol(){}

cParser::cParser(int max) : 
  cObj("cParser"),
  miType(eUNPARSED),
  miLen(0),
  miKWSize(0),
  mbError(false),
  mChunks(max),
  mStrings(NULL),
  mStrMap(0l),
  mMaxChunks(max)
{
  mStrings = new std::string[2 * mMaxChunks];
}

cParser::~cParser()
{
  mChunks.clear();
  if(mStrings != NULL) 
    delete[] mStrings;
  mStrings = NULL;
}

/** GetStr() */
string & cParser::GetStr()
{
  return mStr;
}

/** ReInit() */
void cParser::ReInit()
{
  mStr.resize(0); 
  mStr.reserve(512); 
  miType = eUNPARSED; 
  miLen = 0; 
  miKWSize = 0; 
  mbError = false; 
  mChunks.clear(); 
  mChunks.resize(mMaxChunks); 
  mStrMap = 0l; 
}

void cParser::SetChunk(int n, int start, int len)
{
  tChunk &c = mChunks[n];
  c.first = start;
  c.second = len;
}

bool cParser::SplitOnTwo(size_t start, const string &lim, int cn1, int cn2, size_t len, bool left)
{
  size_t i;
  if(!len) len = miLen;
  if(left) { 
    i = mStr.find(lim, start); 
    if(i == mStr.npos || i - start >= len) return false;
  } else { 
    i = mStr.rfind(lim, start + len - lim.length()); 
    if(i == mStr.npos || i < start) return false;
  }
  SetChunk(cn1, (int)start, int(i - start));
  SetChunk(cn2, int(i + lim.length()), int(miLen - i - lim.length()));
  return true;
}

bool cParser::SplitOnTwo(size_t start, const char lim, int cn1, int cn2, size_t len, bool left)
{
  size_t i;
  if(!len) len = miLen;
  if(left) { 
    i = mStr.find_first_of(lim, start); 
    if(i == mStr.npos || i - start >= len) return false;
  } else { 
    i = mStr.find_last_of(lim, start + len - 1);
    if(i == mStr.npos || i < start) return false;
  }
  SetChunk(cn1, (int)start, int(i - start));
  SetChunk(cn2, int(i + 1), int(miLen - i - 1));
  return true;
}

bool cParser::SplitOnTwo(const string &lim, int ch, int cn1, int cn2, bool left)
{
  tChunk &chu = mChunks[ch];
  return SplitOnTwo(chu.first, lim, cn1, cn2, chu.second, left);  
}

bool cParser::SplitOnTwo(const char lim, int ch, int cn1, int cn2, bool left)
{
  tChunk &chu = mChunks[ch];
  return SplitOnTwo(chu.first, lim, cn1, cn2, chu.second, left);  
}


bool cParser::ChunkRedRight(int cn, int amount)
{
  mChunks[cn].second -= amount;
  return true;
}

bool cParser::ChunkRedLeft(int cn, int amount)
{
  tChunk &ch = mChunks[cn];
  if(unsigned(ch.first + amount) < miLen) {
    ch.first += amount;
    ch.second -= amount;
    return true;
  } else return false;
}

};
