/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * Copyright (C) 2009-2010 by Setuper
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

#include "ctime.h"
#include <sstream>
#if HAVE_STRING_H
	#include <string.h>
#endif

using namespace std;

#ifdef _WIN32
	#ifndef __int64
		#define __int64 long long
	#endif
	void gettimeofday(struct timeval *tv, struct timezone *tz) {
		union {
			FILETIME ft;
			unsigned __int64 ns100;
		} now;
		GetSystemTimeAsFileTime(&now.ft);
		//116444736000000000 = (24 * 3600) * ((1970 - 1601) * 365 + 89) * (1000000000 / 100)
		tv->tv_sec = (long) ((now.ns100 - 116444736000000000LL) / 10000000LL);
		tv->tv_usec = (long) ((now.ns100 / 10LL) % 1000000LL);
	}
#else
	#include <cstring> // for strlen
#endif // _WIN32

namespace nUtils {

string cTime::AsString() const {
	ostringstream os;
	os << (*this);
	return os.str();
}

void cTime::AsTimeVals(int &w, int &d, int &h, int &m) const {
	long rest = tv_sec;
	w = rest / (24 * 3600 * 7);
	rest %= (24 * 3600 * 7);
	d = rest / (24 * 3600);
	rest %= (24 * 3600);
	h = rest / 3600;
	rest %= 3600;
	m = rest / 60;
}

cTime & cTime::Get() {
	gettimeofday(this, NULL);
	return *this;
}

cTime & cTime::Normalize() {
	if(tv_usec >= 1000000 || tv_usec <= -1000000) {
		tv_sec += tv_usec/1000000;
		tv_usec %= 1000000;
	}
	if( tv_sec < 0 && tv_usec > 0) {
		++tv_sec;
		tv_usec -= 1000000;
	}
	if( tv_sec > 0 && tv_usec < 0) {
		--tv_sec;
		tv_usec += 1000000;
	}
	return *this;
}

std::ostream &operator << (std::ostream &os, const cTime &t) {
	#if !defined(_WIN32)
		char * buf;
		static char buff[26];
	#elif defined(_WIN32) && !defined(_MSC_VER) || (_MSC_VER < 1400)
		char * buf;
	#else
		static char buf[26];
	#endif
	long n, rest;

	switch (t.mPrintType) {
	case 4: /** AsDateMS */
	case 1: /** AsDate */
		#ifndef _WIN32
			const time_t * ta;
			ta = (time_t*)&t.tv_sec;
			buf = ctime_r(ta, buff);
		#else
			time_t ta;
			ta = (time_t)t.tv_sec;
			#if defined(_MSC_VER) && (_MSC_VER >= 1400)
				ctime_s(buf, 26, &ta);
			#else
				buf = ctime(&ta);
			#endif
		#endif
		buf[strlen(buf) - 1] = 0;
		os << buf;
		if(t.mPrintType == 4)
			os << "|" << t.tv_usec / 1000;
		break;
	case 2: /** AsPeriod */
		rest = t.tv_sec;
		os << rest << " sec ";
		os << t.tv_usec / 1000 << " ms ";
		os << t.tv_usec % 1000 << " µs ";
		break;
	case 3: /** AsFullPeriod */
		rest = t.tv_sec;

		n = rest / (24 * 3600 * 7);
		rest %= (24 * 3600 * 7);
		if(n) os << n << " weeks ";

		n = rest / (24 * 3600);
		rest %= (24 * 3600);
		if(n) os << n << " days ";

		n = rest / 3600;
		rest %= 3600;
		if(n) os << n << " hours ";

		n = rest / 60;
		if(n) os << n << " min ";
		rest %= 60;
		os << rest << " sec";
		break;
	default: /** AsString */
		os << t.tv_sec << " s " << t.tv_usec << " µs";
		break;
	}
	return os;
}

}; /** nUtils */
