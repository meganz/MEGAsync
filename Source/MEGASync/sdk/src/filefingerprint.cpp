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
	if (lhs.size != rhs.size) return false;

	// mtime differs - cannot be equal
	if (lhs.mtime != rhs.mtime) return false;

	// FileFingerprints not fully available - give it the benefit of the doubt
	if (!lhs.isvalid || !rhs.isvalid) return true;

	return !memcmp(lhs.crc,rhs.crc,sizeof lhs.crc);
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
	memcpy(crc,rhs.crc,sizeof crc);

	return *this;
}

bool FileFingerprint::genfingerprint(FileAccess* fa)
{
	bool changed = false;

	if (mtime != fa->mtime)
	{
		mtime = fa->mtime;
		changed = true;
	}

	if (size != fa->size)
	{
		size = fa->size;
		changed = true;
	}

	if (size <= (m_off_t)sizeof crc)
	{
		// tiny file: just read, NUL pad
		fa->frawread(crc,size,0);
		memset(crc+size,0,sizeof crc-size);
	}
	else if (size <= (m_off_t)(sizeof crc*sizeof crc))
	{
		// small file: read byte pattern, no CRC
		for (unsigned i = 0; i < sizeof crc; i++) fa->frawread(crc+i,1,i*(size-1)/(sizeof crc-1));
	}
	else
	{
		byte newcrc[sizeof crc];

		// larger file: parallel sparse CRC block pattern
		byte block[4*sizeof crc];
		unsigned blocks = size/(sizeof crc*sizeof crc);

		if (blocks > 32) blocks = 32;

		for (unsigned i = 0; i < sizeof crc/4; i++)
		{
			HashCRC32 crc32;

			for (unsigned j = 0; j < blocks; j++)
			{
				if (!fa->frawread(block,sizeof block,(size-sizeof block)*(i*blocks+j)/(sizeof crc/4*blocks-1)))
				{
					size = -1;
					return true;
				}

				crc32.add(block,sizeof block);
			}

			crc32.get(newcrc+4*i);
		}

		if (memcmp(crc,newcrc,sizeof crc))
		{
			memcpy(crc,newcrc,sizeof crc);
			changed = true;
		}
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
	byte buf[sizeof crc+1+sizeof mtime];
	int l;

	memcpy(buf,crc,sizeof crc);
	l = Serialize64::serialize(buf+sizeof crc,mtime);

	d->resize((sizeof crc+l)*4/3+4);
	d->resize(Base64::btoa(buf,sizeof crc+l,(char*)d->c_str()));
}

// decode and set base64-encoded fingerprint
int FileFingerprint::unserializefingerprint(string* d)
{
	byte buf[sizeof crc+sizeof mtime+1];
	unsigned l;
	int64_t t;

	if ((l = Base64::atob(d->c_str(),buf,sizeof buf)) < sizeof crc+1) return 0;
	if (Serialize64::unserialize(buf+sizeof crc,l-sizeof crc,&t) < 0) return 0;

	memcpy(crc,buf,sizeof crc);

	mtime = t;

	isvalid = true;

	return 1;
}

bool FileFingerprintCmp::operator() (const FileFingerprint* a, const FileFingerprint* b) const
{
	if (a->size < b->size) return true;
	if (a->size > b->size) return false;
	if (a->mtime < b->mtime) return true;
	if (a->mtime > b->mtime) return false;
	return memcmp(a->crc,b->crc,sizeof a->crc) < 0;
}

} // namespace
