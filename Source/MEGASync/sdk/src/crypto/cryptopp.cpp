/**
 * @file cryptopp.cpp
 * @brief Crypto layer using Crypto++
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

#include "mega.h"

namespace mega {
#ifndef htobe64
#define htobe64(x) (((uint64_t)htonl((uint32_t)((x) >> 32))) | (((uint64_t)htonl((uint32_t)x)) << 32))
#endif

using namespace CryptoPP;

AutoSeededRandomPool PrnGen::rng;

// cryptographically strong random byte sequence
void PrnGen::genblock(byte* buf, int len)
{
    rng.GenerateBlock(buf, len);
}

// random number from 0 ... max-1
uint32_t PrnGen::genuint32(uint64_t max)
{
    uint32_t t;

    genblock((byte*)&t, sizeof t);

    return (uint32_t)(((uint64_t)t) / ((((uint64_t)(~(uint32_t)0)) + 1) / max));
}

SymmCipher::SymmCipher()
{
    keyvalid = 0;
}

SymmCipher::SymmCipher(const byte* key)
{
    setkey(key);
}

byte SymmCipher::zeroiv[BLOCKSIZE];

void SymmCipher::setkey(const byte* newkey, int type)
{
    memcpy(key, newkey, KEYLENGTH);
    if (!type)
    {
        xorblock(newkey + KEYLENGTH, key);
    }

    aesecb_e.SetKey(key, KEYLENGTH);
    aesecb_d.SetKey(key, KEYLENGTH);

    aescbc_e.SetKeyWithIV(key, KEYLENGTH, zeroiv);
    aescbc_d.SetKeyWithIV(key, KEYLENGTH, zeroiv);

    keyvalid = 1;
}

void SymmCipher::cbc_encrypt(byte* data, unsigned len)
{
    aescbc_e.Resynchronize(zeroiv);
    aescbc_e.ProcessData(data, data, len);
}

void SymmCipher::cbc_decrypt(byte* data, unsigned len)
{
    aescbc_d.Resynchronize(zeroiv);
    aescbc_d.ProcessData(data, data, len);
}

void SymmCipher::ecb_encrypt(byte* data, byte* dst, unsigned len)
{
    aesecb_e.ProcessData(dst ? dst : data, data, len);
}

void SymmCipher::ecb_decrypt(byte* data, unsigned len)
{
    aesecb_d.ProcessData(data, data, len);
}

void SymmCipher::setint64(int64_t value, byte* data)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    value = htobe64(value);
#else
#if __BYTE_ORDER != __BIG_ENDIAN
#error "Unknown or unsupported endianness"
#endif
#endif
    memcpy(data, (char*)&value, sizeof value);
}

void SymmCipher::xorblock(const byte* src, byte* dst)
{
    long* lsrc = (long*)src;
    long* ldst = (long*)dst;

    for (int i = BLOCKSIZE / sizeof(long); i--;)
    {
        ldst[i] ^= lsrc[i];
    }
}

void SymmCipher::xorblock(const byte* src, byte* dst, int len)
{
    while (len--)
    {
        dst[len] ^= src[len];
    }
}

void SymmCipher::incblock(byte* dst, unsigned len)
{
    while (len)
    {
        if (++dst[--len])
        {
            break;
        }
    }
}

// encryption: data must be NUL-padded to BLOCKSIZE
// decryption: data must be padded to BLOCKSIZE
// len must be < 2^31
void SymmCipher::ctr_crypt(byte* data, unsigned len, m_off_t pos, ctr_iv ctriv, byte* mac, int encrypt)
{
    assert(!(pos & (KEYLENGTH - 1)));

    byte ctr[BLOCKSIZE], tmp[BLOCKSIZE];

    MemAccess::set<int64_t>(ctr,ctriv);
    setint64(pos / BLOCKSIZE, ctr + sizeof ctriv);

    memcpy(mac, ctr, sizeof ctriv);
    memcpy(mac + sizeof ctriv, ctr, sizeof ctriv);

    while ((int)len > 0)
    {
        if (encrypt)
        {
            xorblock(data, mac);
            ecb_encrypt(mac);
            ecb_encrypt(ctr, tmp);
            xorblock(tmp, data);
        }
        else
        {
            ecb_encrypt(ctr, tmp);
                xorblock(tmp,  data);
            if (len >= (unsigned)BLOCKSIZE)
            {
                xorblock(data, mac);
            }
            else
            {
                xorblock(data, mac, len);
            }
            ecb_encrypt(mac);
        }

        len -= BLOCKSIZE;
        data += BLOCKSIZE;

        incblock(ctr);
    }
}

static void rsaencrypt(Integer* key, Integer* m)
{
    *m = a_exp_b_mod_c(*m, key[1], key[0]);
}

unsigned AsymmCipher::rawencrypt(const byte* plain, int plainlen, byte* buf, int buflen)
{
    Integer t(plain, plainlen);

    rsaencrypt(key, &t);

    int i = t.ByteCount();

    if (i > buflen)
    {
        return 0;
    }

    while (i--)
    {
        *buf++ = t.GetByte(i);
    }

    return t.ByteCount();
}

int AsymmCipher::encrypt(const byte* plain, int plainlen, byte* buf, int buflen)
{
    if ((int)key[0].ByteCount() + 2 > buflen)
    {
        return 0;
    }

    if (buf != plain)
    {
        memcpy(buf, plain, plainlen);
    }

    // add random padding
    PrnGen::genblock(buf + plainlen, key[0].ByteCount() - plainlen - 2);

    Integer t(buf, key[0].ByteCount() - 2);

    rsaencrypt(key, &t);

    int i = t.BitCount();

    byte* ptr = buf;

    *ptr++ = (byte)(i >> 8);
    *ptr++ = (byte)i;

    i = t.ByteCount();

    while (i--)
    {
        *ptr++ = t.GetByte(i);
    }

    return ptr - buf;
}

static void rsadecrypt(Integer* key, Integer* m)
{
    Integer xp = a_exp_b_mod_c(*m % key[AsymmCipher::PRIV_P],
                               key[AsymmCipher::PRIV_D] % (key[AsymmCipher::PRIV_P] - Integer::One()),
                               key[AsymmCipher::PRIV_P]);
    Integer xq = a_exp_b_mod_c(*m % key[AsymmCipher::PRIV_Q],
                               key[AsymmCipher::PRIV_D] % (key[AsymmCipher::PRIV_Q] - Integer::One()),
                               key[AsymmCipher::PRIV_Q]);

    if (xp > xq)
    {
        *m = key[AsymmCipher::PRIV_Q] - (((xp - xq) * key[AsymmCipher::PRIV_U]) % key[AsymmCipher::PRIV_Q]);
    }
    else
    {
        *m = ((xq - xp) * key[AsymmCipher::PRIV_U]) % key[AsymmCipher::PRIV_Q];
    }

    *m = *m * key[AsymmCipher::PRIV_P] + xp;
}

unsigned AsymmCipher::rawdecrypt(const byte* c, int cl, byte* buf, int buflen)
{
    Integer m(c, cl);

    rsadecrypt(key, &m);

    int i = m.ByteCount();

    if (i > buflen)
    {
        return 0;
    }

    while (i--)
    {
        *buf++ = m.GetByte(i);
    }

    return m.ByteCount();
}

int AsymmCipher::decrypt(const byte* c, int cl, byte* out, int numbytes)
{
    Integer m;

    if (!decodeintarray(&m, 1, c, cl))
    {
        return 0;
    }

    rsadecrypt(key, &m);

    unsigned l = key[AsymmCipher::PRIV_D].ByteCount() - 2;

    if (m.ByteCount() > l)
    {
        l = m.ByteCount();
    }

    l -= numbytes;

    while (numbytes--)
    {
        out[numbytes] = m.GetByte(l++);
    }

    return 1;
}

int AsymmCipher::setkey(int numints, const byte* data, int len)
{
    return decodeintarray(key, numints, data, len);
}

void AsymmCipher::serializekey(string* d, int keytype)
{
    serializeintarray(key, keytype, d);
}

void AsymmCipher::serializeintarray(Integer* t, int numints, string* d)
{
    unsigned size = 0;
    char c;

    for (int i = numints; i--;)
    {
        size += t[i].ByteCount() + 2;
    }

    d->reserve(d->size() + size);

    for (int i = 0; i < numints; i++)
    {
        c = t[i].BitCount() >> 8;
        d->append(&c, sizeof c);

        c = (char)t[i].BitCount();
        d->append(&c, sizeof c);

        for (int j = t[i].ByteCount(); j--;)
        {
            c = t[i].GetByte(j);
            d->append(&c, sizeof c);
        }
    }
}

int AsymmCipher::decodeintarray(Integer* t, int numints, const byte* data, int len)
{
    int p, i, n;

    p = 0;

    for (i = 0; i < numints; i++)
    {
        if (p + 2 > len)
        {
            break;
        }

        n = ((data[p] << 8) + data[p + 1] + 7) >> 3;

        p += 2;
        if (p + n > len)
        {
            break;
        }

        t[i] = Integer(data + p, n);

        p += n;
    }

    return i == numints && len - p < 16;
}

int AsymmCipher::isvalid()
{
    return key[0].BitCount() && key[1].BitCount();
}

// adapted from CryptoPP, rsa.cpp
class RSAPrimeSelector : public PrimeSelector
{
    Integer m_e;

public:
    RSAPrimeSelector(const Integer &e) : m_e(e) { }

    bool IsAcceptable(const Integer &candidate) const
    {
        return RelativelyPrime(m_e, candidate - Integer::One());
    }
};

// generate RSA keypair
void AsymmCipher::genkeypair(Integer* privk, Integer* pubk, int size)
{
    pubk[PUB_E] = 17;

    RSAPrimeSelector selector(pubk[PUB_E]);
    AlgorithmParameters primeParam
            = MakeParametersForTwoPrimesOfEqualSize(size)
                (Name::PointerToPrimeSelector(), selector.GetSelectorPointer());

    privk[PRIV_P].GenerateRandom(PrnGen::rng, primeParam);
    privk[PRIV_Q].GenerateRandom(PrnGen::rng, primeParam);

    privk[PRIV_D] = pubk[PUB_E].InverseMod(LCM(privk[PRIV_P] - Integer::One(), privk[PRIV_Q] - Integer::One()));
    pubk[PUB_PQ] = privk[PRIV_P] * privk[PRIV_Q];
    privk[PRIV_U] = privk[PRIV_P].InverseMod(privk[PRIV_Q]);
}

void Hash::add(const byte* data, unsigned len)
{
    hash.Update(data, len);
}

void Hash::get(string* out)
{
    out->resize(hash.DigestSize());
    hash.Final((byte*)out->data());
}

void HashCRC32::add(const byte* data, unsigned len)
{
    hash.Update(data, len);
}

void HashCRC32::get(byte* out)
{
    hash.Final(out);
}
} // namespace
