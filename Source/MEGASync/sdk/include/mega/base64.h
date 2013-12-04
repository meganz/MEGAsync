/**
 * @file mega/base64.h
 * @brief modified base64 encoding/decoding
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

#ifndef MEGA_BASE64_H
#define MEGA_BASE64_H 1

#include "types.h"

namespace mega {

// modified base64 encoding/decoding (unpadded, -_ instead of +/)
class Base64
{
	static byte to64(byte);
	static byte from64(byte);

public:
	static int btoa(const byte*, int, char*);
	static int atob(const char*, byte*, int);
};

} // namespace

#endif
