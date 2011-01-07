/*
 * RusHub - hub server for Direct Connect peer to peer network.

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

#include "cconfigloader.h"
#include "stringutils.h"
#include "cconfiglist.h"

#include "cserver.h" /** for version and hub name */
#include "tinyxml/tinyxml.h"

namespace nConfig {

cConfigLoader::cConfigLoader() : cObj("cConfigLoader") {
}

cConfigLoader::~cConfigLoader() {
}

/** Loading configs */
bool cConfigLoader::Load(cConfigListBase *ConfigList, const char* sFileName) {
	mConfigList = ConfigList;
	msFileName = sFileName;
	return LoadFromXml();
}

/** Saving configs */
bool cConfigLoader::Save(cConfigListBase *ConfigList, const char* sFileName) {
	mConfigList = ConfigList;
	msFileName = sFileName;
	return SaveToXml();
}

bool cConfigLoader::LoadFromXml() {
	TiXmlDocument file(msFileName.c_str());
	if(!file.LoadFile()) {
		if(Log(1)) LogStream() << "Can't open file '" << msFileName << "' for reading." << endl;
		return false;
	}

	TiXmlHandle MainHandle(&file);
	TiXmlElement *MainItem = MainHandle.FirstChild(INTERNALNAME).Element();
	if(MainItem == NULL)
		return false;

	/** Check version */
	const char * sVersion;
	if(MainItem->ToElement() == NULL || (sVersion = MainItem->ToElement()->Attribute("Version")) == NULL)
		return false;

	cConfig *ci;
	char * sName, * sData;
	istringstream *iss;
	TiXmlNode *Value = NULL;
	while((Value = MainItem->IterateChildren(Value)) != NULL) {
		if(Value->ToElement() == NULL || (sName = (char *)Value->ToElement()->Attribute("Name")) == NULL)
			continue;
		sData = (char *)Value->ToElement()->GetText();
		if((ci = mConfigList->operator[](sName)) != NULL) {
			iss = new istringstream(sData != NULL ? sData : "");
			iss->seekg(0, istream::beg);
			(*iss) >> *ci;
			delete iss;
			iss = NULL;
		}
		else if(Log(3)) LogStream() << "Uknown variable '" << sName << "' in file '" << msFileName << "', ignoring it" << endl;
	}
	if(strcmp(sVersion, INTERNALVERSION) != 0) return false;
	return true;
}

bool cConfigLoader::SaveToXml() {
	TiXmlDocument file(msFileName.c_str());
	file.InsertEndChild(TiXmlDeclaration("1.0", "windows-1251", "yes"));
	TiXmlElement MainItem(INTERNALNAME);
	MainItem.SetAttribute("Version", INTERNALVERSION);
	for(cConfigListBase::tHLMIt it = mConfigList->mList.begin(); it != mConfigList->mList.end(); ++it) {
		TiXmlElement Item("Item");
		stringstream ss;
		ss << *(*it);
		Item.SetAttribute("Name", (*it)->msName.c_str());
		Item.InsertEndChild(TiXmlText(ss.str().c_str()));
		MainItem.InsertEndChild(Item);
	}
	file.InsertEndChild(MainItem);
	file.SaveFile();
	return true;
}

/** Loading from file */
bool cConfigLoader::LoadFromFile() {
	string sName, sData;
	char ch;
	istringstream *iss;
	cConfig *ci;
	ifstream ifs(msFileName.c_str());
	if(!ifs.is_open()) {
		if(Log(1)) LogStream() << "Can't open file '" << msFileName << "' for reading." << endl;
		return false;
	}
	while(!ifs.eof()) {
		ifs >> sName;
		if(sName[sName.size()-1] != '=')
			ifs >> ch;
		else {
			ch = '=';
			sName.assign(sName, 0, sName.size() - 1);
		}
		if(ch != '=') break;
		getline(ifs, sData);
		if(sData[0] == ' ') sData.assign(sData, 1, sData.size());
		if((ci = mConfigList->operator[](sName)) != NULL) {
			tItemType tIT = ci->GetTypeID();
			if((tIT == eIT_STRING) || (tIT == eIT_PCHAR))
				sData = ReplaceSp(sData, true);

			iss = new istringstream(sData);
			iss->seekg(0, istream::beg);
			(*iss) >> *ci;
			delete iss;
			iss = NULL;
		}
		else if(Log(3)) LogStream() << "Uknown variable '" << sName << "' in file '" << msFileName << "', ignoring it" << endl;
	}
	ifs.close();
	return true;
}

/** Saving to stream */
bool cConfigLoader::SaveToStream(ostream &os) {
	for(cConfigListBase::tHLMIt it = mConfigList->mList.begin(); it != mConfigList->mList.end(); ++it) {
		string sData;
		tItemType tIT = (*it)->GetTypeID();
		if((tIT == eIT_STRING) || (tIT == eIT_PCHAR)) {
			sData = ReplaceSp(*(*it));
			os << (*it)->msName << " = " << sData << endl;
		}
		else os << (*it)->msName << " = " << *(*it) << endl;
	}
	return true;
}

/** Saving to file */
bool cConfigLoader::SaveToFile() {
	ofstream ofs(msFileName.c_str());
	SaveToStream(ofs);
	ofs.close();
	return true;
}

}; // nConfig
