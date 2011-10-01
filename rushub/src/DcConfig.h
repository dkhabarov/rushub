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

#ifndef DC_CONFIG_H
#define DC_CONFIG_H

#include "ConfigList.h"
#include "ConfigLoader.h"
#include "NmdcParser.h" // for NMDC_TYPE_UNKNOWN (PROTOCOL NMDC)

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

	int mStartPing;       ///< Time interval in sec, after which begins ping user
	int mMaxPassiveRes;   ///< Max results in the passive search

	int mUsersLimit;	///< User's limit

	unsigned long mMaxNmdcCommandLength;
	unsigned long mMaxWebCommandLength; ///< Max size of web cmd
	unsigned mMaxCmdLen[NMDC_TYPE_UNKNOWN + 1];
	unsigned mMaxNickLen;	///< Max length of nick
	unsigned mMinNickLen; ///< Min length of nick

	/** Allowed number reconnection during miTimeReconnIp sec */
	unsigned mFloodCountReconnIp;

	/** Reconnection times */
	double mFloodTimeReconnIp;

	unsigned mFloodCount[NMDC_TYPE_UNKNOWN + 1];
	unsigned mFloodCount2[NMDC_TYPE_UNKNOWN + 1];
	double mFloodTime[NMDC_TYPE_UNKNOWN + 1];
	double mFloodTime2[NMDC_TYPE_UNKNOWN + 1];

	double mPingInterval;       ///< User's ping interval
	double mTimeout[5];         ///< Timeouts of the protocol entering commands
	double mTimeoutAny;         ///< Timeout connection
	double mWebTimeout;         ///< Timeout connection with web-client (in sec)
	double mSysLoading;         ///< Factor of the system loading

	bool mUdpServer;            ///< UDP server on/off
	bool mAdcOn;                ///< Use ADC protocol

	bool mWebServer;            ///< Web-server on/off
	bool mDisableNoDCCmd;       ///< Allow DC commands only

	bool mNicklistOnLogin;      ///< Send user-list on login
	bool mDelayedLogin;         ///< Logined user after send to it full user-list only (to big hubs)

	bool mCheckSearchIp;        ///< Check IP in passive search
	bool mCheckSrNick;          ///< Check nick in SR commands
	bool mCheckCtmIp;           ///< Check IP in CTM commands
	bool mCheckRctmNick;        ///< Check nick in RCTM commands

	bool mDelayedMyinfo;        ///< Delay in sending cmds: $MyINFO, $Hello, $Quit (optimisation)
	bool mSendUserIp;           ///< Use UserIP & UserIP2 features
	bool mRegMainBot;           ///< Reg main bot
	bool mMainBotKey;           ///< Key for main bot

	string mAddresses;          ///< "Ip1[:port1] Ip2[:port2] ... IpN[:portN]" - addresses of server
	string mUdpAddresses;
	string mWebAddresses;       ///< Wib addresses "ip1[:port1] host2[:port2] ... ipN[:portN]"
	string mHubBot;             ///< Nick of the main bot
	string mHubName;            ///< Name of the hub
	string mTopic;              ///< Hub topic
	string mLocale;             ///< Main hub locale
	string mMainBotMyinfo;		///< MyINFO string for the main bot
	string mMainBotIp;          ///< IP for the main bot

	string mMainPath;           ///< Main path
	string mLogPath;            ///< Logs path
	string mPluginPath;         ///< Plugin path
	string mLangPath;           ///< Langage path
	string mLang;               ///< Langage

	string mUserName;           ///< OS user name
	string mGroupName;          ///< OS group name

public:

	DcConfig(ConfigLoader *, Server *, const string & cfgFile);
	virtual ~DcConfig();
	void addVars(Server *);

	int load();
	int save();
	int reload();

private:

	ConfigLoader * mConfigLoader;
	ConfigStore mConfigStore;
	string mConfigPath;
	
}; // DcConfig



/** Language configuration (settings) */
class DcLang : public ConfigList {

public:

	string mFirstMsg;          ///< First message
	string mBadChatNick;       ///< Message about bad nick in the chat
	string mBadLoginSequence;  ///< Message about bad login sequence
	string mBadMyinfoNick;     ///< Message about bad nick in MyINFO command
	string mUnits[7];          ///< Units: B, KB, MB, GB, TB, PB, EB
	string mTimeout;           ///< Message about timeout with the reason
	string mTimeoutCmd[5];     ///< Timeouts names
	string mTimeoutAny;        ///< Message about connection timeout
	string mForceMove;         ///< Redirection message
	string mUsersLimit;

	string mFlood[NMDC_TYPE_UNKNOWN + 1];

	string mBadSearchIp;   ///< Message about bad IP in the active search
	string mBadSrNick;     ///< Message about bad nick in the SR commands
	string mBadCtmIp;      ///< Message about bad IP in the CTM commands
	string mBadRevConNick; ///< Message about bad nick in the passive search
	string mUsedNick;      ///< Message about occupied nick
	string mBadNickLen;    ///< Message about ban length for the nick
	string mBadChars;      ///< Message about bad symbols in the nick
	string mTimes[5];      ///< weeks, days, hours, min, sec

public:

	DcLang(ConfigLoader *, ConfigListBase *);
	~DcLang();
	void addVars();

	int load();
	int save();
	int reload();

private:

	ConfigLoader * mConfigLoader;
	ConfigStore mConfigStore;

}; // DcLang

}; // namespace dcserver

#endif // DC_CONFIG_H

/**
 * $Id$
 * $HeadURL$
 */
