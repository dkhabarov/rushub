/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz

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

#include "DcConfig.h"
#include "DcServer.h"
#include "Dir.h"

#include <string>

using ::std::string;
using ::server::Server;


namespace dcserver {



DcConfig::DcConfig(ConfigLoader * configLoader, Server * server, const char * cfgFile) : 
	mConfigLoader(configLoader)
{

	mConfigStore.mPath = cfgFile;
	mConfigStore.mName = "";

	addVars(server);
	reload();

	Dir::checkPath(mMainPath);
	Dir::checkPath(mLogPath);
	Dir::checkPath(mLangPath);
	Dir::checkPath(mPluginPath);

	msPath = &mLogPath; // Set log point path

	#ifndef _WIN32
		// Go to the work directory
		if ((chdir(mMainPath.c_str())) < 0) {
			if (ErrLog(1)) {
				LogStream() << "Can not go to the work directory '" << mMainPath << "'. Error: " << strerror(errno) << endl;
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
			string("0.0.0.0:80")
		#else
			string("0.0.0.0:8080")
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
		string s("iTimeout");
		s.append(timeoutName[i]);
		add(s, mTimeout[i], timeoutDefault[i]);
	}

	add("iTimeoutAny",            mTimeoutAny,                          600.);
	add("iTimerServPeriod",       server->mTimerServPeriod,             2000);
	add("iTimerConnPeriod",       server->mTimerConnPeriod,             4000);
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
	add("iMaxLevel",              miMaxLevel,                           miMaxLevel); // set this default value for log
	add("iMaxErrLevel",           miMaxErrLevel,                        miMaxErrLevel); // set this default value for log

	add("sUDPAddresses",          mUdpAddresses,                        string("0.0.0.0:1209") );
	add("bUDPServer",             mUdpServer,                           false);
	
	add("sLocale",                mLocale,                              string(setlocale(LC_ALL, "")) );
	add("sMainBotIP",             mMainBotIp,                           string("127.0.0.1") );
	add("sMainBotMyINFO",         mMainBotMyinfo,                       string("RusHub bot<Bot V:1.0,M:A,H:0/0/1,S:0>$ $$$0$") );
	add("bMainBotKey",            mMainBotKey,                          true);
	add("sHubBot",                mHubBot,                              string("RusHub") );
	add("bRegMainBot",            mRegMainBot,                          true);
	add("sTopic",                 mTopic,                               string("Russian hub software") );
	add("sHubName",               mHubName,                             string("RusHub") );
	add("sAddresses",             mAddresses,
		#ifdef _WIN32
			string("0.0.0.0:411")
		#else
			string("0.0.0.0:4111")
		#endif
	);

	string mainPath(Dir::pathForFile(mConfigStore.mPath.c_str()));

	string langPath(mainPath + "lang/");
	string logPath(mainPath + "logs/");
	string pluginPath(mainPath + "plugins/");

	// TODO: auto detect lang
	add("sLang",       mLang,       string("Russian") );
	add("sLangPath",   mLangPath,   langPath);
	add("sLogPath",    mLogPath,    logPath);
	add("sPluginPath", mPluginPath, pluginPath);
	add("sMainPath",   mMainPath,   mainPath);
}



int DcConfig::reload() {
	if (load() < 0) {

		// Set default log levels
		miMaxErrLevel = 2;
		miMaxLevel = 0;

		save();
		return 1;
	}
	return 0;
}





DcLang::DcLang(ConfigLoader * configLoader, ConfigListBase * configListBase) :
	mConfigLoader(configLoader)
{

	Config * langConfig = configListBase->operator[] ("sLang");
	Config * langPathConfig = configListBase->operator[] ("sLangPath");
	if (langConfig && langPathConfig) {
		string path, lang;
		langPathConfig->convertTo(path);
		langConfig->convertTo(lang);

		// set xml file for lang
		if (Dir::checkPath(path)) {
			mConfigStore.mPath = path;
			mConfigStore.mName = lang + ".xml";
		} else {
			mConfigStore.mPath = "./";
		}
	}

	addVars();
	reload();
}



DcLang::~DcLang() {
}



void DcLang::addVars() {

	add("sFirstMsg", mFirstMsg, string("Этот хаб работает под управлением %[HUB] (Время работы: %[uptime] / Юзеров: %[users] / Шара: %[share])."));
	add("sBadChatNick", mBadChatNick, string("Неверный ник: %[nick]."));
	add("sBadLoginSequence", mBadLoginSequence, string("Не правильная последовательность отосланных команд при входе."));
	add("sBadMyinfoNick", mBadMyinfoNick, string("Неверный ник в MyINFO команде."));
	add("sBadRevConNick", mBadRevConNick, string("Неверный ник при соединении: %[nick]. Ваш реальный ник: %[real_nick]."));
	add("sTimeout", mTimeout, string("Тайм-аут %[reason]."));
	add("sTimeoutAny", mTimeoutAny, string("Тайм-аут соединения."));
	add("sBadCTMIp", mBadCtmIp, string("В запросе на подключение вы отсылаете неверный ip адрес: %[ip], ваш реальный ip: %[real_ip]."));
	add("sBadSRNick", mBadSrNick, string("Неверный ник в результатах поиска: %[nick]. Ваш реальный ник: %[real_nick]."));
	add("sBadSearchIp", mBadSearchIp, string("В поисковом запросе вы отсылаете неверный ip адрес: %[ip], ваш реальный ip: %[real_ip]."));
	add("sUsedNick", mUsedNick, string("Этот ник %[nick] уже используется другим пользователем."));

	add("sBadNickLen", mBadNickLen, string("Недопустимая длина ника. Допустимая длина ника от %[min] до %[max] символов."));
	add("sBadChars", mBadChars, string("Недопустимые символы в нике."));
	add("sUsersLimit", mUsersLimit, string("Достигнут придел по количеству подключенных пользователей."));
	add("sForceMove", mForceMove, string("Вы были перенаправлены на хаб dchub://%[address] причина: %[reason]"));

	add("sFloodMyINFO", mFlood[NMDC_TYPE_MYNIFO], string("Пожалуйста не флудите командой MyINFO."));
	add("sFloodSearch", mFlood[NMDC_TYPE_SEARCH], string("Пожалуйста не используйте поиск так часто."));
	add("sFloodSearchPassive", mFlood[NMDC_TYPE_SEARCH_PAS], string("Пожалуйста не используйте поиск так часто."));
	add("sFloodMultiSearch", mFlood[NMDC_TYPE_MSEARCH], string("Пожалуйста не используйте поиск так часто."));
	add("sFloodMultiSearchPassive", mFlood[NMDC_TYPE_MSEARCH_PAS], string("Пожалуйста не используйте поиск так часто."));
	add("sFloodSR", mFlood[NMDC_TYPE_SR], string("Не флудите результатами поиска."));
	add("sFloodSRUDP", mFlood[NMDC_TYPE_SR_UDP], string("Не флудите результатами поиска."));
	add("sFloodChat", mFlood[NMDC_TYPE_CHAT], string("Пожалуйста не флудите!"));
	add("sFloodTo", mFlood[NMDC_TYPE_TO], string("Пожалуйста не флудите в привате."));
	add("sFloodNickList", mFlood[NMDC_TYPE_GETNICKLIST], string("Пожалуйста не флудите с помощью слишком частого получения списка пользователей."));
	add("sFloodCTM", mFlood[NMDC_TYPE_CONNECTTOME], string("Пожалуйста не флудите частыми запросами на соединение с пользователями хаба."));
	add("sFloodRCTM", mFlood[NMDC_TYPE_RCONNECTTOME], string("Пожалуйста не флудите частыми запросами на соединение с активнми пользователями хаба."));
	add("sFloodMCTo", mFlood[NMDC_TYPE_MCTO], string("Пожалуйста не флудите!"));
	add("sFloodUserIp", mFlood[NMDC_TYPE_USERIP], string("Пожалуйста не флудите запросами UserIP!"));
	add("sFloodPing", mFlood[NMDC_TYPE_PING], string("Ваш клиент слишком часто пингует хаб."));
	add("sFloodUnknown", mFlood[NMDC_TYPE_UNKNOWN], string("Не флудите неизвестными командами."));

	const char * reason[] = {
		"получения ключа",
		"валидации ника",
		"входа",
		"получения MyINFO",
		"получения пароля"
	};

	const char * name[] = {
		"Key",
		"Nick",
		"Login",
		"Myinfo",
		"Getpass"
	};

	for (int i = 0; i < 5; ++i) {
		string s("sTimeout");
		s.append(name[i]);
		add(s, mTimeoutCmd[i], reason[i]);
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
		"Б",
		"КБ",
		"МБ",
		"ГБ",
		"ТБ",
		"ПБ",
		"ЭБ"
	};

	for (int i = 0; i < 7; ++i) {
		mUnits[i] = unitsDef[i];
		string s("sUnit");
		s.append(units[i]);
		add(s, mUnits[i], unitsDef[i]);
	}

	const char * times[] = {
		"Weeks",
		"Days",
		"Hours",
		"Min",
		"Sec"
	};

	const char * timesDef[] = {
		"нед.",
		"дн.",
		"ч.",
		"мин.",
		"сек."
	};

	for (int i = 0; i < 5; ++i) {
		mTimes[i] = timesDef[i];
		string s("sTimes");
		s.append(times[i]);
		add(s, mTimes[i], timesDef[i]);
	}

}



int DcLang::reload() {

	if (load() < 0) {

		// save default lang file
		string name = mConfigStore.mName;
		mConfigStore.mName = "Russian.xml";

		string file(mConfigStore.mPath + mConfigStore.mName);
		if (!Dir::isFileExist(file.c_str())) {
			save();
			mConfigStore.mName = name;
			return 2;
		}

		mConfigStore.mName = name;
		return 1;
	}

	return 0;
}


}; // namespace dcserver

/**
 * $Id$
 * $HeadURL$
 */
