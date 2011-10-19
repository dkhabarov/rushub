/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz
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
	mConfigStore.mName = "";

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
			if (errLog(1)) {
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

	add("iFloodCountReconnIp",    mFloodCountReconnIp,                  1     );
	add("iFloodCountMyINFO",      mFloodCount [NMDC_TYPE_MYNIFO],       6     );
	add("iFloodCountMyINFO2",     mFloodCount2[NMDC_TYPE_MYNIFO],       30    );
	add("iFloodCountSearch",      mFloodCount [NMDC_TYPE_SEARCH],       3     );
	add("iFloodCountSearch2",     mFloodCount2[NMDC_TYPE_SEARCH],       10    );
	add("iFloodCountSearchPas",  	mFloodCount [NMDC_TYPE_SEARCH_PAS],   3     );
	add("iFloodCountSearchPas2",  mFloodCount2[NMDC_TYPE_SEARCH_PAS],   10    );
	add("iFloodCountMSearch",     mFloodCount [NMDC_TYPE_MSEARCH],      3     );
	add("iFloodCountMSearch2",    mFloodCount2[NMDC_TYPE_MSEARCH],      10    );
	add("iFloodCountMSearchPas",  mFloodCount [NMDC_TYPE_MSEARCH_PAS],  3     );
	add("iFloodCountMSearchPas2", mFloodCount2[NMDC_TYPE_MSEARCH_PAS],  10    );
	add("iFloodCountSR",          mFloodCount [NMDC_TYPE_SR],           1000  );
	add("iFloodCountSR2",         mFloodCount2[NMDC_TYPE_SR],           10000 );
	add("iFloodCountSRUDP",       mFloodCount [NMDC_TYPE_SR_UDP],       1000  );
	add("iFloodCountSRUDP2",      mFloodCount2[NMDC_TYPE_SR_UDP],       10000 );
	add("iFloodCountChat",        mFloodCount [NMDC_TYPE_CHAT],         3     );
	add("iFloodCountChat2",       mFloodCount2[NMDC_TYPE_CHAT],         20    );
	add("iFloodCountPm",          mFloodCount [NMDC_TYPE_TO],           5     );
	add("iFloodCountPm2",         mFloodCount2[NMDC_TYPE_TO],           30    );
	add("iFloodCountNickList",    mFloodCount [NMDC_TYPE_GETNICKLIST],  1     );
	add("iFloodCountNickList2",   mFloodCount2[NMDC_TYPE_GETNICKLIST],  1     );
	add("iFloodCountCTM",         mFloodCount [NMDC_TYPE_CONNECTTOME],  500   );
	add("iFloodCountCTM2",        mFloodCount2[NMDC_TYPE_CONNECTTOME],  5000  );
	add("iFloodCountRCTM",        mFloodCount [NMDC_TYPE_RCONNECTTOME], 250   );
	add("iFloodCountRCTM2",       mFloodCount2[NMDC_TYPE_RCONNECTTOME], 2500  );
	add("iFloodCountMCTo",        mFloodCount [NMDC_TYPE_MCTO],         5     );
	add("iFloodCountMCTo2",       mFloodCount2[NMDC_TYPE_MCTO],         30    );
	add("iFloodCountUserIp",      mFloodCount [NMDC_TYPE_USERIP],       100   );
	add("iFloodCountUserIp2",     mFloodCount2[NMDC_TYPE_USERIP],       1000  );
	add("iFloodCountPing",        mFloodCount [NMDC_TYPE_PING],         5     );
	add("iFloodCountPing2",       mFloodCount2[NMDC_TYPE_PING],         20    );
	add("iFloodCountUnknown",     mFloodCount [NMDC_TYPE_UNKNOWN],      1     );
	add("iFloodCountUnknown2",    mFloodCount2[NMDC_TYPE_UNKNOWN],      10    );

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

	add("iLenCmdMSearch",         mMaxCmdLen[NMDC_TYPE_MSEARCH],        256   );
	add("iLenCmdMSearchPas",      mMaxCmdLen[NMDC_TYPE_MSEARCH_PAS],    256   );
	add("iLenCmdSearchPas",       mMaxCmdLen[NMDC_TYPE_SEARCH_PAS],     256   );
	add("iLenCmdSearch",          mMaxCmdLen[NMDC_TYPE_SEARCH],         256   );
	add("iLenCmdSR",              mMaxCmdLen[NMDC_TYPE_SR],             1024  );
	add("iLenCmdSRUDP",           mMaxCmdLen[NMDC_TYPE_SR_UDP],         1024  );
	add("iLenCmdMyINFO",          mMaxCmdLen[NMDC_TYPE_MYNIFO],         256   );
	add("iLenCmdSupports",        mMaxCmdLen[NMDC_TYPE_SUPPORTS],       1024  );
	add("iLenCmdKey",             mMaxCmdLen[NMDC_TYPE_KEY],            128   );
	add("iLenCmdValidateNick",    mMaxCmdLen[NMDC_TYPE_VALIDATENICK],   64    );
	add("iLenCmdVersion",         mMaxCmdLen[NMDC_TYPE_VERSION],        32    );
	add("iLenCmdGetNickList",     mMaxCmdLen[NMDC_TYPE_GETNICKLIST],    12    );
	add("iLenCmdChat",            mMaxCmdLen[NMDC_TYPE_CHAT],           65536 );
	add("iLenCmdTo",              mMaxCmdLen[NMDC_TYPE_TO],             65536 );
	add("iLenCmdQuit",            mMaxCmdLen[NMDC_TYPE_QUIT],           64    );
	add("iLenCmdMyPass",          mMaxCmdLen[NMDC_TYPE_MYPASS],         64    );
	add("iLenCmdCTM",             mMaxCmdLen[NMDC_TYPE_CONNECTTOME],    128   );
	add("iLenCmdRCTM",            mMaxCmdLen[NMDC_TYPE_RCONNECTTOME],   128   );
	add("iLenCmdMCTM",            mMaxCmdLen[NMDC_TYPE_MCONNECTTOME],   128   );
	add("iLenCmdKick",            mMaxCmdLen[NMDC_TYPE_KICK],           64    );
	add("iLenCmdOFM",             mMaxCmdLen[NMDC_TYPE_OPFORCEMOVE],    512   );
	add("iLenCmdGetINFO",         mMaxCmdLen[NMDC_TYPE_GETINFO],        128   );
	add("iLenCmdMCTo",            mMaxCmdLen[NMDC_TYPE_MCTO],           65536 );
	add("iLenCmdUserIp",          mMaxCmdLen[NMDC_TYPE_USERIP],         1024  );
	add("iLenCmdUnknown",         mMaxCmdLen[NMDC_TYPE_UNKNOWN],        128   );
	mMaxCmdLen[NMDC_TYPE_PING] = 0; // ping length

	add("iWebTimeout",            mWebTimeout,                          30.);
	add("iWebStrSizeMax",         mMaxWebCommandLength,                 10240);
	add("sWebAddresses",          mWebAddresses,
		#ifdef _WIN32
			"0.0.0.0:80"
		#else
			"0.0.0.0:8080"
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
		add(string("iTimeout").append(timeoutName[i]), mTimeout[i], timeoutDefault[i]);
	}

	add("iTimeoutAny",            mTimeoutAny,                          600.);
	add("iTimerServPeriod",       server->mTimerServPeriod,             2000);
	add("iTimerConnPeriod",       server->mTimerConnPeriod,             4000);
	add("iMaxSendSize",           server->mMaxSendSize,                 MAX_SEND_SIZE);
	add("iStepDelay",             server->mStepDelay,                   0);
	add("iStrSizeMax",            mMaxNmdcCommandLength,                10240);
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
	add("iMaxNickLen",            mMaxNickLen,                          32);
	add("iMinNickLen",            mMinNickLen,                          2);
	add("iUsersLimit",            mUsersLimit,                          -1);
	add("iMaxLevel",              mMaxLevel,                            mMaxLevel); // set this default value for log
	add("iMaxErrLevel",           mMaxErrLevel,                         mMaxErrLevel); // set this default value for log

	add("sUDPAddresses",          mUdpAddresses,                        "0.0.0.0:1209");
	add("bUDPServer",             mUdpServer,                           false);

	add("bAdcOn",                 mAdcOn,                               false);

	#ifdef HAVE_LIBCAP
		add("sGroupName",             mGroupName,                            "root");
		add("sUserName",              mUserName,                            "root");
	#endif

	add("sLocale",                mLocale,                              setlocale(LC_ALL, ""));
	add("sMainBotIP",             mMainBotIp,                           "127.0.0.1");
	add("sMainBotMyINFO",         mMainBotMyinfo,                       "RusHub bot<Bot V:1.0,M:A,H:0/0/1,S:0>$ $$$0$");
	add("bMainBotKey",            mMainBotKey,                          true);
	add("sHubBot",                mHubBot,                              "RusHub");
	add("bRegMainBot",            mRegMainBot,                          true);
	add("sTopic",                 mTopic,                               "Russian hub software");
	add("sHubName",               mHubName,                             "RusHub");
	add("sAddresses",             mAddresses,
		#ifdef _WIN32
			"0.0.0.0:411"
		#else
			"0.0.0.0:4111"
		#endif
	);

	// TODO: auto detect lang
	add("sLang",       mLang,       "Russian");
	add("sLangPath",   mLangPath,   "./lang/");
	add("sLogPath",    mLogPath,    "./logs/");
	add("sPluginPath", mPluginPath, "./plugins/");
	add("sMainPath",   mMainPath,   "./");
}



