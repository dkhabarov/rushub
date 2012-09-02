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

#ifndef ZLIB_FILTER_H
#define ZLIB_FILTER_H

#ifdef HAVE_LIBZLIB

#include "stdinc.h"

#ifdef _WIN32
# include "zlib/zlib.h"
#else
# include <zlib.h>
#endif

#include <string>

using namespace ::std;

namespace utils {

class ZlibFilter {

public:

	ZlibFilter();

	void init();
	void finish();
	bool compress(const char * in, size_t & inSize, char * out, size_t & outSize);

	static bool compressFull(const char * in, size_t & inSize, char * out, size_t & outSize);
	static bool compressFull(const char * in, size_t inSize, string & out);

private:

	int64_t mTotalIn;
	int64_t mTotalOut;
	bool mCompressing;
	z_stream mStream;

}; // class ZlibFilter

} // namespace utils

#endif // HAVE_LIBZLIB

#endif // ZLIB_FILTER_H

/**
 * $Id$
 * $HeadURL$
 */
