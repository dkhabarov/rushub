/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * PosixCapabilities for RusHub
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

#include "PosixCapabilities.h"

namespace utils {

using namespace std;

bool setcapabilities(string mUser, string mGroup) {

	cap_t caps;
	cap_flag_value_t nbs_flag;
	struct passwd *user = NULL;
	
	user = getpwnam(mUser.c_str());

	if(user) {
		// Keep capabilities across UID change
		if(prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0) != 0) {
			////log << "prctl(PR_SET_KEEPCAPS) failed." << endl;
			return false;
		}

		// Change supplementary groups
		if(initgroups(user->pw_name, user->pw_gid) != 0) {
			//log << "initgroups() for user " <<  user->pw_name << " failed."<< endl;
			return false;
		}

		// Change GID
		if(setgid(user->pw_gid) != 0) {
			//log << "Cannot set GID to " << user->pw_gid << endl;
			return false;
		}

		// Change UID
		if(setuid(user->pw_uid) != 0) {
			//log << "Cannot set UID to " << user->pw_uid << endl;
			return false;
		}
	}
	// Check capability to bind privileged ports
	caps = cap_get_proc();
	if(!caps) {
		//log << "cap_get_proc() failed to get capabilities." << endl;
		return false;
	}
	if(cap_get_flag(caps, CAP_NET_BIND_SERVICE, CAP_PERMITTED, &nbs_flag) != 0) {
		cap_free(caps);
		//log << "cap_get_flag() failed to get CAP_NET_BIND_SERVICE state." << endl;
		return false;
	}
	cap_free(caps);

	// Drop all capabilities except privileged ports binding
	caps = cap_from_text(nbs_flag == CAP_SET ? "cap_net_bind_service=ep" : "=");
	if(!caps) {
		//log << "cap_from_text() failed to parse capabilities string!\n" << endl;
		return false;
	}
	if(cap_set_proc(caps) != 0) {
		cap_free(caps);
		//log << "cap_set_proc() failed to set capabilities." << endl;
		return false;
	}
	cap_free(caps);
	return true;
}

} // closing namespace utils