int DcConfig::load() {
	int res = mConfigLoader->load(this, mConfigStore);
	// Replace in start of the string only
	stringReplace(mMainPath,   "./", mMainPath,   mConfigPath, true, true);
	stringReplace(mPluginPath, "./", mPluginPath, mConfigPath, true, true);
	stringReplace(mLogPath,    "./", mLogPath,    mConfigPath, true, true);
	stringReplace(mLangPath,   "./", mLangPath,   mConfigPath, true, true);
	return res;
}



int DcConfig::save() {
	// Replace in start of the string only
	stringReplace(mMainPath,   mConfigPath, mMainPath,   "./", true, true);
	stringReplace(mPluginPath, mConfigPath, mPluginPath, "./", true, true);
	stringReplace(mLogPath,    mConfigPath, mLogPath,    "./", true, true);
	stringReplace(mLangPath,   mConfigPath, mLangPath,   "./", true, true);

	int res = mConfigLoader->save(this, mConfigStore);

	stringReplace(mMainPath,   "./", mMainPath,   mConfigPath, true, true);
	stringReplace(mPluginPath, "./", mPluginPath, mConfigPath, true, true);
	stringReplace(mLogPath,    "./", mLogPath,    mConfigPath, true, true);
	stringReplace(mLangPath,   "./", mLangPath,   mConfigPath, true, true);

	return res;
}



