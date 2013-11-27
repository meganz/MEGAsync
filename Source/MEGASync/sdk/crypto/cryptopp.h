/*

MEGA SDK crypto layer using Crypto++

(c) 2013 by Mega Limited, Wellsford, New Zealand

Applications using the MEGA API must present a valid application key
and comply with the the rules set forth in the Terms of Service.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#ifdef USE_CRYPTOPP
#ifndef CRYPTOCRYPTOPP_H
#define CRYPTOCRYPTOPP_H 1

#include <cryptlib.h>
#include <modes.h>
#include <integer.h>
#include <aes.h>
#include <osrng.h>
#include <sha.h>
#include <rsa.h>
#include <crc.h>
#include <nbtheory.h>
#include <algparam.h>

// generic pseudo-random number generator
class PrnGen
{
public:
	static CryptoPP::AutoSeededRandomPool rng;

	static void genblock(byte*, int);
	static uint32_t genuint32(uint64_t);
};

// symmetric cryptography: AES-128
class SymmCipher
{
	CryptoPP::ECB_Mode<CryptoPP::AES>::Encryption aesecb_e;
	CryptoPP::ECB_Mode<CryptoPP::AES>::Decryption aesecb_d;

	CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption aescbc_e;
	CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption aescbc_d;

public:
	static byte zeroiv[CryptoPP::AES::BLOCKSIZE];

	static const int BLOCKSIZE = CryptoPP::AES::BLOCKSIZE;
	static const int KEYLENGTH = CryptoPP::AES::BLOCKSIZE;

	byte key[KEYLENGTH];

	int keyvalid;

	typedef uint64_t ctr_iv;

	void setkey(const byte*, int = 1);

	void ecb_encrypt(byte*, byte* = NULL, unsigned = BLOCKSIZE);
	void ecb_decrypt(byte*, unsigned = BLOCKSIZE);

	void cbc_encrypt(byte*, unsigned);
	void cbc_decrypt(byte*, unsigned);

	void ctr_crypt(byte*, unsigned, m_off_t, ctr_iv, byte*, int);

	static void setint64(int64_t, byte*);
	static int64_t getint64(const byte*);

	static void xorblock(const byte*, byte*);
	static void xorblock(const byte*, byte*, int);

	static void incblock(byte*, unsigned = BLOCKSIZE);

	SymmCipher();
	SymmCipher(const byte*);
};

// asymmetric cryptography: RSA
class AsymmCipher
{
	int decodeintarray(CryptoPP::Integer*, int, const byte*, int);

public:
	enum { PRIV_P, PRIV_Q, PRIV_D, PRIV_U };
	enum { PUB_PQ, PUB_E };

	static const int PRIVKEY = 4;
	static const int PUBKEY = 2;

	CryptoPP::Integer key[PRIVKEY];

	static const int MAXKEYLENGTH = 1026;	// in bytes, allows for RSA keys up to 8192 bits

	int setkey(int, const byte*, int);

	int isvalid();

	int encrypt(const byte*, int, byte*, int);
	int decrypt(const byte*, int, byte*, int);

	unsigned rawencrypt(const byte* plain, int plainlen, byte* buf, int buflen);
	unsigned rawdecrypt(const byte* c, int cl, byte* buf, int buflen);

	static void serializeintarray(CryptoPP::Integer*, int, std::string*);
	void serializekey(std::string*, int);
	void genkeypair(CryptoPP::Integer* privk, CryptoPP::Integer* pubk, int size);
};

class Hash
{
	CryptoPP::SHA512 hash;

public:
	void add(const byte*, unsigned);
	void get(std::string*);
};

class HashCRC32
{
	CryptoPP::CRC32 hash;

public:
	void add(const byte*, unsigned);
	void get(byte*);
};

#endif
#endif
