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

#ifndef ANY_H
#define ANY_H

namespace utils {

/// Class Any
class Any {

public:

	Any() : mObject(NULL)	{
	}

	~Any() {
		if (mObject) {
			delete mObject;
		}
	}

	template <class T>
	Any & operator = (T const & value) {
		if (mObject) {
			delete mObject;
		}
		mObject = new Type<T>(value);
		return *this;
	}

	template <class T>
	Any & operator = (T * value) {
		if (mObject) {
			delete mObject;
		}
		mObject = new Type<T>(value);
		return *this;
	}

	template <class T>
	operator const T & () const {
		if (!mObject) {
			throw "never";
		}
		return dynamic_cast<Type<T> &> (*mObject).get();
	}

	friend ostream & operator << (ostream & os, const Any & any) {
		if (any.mObject) {
			any.mObject->toStream(os);
		}
		return os;
	}

private:

	class TypeBase {
	public:
		virtual ~TypeBase() {
		}
		virtual void toStream(ostream & os) = 0;
	};

	template <class T>
	class Type : public TypeBase {
	public:
		Type(T const & t) : mObject(t), mRef(NULL) {
		}
		Type(T * t) : mRef(t) {
		}
		const T & get() const {
			return mRef ? *mRef : mObject;
		}
		void toStream(ostream & os) {
			os << (mRef ? *mRef : mObject);
		}
	private:
		T mObject;
		T const * mRef;

		Type(const Type<T> &);
		Type<T> & operator = (const Type<T> &);
	};

	Any(const Any &);
	Any & operator = (const Any &) {
		return *this;
	}

private:

	TypeBase * mObject;

}; // class Any

} // namespace utils

#endif // ANY_H

/**
 * $Id$
 * $HeadURL$
 */
