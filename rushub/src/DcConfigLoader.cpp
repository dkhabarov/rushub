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

#include "DcConfigLoader.h"
#include "ConfigList.h"
#include "Server.h" // for version and hub name
#include "stringutils.h"

#include "tinyxml/tinyxml.h"


namespace dcserver {


DcConfigLoader::DcConfigLoader() : Obj("DcConfigLoader") {
}



DcConfigLoader::~DcConfigLoader() {
}



/** Loading configs */
int DcConfigLoader::load(ConfigListBase * configListBase, const ConfigStore & configStore) {

	// TODO: choose loader
	string file(configStore.mPath + configStore.mName);
	return loadFromXml(configListBase, file.c_str());
}



/** Saving configs */
int DcConfigLoader::save(ConfigListBase * configListBase, const ConfigStore & configStore) {

	// TODO: choose loader
	string file(configStore.mPath + configStore.mName);
	return saveToXml(configListBase, file.c_str());
}



int DcConfigLoader::loadFromXml(ConfigListBase * configListBase, const char * fileName) {
	TiXmlDocument file(fileName);
	if (!file.LoadFile()) {
		if (Log(0)) {
			LogStream() << "Can't open file '" << fileName <<
			"' for reading." << endl;
		}
		return -1;
	}

	TiXmlHandle MainHandle(&file);
	TiXmlElement * mainItem = MainHandle.FirstChild(INTERNALNAME).Element();
	if (mainItem == NULL) {
		return -2;
	}

	/** Check version */
	const char * version = NULL;
	if (mainItem->ToElement() == NULL || (version = mainItem->ToElement()->Attribute("Version")) == NULL) {
		return -3;
	}

	Config * config = NULL;
	const char * name = NULL;
	const char * data = NULL;
	istringstream * iss = NULL;
	TiXmlNode * value = NULL;
	while ((value = mainItem->IterateChildren(value)) != NULL) {
		if (value->ToElement() == NULL || (name = value->ToElement()->Attribute("Name")) == NULL) {
			continue;
		}
		data = value->ToElement()->GetText();
		if ((config = configListBase->operator[](name)) != NULL) {
			iss = new istringstream(data != NULL ? data : "");
			iss->seekg(0, istream::beg);
			(*iss) >> *config;
			delete iss;
			iss = NULL;
		} else {
			if (Log(4)) {
				LogStream() << "Uknown variable '" << name <<
				"' in file '" << fileName << "', ignoring it" << endl;
			}
		}
	}
	if (strcmp(version, INTERNALVERSION) != 0) {
		return -4;
	}
	return 0;
}



int DcConfigLoader::saveToXml(ConfigListBase * configListBase, const char * fileName) {
	TiXmlDocument file(fileName);
	file.InsertEndChild(TiXmlDeclaration("1.0", "windows-1251", "yes"));
	TiXmlElement mainItem(INTERNALNAME);
	mainItem.SetAttribute("Version", INTERNALVERSION);
	for (ConfigListBase::tHLMIt it = configListBase->mList.begin(); it != configListBase->mList.end(); ++it) {
		TiXmlElement item("Item");
		stringstream ss;
		ss << *(*it);
		item.SetAttribute("Name", (*it)->mName.c_str());
		item.InsertEndChild(TiXmlText(ss.str().c_str()));
		mainItem.InsertEndChild(item);
	}
	file.InsertEndChild(mainItem);
	file.SaveFile();
	return 0;
}



/** Loading from file */
int DcConfigLoader::loadFromFile(ConfigListBase * configListBase, const char * fileName) {
	ifstream ifs(fileName);
	if (!ifs.is_open()) {
		if (Log(0)) {
			LogStream() << "Can't open file '" << fileName <<
			"' for reading." << endl;
		}
		return -1;
	}

	string name, data;
	char c;
	istringstream * iss;
	Config * config;
	while (!ifs.eof()) {
		ifs >> name;
		if (name[name.size() - 1] != '=') {
			ifs >> c;
		} else {
			c = '=';
			name.assign(name, 0, name.size() - 1);
		}
		if (c != '=') {
			break;
		}
		getline(ifs, data);
		if (data[0] == ' ') {
			data.assign(data, 1, data.size());
		}
		if ((config = configListBase->operator[](name)) != NULL) {
			ItemType itemType = config->getTypeId();
			if ((itemType == ITEM_TYPE_STRING) || (itemType == ITEM_TYPE_PCHAR)) {
				data = replaceSp(data, true);
			}

			iss = new istringstream(data);
			iss->seekg(0, istream::beg);
			(*iss) >> *config;
			delete iss;
			iss = NULL;
		} else {
			if (Log(4)) {
				LogStream() << "Uknown variable '" << name <<
				"' in file '" << fileName << "', ignoring it" << endl;
			}
		}
	}
	ifs.close();
	return 0;
}



/** Saving to stream */
int DcConfigLoader::saveToStream(ConfigListBase * configListBase, ostream & os) {

	for (ConfigListBase::tHLMIt it = configListBase->mList.begin(); it != configListBase->mList.end(); ++it) {

		ItemType itemType = (*it)->getTypeId();
		if ((itemType == ITEM_TYPE_STRING) || (itemType == ITEM_TYPE_PCHAR)) {
			string data = replaceSp(*(*it));
			os << (*it)->mName << " = " << data << endl;
		} else {
			os << (*it)->mName << " = " << *(*it) << endl;
		}

	}
	return 0;
}



/** Saving to file */
int DcConfigLoader::saveToFile(ConfigListBase * configListBase, const char * fileName) {
	ofstream ofs(fileName);
	saveToStream(configListBase, ofs);
	ofs.close();
	return 0;
}


}; // namespace configuration

/**
* $Id$
* $HeadURL$
*/
