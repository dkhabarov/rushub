/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
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

#include "DcConfigLoader.h"
#include "ConfigList.h"
#include "Server.h" // for version and hub name
#include "stringutils.h"

#ifndef _WIN32
	#include "config.h"
#endif

#if HAVE_LIBTINYXML
	#include <tinyxml.h>
#else
	#include "tinyxml/tinyxml.h"
#endif
namespace dcserver {


DcConfigLoader::DcConfigLoader() : Obj("DcConfigLoader") {
}



DcConfigLoader::~DcConfigLoader() {
}



/** Loading configs */
int DcConfigLoader::load(ConfigListBase * configListBase, const ConfigStore & configStore) {
	return loadFromXml(configListBase, string(configStore.mPath).append(configStore.mName).c_str());
}



/** Saving configs */
int DcConfigLoader::save(ConfigListBase * configListBase, const ConfigStore & configStore) {
	return saveToXml(configListBase, string(configStore.mPath).append(configStore.mName).c_str());
}



int DcConfigLoader::loadFromXml(ConfigListBase * configListBase, const string & fileName) {
	TiXmlDocument file(fileName.c_str());
	if (!file.LoadFile()) {
		LOG(LEVEL_WARN, "Can't open file '" << fileName << "' for reading.");
		return -1;
	}

	TiXmlHandle MainHandle(&file);
	TiXmlElement * mainItem = MainHandle.FirstChild(INTERNALNAME).Element();
	if (mainItem == NULL) {
		return -2;
	}

	// Check version
	const char * version = NULL;
	if (mainItem->ToElement() == NULL || (version = mainItem->ToElement()->Attribute("Version")) == NULL) {
		return -3;
	}

	ConfigItem * configItem = NULL;
	const char * name = NULL;
	const char * data = NULL;
	istringstream * iss = NULL;
	TiXmlNode * value = NULL;
	while ((value = mainItem->IterateChildren(value)) != NULL) {
		if (value->ToElement() == NULL || (name = value->ToElement()->Attribute("Name")) == NULL) {
			continue;
		}
		data = value->ToElement()->GetText();
		if ((configItem = configListBase->operator[](name)) != NULL) {
			iss = new istringstream(data != NULL ? data : "");
			iss->seekg(0, istream::beg);
			(*iss) >> *configItem;
			delete iss;
			iss = NULL;
		} else {
			LOG(LEVEL_TRACE, "Uknown variable '" << name << "' in file '" << fileName << "', ignoring it");
		}
	}
	if (strcmp(version, INTERNALVERSION) != 0) {
		return -4;
	}
	return 0;
}



int DcConfigLoader::saveToXml(ConfigListBase * configListBase, const string & fileName) {
	TiXmlDocument file(fileName.c_str());
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


} // namespace configuration

/**
 * $Id$
 * $HeadURL$
 */
