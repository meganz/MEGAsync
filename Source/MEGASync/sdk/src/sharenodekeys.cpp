/**
 * @file sharenodekeys.cpp
 * @brief cr element share/node map key generator
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

#include "mega/sharenodekeys.h"
#include "mega/node.h"
#include "mega/base64.h"
#include "mega/megaclient.h"
#include "mega/command.h"

namespace mega {
// add share node and return its index
int ShareNodeKeys::addshare(Node* sn)
{
    for (int i = shares.size(); i--;)
    {
        if (shares[i] == sn)
        {
            return i;
        }
    }

    shares.push_back(sn);

    return shares.size() - 1;
}

void ShareNodeKeys::add(Node* n, Node* sn, int specific)
{
    if (!sn)
    {
        sn = n;
    }

    add((NodeCore*)n, sn, specific);
}

// add a nodecore (!sn: all relevant shares, otherwise starting from sn, fixed: only sn)
void ShareNodeKeys::add(NodeCore* n, Node* sn, int specific, const byte* item, int itemlen)
{
    char buf[96];
    char* ptr;
    byte key[FILENODEKEYLENGTH];

    int addnode = 0;

    // emit all share nodekeys for known shares
    do
    {
        if (sn->sharekey)
        {
            sprintf(buf, ",%d,%d,\"", addshare(sn), (int)items.size());

            sn->sharekey->ecb_encrypt((byte*)n->nodekey.data(), key, n->nodekey.size());

            ptr = strchr(buf + 5, 0);
            ptr += Base64::btoa(key, n->nodekey.size(), ptr);
            *ptr++ = '"';

            keys.append(buf, ptr - buf);
            addnode = 1;
        }
    }
    while (!specific && (sn = sn->parent));

    if (addnode)
    {
        items.resize(items.size() + 1);

        if (item)
        {
            items[items.size() - 1].assign((const char*)item, itemlen);
        }
        else
        {
            items[items.size() - 1].assign((const char*)&n->nodehandle, MegaClient::NODEHANDLE);
        }
    }
}

void ShareNodeKeys::get(Command* c)
{
    if (keys.size())
    {
        c->beginarray("cr");

        // emit share node handles
        c->beginarray();
        for (unsigned i = 0; i < shares.size(); i++)
        {
            c->element((const byte*)&shares[i]->nodehandle, MegaClient::NODEHANDLE);
        }

        c->endarray();

        // emit item handles (can be node handles or upload tokens)
        c->beginarray();
        for (unsigned i = 0; i < items.size(); i++)
        {
            c->element((const byte*)items[i].c_str(), items[i].size());
        }

        c->endarray();

        // emit linkage/keys
        c->beginarray();
        c->appendraw(keys.c_str() + 1, keys.size() - 1);
        c->endarray();

        c->endarray();
    }
}
} // namespace
