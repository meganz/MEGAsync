/**
 * @file mega/utils.h
 * @brief Mega SDK various utilities and helper classes
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

#ifndef MEGA_UTILS_H
#define MEGA_UTILS_H 1

#include "types.h"

namespace mega {
// convert 2...8 character ID to int64 integer (endian agnostic)
#define MAKENAMEID2(a, b) (nameid)(((a) << 8) + (b))
#define MAKENAMEID3(a, b, c) (nameid)(((a) << 16) + ((b) << 8) + (c))
#define MAKENAMEID4(a, b, c, d) (nameid)(((a) << 24) + ((b) << 16) + ((c) << 8) + (d))
#define MAKENAMEID5(a, b, c, d, e) (nameid)((((uint64_t)a) << 32) + ((b) << 24) + ((c) << 16) + ((d) << 8) + (e))
#define MAKENAMEID6(a, b, c, d, e, f) (nameid)((((uint64_t)a) << 40) + (((uint64_t)b) << 32) + ((c) << 24) + ((d) << 16) + ((e) << 8) + (f))
#define MAKENAMEID7(a, b, c, d, e, f, g) (nameid)((((uint64_t)a) << 48) + (((uint64_t)b) << 40) + (((uint64_t)c) << 32) + ((d) << 24) + ((e) << 16) + ((f) << 8) + (g))
#define MAKENAMEID8(a, b, c, d, e, f, g, h) (nameid)((((uint64_t)a) << 56) + (((uint64_t)b) << 48) + (((uint64_t)c) << 40) + (((uint64_t)d) << 32) + ((e) << 24) + ((f) << 16) + ((g) << 8) + (h))

struct MEGA_API ChunkedHash
{
    static const int SEGSIZE = 131072;

    static m_off_t chunkfloor(m_off_t);
    static m_off_t chunkceil(m_off_t);
};

// padded CBC encryption
struct MEGA_API PaddedCBC
{
    static void encrypt(string*, SymmCipher*);
    static bool decrypt(string*, SymmCipher*);
};

class MEGA_API HashSignature
{
    Hash* hash;

public:
    // add data
    void add(const byte*, unsigned);

    // generate signature
    unsigned get(AsymmCipher*, byte*, unsigned);

    // verify signature
    int check(AsymmCipher*, const byte*, unsigned);

    HashSignature(Hash*);
    ~HashSignature();
};

// read/write multibyte words
struct MEGA_API MemAccess
{
#ifdef NO_DIRECT_WORD_ACCESS
    template<typename T> static T get(const char* ptr)
    {
        T val;
        memcpy(&val,ptr,sizeof(T));
        return val;
    }

    template<typename T> static void set(const byte* ptr, T val)
    {
        memcpy(ptr,&val,sizeof val);
    }
#else
    template<typename T> static T get(const char* ptr)
    {
        return *(T*)ptr;
    }

    template<typename T> static void set(const byte* ptr, T val)
    {
        *(T*)ptr = val;
    }
#endif
};
} // namespace

#endif
