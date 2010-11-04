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

#ifndef CDCCONFIG_H
#define CDCCONFIG_H

#include "cconfiglist.h"
#include "cdcparser.h" // for eDC_UNKNOWN

using std::string;
using namespace nConfig;

namespace nDCServer {

namespace nProtocol { class cDCProtocol; };

using nProtocol::cDCProtocol;
using nProtoEnums::eDC_UNKNOWN;


class cDCServer;
class cDCConn;

/** Server's configuration (settings)
  Main config class */
class cDCConfig : public cConfigList {

public:
  cDCServer * mDCServer;
  
  int miStartPing;        //< Time interval in sec, after which begins ping user
  int miMaxPassiveRes;    //< Max results in the passive search
  int miWebServerPort;    //< Web-server port
  
	int miUsersLimit;  //< User's limit

  unsigned long miWebStrSizeMax; //< Max size of web cmd
  unsigned mMaxCmdLen[eDC_UNKNOWN + 1];
  unsigned miMaxNickLen;  //< Max length of nick
  unsigned miMinNickLen;  //< Min length of nick
  unsigned miWebTimeout;  //< Connection timeout with web-client (in sec)

  /** Allowed number reconnection during miTimeReconnIp sec */
  unsigned miFloodCountReconnIp;
  unsigned miFloodCountMyINFO;
  unsigned miFloodCountMyINFO2;
  unsigned miFloodCountSearch;
  unsigned miFloodCountSearch2;
  unsigned miFloodCountSR;
  unsigned miFloodCountSR2;
  unsigned miFloodCountChat;
  unsigned miFloodCountChat2;
  unsigned miFloodCountTo;
  unsigned miFloodCountTo2;
  unsigned miFloodCountNickList;
  unsigned miFloodCountNickList2;
  unsigned miFloodCountCTM;
  unsigned miFloodCountCTM2;
  unsigned miFloodCountRCTM;
  unsigned miFloodCountRCTM2;
  unsigned miFloodCountUnknown;
  unsigned miFloodCountUnknown2;

  /** Reconnection times */
  double miFloodTimeReconnIp;
  double miFloodTimeMyINFO;
  double miFloodTimeMyINFO2;
  double miFloodTimeSearch;
  double miFloodTimeSearch2;
  double miFloodTimeSR;
  double miFloodTimeSR2;
  double miFloodTimeChat;
  double miFloodTimeChat2;
  double miFloodTimeTo;
  double miFloodTimeTo2;
  double miFloodTimeNickList;
  double miFloodTimeNickList2;
  double miFloodTimeCTM;
  double miFloodTimeCTM2;
  double miFloodTimeRCTM;
  double miFloodTimeRCTM2;
  double miFloodTimeUnknown;
  double miFloodTimeUnknown2;
  
  double miPingInterval;      //< User's ping interval
  double miTimeout[5];        //< Timeouts of the protocol commands
  double miMinClientPingInt;  //< Min ping interval from the client side
  double miSysLoading;        //< Factor of the system loading

  bool mbWebServer;           //< Web-server on/off
  bool mbDisableNoDCCmd;      //< Allow DC commands only

  bool mbNicklistOnLogin;     //< Send user-list on login
  bool mbDelayedLogin;        //< Logined user after send to it full user-list only (to big hubs)

  bool mbCheckSearchIp;       //< Check IP in passive search
  bool mbCheckSRNick;         //< Check nick in SR commands
  bool mbCheckCTMIp;          //< Check IP in CTM commands
  bool mbCheckRctmNick;       //< Check nick in RCTM commands

  bool mbDelayedMyINFO;       //< Delay in sending cmds: $MyINFO, $Hello, $Quit (optimisation)
  bool mbSendUserIp;          //< Use UserIP & UserIP2 features
  bool mbRegMainBot;          //< Reg main bot
  bool mbMainBotKey;          //< Key for main bot

  string msWebServerIP;       //< Main bot IP address
  string msSubPorts;          //< Additional listening hub ports (separator: space)
  string msHubBot;            //< Nick of the main bot
  string msHubName;           //< Name of the hub
  string msTopic;             //< Hub topic
  string msLocale;            //< Main hub locale
  string msMainBotMyINFO;     //< MyINFO string for the main bot
  string msMainBotIP;         //< IP for the main bot

public:
  cDCConfig();
  virtual ~cDCConfig();
  void SetServer(cDCServer *);
  bool Load();
  bool Save();
  void AddVars();
}; // cDCConfig



/** Language configuration (settings) */
class cDCLang  : public cConfigList
{

public:
  cDCServer * mDCServer;
  string msFirstMsg;          //< First message
  string msBadChatNick;       //< Message about bad nick in the chat
  string msMyinfoError;       //< Message about syntax error in MyINFO command
  string msBadLoginSequence;  //< Message about bad login sequence
  string msBadMyinfoNick;     //< Message about bad nick in MyINFO command
  string msUnits[7];          //< Units: B, KB, MB, GB, TB, PB, EB
  string msTimeout;           //< Message about timeout with the reason
  string msTimeoutCmd[6];     //< Timeouts names
  string msTimeoutAny;        //< Message about connection timeout
  string msFreqClientPing;    //< Message about frequent pings on the client side (one of the methods of the attack)
	string msUsersLimit;

  string msFloodMyINFO;
  string msFloodSearch;
  string msFloodSR;
  string msFloodChat;
  string msFloodTo;
  string msFloodNickList;
  string msFloodCTM;
  string msFloodRCTM;
  string msFloodUnknown;

  string msBadSearchIp;   //< Message about bad IP in the active search
  string msBadSRNick;     //< Message about bad nick in the SR commands
  string msBadCTMIp;      //< Message about bad IP in the CTM commands
  string msBadRevConNick; //< Message about bad nick in the passive search
  string msUsedNick;      //< Message about occupied nick
  string msBadNickLen;    //< Message about ban length for the nick
  string msBadChars;      //< Message about bad symbols in the nick
  string msTimes[5];      //< weeks, days, hours, min, sec

public:
  cDCLang();
  ~cDCLang();
  void SetServer(cDCServer *);
  bool Load();
  bool Save();
  void AddVars();
}; // cDCLang

}; // nDCServer

#endif // CDCCONFIG_H
