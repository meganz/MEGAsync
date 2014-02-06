/**
 * @file filefingerprint.cpp
 * @brief Sparse file fingerprint
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

#include "mega/filefingerprint.h"
#include "mega/serialize64.h"
#include "mega/base64.h"

namespace mega {
bool operator==(FileFingerprint& lhs, FileFingerprint& rhs)
{
    // size differs - cannot be equal
    if (lhs.size != rhs.size)
    {
        return false;
    }

    // mtime differs - cannot be equal
    if (lhs.mtime != rhs.mtime)
    {
        return false;
    }

    // FileFingerprints not fully available - give it the benefit of the doubt
    if (!lhs.isvalid || !rhs.isvalid)
    {
        return true;
    }

    return !memcmp(lhs.crc, rhs.crc, sizeof lhs.crc);
}

FileFingerprint::FileFingerprint()
{
    // mark as invalid
    size = -1;
    mtime = 0;
    isvalid = false;
}

FileFingerprint& FileFingerprint::operator=(FileFingerprint& rhs)
{
    isvalid = rhs.isvalid;
    size = rhs.size;
    mtime = rhs.mtime;
    memcpy(crc, rhs.crc, sizeof crc);

    return *this;
}

bool FileFingerprint::genfingerprint(FileAccess* fa, bool ignoremtime)
{
    bool changed = false;
    int32_t newcrc[sizeof crc / sizeof *crc], crcval;

    if (mtime != fa->mtime)
    {
        mtime = fa->mtime;
        changed = !ignoremtime;
    }

    if (size != fa->size)
    {
        size = fa->size;
        changed = true;
    }

    if (size <= (m_off_t)sizeof crc)
    {
        // tiny file: read verbatim, NUL pad
        fa->frawread((byte*)newcrc, size, 0);
        memset((byte*)newcrc + size, 0, sizeof crc - size);
    }
    else if (size <= MAXFULL)
    {
        // small file: full coverage, four full CRC32s
        HashCRC32 crc32;
        byte buf[MAXFULL];

        if (!fa->frawread(buf, size, 0))
        {
            size = -1;
            return true;
        }

        for (unsigned i = 0; i < sizeof crc / sizeof *crc; i++)
        {
            int begin = i * size / ( sizeof crc / sizeof *crc );
            int end = ( i + 1 ) * size / ( sizeof crc / sizeof *crc );

            crc32.add(buf + begin, end - begin);
            crc32.get((byte*)&crcval);
            newcrc[i] = htonl(crcval);
        }
    }
    else
    {
        // large file: sparse coverage, four sparse CRC32s
        HashCRC32 crc32;
        byte block[4 * sizeof crc];
        const unsigned blocks = MAXFULL / ( sizeof block * sizeof crc / sizeof *crc );

        for (unsigned i = 0; i < sizeof crc / sizeof *crc; i++)
        {
            for (unsigned j = 0; j < blocks; j++)
            {
                if (!fa->frawread(block, sizeof block,
                                  ( size - sizeof block )
                                  * ( i * blocks + j )
                                  / ( sizeof crc / sizeof *crc * blocks - 1 )))
                {
                    size = -1;
                    return true;
                }

                crc32.add(block, sizeof block);
            }

            crc32.get((byte*)&crcval);
            newcrc[i] = htonl(crcval);
        }
    }

    if (memcmp(crc, newcrc, sizeof crc))
    {
        memcpy(crc, newcrc, sizeof crc);
        changed = true;
    }

    if (!isvalid)
    {
        isvalid = true;
        changed = true;
    }

    return changed;
}

// convert this FileFingerprint to string
void FileFingerprint::serializefingerprint(string* d)
{
    byte buf[sizeof crc + 1 + sizeof mtime];
    int l;

        memcpy(buf, crc, sizeof crc);
    l = Serialize64::serialize(buf + sizeof crc, mtime);

    d->resize(( sizeof crc + l ) * 4 / 3 + 4);
    d->resize(Base64::btoa(buf, sizeof crc + l, (char*)d->c_str()));
}

// decode and set base64-encoded fingerprint
int FileFingerprint::unserializefingerprint(string* d)
{
    byte buf[sizeof crc + sizeof mtime + 1];
    unsigned l;
    int64_t t;

    if (( l = Base64::atob(d->c_str(), buf, sizeof buf)) < sizeof crc + 1)
    {
        return 0;
    }
    if (Serialize64::unserialize(buf + sizeof crc, l - sizeof crc, &t) < 0)
    {
        return 0;
    }

        memcpy(crc, buf, sizeof crc);

    mtime = t;

    isvalid = true;

    return 1;
}

bool FileFingerprintCmp::operator()(const FileFingerprint* a, const FileFingerprint* b) const
{
    if (a->size < b->size)
    {
        return true;
    }
    if (a->size > b->size)
    {
        return false;
    }
    if (a->mtime < b->mtime)
    {
        return true;
    }
    if (a->mtime > b->mtime)
    {
        return false;
    }
    return memcmp(a->crc, b->crc, sizeof a->crc) < 0;
}
} // namespace
