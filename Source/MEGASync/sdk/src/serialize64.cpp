/**
 * @file serialize64.cpp
 * @brief 64-bit int serialization/unserialization
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

#include "mega/serialize64.h"

namespace mega {
int Serialize64::serialize(byte* b, uint64_t v)
{
    int p = 0;

    while (v)
    {
        b[++p] = (byte)v;
        v >>= 8;
    }

    return (*b = p) + 1;
}

int Serialize64::unserialize(byte* b, int blen, uint64_t* v)
{
    byte p = *b;

    if ((p > sizeof(*v)) || (p >= blen))
    {
        return -1;
    }

    *v = 0;

    while (p)
    {
        *v = (*v << 8) + b[(int)p--];
    }

    return *b + 1;
}
} // namespace
