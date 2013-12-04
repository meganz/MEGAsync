/**
 * @file mega/console.h
 * @brief Generic class for accessing console
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

#ifndef MEGA_CONSOLE_H
#define MEGA_CONSOLE_H 1

namespace mega {

struct Console
{
	virtual void readpwchar(char*, int, int* pw_buf_pos, char**) = 0;
	virtual void setecho(bool) = 0;

	virtual ~Console() { }
};

} // namespace

#endif
