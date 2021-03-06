/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * Copyright (C) 2009-2013 by Setuper
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

#include "ZlibFilter.h"

#if HAVE_LIBZLIB

#include <string.h>

namespace utils {

#define BUF_SIZE 1024

ZlibFilter::ZlibFilter() {
	init();
}

void ZlibFilter::init() {
	mTotalIn = 0;
	mTotalOut = 0;
	mCompressing = true;
	memset(&mStream, 0 , sizeof (mStream));
	deflateInit(&mStream, Z_BEST_COMPRESSION);
}

void ZlibFilter::finish() {
	deflateEnd(&mStream);
}

bool ZlibFilter::compress(const char * in, size_t & inSize, char * out, size_t & outSize) {
	if (outSize == 0) {
		return false;
	}

	mStream.next_in = const_cast<Bytef *> (reinterpret_cast<const Bytef *> (in));
	mStream.next_out = reinterpret_cast<Bytef *> (out);

	// Check if there's any use compressing; if not, save some cpu...
	if (mCompressing &&
			inSize > 0 &&
			outSize > 16 &&
			mTotalIn > 64 * 1024 &&
			(static_cast<double> (mTotalOut) / static_cast<double> (mTotalIn)) > 0.97
	) {
		mStream.avail_in = 0;
		mStream.avail_out = outSize;
		if (deflateParams(&mStream, 0, Z_DEFAULT_STRATEGY) != Z_OK) {
			return false;
		}
		mStream.avail_in = inSize;
		mCompressing = false;

		// Check if we ate all space already...
		if (mStream.avail_out == 0) {
			inSize -= mStream.avail_in;
			mTotalIn += inSize;
			return true;
		}
	} else {
		mStream.avail_in = inSize;
		mStream.avail_out = outSize;
	}

	int err = ::deflate(&mStream, inSize == 0 ? Z_FINISH : Z_NO_FLUSH);
	if (err != Z_OK && (inSize != 0 || err != Z_STREAM_END)) {
		return false;
	}
	inSize -= mStream.avail_in;
	mTotalIn += inSize;
	outSize -= mStream.avail_out;
	mTotalOut += outSize;
	return true;
}


bool ZlibFilter::compressFull(const char * in, size_t & inSize, char * out, size_t & outSize) {
	if (inSize == 0 || outSize == 0) {
		return false;
	}

	z_stream stream;
	memset(&stream, 0 , sizeof (stream));
	deflateInit(&stream, Z_BEST_COMPRESSION);

	stream.next_in = const_cast<Bytef *> (reinterpret_cast<const Bytef *> (in));
	stream.avail_in = inSize;
	stream.next_out = reinterpret_cast<Bytef *> (out);
	stream.avail_out = outSize;

	int err = ::deflate(&stream, Z_FINISH);
	if (err != Z_STREAM_END) {
		deflateEnd(&stream);
		return false;
	}

	inSize -= stream.avail_in;
	outSize -= stream.avail_out;
	deflateEnd(&stream);
	return true;
}



bool ZlibFilter::compressFull(const char * in, size_t inSize, string & out) {
	if (inSize == 0) {
		return false;
	}

	char outBuf[BUF_SIZE] = { 0 };
	size_t i = 0;
	ZlibFilter filter;

	while (i <= inSize) {
		size_t outSize = BUF_SIZE, j = inSize - i;
		if (!filter.compress(in + i, j, outBuf, outSize)) {
			filter.finish();
			return false;
		}
		out.append(outBuf, outSize);
		i += (j != 0 ? j : 1);
	}
	filter.finish();
	return true;
}


} // namespace utils

#endif // HAVE_LIBZLIB

/**
 * $Id$
 * $HeadURL$
 */
