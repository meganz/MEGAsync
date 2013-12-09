/**
 * @file mega/win32/megaconsole.h
 * @brief Win32 console I/O
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

#ifndef CONSOLE_CLASS
#define CONSOLE_CLASS WinConsole

namespace mega {

struct WinConsole : public Console
{
	void readpwchar(char*, int, int* pw_buf_pos, char**);
	void setecho(bool);

	WinConsole();
	~WinConsole();
};

} // namespace

#endif
