/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
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

#ifndef STR_TO_ARG_H
#define STR_TO_ARG_H

#ifdef _WIN32

#include <string>

using namespace ::std;

namespace utils {

class StringToArg {

private:

	static int CopyRawString(char **, char **);
	static int CopyCookedString(char **, char **);

public:

	static int String2Arg(char const * str, int * argc, char *** argv);
	static int String2Arg(string str, int * argc, char *** argv) {
		return String2Arg(str.c_str(), argc, argv);
	}

}; // StringToArg

}; // namespace utils

#endif // _WIN32

#endif // STR_TO_ARG_H

/**
 * $Id$
 * $HeadURL$
 */
