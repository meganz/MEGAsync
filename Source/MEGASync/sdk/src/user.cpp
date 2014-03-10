/**
 * @file user.cpp
 * @brief Class for manipulating user / contact data
 *
 * (c) 2013-2014 by Mega Limited, Wellsford, New Zealand
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

#include "mega/user.h"
#include "mega/megaclient.h"

namespace mega {
User::User(const char* cemail)
{
    userhandle = UNDEF;
    show = VISIBILITY_UNKNOWN;
    ctime = 0;
    pubkrequested = 0;

    if (cemail)
    {
        email = cemail;
    }
}

bool User::serialize(string* d)
{
    unsigned char l;
    attr_map::iterator it;

    d->reserve(d->size() + 100 + attrs.storagesize(10));

    d->append((char*)&userhandle, sizeof userhandle);
    d->append((char*)&ctime, sizeof ctime);
    d->append((char*)&show, sizeof show);

    l = email.size();
    d->append((char*)&l, sizeof l);
    d->append(email.c_str(), l);

    d->append("\0\0\0\0\0\0\0", 8);

    attrs.serialize(d);

    if (pubk.isvalid())
    {
        pubk.serializekey(d, AsymmCipher::PUBKEY);
    }

    return true;
}

User* User::unserialize(MegaClient* client, string* d)
{
    handle uh;
    time_t ts;
    visibility_t v;
    unsigned char l;
    string m;
    User* u;
    const char* ptr = d->data();
    const char* end = ptr + d->size();
    int i;

    if (ptr + sizeof(handle) + sizeof(time_t) + sizeof(visibility_t) + 2 > end)
    {
        return NULL;
    }

    uh = MemAccess::get<handle>(ptr);
    ptr += sizeof uh;

    ts = MemAccess::get<time_t>(ptr);
    ptr += sizeof ts;

    v = MemAccess::get<visibility_t>(ptr);
    ptr += sizeof v;

    l = *ptr++;
    if (l)
    {
        m.assign(ptr, l);
    }
    ptr += l;

    for (i = 8; i--;)
    {
        if (ptr + MemAccess::get<unsigned char>(ptr) < end)
        {
            ptr += MemAccess::get<unsigned char>(ptr) + 1;
        }
    }

    if ((i >= 0) || !(u = client->finduser(uh, 1)))
    {
        return NULL;
    }

    if (v == ME)
    {
        client->me = uh;
    }

    client->mapuser(uh, m.c_str());
    u->set(v, ts);

    if ((ptr < end) && !(ptr = u->attrs.unserialize(ptr, end - ptr)))
    {
        return NULL;
    }

    if ((ptr < end) && !u->pubk.setkey(AsymmCipher::PUBKEY, (byte*)ptr, end - ptr))
    {
        return NULL;
    }

    return u;
}

// update user attributes
void User::set(visibility_t v, time_t ct)
{
    show = v;
    ctime = ct;
}
} // namespace
