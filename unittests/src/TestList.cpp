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

#if (defined _DEBUG) && (!defined UNITTESTS_OFF)


#include "UnitTest++.h"
#include "List.h"


using namespace luaplugin;


SUITE(TestList) {


TEST(TestList_add) {

	List<int> list;

	list.add(5);
	list.add(7);
	list.add(5);

	CHECK_EQUAL(3, list.size());
}


TEST(TestList_clear) {
	
	List<int> list;

	list.add(1);

	list.clear();

	CHECK_EQUAL(0, list.size());
}



bool func(int i) {
	return i == 1;
}

void func1(int) {

}

TEST(TestList_removeIf) {

	List<int> list;

	list.add(1);
	list.add(2);
	list.add(1);

	list.removeIf(func);
	list.loop(func1);

	CHECK_EQUAL(1, list.size());
}


}; // SUITE(TestList)

#endif // _DEBUG

/**
 * $Id$
 * $HeadURL$
 */
