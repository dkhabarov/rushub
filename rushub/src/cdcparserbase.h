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

#ifndef CDCPARSERBASE_H
#define CDCPARSERBASE_H

#include <string>

using namespace std;

#ifndef DC_SEPARATOR
  #define DC_SEPARATOR "|" /** DC Protocol separator */
#endif

namespace nDCServer {

namespace nProtoEnums {


typedef enum /** Types of the commands (for field mType) */
{
  eDC_NO = -1,
  eDC_MSEARCH,      //< 0  = $MultiSearch
  eDC_MSEARCH_PAS,  //< 1  = $MultiSearch Hub:
  eDC_SEARCH_PAS,   //< 2  = $Search Hub:
  eDC_SEARCH,       //< 3  = $Search
  eDC_SR,           //< 4  = $SR
  eDC_MYNIFO,       //< 5  = $MyNIFO
  eDC_SUPPORTS,     //< 6  = $Support
  eDC_KEY,          //< 7  = $Key
  eDC_VALIDATENICK, //< 8  = $ValidateNick
  eDC_VERSION,      //< 9  = $Version
  eDC_GETNICKLIST,  //< 10 = $GetNickList
  eDC_CHAT,         //< 11 = Chat
  eDC_TO,           //< 12 = $To
  eDC_QUIT,         //< 13 = $Quit
  eDC_MYPASS,       //< 14 = $MyPass
  eDC_CONNECTTOME,  //< 15 = $ConnecToMe
  eDC_RCONNECTTOME, //< 16 = $RevConnectToMe
  eDC_MCONNECTTOME, //< 17 = $MultiConnectToMe
  eDC_KICK,         //< 18 = $Kick
  eDC_OPFORCEMOVE,  //< 19 = $OpForceMove
  eDC_GETINFO,      //< 20 = $GetINFO
  eDC_UNKNOWN       //< 21 = $Unknown
} tDCType;


/** The Following constants were developed for accomodation 
  corresponding to parameter for each enterring commands of 
  the protocol DC in variable mChunks, are used in function 
  cDCServer::DC_* as well as in cDCParser::SplitChunks... 
  they must correspond
*/

/** A number of the chunks for simple commands (without parameter) */
enum { eCH_0_ALL };

/** A number of the chunks for commands with one parameter.
    $Key [key], $ValidateNick [nick], $Version [1,0091], $Quit [nick], $MyPass [pass], $Kick [nick]
*/
enum { eCH_1_ALL, eCH_1_PARAM }; 

/** A number of the chunks for partition chat message
    <[nick]> [msg]
*/
enum {eCH_CH_ALL, eCH_CH_NICK, eCH_CH_MSG};

/** A number of the chunks for the $GetINFO command
    $GetINFO [remote_nick] [nick]
*/
enum {eCH_GI_ALL, eCH_GI_OTHER, eCH_GI_NICK};

/** A number of the chunks for the $RevConnectToMe command
    $RevConnectToMe [nick] [remote_nick]
*/
enum {eCH_RC_ALL, eCH_RC_NICK, eCH_RC_OTHER};

/** A number of the chunks for the private message
    $To: [remote_nick] From: [nick] $<[[nick]> [msg]]
*/
enum {eCH_PM_ALL, eCH_PM_TO, eCH_PM_FROM, eCH_PM_CHMSG, eCH_PM_NICK, eCH_PM_MSG};

/** A number of the chunks for the $MyINFO command
    $MyINFO $ALL [nick] [[desc]$ $[speed]$[email]$[share]$]
*/
enum {eCH_MI_ALL, eCH_MI_DEST, eCH_MI_NICK, eCH_MI_INFO, eCH_MI_DESC, eCH_MI_SPEED, eCH_MI_MAIL, eCH_MI_SIZE};

/** A number of the chunks for the $ConnectToMe command
    $ConnectToMe [remote_nick] [ip]:[port]
*/
enum {eCH_CM_ALL, eCH_CM_NICK, eCH_CM_ACTIVE, eCH_CM_IP, eCH_CM_PORT};

/** A number of the chunks for the $OpForceMove command
    $OpForceMove $Who:[remote_nick]$Where:[address]$Msg:[reason]
*/
enum {eCH_FM_ALL, eCH_FM_NICK, eCH_FM_DEST, eCH_FM_REASON };

/** A number of the chunks for the active search command
    $Search [[ip]:[port]] [[sizerestricted?isminimumsize?size?datatype]?[searchpattern]]
*/
enum {eCH_AS_ALL, eCH_AS_ADDR, eCH_AS_IP, eCH_AS_PORT, eCH_AS_QUERY, eCH_AS_SEARCHLIMITS, eCH_AS_SEARCHPATTERN};

/** A number of the chunks for the passive search command
    $Search Hub:[nick] [[sizerestricted?isminimumsize?size?datatype]?[searchpattern]]
*/
enum {eCH_PS_ALL, eCH_PS_NICK, eCH_PS_QUERY, eCH_PS_SEARCHLIMITS, eCH_PS_SEARCHPATTERN};

/** A number of the chunks for the search results command
    $SR [nick] [file/path][0x05][filesize] [freeslots]/[totalslots][0x05][hubname] ([hubhost][:[hubport]])[0x05][searching_nick]
*/
enum {eCH_SR_ALL, eCH_SR_FROM, eCH_SR_PATH, eCH_SR_SIZE, eCH_SR_SLOTS, eCH_SR_SL_FR, eCH_SR_SL_TO, eCH_SR_HUBINFO, eCH_SR_TO};

}; // nProtoEnums

class cDCParserBase
{
public:
  string & msStr; //< address of the string with command
public:
  cDCParserBase(string & sStr):msStr(sStr){}
  virtual string & ChunkString(unsigned int n) = 0; /** Get string address for the chunk of command */
  virtual int GetType() const = 0; /** Get command type */
}; // cDCParserBase

}; // nDCServer

#endif // CDCPARSERBASE_H
