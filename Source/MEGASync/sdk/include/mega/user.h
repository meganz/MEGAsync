/**
 * @file mega/user.h
 * @brief Class for manipulating user / contact data
 *
 * (c) 2013 by Mega Limited, Wellsford, New Zealand
 *
 * This file is part of the MEGA SDK - Client Access Engine.
 *
 * Applications using the MEGA API must present a valid application key
 * and comply with the the rules set forth in the Terms of Service.
 *
 * The MEGA SDK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * @copyright Simplified (2-clause) BSD License.
 *
 * You should have received a copy of the license along with this
 * program.
 */

#ifndef MEGA_USER_H
#define MEGA_USER_H 1

#include "attrmap.h"

namespace mega {

// user/contact
struct User : public Cachable
{
	// user handle
	handle userhandle;

	// string identifier for API requests (either e-mail address or ASCII user handle)
	string uid;

	// e-mail address
	string email;

	// persistent attributes (n = name, a = avatar)
	AttrMap attrs;

	// visibility status
	visibility show;

	// shares by this user
	handle_set sharing;

	// contact establishment timestamp
	time_t ctime;

	// user's public key
	AsymmCipher pubk;
	int pubkrequested;

	// actions to take after arrival of the public key
	deque<class PubKeyAction*> pkrs;

	void set(visibility, time_t);

	bool serialize(string*);
	static User* unserialize(class MegaClient*, string*);

	User(const char* = NULL);
};

} // namespace

#endif
