/**
 * @file mega/share.h
 * @brief Classes for manipulating share credentials
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

#ifndef MEGA_SHARE_H
#define MEGA_SHARE_H 1

#include "types.h"
#include "user.h"
#include "megaclient.h"

namespace mega {

// share credentials
struct Share
{
	accesslevel access;
	User* user;
	time_t ts;

	void removeshare(handle);
	void update(accesslevel, time_t);

	void serialize(string*);
	static bool unserialize(MegaClient*, int, handle, const byte*, const char**, const char*);

	Share(User*, accesslevel, time_t);
};

// new share credentials (will be merged into node as soon as it appears)
struct NewShare
{
	handle h;
	int outgoing;
	handle peer;
	accesslevel access;
	time_t ts;

	bool have_key, have_auth;

	byte key[SymmCipher::BLOCKSIZE];
	byte auth[SymmCipher::BLOCKSIZE];

	NewShare(handle, int, handle, accesslevel, time_t, const byte*, const byte* = NULL);
};

} // namespace

#endif
