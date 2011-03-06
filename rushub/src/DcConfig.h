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

#ifndef DC_CONFIG_H
#define DC_CONFIG_H

#include "ConfigList.h"
#include "ConfigLoader.h"
#include "DcParser.h" // for NMDC_TYPE_UNKNOWN

using ::std::string;
using namespace ::configuration;

namespace server {
	class Server;
}; // namespace server

namespace dcserver {

using ::dcserver::protoenums::NMDC_TYPE_UNKNOWN;



/** Server's configuration (settings)
	Main config class */
class DcConfig : public ConfigList {

public:

	int miStartPing;        //< Time interval in sec, after which begins ping user
	int miMaxPassiveRes;    //< Max results in the passive search

	int miUsersLimit;  //< User's limit

	unsigned long miWebStrSizeMax; //< Max size of web cmd
	unsigned mMaxCmdLen[NMDC_TYPE_UNKNOWN + 1];
	unsigned miMaxNickLen;  //< Max length of nick
	unsigned miMinNickLen;  //< Min length of nick

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
	unsigned miFloodCountMCTo;
	unsigned miFloodCountMCTo2;
	unsigned miFloodCountPing;
	unsigned miFloodCountPing2;
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
	double miFloodTimeMCTo;
	double miFloodTimeMCTo2;
	double miFloodTimePing;
	double miFloodTimePing2;
	double miFloodTimeUnknown;
	double miFloodTimeUnknown2;

	double miPingInterval;      //< User's ping interval
	double miTimeout[5];        //< Timeouts of the protocol entering commands
	double miTimeoutAny;        //< Timeout connection
	double miWebTimeout;        //< Timeout connection with web-client (in sec)
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

	string msWebAddresses;      //< Wib addresses "ip1[:port1] host2[:port2] ... ipN[:portN]"
	string msHubBot;            //< Nick of the main bot
	string msHubName;           //< Name of the hub
	string msTopic;             //< Hub topic
	string msLocale;            //< Main hub locale
	string msMainBotMyINFO;     //< MyINFO string for the main bot
	string msMainBotIP;         //< IP for the main bot

	string mMainPath;           //< Main path
	string mLogPath;            //< Logs path
	string mPluginPath;         //< Plugin path
	string mLangPath;           //< Langage path
	string mLang;               //< Langage
	

public:

	DcConfig(ConfigLoader *, Server *, const char * cfgFile);
	virtual ~DcConfig();
	void addVars(Server *);

	inline int load() {
		return mConfigLoader->load(this, mConfigStore);
	}

	inline int save() {
		return mConfigLoader->save(this, mConfigStore);
	}

	int reload();

private:

	ConfigLoader * mConfigLoader;
	ConfigStore mConfigStore;
	
}; // DcConfig



/** Language configuration (settings) */
class DcLang : public ConfigList {

public:

	string msFirstMsg;          //< First message
	string msBadChatNick;       //< Message about bad nick in the chat
	string msMyinfoError;       //< Message about syntax error in MyINFO command
	string msBadLoginSequence;  //< Message about bad login sequence
	string msBadMyinfoNick;     //< Message about bad nick in MyINFO command
	string msUnits[7];          //< Units: B, KB, MB, GB, TB, PB, EB
	string msTimeout;           //< Message about timeout with the reason
	string msTimeoutCmd[5];     //< Timeouts names
	string msTimeoutAny;        //< Message about connection timeout
	string msForceMove;         //< Redirection message
	string msUsersLimit;

	string msFloodMyINFO;
	string msFloodSearch;
	string msFloodSR;
	string msFloodChat;
	string msFloodTo;
	string msFloodNickList;
	string msFloodCTM;
	string msFloodRCTM;
	string msFloodMCTo;
	string msFloodPing;
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

	DcLang(ConfigLoader *, ConfigListBase *);
	~DcLang();
	void addVars();

	inline int load() {
		return mConfigLoader->load(this, mConfigStore);
	}

	inline int save() {
		return mConfigLoader->save(this, mConfigStore);
	}

	int reload();

private:

	ConfigLoader * mConfigLoader;
	ConfigStore mConfigStore;

}; // DcLang

}; // namespace dcserver

#endif // DC_CONFIG_H
