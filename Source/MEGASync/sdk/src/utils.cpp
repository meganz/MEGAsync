/**
 * @file utils.cpp
 * @brief Mega SDK various utilities and helper classes
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

#include "mega/utils.h"

namespace mega {

Cachable::Cachable()
{
	dbid = 0;
	notified = 0;
}

// pad and CBC-encrypt
void PaddedCBC::encrypt(string* data, SymmCipher* key)
{
	// pad to blocksize and encrypt
	data->append("E");
	data->resize((data->size()+key->BLOCKSIZE-1)&-key->BLOCKSIZE,'P');
	key->cbc_encrypt((byte*)data->data(),data->size());
}

// CBC-decrypt and unpad
bool PaddedCBC::decrypt(string* data, SymmCipher* key)
{
	if ((data->size() & (key->BLOCKSIZE-1))) return false;

	// decrypt and unpad
	key->cbc_decrypt((byte*)data->data(),data->size());

	size_t p = data->find_last_of('E');

	if (p == string::npos) return false;

	data->resize(p);

	return true;
}

// start of chunk
m_off_t ChunkedHash::chunkfloor(m_off_t p)
{
	m_off_t cp, np;

	cp = 0;

	for (unsigned i = 1; i <= 8; i++)
	{
		np = cp+i*SEGSIZE;
		if (p >= cp && p < np) return cp;
		cp = np;
	}

	return ((p-cp)&-(8*SEGSIZE))+cp;
}

// end of chunk (== start of next chunk)
m_off_t ChunkedHash::chunkceil(m_off_t p)
{
	m_off_t cp, np;

	cp = 0;

	for (unsigned i = 1; i <= 8; i++)
	{
		np = cp+i*SEGSIZE;
		if (p >= cp && p < np) return np;
		cp = np;
	}

	return ((p-cp)&-(8*SEGSIZE))+cp+8*SEGSIZE;
}


// cryptographic signature generation/verification
HashSignature::HashSignature(Hash* h)
{
	hash = h;
}

HashSignature::~HashSignature()
{
	delete hash;
}

void HashSignature::add(const byte* data, unsigned len)
{
	hash->add(data,len);
}

unsigned HashSignature::get(AsymmCipher* privk, byte* sigbuf, unsigned sigbuflen)
{
	string h;

	hash->get(&h);

	return privk->rawdecrypt((const byte*)h.data(),h.size(),sigbuf,sigbuflen);
}

int HashSignature::check(AsymmCipher* pubk, const byte* sig, unsigned len)
{
	string h, s;

	hash->get(&h);

	s.resize(h.size());

	if (pubk->rawencrypt(sig,len,(byte*)s.data(),s.size()) != h.size()) return 0;

	return s == h;
}

} // namespace
