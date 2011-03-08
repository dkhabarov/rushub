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
	
	Add("iFloodCountReconnIp",    mFloodCountReconnIp,                  1     );
	Add("iFloodCountMyINFO",      mFloodCount [NMDC_TYPE_MYNIFO],       6     );
	Add("iFloodCountMyINFO2",     mFloodCount2[NMDC_TYPE_MYNIFO],       30    );
	Add("iFloodCountSearch",      mFloodCount [NMDC_TYPE_SEARCH],       3     );
	Add("iFloodCountSearch2",     mFloodCount2[NMDC_TYPE_SEARCH],       10    );
	Add("iFloodCountSearchPas",  	mFloodCount [NMDC_TYPE_SEARCH_PAS],   3     );
	Add("iFloodCountSearchPas2",  mFloodCount2[NMDC_TYPE_SEARCH_PAS],   10    );
	Add("iFloodCountMSearch",     mFloodCount [NMDC_TYPE_MSEARCH],      3     );
	Add("iFloodCountMSearch2",    mFloodCount2[NMDC_TYPE_MSEARCH],      10    );
	Add("iFloodCountMSearchPas",  mFloodCount [NMDC_TYPE_MSEARCH_PAS],  3     );
	Add("iFloodCountMSearchPas2", mFloodCount2[NMDC_TYPE_MSEARCH_PAS],  10    );
	Add("iFloodCountSR",          mFloodCount [NMDC_TYPE_SR],           1000  );
	Add("iFloodCountSR2",         mFloodCount2[NMDC_TYPE_SR],           10000 );
	Add("iFloodCountSRUDP",       mFloodCount [NMDC_TYPE_SR_UDP],       1000  );
	Add("iFloodCountSRUDP2",      mFloodCount2[NMDC_TYPE_SR_UDP],       10000 );
	Add("iFloodCountChat",        mFloodCount [NMDC_TYPE_CHAT],         3     );
	Add("iFloodCountChat2",       mFloodCount2[NMDC_TYPE_CHAT],         20    );
	Add("iFloodCountPm",          mFloodCount [NMDC_TYPE_TO],           5     );
	Add("iFloodCountPm2",         mFloodCount2[NMDC_TYPE_TO],           30    );
	Add("iFloodCountNickList",    mFloodCount [NMDC_TYPE_GETNICKLIST],  1     );
	Add("iFloodCountNickList2",   mFloodCount2[NMDC_TYPE_GETNICKLIST],  1     );
	Add("iFloodCountCTM",         mFloodCount [NMDC_TYPE_CONNECTTOME],  500   );
	Add("iFloodCountCTM2",        mFloodCount2[NMDC_TYPE_CONNECTTOME],  5000  );
	Add("iFloodCountRCTM",        mFloodCount [NMDC_TYPE_RCONNECTTOME], 250   );
	Add("iFloodCountRCTM2",       mFloodCount2[NMDC_TYPE_RCONNECTTOME], 2500  );
	Add("iFloodCountMCTo",        mFloodCount [NMDC_TYPE_MCTO],         5     );
	Add("iFloodCountMCTo2",       mFloodCount2[NMDC_TYPE_MCTO],         30    );
	Add("iFloodCountPing",        mFloodCount [NMDC_TYPE_PING],         5     );
	Add("iFloodCountPing2",       mFloodCount2[NMDC_TYPE_PING],         20    );
	Add("iFloodCountUnknown",     mFloodCount [NMDC_TYPE_UNKNOWN],      1     );
	Add("iFloodCountUnknown2",    mFloodCount2[NMDC_TYPE_UNKNOWN],      10    );

	Add("iFloodTimeReconnIp",     mFloodTimeReconnIp,                   5.    );
	Add("iFloodTimeMyINFO",       mFloodTime [NMDC_TYPE_MYNIFO],        60.   );
	Add("iFloodTimeMyINFO2",      mFloodTime2[NMDC_TYPE_MYNIFO],        900.  );
	Add("iFloodTimeSearch",       mFloodTime [NMDC_TYPE_SEARCH],        10.   );
	Add("iFloodTimeSearch2",      mFloodTime2[NMDC_TYPE_SEARCH],        60.   );
	Add("iFloodTimeSearchPas",    mFloodTime [NMDC_TYPE_SEARCH_PAS],    10.   );
	Add("iFloodTimeSearchPas2",   mFloodTime2[NMDC_TYPE_SEARCH_PAS],    60.   );
	Add("iFloodTimeMSearch",      mFloodTime [NMDC_TYPE_MSEARCH],       10.   );
	Add("iFloodTimeMSearch2",     mFloodTime2[NMDC_TYPE_MSEARCH],       60.   );
	Add("iFloodTimeMSearchPas",   mFloodTime [NMDC_TYPE_MSEARCH_PAS],   10.   );
	Add("iFloodTimeMSearchPas2",  mFloodTime2[NMDC_TYPE_MSEARCH_PAS],   60.   );
	Add("iFloodTimeSR",           mFloodTime [NMDC_TYPE_SR],            60.   );
	Add("iFloodTimeSR2",          mFloodTime2[NMDC_TYPE_SR],            600.  );
	Add("iFloodTimeSRUDP",        mFloodTime [NMDC_TYPE_SR_UDP],        60.   );
	Add("iFloodTimeSRUDP2",       mFloodTime2[NMDC_TYPE_SR_UDP],        600.  );
	Add("iFloodTimeChat",         mFloodTime [NMDC_TYPE_CHAT],          5.    );
	Add("iFloodTimeChat2",        mFloodTime2[NMDC_TYPE_CHAT],          20.   );
	Add("iFloodTimePm",           mFloodTime [NMDC_TYPE_TO],            10.   );
	Add("iFloodTimePm2",          mFloodTime2[NMDC_TYPE_TO],            30.   );
	Add("iFloodTimeNickList",     mFloodTime [NMDC_TYPE_GETNICKLIST],   60.   );
	Add("iFloodTimeNickList2",    mFloodTime2[NMDC_TYPE_GETNICKLIST],   1800. );
	Add("iFloodTimeCTM",          mFloodTime [NMDC_TYPE_CONNECTTOME],   60.   );
	Add("iFloodTimeCTM2",         mFloodTime2[NMDC_TYPE_CONNECTTOME],   600.  );
	Add("iFloodTimeRCTM",         mFloodTime [NMDC_TYPE_RCONNECTTOME],  60.   );
	Add("iFloodTimeRCTM2",        mFloodTime2[NMDC_TYPE_RCONNECTTOME],  600.  );
	Add("iFloodTimeMCTo",         mFloodTime [NMDC_TYPE_MCTO],          10.   );
	Add("iFloodTimeMCTo2",        mFloodTime2[NMDC_TYPE_MCTO],          30.   );
	Add("iFloodTimePing",         mFloodTime [NMDC_TYPE_PING],          1.    );
	Add("iFloodTimePing2",        mFloodTime2[NMDC_TYPE_PING],          30.   );
	Add("iFloodTimeUnknown",      mFloodTime [NMDC_TYPE_UNKNOWN],       3.    );
	Add("iFloodTimeUnknown2",     mFloodTime2[NMDC_TYPE_UNKNOWN],       60.   );

	Add("iLenCmdMSearch",         mMaxCmdLen[NMDC_TYPE_MSEARCH],        256   );
	Add("iLenCmdMSearchPas",      mMaxCmdLen[NMDC_TYPE_MSEARCH_PAS],    256   );
	Add("iLenCmdSearchPas",       mMaxCmdLen[NMDC_TYPE_SEARCH_PAS],     256   );
	Add("iLenCmdSearch",          mMaxCmdLen[NMDC_TYPE_SEARCH],         256   );
	Add("iLenCmdSR",              mMaxCmdLen[NMDC_TYPE_SR],             1024  );
	Add("iLenCmdMyINFO",          mMaxCmdLen[NMDC_TYPE_MYNIFO],         256   );
	Add("iLenCmdSupports",        mMaxCmdLen[NMDC_TYPE_SUPPORTS],       1024  );
	Add("iLenCmdKey",             mMaxCmdLen[NMDC_TYPE_KEY],            128   );
	Add("iLenCmdValidateNick",    mMaxCmdLen[NMDC_TYPE_VALIDATENICK],   64    );
	Add("iLenCmdVersion",         mMaxCmdLen[NMDC_TYPE_VERSION],        32    );
	Add("iLenCmdGetNickList",     mMaxCmdLen[NMDC_TYPE_GETNICKLIST],    12    );
	Add("iLenCmdChat",            mMaxCmdLen[NMDC_TYPE_CHAT],           65536 );
	Add("iLenCmdTo",              mMaxCmdLen[NMDC_TYPE_TO],             65536 );
	Add("iLenCmdQuit",            mMaxCmdLen[NMDC_TYPE_QUIT],           64    );
	Add("iLenCmdMyPass",          mMaxCmdLen[NMDC_TYPE_MYPASS],         64    );
	Add("iLenCmdCTM",             mMaxCmdLen[NMDC_TYPE_CONNECTTOME],    128   );
	Add("iLenCmdRCTM",            mMaxCmdLen[NMDC_TYPE_RCONNECTTOME],   128   );
	Add("iLenCmdMCTM",            mMaxCmdLen[NMDC_TYPE_MCONNECTTOME],   128   );
	Add("iLenCmdKick",            mMaxCmdLen[NMDC_TYPE_KICK],           64    );
	Add("iLenCmdOFM",             mMaxCmdLen[NMDC_TYPE_OPFORCEMOVE],    512   );
	Add("iLenCmdGetINFO",         mMaxCmdLen[NMDC_TYPE_GETINFO],        128   );
	Add("iLenCmdMCTo",            mMaxCmdLen[NMDC_TYPE_MCTO],           65536 );
	Add("iLenCmdUnknown",         mMaxCmdLen[NMDC_TYPE_UNKNOWN],        128   );
	mMaxCmdLen[NMDC_TYPE_PING] = 0; // ping length

	Add("iWebTimeout",            mWebTimeout,                          30.);
	Add("iWebStrSizeMax",         mWebStrSizeMax,                       10240);
	Add("sWebAddresses",          mWebAddresses,
		#ifdef _WIN32
			string("0.0.0.0:80")
		#else
			string("0.0.0.0:8080")
		#endif
	);
	Add("bWebServer",             mWebServer,                           false);

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
		Add(s, mTimeout[i], timeoutDefault[i]);
	}

	Add("iTimeoutAny",            mTimeoutAny,                          600.);
	Add("iTimerServPeriod",       server->mTimerServPeriod,             2000);
	Add("iTimerConnPeriod",       server->mTimerConnPeriod,             4000);
	Add("iStepDelay",             server->mStepDelay,                   0);
	Add("iStrSizeMax",            server->mStrSizeMax,                  10240);
	Add("iSysLoading",            mSysLoading,                          1.);
	Add("iStartPing",             mStartPing,                           300);
	Add("iPingInterval",          mPingInterval,                        60.);
	Add("bNicklistOnLogin",       mNicklistOnLogin,                     true);
	Add("bDelayedLogin",          mDelayedLogin,                        true);
	mDelayedMyinfo = false;//Add("bDelayedMyINFO",    mDelayedMyinfo,   false);
	Add("bDisableNoDCCmd",        mDisableNoDCCmd,                      true);
	Add("bCheckSearchIp",         mCheckSearchIp,                       true);
	Add("bCheckSRNick",           mCheckSrNick,                         true);
	Add("bCheckRctmNick",         mCheckRctmNick,                       true);
	Add("bCheckCTMIp",            mCheckCtmIp,                          true);
	Add("bSendUserIp",            mSendUserIp,                          true);
	Add("bMAC",                   server->mMac,                         false);
	Add("iMaxPassiveRes",         mMaxPassiveRes,                       25);
	Add("iMaxNickLen",            mMaxNickLen,                          32);
	Add("iMinNickLen",            mMinNickLen,                          2);
	Add("iUsersLimit",            mUsersLimit,                          -1);
	Add("iMaxLevel",              miMaxLevel,                           miMaxLevel); // set this default value for log
	Add("iMaxErrLevel",           miMaxErrLevel,                        miMaxErrLevel); // set this default value for log

	Add("sUDPAddresses",          mUdpAddresses,                        string("0.0.0.0:1209") );
	Add("bUDPServer",             mUdpServer,                           false);
	
	Add("sLocale",                mLocale,                              string(setlocale(LC_ALL, "")) );
	Add("sMainBotIP",             mMainBotIp,                           string("127.0.0.1") );
	Add("sMainBotMyINFO",         mMainBotMyinfo,                       string("RusHub bot<Bot V:1.0,M:A,H:0/0/1,S:0>$ $$$0$") );
	Add("bMainBotKey",            mMainBotKey,                          true);
	Add("sHubBot",                mHubBot,                              string("RusHub") );
	Add("bRegMainBot",            mRegMainBot,                          true);
	Add("sTopic",                 mTopic,                               string("Russian hub software") );
	Add("sHubName",               mHubName,                             string("RusHub") );
	Add("sAddresses",             mAddresses,
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
	Add("sLang",       mLang,       string("Russian") );
	Add("sLangPath",   mLangPath,   langPath);
	Add("sLogPath",    mLogPath,    logPath);
	Add("sPluginPath", mPluginPath, pluginPath);
	Add("sMainPath",   mMainPath,   mainPath);
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

	Add("sFirstMsg", mFirstMsg, string("Этот хаб работает под управлением %[HUB] (Время работы: %[uptime] / Юзеров: %[users] / Шара: %[share])."));
	Add("sBadChatNick", mBadChatNick, string("Неверный ник: %[nick]."));
	Add("sBadLoginSequence", mBadLoginSequence, string("Не правильная последовательность отосланных команд при входе."));
	Add("sBadMyinfoNick", mBadMyinfoNick, string("Неверный ник в MyINFO команде."));
	Add("sBadRevConNick", mBadRevConNick, string("Неверный ник при соединении: %[nick]. Ваш реальный ник: %[real_nick]."));
	Add("sTimeout", mTimeout, string("Тайм-аут %[reason]."));
	Add("sTimeoutAny", mTimeoutAny, string("Тайм-аут соединения."));
	Add("sBadCTMIp", mBadCtmIp, string("В запросе на подключение вы отсылаете неверный ip адрес: %[ip], ваш реальный ip: %[real_ip]."));
	Add("sBadSRNick", mBadSrNick, string("Неверный ник в результатах поиска: %[nick]. Ваш реальный ник: %[real_nick]."));
	Add("sBadSearchIp", mBadSearchIp, string("В поисковом запросе вы отсылаете неверный ip адрес: %[ip], ваш реальный ip: %[real_ip]."));
	Add("sUsedNick", mUsedNick, string("Этот ник %[nick] уже используется другим пользователем."));

	Add("sBadNickLen", mBadNickLen, string("Недопустимая длина ника. Допустимая длина ника от %[min] до %[max] символов."));
	Add("sBadChars", mBadChars, string("Недопустимые символы в нике."));
	Add("sUsersLimit", mUsersLimit, string("Достигнут придел по количеству подключенных пользователей."));
	Add("sForceMove", mForceMove, string("Вы были перенаправлены на хаб dchub://%[address] причина: %[reason]"));

	Add("sFloodMyINFO", mFlood[NMDC_TYPE_MYNIFO], string("Пожалуйста не флудите командой MyINFO."));
	Add("sFloodSearch", mFlood[NMDC_TYPE_SEARCH], string("Пожалуйста не используйте поиск так часто."));
	Add("sFloodSearchPassive", mFlood[NMDC_TYPE_SEARCH_PAS], string("Пожалуйста не используйте поиск так часто."));
	Add("sFloodMultiSearch", mFlood[NMDC_TYPE_MSEARCH], string("Пожалуйста не используйте поиск так часто."));
	Add("sFloodMultiSearchPassive", mFlood[NMDC_TYPE_MSEARCH_PAS], string("Пожалуйста не используйте поиск так часто."));
	Add("sFloodSR", mFlood[NMDC_TYPE_SR], string("Не флудите результатами поиска."));
	Add("sFloodSRUDP", mFlood[NMDC_TYPE_SR_UDP], string("Не флудите результатами поиска."));
	Add("sFloodChat", mFlood[NMDC_TYPE_CHAT], string("Пожалуйста не флудите!"));
	Add("sFloodTo", mFlood[NMDC_TYPE_TO], string("Пожалуйста не флудите в привате."));
	Add("sFloodNickList", mFlood[NMDC_TYPE_GETNICKLIST], string("Пожалуйста не флудите с помощью слишком частого получения списка пользователей."));
	Add("sFloodCTM", mFlood[NMDC_TYPE_CONNECTTOME], string("Пожалуйста не флудите частыми запросами на соединение с пользователями хаба."));
	Add("sFloodRCTM", mFlood[NMDC_TYPE_RCONNECTTOME], string("Пожалуйста не флудите частыми запросами на соединение с активнми пользователями хаба."));
	Add("sFloodMCTo", mFlood[NMDC_TYPE_MCTO], string("Пожалуйста не флудите!"));
	Add("sFloodPing", mFlood[NMDC_TYPE_PING], string("Ваш клиент слишком часто пингует хаб."));
	Add("sFloodUnknown", mFlood[NMDC_TYPE_UNKNOWN], string("Не флудите неизвестными командами."));

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
		Add(s, mTimeoutCmd[i], reason[i]);
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
		Add(s, mUnits[i], unitsDef[i]);
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
		Add(s, mTimes[i], timesDef[i]);
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
