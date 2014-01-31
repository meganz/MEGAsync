/**
 * @file mega/serialize64.h
 * @brief 64-bit int serialization/unserialization
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

#ifndef MEGA_SERIALIZE64_H
#define MEGA_SERIALIZE64_H 1

#include "types.h"

namespace mega {

// 64-bit int serialization/unserialization
struct MEGA_API Serialize64
{
	static int serialize(byte*, int64_t);
	static int unserialize(byte*, int, int64_t*);
};

} // namespace

#endif
