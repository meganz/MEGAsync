/**
 * @file share.cpp
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

#include "mega/share.h"

namespace mega {

Share::Share(User* u, accesslevel_t a, time_t t)
{
	user = u;
	access = a;
	ts = t;
}

void Share::serialize(string* d)
{
	handle uh = user ? user->userhandle : 0;
	char a = (char)access;

	d->append((char*)&uh,sizeof uh);
	d->append((char*)&ts,sizeof ts);
	d->append((char*)&a,1);
	d->append("",1);
}

bool Share::unserialize(MegaClient* client, int direction, handle h, const byte* key, const char** ptr, const char* end)
{
	if (*ptr+sizeof(handle)+sizeof(time_t)+2 > end) return 0;

	client->newshares.push_back(new NewShare(h,direction,*(handle*)*ptr,(accesslevel_t)(*ptr)[sizeof(handle)+sizeof(time_t)],*(time_t*)(*ptr+sizeof(handle)),key));

	*ptr += sizeof(handle)+sizeof(time_t)+2;

	return true;
}

void Share::update(accesslevel_t a, time_t t)
{
	access = a;
	ts = t;
}

// coutgoing: < 0 - don't authenticate, > 0 - authenticate using handle auth
NewShare::NewShare(handle ch, int coutgoing, handle cpeer, accesslevel_t caccess, time_t cts, const byte* ckey, const byte* cauth)
{
	h = ch;
	outgoing = coutgoing;
	peer = cpeer;
	access = caccess;
	ts = cts;

	if (ckey)
	{
		memcpy(key,ckey,sizeof key);
		have_key = 1;
	}
	else have_key = 0;

	if (outgoing > 0 && cauth)
	{
		memcpy(auth,cauth,sizeof auth);
		have_auth = 1;
	}
	else have_auth = 0;
}

} // namespace
