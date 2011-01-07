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

#include "cdcconfig.h"
#include "cdcserver.h" /** for mDCServer */
#include <string>

using namespace std;

namespace nDCServer {

cDCConfig::cDCConfig() {
}

cDCConfig::~cDCConfig() {
}

void cDCConfig::SetServer(cDCServer * server) {
	mDCServer = server;
	AddVars();
}

void cDCConfig::AddVars() {

	Add("iFloodCountReconnIp",  miFloodCountReconnIp,         1);
	Add("iFloodCountMyINFO",    miFloodCountMyINFO,           6);
	Add("iFloodCountSearch",    miFloodCountSearch,           3);
	Add("iFloodCountSR",        miFloodCountSR,               1000);
	Add("iFloodCountChat",      miFloodCountChat,             3);
	Add("iFloodCountPm",        miFloodCountTo,               5);
	Add("iFloodCountNickList",  miFloodCountNickList,         1);
	Add("iFloodCountCTM",       miFloodCountCTM,              500);
	Add("iFloodCountRCTM",      miFloodCountRCTM,             250);
	Add("iFloodCountMCTo",      miFloodCountMCTo,             5);
	Add("iFloodCountPing",      miFloodCountPing,             5);
	Add("iFloodCountUnknown",   miFloodCountUnknown,          1);
	Add("iFloodCountMyINFO2",   miFloodCountMyINFO2,          30);
	Add("iFloodCountSearch2",   miFloodCountSearch2,          10);
	Add("iFloodCountSR2",       miFloodCountSR2,              10000);
	Add("iFloodCountChat2",     miFloodCountChat2,            20);
	Add("iFloodCountPm2",       miFloodCountTo2,              30);
	Add("iFloodCountNickList2", miFloodCountNickList2,        1);
	Add("iFloodCountCTM2",      miFloodCountCTM2,             5000);
	Add("iFloodCountRCTM2",     miFloodCountRCTM2,            2500);
	Add("iFloodCountMCTo2",     miFloodCountMCTo2,            30);
	Add("iFloodCountPing2",     miFloodCountPing2,            20);
	Add("iFloodCountUnknown2",  miFloodCountUnknown2,         10);

	Add("iFloodTimeReconnIp",   miFloodTimeReconnIp,          5.);
	Add("iFloodTimeMyINFO",     miFloodTimeMyINFO,            60.);
	Add("iFloodTimeSearch",     miFloodTimeSearch,            10.);
	Add("iFloodTimeSR",         miFloodTimeSR,                60.);
	Add("iFloodTimeChat",       miFloodTimeChat,              5.);
	Add("iFloodTimePm",         miFloodTimeTo,                10.);
	Add("iFloodTimeNickList",   miFloodTimeNickList,          60.);
	Add("iFloodTimeCTM",        miFloodTimeCTM,               60.);
	Add("iFloodTimeRCTM",       miFloodTimeRCTM,              60.);
	Add("iFloodTimeMCTo",       miFloodTimeMCTo,              10.);
	Add("iFloodTimePing",       miFloodTimePing,              1.);
	Add("iFloodTimeUnknown",    miFloodTimeUnknown,           3.);
	Add("iFloodTimeMyINFO2",    miFloodTimeMyINFO2,           900.);
	Add("iFloodTimeSearch2",    miFloodTimeSearch2,           60.);
	Add("iFloodTimeSR2",        miFloodTimeSR2,               600.);
	Add("iFloodTimeChat2",      miFloodTimeChat2,             20.);
	Add("iFloodTimePm2",        miFloodTimeTo2,               30.);
	Add("iFloodTimeNickList2",  miFloodTimeNickList2,         1800.);
	Add("iFloodTimeCTM2",       miFloodTimeCTM2,              600.);
	Add("iFloodTimeRCTM2",      miFloodTimeRCTM2,             600.);
	Add("iFloodTimeMCTo2",      miFloodTimeMCTo2,             30.);
	Add("iFloodTimePing2",      miFloodTimePing2,             30.);
	Add("iFloodTimeUnknown2",   miFloodTimeUnknown2,          60.);

	Add("iLenCmdMSearch",       mMaxCmdLen[eDC_MSEARCH],      256   );
	Add("iLenCmdMSearchPas",    mMaxCmdLen[eDC_MSEARCH_PAS],  256   );
	Add("iLenCmdSearchPas",     mMaxCmdLen[eDC_SEARCH_PAS],   256   );
	Add("iLenCmdSearch",        mMaxCmdLen[eDC_SEARCH],       256   );
	Add("iLenCmdSR",            mMaxCmdLen[eDC_SR],           1024  );
	Add("iLenCmdMyINFO",        mMaxCmdLen[eDC_MYNIFO],       256   );
	Add("iLenCmdSupports",      mMaxCmdLen[eDC_SUPPORTS],     1024  );
	Add("iLenCmdKey",           mMaxCmdLen[eDC_KEY],          128   );
	Add("iLenCmdValidateNick",  mMaxCmdLen[eDC_VALIDATENICK], 64    );
	Add("iLenCmdVersion",       mMaxCmdLen[eDC_VERSION],      32    );
	Add("iLenCmdGetNickList",   mMaxCmdLen[eDC_GETNICKLIST],  12    );
	Add("iLenCmdChat",          mMaxCmdLen[eDC_CHAT],         65536 );
	Add("iLenCmdTo",            mMaxCmdLen[eDC_TO],           65536 );
	Add("iLenCmdQuit",          mMaxCmdLen[eDC_QUIT],         64    );
	Add("iLenCmdMyPass",        mMaxCmdLen[eDC_MYPASS],       64    );
	Add("iLenCmdCTM",           mMaxCmdLen[eDC_CONNECTTOME],  128   );
	Add("iLenCmdRCTM",          mMaxCmdLen[eDC_RCONNECTTOME], 128   );
	Add("iLenCmdMCTM",          mMaxCmdLen[eDC_MCONNECTTOME], 128   );
	Add("iLenCmdKick",          mMaxCmdLen[eDC_KICK],         64    );
	Add("iLenCmdOFM",           mMaxCmdLen[eDC_OPFORCEMOVE],  512   );
	Add("iLenCmdGetINFO",       mMaxCmdLen[eDC_GETINFO],      128   );
	Add("iLenCmdMCTo",          mMaxCmdLen[eDC_MCTO],         65536 );
	Add("iLenCmdUnknown",       mMaxCmdLen[eDC_UNKNOWN],      128   );
	mMaxCmdLen[eDC_PING] = 0; // ping length

	Add("iWebTimeout",          miWebTimeout,                 30.);
	Add("iWebStrSizeMax",       miWebStrSizeMax,              10240);
	Add("sWebAddresses",        msWebAddresses,
		#ifdef _WIN32
			string("0.0.0.0:81")
		#else
			string("0.0.0.0:8080")
		#endif
	);
	Add("bWebServer",           mbWebServer,                  false);

	/* Timeouts of the protocol commands */
	const char *sTimeoutName[] = {"Key", "Nick", "Login", "Myinfo", "Getpass"};
	double iTimeoutDefault[] = { 60. , 30., 600., 40., 300. };
	string s;
	for(int i = 0; i < 5; ++i) {
		miTimeout[i] = iTimeoutDefault[i];
		s = "iTimeout";
		s.append(sTimeoutName[i]);
		Add(s, miTimeout[i], iTimeoutDefault[i]);
	}

	Add("iTimeoutAny",          miTimeoutAny,                 600.);
	Add("iTimerServPeriod",     mDCServer->miTimerServPeriod, 2000);
	Add("iTimerConnPeriod",     mDCServer->miTimerConnPeriod, 4000);
	Add("iStepDelay",           mDCServer->mStepDelay,        0);
	Add("iStrSizeMax",          mDCServer->miStrSizeMax,      10240);
	Add("iSysLoading",          miSysLoading,                 1.);
	Add("iStartPing",           miStartPing,                  300);
	Add("iPingInterval",        miPingInterval,               60.);
	Add("bNicklistOnLogin",     mbNicklistOnLogin,            true);
	Add("bDelayedLogin",        mbDelayedLogin,               true);
	mbDelayedMyINFO = false;//Add("bDelayedMyINFO",       mbDelayedMyINFO,              false);
	Add("bDisableNoDCCmd",      mbDisableNoDCCmd,             true);
	Add("bCheckSearchIp",       mbCheckSearchIp,              true);
	Add("bCheckSRNick",         mbCheckSRNick,                true);
	Add("bCheckRctmNick",       mbCheckRctmNick,              true);
	Add("bCheckCTMIp",          mbCheckCTMIp,                 true);
	Add("bSendUserIp",          mbSendUserIp,                 true);
	Add("bMAC",                 mDCServer->mbMAC,             false);
	Add("iMaxPassiveRes",       miMaxPassiveRes,              25);
	Add("iMaxNickLen",          miMaxNickLen,                 32);
	Add("iMinNickLen",          miMinNickLen,                 2);
	Add("iUsersLimit",          miUsersLimit,                 -1);
	Add("iMaxLevel",            miMaxLevel,                   0);
	Add("iMaxErrLevel",         miMaxErrLevel,                2);
	Add("sLocale",              msLocale,                     string(setlocale(LC_ALL, "")) );
	Add("sMainBotIP",           msMainBotIP,                  string("127.0.0.1")  );
	Add("sMainBotMyINFO",       msMainBotMyINFO,              string("RusHub bot<Bot V:1.0,M:A,H:0/0/1,S:0>$ $$$0$")  );
	Add("bMainBotKey",          mbMainBotKey,                 true);
	Add("sHubBot",              msHubBot,                     string("RusHub")  );
	Add("bRegMainBot",          mbRegMainBot,                 true);
	Add("sTopic",               msTopic,                      string("Russian hub software"));
	Add("sHubName",             msHubName,                    string("RusHub")  );
	Add("sAddresses",           mDCServer->msAddresses,
		#ifdef _WIN32
			string("0.0.0.0:411")
		#else
			string("0.0.0.0:4111")
		#endif
	);
}

bool cDCConfig::Load() {
	string sFile(mDCServer->mMainPath.GetConfPath() + string("config.xml"));
	return mDCServer->mConfigLoader.Load(this, sFile.c_str());
}

bool cDCConfig::Save() {
	string sFile(mDCServer->mMainPath.GetConfPath() + string("config.xml"));
	return mDCServer->mConfigLoader.Save(this, sFile.c_str());
}

cDCLang::cDCLang() {
}

cDCLang::~cDCLang() {
}

void cDCLang::SetServer(cDCServer * server) {
	mDCServer = server;
	AddVars();
}

void cDCLang::AddVars() {
	Add("sFirstMsg", msFirstMsg, string("Этот хаб работает под управлением %[HUB] (Время работы: %[uptime] / Юзеров: %[users] / Шара: %[share])."));
	Add("sBadChatNick", msBadChatNick, string("Неверный ник: %[nick]."));
	Add("sMyinfoError", msMyinfoError, string("Ошибка в синтаксисе команды MyINFO."));
	Add("sBadLoginSequence", msBadLoginSequence, string("Не правильная последовательность отосланных команд при входе."));
	Add("sBadMyinfoNick", msBadMyinfoNick, string("Неверный ник в MyINFO команде."));
	Add("sBadRevConNick", msBadRevConNick, string("Неверный ник при соединении: %[nick]. Ваш реальный ник: %[real_nick]."));
	Add("sTimeout", msTimeout, string("Тайм-аут %[reason]."));
	Add("sTimeoutAny", msTimeoutAny, string("Тайм-аут соединения."));
	Add("sBadCTMIp", msBadCTMIp, string("В запросе на подключение вы отсылаете неверный ip адрес: %[ip], ваш реальный ip: %[real_ip]."));
	Add("sBadSRNick", msBadSRNick, string("Неверный ник в результатах поиска: %[nick]. Ваш реальный ник: %[real_nick]."));
	Add("sBadSearchIp", msBadSearchIp, string("В поисковом запросе вы отсылаете неверный ip адрес: %[ip], ваш реальный ip: %[real_ip]."));
	Add("sUsedNick", msUsedNick, string("Этот ник %[nick] уже используется другим пользователем."));

	Add("sBadNickLen", msBadNickLen, string("Недопустимая длина ника. Допустимая длина ника от %[min] до %[max] символов."));
	Add("sBadChars", msBadChars, string("Недопустимые символы в нике."));
	Add("sUsersLimit", msUsersLimit, string("Достигнут придел по количеству подключенных пользователей."));
	Add("sForceMove", msForceMove, string("Вы были перенаправлены на хаб dchub://%[address] причина: %[reason]"));

	Add("sFloodMyINFO", msFloodMyINFO, string("Пожалуйста не флудите командой MyINFO."));
	Add("sFloodSearch", msFloodSearch, string("Пожалуйста не используйте поиск так часто."));
	Add("sFloodSR", msFloodSR, string("Не флудите результатами поиска."));
	Add("sFloodChat", msFloodChat, string("Пожалуйста не флудите!"));
	Add("sFloodTo", msFloodTo, string("Пожалуйста не флудите в привате."));
	Add("sFloodNickList", msFloodNickList, string("Пожалуйста не флудите с помощью слишком частого получения списка пользователей."));
	Add("sFloodCTM", msFloodCTM, string("Пожалуйста не флудите частыми запросами на соединение с пользователями хаба."));
	Add("sFloodRCTM", msFloodRCTM, string("Пожалуйста не флудите частыми запросами на соединение с активнми пользователями хаба."));
	Add("sFloodMCTo", msFloodTo, string("Пожалуйста не флудите!"));
	Add("sFloodPing", msFloodPing, string("Ваш клиент слишком часто пингует хаб."));
	Add("sFloodUnknown", msFloodUnknown, string("Не флудите неизвестными командами."));

	string s;
	int i;
	const char *sReason[] = {"получения ключа", "валидации ника", "входа", "получения MyINFO", "получения пароля"};
	const char *sName[] = {"Key", "Nick", "Login", "Myinfo", "Getpass"};
	for(i = 0; i < 5; ++i) {
		s = "sTimeout";
		s.append(sName[i]);
		Add(s, msTimeoutCmd[i], sReason[i]);
	}

	const char *sUnits[] = {"B", "KB", "MB", "GB", "TB", "PB", "EB"};
	const char *sUnitsDef[] = {"Б", "КБ", "МБ", "ГБ", "ТБ", "ПБ", "ЭБ"};
	for(i = 0; i < 7; ++i) {
		msUnits[i] = sUnitsDef[i];
		s = "sUnit";
		s.append(sUnits[i]);
		Add(s, msUnits[i], sUnitsDef[i]);
	}

	const char *sTimes[] = {"Weeks", "Days", "Hours", "Min", "Sec"};
	const char *sTimesDef[] = {"нед.", "дн.", "ч.", "мин.", "сек."};
	for(i = 0; i < 5; ++i) {
		msTimes[i] = sTimesDef[i];
		s = "sTimes";
		s.append(sTimes[i]);
		Add(s, msTimes[i], sTimesDef[i]);
	}
}

bool cDCLang::Load() {
	string sFile(mDCServer->mMainPath.GetConfPath() + string("lang.xml"));
	return mDCServer->mConfigLoader.Load(this, sFile.c_str());
}

bool cDCLang::Save() {
	string sFile(mDCServer->mMainPath.GetConfPath() + string("lang.xml"));
	return mDCServer->mConfigLoader.Save(this, sFile.c_str());
}

}; // nDCServer
