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

#include "cdcparser.h"

namespace nDCServer
{

namespace nProtocol
{

using namespace ::nDCServer::nProtoEnums;

/** Основные ключевые слова команд */
cProtocolCmd aDC_Commands[]={
  cProtocolCmd("$MultiSearch "),      // check: ip, delay
  cProtocolCmd("$MultiSearch Hub:"),  // check: nick, delay
  cProtocolCmd("$Search Hub:"),       // check: nick, delay //this must be first!! before the next one
  cProtocolCmd("$Search "),           // check: ip, delay
  cProtocolCmd("$SR "),               // check: nick
  cProtocolCmd("$MyINFO "),           // check: after_nick, nick, share_min_max
  cProtocolCmd("$Supports "),
  cProtocolCmd("$Key "),
  cProtocolCmd("$ValidateNick "),
  cProtocolCmd("$Version "),
  cProtocolCmd("$GetNickList"),
  cProtocolCmd("<"),                  // check: nick, delay, size, line_count
  cProtocolCmd("$To: "),              // check: nick, other_nick
  cProtocolCmd("$Quit "),             // no chech necessary
  cProtocolCmd("$MyPass "),
  cProtocolCmd("$ConnectToMe "),      // check: ip, nick
  cProtocolCmd("$RevConnectToMe "),   // check: nick, other_nick
  cProtocolCmd("$MultiConnectToMe "), // not implemented
  cProtocolCmd("$Kick "),             // check: op, nick, conn
  cProtocolCmd("$OpForceMove $Who:"), // check: op, nick
  cProtocolCmd("$GetINFO ")           // check: logged_in(FI), nick
};


cDCParser::cDCParser() : cParser(10), cDCParserBase(mStr) /** Max number of chunks - 10 */
{
  SetClassName("cDCParser");
}

cDCParser::~cDCParser()
{
}

/** Do parse for command and return type of this command */
int cDCParser::Parse()
{
  miLen = mStr.size(); /** Set cmd len */
  if(miLen) for(int i = 0; i < eDC_UNKNOWN; ++i) {
    if(aDC_Commands[i].Check(mStr)) { /** Check cmd from mStr */
      miType = tDCType(i); /** Set cmd type */
      miKWSize = aDC_Commands[i].mLength; /** Set length of key word for command */
      break;
    }
  }
  if(miType == eUNPARSED) miType = eDC_UNKNOWN; /** Unknown cmd */
  return miType;
}

/** Get string address for the chunk of command */
string & cDCParser::ChunkString(unsigned int n)
{
  if(!n) return mStr;  /** Empty line always full, and this pointer for empty line */
  if(n > mChunks.size()) /** This must not never happen, but if this happens, we are prepared */
    return mStrings[0];

  unsigned long flag = 1 << n;
  if(!(mStrMap & flag)) {
    mStrMap |= flag;
    try
    {
      tChunk &c = mChunks[n];
      if(c.first >= 0 && c.second >= 0 && (unsigned)c.first < mStr.length() && (unsigned)c.second < mStr.length()) 
        mStrings[n].assign(mStr, c.first, c.second); /** Record n part in n element of the array of the lines */
      else if(ErrLog(1)) LogStream() << "Badly parsed message : " << mStr << endl;
    }
    catch(...)
    {
      if(ErrLog(1)) LogStream() << "Ecxeption in chunk string" << endl;
    }
  }
  return mStrings[n];
}

bool cDCParser::IsPassive(const string & sDesc) {
  if (!sDesc.size()) return false;
  size_t l = sDesc.size() - 1;
  if(sDesc[l] != '>') return false;
  size_t i = sDesc.find_last_of('<', l);
  if(i == sDesc.npos) return false;
  string sChunk;
  sChunk.assign(sDesc, int(i), int(l - i));
  if(
    (sChunk.find("M:P") == sDesc.npos) || 
    (sChunk.find("M:5") == sDesc.npos) || 
    (sChunk.find("M:S") == sDesc.npos)
  ) return false;
  return true;
}

/** Split command to chunks */
bool cDCParser::SplitChunks()
{
  SetChunk(0, 0, mStr.length()); /** Zero part - always whole command */
  switch(miType)
  {
    case eDC_MSEARCH:
    case eDC_MSEARCH_PAS:
    case eDC_SEARCH: /** $Search [[ip]:[port]] [[F?T?0?9]?[searchpattern]] */
      if(!SplitOnTwo(miKWSize, ' ', eCH_AS_ADDR, eCH_AS_QUERY)) mbError = 1;
      if(!SplitOnTwo(':', eCH_AS_ADDR, eCH_AS_IP, eCH_AS_PORT)) mbError = 1;
      if(!SplitOnTwo('?', eCH_AS_QUERY, eCH_AS_SEARCHLIMITS, eCH_AS_SEARCHPATTERN, 0)) mbError = 1;
      break;
    case eDC_SEARCH_PAS: /** $Search Hub:[nick] [[F?T?0?9]?[searchpattern]] */
      if(!SplitOnTwo(miKWSize, ' ', eCH_PS_NICK, eCH_PS_QUERY)) mbError =1;
      if(!SplitOnTwo('?', eCH_PS_QUERY, eCH_PS_SEARCHLIMITS, eCH_PS_SEARCHPATTERN, 0)) mbError = 1; /** Searching for on the right */
      break;
    case eDC_SR: /** Return search results
      $SR [result_nick] [file_path]^E[file_size] [free_slots]/[total_slots]^E[hub__name] ([hub_host][:[hub_port]])^E[searching_nick]
      $SR [result_nick] [file_path]^E[file_size] [free_slots]/[total_slots]^ETTH:[TTHROOT] ([hub_ip][:[hub_port]])^E[searching_nick]
      $SR [result_nick] [file_path]              [free_slots]/[total_slots]^ETTH:[TTHROOT] ([hub_ip][:[hub_port]])^E[searching_nick]
      1)  ----FROM-----|------------------------------------------------(PATH)------------------------------------------------------
      2)               |----PATH---|------------------------------------(SIZE)------------------------------------------------------
      3)                           |-----------------------------------(HUBINFO)----------------------------------|----TO-----------
      4)                           |----------------(SIZE)-----------------|---------------HUBINFO----------------|
      5)                           |----SIZE----|---------(SLOTS)----------|
      6)                                            |----SL_FR---/----SL_TO----|

      1)  ----FROM-----|-------------------------------------(PATH)-----------------------------------------------------------------
      2)               |------------------PATH-----------------------------|-----------------------------------(SIZE)---------------
      3)                                                                   |-------------------HUBINFO------------|----TO-----------

    //1)  ----FROM-----|-------------------------------------(PATH)-----------------------------------------------------------------
    //2)               |--------PATH------------|-----------------------(SLOTS)-----------------------------------------------------
    //3)                                        |-----------SLOTS----------|----------------------------------(HUBINFO)-------------
    //4)                                                                   |--------------HUBINFO-----------------|----TO-----------
    */

      if(!SplitOnTwo(miKWSize, ' ', eCH_SR_FROM, eCH_SR_PATH)) mbError = 1;
      if(!SplitOnTwo(0x05, eCH_SR_PATH,    eCH_SR_PATH,    eCH_SR_SIZE)) mbError = 1;
      if(!SplitOnTwo(0x05, eCH_SR_SIZE,    eCH_SR_HUBINFO, eCH_SR_TO, false)) mbError = 1;
      if(SplitOnTwo(0x05,  eCH_SR_HUBINFO, eCH_SR_SIZE,    eCH_SR_HUBINFO))
      {
        if(!SplitOnTwo(' ', eCH_SR_SIZE,  eCH_SR_SIZE,  eCH_SR_SLOTS)) mbError = 1;
        if(!SplitOnTwo('/', eCH_SR_SLOTS, eCH_SR_SL_FR, eCH_SR_SL_TO)) mbError = 1;
      } else SetChunk(eCH_SR_SIZE, 0, 0);
      break;
    case eDC_MYNIFO: /** $MyINFO $ALL [nick] [[desc]$ $[speed]$[email]$[share]$] */
      if(!SplitOnTwo(miKWSize, ' ', eCH_MI_DEST, eCH_MI_NICK)) mbError = 1;
      if(!SplitOnTwo(' ', eCH_MI_NICK, eCH_MI_NICK, eCH_MI_INFO)) mbError = 1;
      if(!SplitOnTwo("$ $", eCH_MI_INFO, eCH_MI_DESC, eCH_MI_SPEED)) mbError = 1;
      if(!SplitOnTwo('$', eCH_MI_SPEED, eCH_MI_SPEED, eCH_MI_MAIL)) mbError = 1;
      if(!SplitOnTwo('$', eCH_MI_MAIL, eCH_MI_MAIL, eCH_MI_SIZE)) mbError = 1;
      if(!ChunkRedRight(eCH_MI_SIZE, 1)) mbError = 1; /** Removing the last char $ */
      break;
    case eDC_SUPPORTS: break; /** This command has a different number parameters */
    /** Commands with one parameter */
    case eDC_KEY: /** $Key [key] */
    case eDC_VALIDATENICK: /** $ValidateNick [nick] */
    case eDC_VERSION: /** $Version [1,0091] */
    case eDC_QUIT: /** $Quit [nick] */
    case eDC_MYPASS: /** $MyPass [pass] */
    case eDC_KICK: /** $Kick [nick] */
      /* can be an empty line? */
      if(miLen == miKWSize) mbError = 1;
      else SetChunk(eCH_1_PARAM, miKWSize, miLen - miKWSize);
      break;
    case eDC_CHAT: /** <[nick]> [msg] */
      if(!SplitOnTwo(miKWSize, "> ", eCH_CH_NICK, eCH_CH_MSG)) mbError = 1;
      break;
    case eDC_TO: /** $To: [remote_nick] From: [nick] $<[[nick]> [msg]] */
      if(!SplitOnTwo(miKWSize," From: ", eCH_PM_TO, eCH_PM_FROM)) mbError = 1;
      if(!SplitOnTwo(" $<", eCH_PM_FROM, eCH_PM_FROM, eCH_PM_CHMSG)) mbError = 1;
      if(!SplitOnTwo("> ", eCH_PM_CHMSG, eCH_PM_NICK, eCH_PM_MSG)) mbError = 1;
      break;
    case eDC_CONNECTTOME: /** $ConnectToMe [nick] [[ip]:[port]] */
      if(!SplitOnTwo(miKWSize, ' ', eCH_CM_NICK, eCH_CM_ACTIVE)) mbError = 1;
      if(!SplitOnTwo(':', eCH_CM_ACTIVE, eCH_CM_IP, eCH_CM_PORT)) mbError = 1;
      break;
    case eDC_RCONNECTTOME: /** $RevConnectToMe [nick] [remote_nick] */
      if(!SplitOnTwo(miKWSize, ' ', eCH_RC_NICK, eCH_RC_OTHER)) mbError = 1;
      break;
    case eDC_MCONNECTTOME:
      break;
    case eDC_OPFORCEMOVE: /** $OpForceMove $Who:[remote_nick]$Where:[address]$Msg:[reason] */
      if(!SplitOnTwo(miKWSize, "$Where:", eCH_FM_NICK, eCH_FM_DEST)) mbError = 1;
      if(!SplitOnTwo("$Msg:", eCH_FM_DEST, eCH_FM_DEST, eCH_FM_REASON)) mbError = 1;
      break;
    case eDC_GETINFO: /** $GetINFO [remote_nick] [nick] */
      if(!SplitOnTwo(miKWSize, ' ', eCH_GI_OTHER, eCH_GI_NICK)) mbError = 1;
      break;
    default:
      break;
  }
  return mbError;
}

}; // nProtocol

}; // nDCServer
