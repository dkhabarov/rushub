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


#include "PythonPlugin.h"
#include "Dir.h"

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif // HAVE_CONFIG_H

#if HAVE_TINYXML_H
  #include <tinyxml.h>
#else
  #include "tinyxml/tinyxml.h"
#endif // HAVE_TINYXML_H

#ifdef _WIN32
	#pragma comment(lib, "tinyxml.lib")
#endif // _WIN32

using namespace ::utils;

PythonPlugin::PythonPlugin() {
	mName = PLUGIN_NAME;
	mVersion = PLUGIN_VERSION;
}

PythonPlugin::~PythonPlugin() {
}

REG_PLUGIN(PythonPlugin);


/**
 * $Id$
 * $HeadURL$
 */
