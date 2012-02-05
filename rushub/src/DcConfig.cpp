/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz
 *
 * modified: 27 Aug 2009
 * Copyright (C) 2009-2012 by Setuper
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

#include "DcConfig.h"
#include "DcServer.h"
#include "Dir.h"

#include <string>

using ::std::string;
using ::server::Server;


namespace dcserver {



DcConfig::DcConfig(ConfigLoader * configLoader, Server * server, const string & cfgFile) : 
	mConfigLoader(configLoader)
{

	mConfigStore.mPath = cfgFile;
	mConfigStore.mName.clear();

	// Path of RusHub.xml
	Dir::pathForFile(mConfigStore.mPath.c_str(), mConfigPath);

	addVars(server);
	reload();

	Dir::checkPath(mMainPath);
	Dir::checkPath(mLogPath);
	Dir::checkPath(mLangPath);
	Dir::checkPath(mPluginPath);

	mLogsPath = &mLogPath; // Set log point path

	#ifndef _WIN32
		// Go to the work directory
		if ((chdir(mMainPath.c_str())) < 0) {
			if (log(LEVEL_ERROR)) {
				logStream() << "Can not go to the work directory '" << mMainPath << "'. Error: " << strerror(errno) << endl;
			}
		}
	#endif
}



DcConfig::~DcConfig() {
}



void DcConfig::addVars(Server * server) {

	for (int i = 0; i <= NMDC_TYPE_UNKNOWN; ++i) {
		mFloodCount[i] = 0;
		mFloodCount2[i] = 0;
		mFloodTime[i] = 0;
		mFloodTime2[i] = 0;
	}

	add("iFloodCountReconnIp",    mFloodCountReconnIp,                  1u     );
	add("iFloodCountMyINFO",      mFloodCount [NMDC_TYPE_MYNIFO],       6u     );
	add("iFloodCountMyINFO2",     mFloodCount2[NMDC_TYPE_MYNIFO],       30u    );
	add("iFloodCountSearch",      mFloodCount [NMDC_TYPE_SEARCH],       3u     );
	add("iFloodCountSearch2",     mFloodCount2[NMDC_TYPE_SEARCH],       10u    );
	add("iFloodCountSearchPas",  	mFloodCount [NMDC_TYPE_SEARCH_PAS],   3u     );
	add("iFloodCountSearchPas2",  mFloodCount2[NMDC_TYPE_SEARCH_PAS],   10u    );
	add("iFloodCountMSearch",     mFloodCount [NMDC_TYPE_MSEARCH],      3u     );
	add("iFloodCountMSearch2",    mFloodCount2[NMDC_TYPE_MSEARCH],      10u    );
	add("iFloodCountMSearchPas",  mFloodCount [NMDC_TYPE_MSEARCH_PAS],  3u     );
	add("iFloodCountMSearchPas2", mFloodCount2[NMDC_TYPE_MSEARCH_PAS],  10u    );
	add("iFloodCountSR",          mFloodCount [NMDC_TYPE_SR],           1000u  );
	add("iFloodCountSR2",         mFloodCount2[NMDC_TYPE_SR],           10000u );
	add("iFloodCountSRUDP",       mFloodCount [NMDC_TYPE_SR_UDP],       1000u  );
	add("iFloodCountSRUDP2",      mFloodCount2[NMDC_TYPE_SR_UDP],       10000u );
	add("iFloodCountChat",        mFloodCount [NMDC_TYPE_CHAT],         3u     );
	add("iFloodCountChat2",       mFloodCount2[NMDC_TYPE_CHAT],         20u    );
	add("iFloodCountPm",          mFloodCount [NMDC_TYPE_TO],           5u     );
	add("iFloodCountPm2",         mFloodCount2[NMDC_TYPE_TO],           30u    );
	add("iFloodCountNickList",    mFloodCount [NMDC_TYPE_GETNICKLIST],  1u     );
	add("iFloodCountNickList2",   mFloodCount2[NMDC_TYPE_GETNICKLIST],  1u     );
	add("iFloodCountCTM",         mFloodCount [NMDC_TYPE_CONNECTTOME],  500u   );
	add("iFloodCountCTM2",        mFloodCount2[NMDC_TYPE_CONNECTTOME],  5000u  );
	add("iFloodCountRCTM",        mFloodCount [NMDC_TYPE_RCONNECTTOME], 250u   );
	add("iFloodCountRCTM2",       mFloodCount2[NMDC_TYPE_RCONNECTTOME], 2500u  );
	add("iFloodCountMCTo",        mFloodCount [NMDC_TYPE_MCTO],         5u     );
	add("iFloodCountMCTo2",       mFloodCount2[NMDC_TYPE_MCTO],         30u    );
	add("iFloodCountUserIp",      mFloodCount [NMDC_TYPE_USERIP],       100u   );
	add("iFloodCountUserIp2",     mFloodCount2[NMDC_TYPE_USERIP],       1000u  );
	add("iFloodCountPing",        mFloodCount [NMDC_TYPE_PING],         5u     );
	add("iFloodCountPing2",       mFloodCount2[NMDC_TYPE_PING],         20u    );
	add("iFloodCountUnknown",     mFloodCount [NMDC_TYPE_UNKNOWN],      1u     );
	add("iFloodCountUnknown2",    mFloodCount2[NMDC_TYPE_UNKNOWN],      10u    );

	add("iFloodTimeReconnIp",     mFloodTimeReconnIp,                   5.    );
	add("iFloodTimeMyINFO",       mFloodTime [NMDC_TYPE_MYNIFO],        60.   );
	add("iFloodTimeMyINFO2",      mFloodTime2[NMDC_TYPE_MYNIFO],        900.  );
	add("iFloodTimeSearch",       mFloodTime [NMDC_TYPE_SEARCH],        10.   );
	add("iFloodTimeSearch2",      mFloodTime2[NMDC_TYPE_SEARCH],        60.   );
	add("iFloodTimeSearchPas",    mFloodTime [NMDC_TYPE_SEARCH_PAS],    10.   );
	add("iFloodTimeSearchPas2",   mFloodTime2[NMDC_TYPE_SEARCH_PAS],    60.   );
	add("iFloodTimeMSearch",      mFloodTime [NMDC_TYPE_MSEARCH],       10.   );
	add("iFloodTimeMSearch2",     mFloodTime2[NMDC_TYPE_MSEARCH],       60.   );
	add("iFloodTimeMSearchPas",   mFloodTime [NMDC_TYPE_MSEARCH_PAS],   10.   );
	add("iFloodTimeMSearchPas2",  mFloodTime2[NMDC_TYPE_MSEARCH_PAS],   60.   );
	add("iFloodTimeSR",           mFloodTime [NMDC_TYPE_SR],            60.   );
	add("iFloodTimeSR2",          mFloodTime2[NMDC_TYPE_SR],            600.  );
	add("iFloodTimeSRUDP",        mFloodTime [NMDC_TYPE_SR_UDP],        60.   );
	add("iFloodTimeSRUDP2",       mFloodTime2[NMDC_TYPE_SR_UDP],        600.  );
	add("iFloodTimeChat",         mFloodTime [NMDC_TYPE_CHAT],          5.    );
	add("iFloodTimeChat2",        mFloodTime2[NMDC_TYPE_CHAT],          20.   );
	add("iFloodTimePm",           mFloodTime [NMDC_TYPE_TO],            10.   );
	add("iFloodTimePm2",          mFloodTime2[NMDC_TYPE_TO],            30.   );
	add("iFloodTimeNickList",     mFloodTime [NMDC_TYPE_GETNICKLIST],   60.   );
	add("iFloodTimeNickList2",    mFloodTime2[NMDC_TYPE_GETNICKLIST],   1800. );
	add("iFloodTimeCTM",          mFloodTime [NMDC_TYPE_CONNECTTOME],   60.   );
	add("iFloodTimeCTM2",         mFloodTime2[NMDC_TYPE_CONNECTTOME],   600.  );
	add("iFloodTimeRCTM",         mFloodTime [NMDC_TYPE_RCONNECTTOME],  60.   );
	add("iFloodTimeRCTM2",        mFloodTime2[NMDC_TYPE_RCONNECTTOME],  600.  );
	add("iFloodTimeMCTo",         mFloodTime [NMDC_TYPE_MCTO],          10.   );
	add("iFloodTimeMCTo2",        mFloodTime2[NMDC_TYPE_MCTO],          30.   );
	add("iFloodTimeUserIp",       mFloodTime [NMDC_TYPE_USERIP],        10.   );
	add("iFloodTimeUserIp2",      mFloodTime2[NMDC_TYPE_USERIP],        30.   );
	add("iFloodTimePing",         mFloodTime [NMDC_TYPE_PING],          1.    );
	add("iFloodTimePing2",        mFloodTime2[NMDC_TYPE_PING],          30.   );
	add("iFloodTimeUnknown",      mFloodTime [NMDC_TYPE_UNKNOWN],       3.    );
	add("iFloodTimeUnknown2",     mFloodTime2[NMDC_TYPE_UNKNOWN],       60.   );

	add("iLenCmdMSearch",         mMaxCmdLen[NMDC_TYPE_MSEARCH],        256u   );
	add("iLenCmdMSearchPas",      mMaxCmdLen[NMDC_TYPE_MSEARCH_PAS],    256u   );
	add("iLenCmdSearchPas",       mMaxCmdLen[NMDC_TYPE_SEARCH_PAS],     256u   );
	add("iLenCmdSearch",          mMaxCmdLen[NMDC_TYPE_SEARCH],         256u   );
	add("iLenCmdSR",              mMaxCmdLen[NMDC_TYPE_SR],             1024u  );
	add("iLenCmdSRUDP",           mMaxCmdLen[NMDC_TYPE_SR_UDP],         1024u  );
	add("iLenCmdMyINFO",          mMaxCmdLen[NMDC_TYPE_MYNIFO],         256u   );
	add("iLenCmdSupports",        mMaxCmdLen[NMDC_TYPE_SUPPORTS],       1024u  );
	add("iLenCmdKey",             mMaxCmdLen[NMDC_TYPE_KEY],            128u   );
	add("iLenCmdValidateNick",    mMaxCmdLen[NMDC_TYPE_VALIDATENICK],   64u    );
	add("iLenCmdVersion",         mMaxCmdLen[NMDC_TYPE_VERSION],        32u    );
	add("iLenCmdGetNickList",     mMaxCmdLen[NMDC_TYPE_GETNICKLIST],    12u    );
	add("iLenCmdChat",            mMaxCmdLen[NMDC_TYPE_CHAT],           65536u );
	add("iLenCmdTo",              mMaxCmdLen[NMDC_TYPE_TO],             65536u );
	add("iLenCmdQuit",            mMaxCmdLen[NMDC_TYPE_QUIT],           64u    );
	add("iLenCmdMyPass",          mMaxCmdLen[NMDC_TYPE_MYPASS],         64u    );
	add("iLenCmdCTM",             mMaxCmdLen[NMDC_TYPE_CONNECTTOME],    128u   );
	add("iLenCmdRCTM",            mMaxCmdLen[NMDC_TYPE_RCONNECTTOME],   128u   );
	add("iLenCmdMCTM",            mMaxCmdLen[NMDC_TYPE_MCONNECTTOME],   128u   );
	add("iLenCmdKick",            mMaxCmdLen[NMDC_TYPE_KICK],           64u    );
	add("iLenCmdOFM",             mMaxCmdLen[NMDC_TYPE_OPFORCEMOVE],    512u   );
	add("iLenCmdGetINFO",         mMaxCmdLen[NMDC_TYPE_GETINFO],        128u   );
	add("iLenCmdMCTo",            mMaxCmdLen[NMDC_TYPE_MCTO],           65536u );
	add("iLenCmdUserIp",          mMaxCmdLen[NMDC_TYPE_USERIP],         1024u  );
	add("iLenCmdUnknown",         mMaxCmdLen[NMDC_TYPE_UNKNOWN],        128u   );
	mMaxCmdLen[NMDC_TYPE_PING] = 0; // ping length

	add("iWebTimeout",            mWebTimeout,                          30.);
	add("iWebStrSizeMax",         mMaxWebCommandLength,                 10240ul);
	add("sWebAddresses",          mWebAddresses,
		#ifdef _WIN32
			string(STR_LEN("0.0.0.0:80"))
		#else
			string(STR_LEN("0.0.0.0:8080"))
		#endif
	);
	add("bWebServer",             mWebServer,                           false);

	/* Timeouts of the protocol commands */
	const char * timeoutName[] = {
		"Key",
		"Nick",
		"Login",
		"Myinfo",
		"Getpass"
	};

	double timeoutDefault[] = {
		60.0,
		30.0,
		600.0,
		40.0,
		300.0
	};

	for (int i = 0; i < 5; ++i) {
		mTimeout[i] = timeoutDefault[i];
		add(string(STR_LEN("iTimeout")).append(timeoutName[i]), mTimeout[i], timeoutDefault[i]);
	}

	unsigned long maxSendSize(MAX_SEND_SIZE);
	
	add("iTimeoutAny",            mTimeoutAny,                          600.);
	add("iTimerServPeriod",       server->mTimerServPeriod,             2000u);
	add("iTimerConnPeriod",       server->mTimerConnPeriod,             4000u);
	add("iMaxSendSize",           server->mMaxSendSize,                 maxSendSize);
	add("iStepDelay",             server->mStepDelay,                   0);
	add("iStrSizeMax",            mMaxNmdcCommandLength,                10240ul);
	add("iSysLoading",            mSysLoading,                          1.);
	add("iStartPing",             mStartPing,                           300);
	add("iPingInterval",          mPingInterval,                        60.);
	add("bNicklistOnLogin",       mNicklistOnLogin,                     true);
	add("bDelayedLogin",          mDelayedLogin,                        true);
	mDelayedMyinfo = false;//add("bDelayedMyINFO",    mDelayedMyinfo,   false);
	add("bDisableNoDCCmd",        mDisableNoDCCmd,                      true);
	add("bCheckSearchIp",         mCheckSearchIp,                       true);
	add("bCheckSRNick",           mCheckSrNick,                         true);
	add("bCheckRctmNick",         mCheckRctmNick,                       true);
	add("bCheckCTMIp",            mCheckCtmIp,                          true);
	add("bSendUserIp",            mSendUserIp,                          true);
	add("bMAC",                   server->mMac,                         false);
	add("iMaxPassiveRes",         mMaxPassiveRes,                       25);
	add("iMaxNickLen",            mMaxNickLen,                          32u);
	add("iMinNickLen",            mMinNickLen,                          2u);
	add("iUsersLimit",            mUsersLimit,                          -1);
	add("iMaxLevel",              mMaxLevel,                            mMaxLevel); // set this default value for log

	add("sUDPAddresses",          mUdpAddresses,                        string(STR_LEN("0.0.0.0:1209")));
	add("bUDPServer",             mUdpServer,                           false);

	add("bAdcOn",                 mAdcOn,                               false);

	#ifdef HAVE_LIBCAP
		add("sGroupName",           mGroupName,                           string(STR_LEN("root")));
		add("sUserName",            mUserName,                            string(STR_LEN("root")));
	#endif

	add("sLocale",                mLocale,                              string(setlocale(LC_ALL, "")));
	add("sMainBotIP",             mMainBotIp,                           string(STR_LEN("127.0.0.1")));
	add("sMainBotMyINFO",         mMainBotMyinfo,                       string(STR_LEN("RusHub bot<Bot V:1.0,M:A,H:0/0/1,S:0>$ $$$0$")));
	add("bMainBotKey",            mMainBotKey,                          true);
	add("sHubBot",                mHubBot,                              string(STR_LEN("RusHub")));
	add("bRegMainBot",            mRegMainBot,                          true);
	add("sTopic",                 mTopic,                               string(STR_LEN("Russian hub software")));
	add("sHubName",               mHubName,                             string(STR_LEN("RusHub")));
	add("sAddresses",             mAddresses,
		#ifdef _WIN32
			string(STR_LEN("0.0.0.0:411"))
		#else
			string(STR_LEN("0.0.0.0:4111"))
		#endif
	);

	// TODO: auto detect lang
	add("sLang",       mLang,       string(STR_LEN("Russian")));
	add("sLangPath",   mLangPath,   string(STR_LEN("./lang/")));
	add("sLogPath",    mLogPath,    string(STR_LEN("./logs/")));
	add("sPluginPath", mPluginPath, string(STR_LEN("./plugins/")));
	add("sMainPath",   mMainPath,   string(STR_LEN("./")));
}



int DcConfig::load() {

	const string curDir(STR_LEN("./"));

	int res = mConfigLoader->load(this, mConfigStore);
	// Replace in start of the string only
	stringReplace(mMainPath,   curDir, mMainPath,   mConfigPath, true, true);
	stringReplace(mPluginPath, curDir, mPluginPath, mConfigPath, true, true);
	stringReplace(mLogPath,    curDir, mLogPath,    mConfigPath, true, true);
	stringReplace(mLangPath,   curDir, mLangPath,   mConfigPath, true, true);
	return res;
}



int DcConfig::save() {

	const string curDir(STR_LEN("./"));

	// Replace in start of the string only
	stringReplace(mMainPath,   mConfigPath, mMainPath,   curDir, true, true);
	stringReplace(mPluginPath, mConfigPath, mPluginPath, curDir, true, true);
	stringReplace(mLogPath,    mConfigPath, mLogPath,    curDir, true, true);
	stringReplace(mLangPath,   mConfigPath, mLangPath,   curDir, true, true);

	int res = mConfigLoader->save(this, mConfigStore);

	stringReplace(mMainPath,   curDir, mMainPath,   mConfigPath, true, true);
	stringReplace(mPluginPath, curDir, mPluginPath, mConfigPath, true, true);
	stringReplace(mLogPath,    curDir, mLogPath,    mConfigPath, true, true);
	stringReplace(mLangPath,   curDir, mLangPath,   mConfigPath, true, true);

	return res;
}



int DcConfig::reload() {
	int ret = load();
	if (ret < 0) {
		if (ret != -4) {
			mMaxLevel = LEVEL_INFO; // Set default log level
		}
		save();
		return 1;
	}
	return 0;
}





DcLang::DcLang(ConfigLoader * configLoader, ConfigListBase * configListBase) :
	mConfigLoader(configLoader)
{

	ConfigItem * langConfig = configListBase->operator[] ("sLang");
	ConfigItem * langPathConfig = configListBase->operator[] ("sLangPath");
	if (langConfig && langPathConfig) {
		langPathConfig->convertTo(mConfigStore.mPath);
		langConfig->convertTo(mConfigStore.mName);
		if (mConfigStore.mPath.size() == 0) {
			mConfigStore.mPath.assign(STR_LEN("./"));
		}
		if (mConfigStore.mName.size() == 0) {
			mConfigStore.mName.assign(STR_LEN("English"));
		}

		// set xml file for lang
		Dir::checkPath(mConfigStore.mPath);
		mConfigStore.mName.append(STR_LEN(".xml"));
	}

	addVars();
	reload();
}



DcLang::~DcLang() {
}



void DcLang::addVars() {

	add("sFirstMsg", mFirstMsg, string(STR_LEN("���� ��� �������� ��� ����������� %[HUB] (����� ������: %[uptime] / ������: %[users] / ����: %[share]).")));
	add("sBadChatNick", mBadChatNick, string(STR_LEN("�������� ���: %[nick].")));
	add("sBadLoginSequence", mBadLoginSequence, string(STR_LEN("�� ���������� ������������������ ���������� ������ ��� �����.")));
	add("sBadMyinfoNick", mBadMyinfoNick, string(STR_LEN("�������� ��� � MyINFO �������.")));
	add("sBadRevConNick", mBadRevConNick, string(STR_LEN("�������� ��� ��� ����������: %[nick]. ��� �������� ���: %[real_nick].")));
	add("sTimeout", mTimeout, string(STR_LEN("����-��� %[reason].")));
	add("sTimeoutAny", mTimeoutAny, string(STR_LEN("����-��� ����������.")));
	add("sBadCTMIp", mBadCtmIp, string(STR_LEN("� ������� �� ����������� �� ��������� �������� ip �����: %[ip], ��� �������� ip: %[real_ip].")));
	add("sBadSRNick", mBadSrNick, string(STR_LEN("�������� ��� � ����������� ������: %[nick]. ��� �������� ���: %[real_nick].")));
	add("sBadSearchIp", mBadSearchIp, string(STR_LEN("� ��������� ������� �� ��������� �������� ip �����: %[ip], ��� �������� ip: %[real_ip].")));
	add("sUsedNick", mUsedNick, string(STR_LEN("���� ��� %[nick] ��� ������������ ������ �������������.")));

	add("sBadNickLen", mBadNickLen, string(STR_LEN("������������ ����� ����. ���������� ����� ���� �� %[min] �� %[max] ��������.")));
	add("sBadChars", mBadChars, string(STR_LEN("������������ ������� � ����.")));
	add("sUsersLimit", mUsersLimit, string(STR_LEN("��������� ������ �� ���������� ������������ �������������.")));
	add("sForceMove", mForceMove, string(STR_LEN("�� ���� �������������� �� ��� dchub://%[address] �������: %[reason]")));

	add("sFloodMyINFO", mFlood[NMDC_TYPE_MYNIFO], string(STR_LEN("���������� �� ������� �������� MyINFO.")));
	add("sFloodSearch", mFlood[NMDC_TYPE_SEARCH], string(STR_LEN("���������� �� ����������� ����� ��� �����.")));
	add("sFloodSearchPassive", mFlood[NMDC_TYPE_SEARCH_PAS], string(STR_LEN("���������� �� ����������� ����� ��� �����.")));
	add("sFloodMultiSearch", mFlood[NMDC_TYPE_MSEARCH], string(STR_LEN("���������� �� ����������� ����� ��� �����.")));
	add("sFloodMultiSearchPassive", mFlood[NMDC_TYPE_MSEARCH_PAS], string(STR_LEN("���������� �� ����������� ����� ��� �����.")));
	add("sFloodSR", mFlood[NMDC_TYPE_SR], string(STR_LEN("�� ������� ������������ ������.")));
	add("sFloodSRUDP", mFlood[NMDC_TYPE_SR_UDP], string(STR_LEN("�� ������� ������������ ������.")));
	add("sFloodChat", mFlood[NMDC_TYPE_CHAT], string(STR_LEN("���������� �� �������!")));
	add("sFloodTo", mFlood[NMDC_TYPE_TO], string(STR_LEN("���������� �� ������� � �������.")));
	add("sFloodNickList", mFlood[NMDC_TYPE_GETNICKLIST], string(STR_LEN("���������� �� ������� � ������� ������� ������� ��������� ������ �������������.")));
	add("sFloodCTM", mFlood[NMDC_TYPE_CONNECTTOME], string(STR_LEN("���������� �� ������� ������� ��������� �� ���������� � �������������� ����.")));
	add("sFloodRCTM", mFlood[NMDC_TYPE_RCONNECTTOME], string(STR_LEN("���������� �� ������� ������� ��������� �� ���������� � �������� �������������� ����.")));
	add("sFloodMCTo", mFlood[NMDC_TYPE_MCTO], string(STR_LEN("���������� �� �������!")));
	add("sFloodUserIp", mFlood[NMDC_TYPE_USERIP], string(STR_LEN("���������� �� ������� ��������� UserIP!")));
	add("sFloodPing", mFlood[NMDC_TYPE_PING], string(STR_LEN("��� ������ ������� ����� ������� ���.")));
	add("sFloodUnknown", mFlood[NMDC_TYPE_UNKNOWN], string(STR_LEN("�� ������� ������������ ���������.")));

	const char * reason[] = {
		"��������� �����",
		"��������� ����",
		"�����",
		"��������� MyINFO",
		"��������� ������"
	};

	const char * name[] = {
		"Key",
		"Nick",
		"Login",
		"Myinfo",
		"Getpass"
	};

	for (int i = 0; i < 5; ++i) {
		add(string(STR_LEN("sTimeout")).append(name[i]), mTimeoutCmd[i], string(reason[i]));
	}

	const char * units[] = {
		"B",
		"KB",
		"MB",
		"GB",
		"TB",
		"PB",
		"EB"
	};

	const char * unitsDef[] = {
		"�",
		"��",
		"��",
		"��",
		"��",
		"��",
		"��"
	};

	for (int i = 0; i < 7; ++i) {
		mUnits[i] = unitsDef[i];
		add(string(STR_LEN("sUnit")).append(units[i]), mUnits[i], string(unitsDef[i]));
	}

	const char * times[] = {
		"Weeks",
		"Days",
		"Hours",
		"Min",
		"Sec"
	};

	const char * timesDef[] = {
		"���.",
		"��.",
		"�.",
		"���.",
		"���."
	};

	for (int i = 0; i < 5; ++i) {
		mTimes[i] = timesDef[i];
		add(string(STR_LEN("sTimes")).append(times[i]), mTimes[i], string(timesDef[i]));
	}

}



int DcLang::load() {
	return mConfigLoader->load(this, mConfigStore);
}



int DcLang::save() {
	return mConfigLoader->save(this, mConfigStore);
}



int DcLang::reload() {

	int ret = load();
	if (ret < 0) {

		if (ret != -4) {
			// save default lang file
			mConfigStore.mName.assign(STR_LEN("Russian.xml"));
		}

		save();
		return 1;
	}

	return 0;
}


}; // namespace dcserver

/**
 * $Id$
 * $HeadURL$
 */
