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

#ifndef CTIMEOUT_H
#define CTIMEOUT_H

#include "ctime.h"

namespace nUtils {

/** Controller with minimum and maximum delay */
class cTimeOut {

private:
	cTime mMinDelay; /** Minimum delay between events */
	cTime mMaxDelay; /** Maximum delay between events */
	cTime mLast; /** Time of the last event */

public: 
	cTimeOut() : mMinDelay(0l), mMaxDelay(0l), mLast(0l) {}
	cTimeOut(double iMin, double iMax, const cTime &now) : mMinDelay(iMin), mMaxDelay(iMax), mLast(now) {}
	~cTimeOut(){}

	inline void SetMinDelay(double iMin) { mMinDelay = iMin; }
	inline void SetMaxDelay(double iMax) { mMaxDelay = iMax; }

	/** Set time of the last event */
	inline void Reset(const cTime & now) { mLast = now; }
	inline void Disable() { mLast = 0.; }

	/** Check enent
	0 - ok (event is absent or in interval);
	-1 - event not in interval (< min);
	-2 - event was beyond the scope of interval (> max).
	Flag bEvent checks lower verge and reset timer */
	int Check(const cTime &now, bool bEvent = 0) {
		if(!bool(mLast)) return 0;
		cTime diff(now);
		diff -= mLast;
		if(bEvent && (bool)mMinDelay && (mMinDelay > diff)) return -1;
		if(bool(mMaxDelay) && (mMaxDelay < diff)) return -2;
		if(bEvent) Reset(now);
		return 0;
	}

}; // cTimeOut

}; // nUtils

#endif // CTIMEOUT_H
