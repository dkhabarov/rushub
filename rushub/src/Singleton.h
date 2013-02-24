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

#ifndef SINGLETON_H
#define SINGLETON_H

#include <string>

namespace utils {

/**
 * Singleton
 */
template <class T>
class Singleton {

public:

	static T * getInstance() {
		if (!mSelf) {
			mSelf = new T;
		}
		return mSelf;
	}

	void freeInstance() {
		if (mSelf) {
			delete this;
			mSelf = NULL;
		}
	}

protected:

	Singleton() {
	}

	virtual ~Singleton() {
		mSelf = NULL;
	}

private:

	Singleton(const Singleton &);
	Singleton & operator = (const Singleton &);

private:

	static T * mSelf;

}; // class Singleton

template <class T>
T * Singleton<T>::mSelf = NULL;

} // namespace utils

#endif // SINGLETON_H

/**
 * $Id$
 * $HeadURL$
 */