int DcConfig::reload() {
	int ret = load();
	if (ret < 0) {

		if (ret != -4) {
			// Set default log levels
			mMaxErrLevel = 2;
			mMaxLevel = 0;
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
			mConfigStore.mPath = "./";
		}
		if (mConfigStore.mName.size() == 0) {
			mConfigStore.mName = "English";
		}

		// set xml file for lang
		Dir::checkPath(mConfigStore.mPath);
		mConfigStore.mName.append(".xml", 4);
	}

	addVars();
	reload();
}



DcLang::~DcLang() {
}



void DcLang::addVars() {

	add("sFirstMsg", mFirstMsg, "���� ��� �������� ��� ����������� %[HUB] (����� ������: %[uptime] / ������: %[users] / ����: %[share]).");
	add("sBadChatNick", mBadChatNick, "�������� ���: %[nick].");
	add("sBadLoginSequence", mBadLoginSequence, "�� ���������� ������������������ ���������� ������ ��� �����.");
	add("sBadMyinfoNick", mBadMyinfoNick, "�������� ��� � MyINFO �������.");
	add("sBadRevConNick", mBadRevConNick, "�������� ��� ��� ����������: %[nick]. ��� �������� ���: %[real_nick].");
	add("sTimeout", mTimeout, "����-��� %[reason].");
	add("sTimeoutAny", mTimeoutAny, "����-��� ����������.");
	add("sBadCTMIp", mBadCtmIp, "� ������� �� ����������� �� ��������� �������� ip �����: %[ip], ��� �������� ip: %[real_ip].");
	add("sBadSRNick", mBadSrNick, "�������� ��� � ����������� ������: %[nick]. ��� �������� ���: %[real_nick].");
	add("sBadSearchIp", mBadSearchIp, "� ��������� ������� �� ��������� �������� ip �����: %[ip], ��� �������� ip: %[real_ip].");
	add("sUsedNick", mUsedNick, "���� ��� %[nick] ��� ������������ ������ �������������.");

	add("sBadNickLen", mBadNickLen, "������������ ����� ����. ���������� ����� ���� �� %[min] �� %[max] ��������.");
	add("sBadChars", mBadChars, "������������ ������� � ����.");
	add("sUsersLimit", mUsersLimit, "��������� ������ �� ���������� ������������ �������������.");
	add("sForceMove", mForceMove, "�� ���� �������������� �� ��� dchub://%[address] �������: %[reason]");

	add("sFloodMyINFO", mFlood[NMDC_TYPE_MYNIFO], "���������� �� ������� �������� MyINFO.");
	add("sFloodSearch", mFlood[NMDC_TYPE_SEARCH], "���������� �� ����������� ����� ��� �����.");
	add("sFloodSearchPassive", mFlood[NMDC_TYPE_SEARCH_PAS], "���������� �� ����������� ����� ��� �����.");
	add("sFloodMultiSearch", mFlood[NMDC_TYPE_MSEARCH], "���������� �� ����������� ����� ��� �����.");
	add("sFloodMultiSearchPassive", mFlood[NMDC_TYPE_MSEARCH_PAS], "���������� �� ����������� ����� ��� �����.");
	add("sFloodSR", mFlood[NMDC_TYPE_SR], "�� ������� ������������ ������.");
	add("sFloodSRUDP", mFlood[NMDC_TYPE_SR_UDP], "�� ������� ������������ ������.");
	add("sFloodChat", mFlood[NMDC_TYPE_CHAT], "���������� �� �������!");
	add("sFloodTo", mFlood[NMDC_TYPE_TO], "���������� �� ������� � �������.");
	add("sFloodNickList", mFlood[NMDC_TYPE_GETNICKLIST], "���������� �� ������� � ������� ������� ������� ��������� ������ �������������.");
	add("sFloodCTM", mFlood[NMDC_TYPE_CONNECTTOME], "���������� �� ������� ������� ��������� �� ���������� � �������������� ����.");
	add("sFloodRCTM", mFlood[NMDC_TYPE_RCONNECTTOME], "���������� �� ������� ������� ��������� �� ���������� � �������� �������������� ����.");
	add("sFloodMCTo", mFlood[NMDC_TYPE_MCTO], "���������� �� �������!");
	add("sFloodUserIp", mFlood[NMDC_TYPE_USERIP], "���������� �� ������� ��������� UserIP!");
	add("sFloodPing", mFlood[NMDC_TYPE_PING], "��� ������ ������� ����� ������� ���.");
	add("sFloodUnknown", mFlood[NMDC_TYPE_UNKNOWN], "�� ������� ������������ ���������.");

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
		add(string("sTimeout").append(name[i]), mTimeoutCmd[i], reason[i]);
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
		add(string("sUnit").append(units[i]), mUnits[i], unitsDef[i]);
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
		add(string("sTimes").append(times[i]), mTimes[i], timesDef[i]);
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
			mConfigStore.mName = "Russian.xml";
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
