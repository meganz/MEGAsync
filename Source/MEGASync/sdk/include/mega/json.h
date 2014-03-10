/**
 * @file mega/json.h
 * @brief Linear non-strict JSON scanner
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

#ifndef MEGA_JSON_H
#define MEGA_JSON_H 1

#include "types.h"

namespace mega {

// linear non-strict JSON scanner
struct MEGA_API JSON
{
    const char* pos;	// make private

    bool isnumeric();

    void begin(const char*);

    m_off_t getint();
    double getfloat();
    const char* getvalue();

    nameid getnameid();
    nameid getnameid(const char*);

    bool is(const char*);

    int storebinary(byte*, int);
    bool storebinary(string*);

    handle gethandle(int = 6);

    bool enterarray();
    bool leavearray();

    bool enterobject();
    bool leaveobject();

    bool storestring(string*);
    bool storeobject(string* = NULL);

    static void unescape(string*);
};

} // namespace

#endif
