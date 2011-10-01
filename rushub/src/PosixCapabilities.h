/*
 * RusHub - hub server for Direct Connect peer to peer network.
 * 
 * PosixCapabilities headers for RusHub
 * E-Mail: admin at klan-hub dot ru (admin@klan-hub.ru)
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

#ifndef POSIX_CAPABILITIES_H
#define POSIX_CAPABILITIES_H

#include <grp.h>
#include <pwd.h>
#include <stdarg.h>
#include <sys/capability.h>
#include <sys/prctl.h>
#include <iostream>
#include <string>

namespace utils {

using namespace std;

bool setcapabilities(string mUser, string mGroup);

} // closing namespace utils
#endif //POSIX_CAPABILITIES_H
