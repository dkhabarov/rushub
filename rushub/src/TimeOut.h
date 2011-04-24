/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz

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

#ifndef TIMEOUT_H
#define TIMEOUT_H

#include "Times.h"

namespace utils {



/** Controller with minimum and maximum delay */
class TimeOut {

public: 

	TimeOut() : mMinDelay(0l), mMaxDelay(0l), mLast(0l) {
	}

	TimeOut(double min, double max, const Time & now) : mMinDelay(min), mMaxDelay(max), mLast(now) {
	}

	~TimeOut() {
	}

	inline void setMinDelay(double min) {
		mMinDelay = min;
	}

	inline void setMaxDelay(double max) {
		mMaxDelay = max;
	}

	/** Set time of the last event */
	inline void reset(const Time & now) {
		mLast = now;
	}

	inline void disable() {
		mLast = 0.;
	}

	/** Check enent
	0 - ok (event is absent or in interval);
	-1 - event not in interval (< min);
	-2 - event was beyond the scope of interval (> max).
	Flag event checks lower verge and reset timer */
	int check(const Time & now, bool event = false) {
		if (!bool(mLast)) {
			return 0;
		}
		Time diff(now);
		diff -= mLast;
		if (event && (bool)mMinDelay && (mMinDelay > diff)) {
			return -1;
		}
		if (bool(mMaxDelay) && (mMaxDelay < diff)) {
			return -2;
		}
		if (event) {
			reset(now);
		}
		return 0;
	}

private:

	/** Minimum delay between events */
	Time mMinDelay;

	/** Maximum delay between events */
	Time mMaxDelay;

	/** Time of the last event */
	Time mLast;

}; // class TimeOut

}; // namespace utils

#endif // TIMEOUT_H
