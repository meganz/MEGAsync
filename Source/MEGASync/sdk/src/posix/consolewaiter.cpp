/**
 * @file posix/consolewaiter.cpp
 * @brief POSIX event/timeout handling, listens for stdin
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

#include "megaconsolewaiter.h"

namespace mega {
int PosixConsoleWaiter::wait()
{
    int numfd;

    // application's own wakeup criteria:
    // wake up upon user input
    FD_SET(STDIN_FILENO, &rfds);
    bumpmaxfd(STDIN_FILENO);

    numfd = select();

    // timeout or error
    if (numfd <= 0)
    {
        return NEEDEXEC;
    }

    // application's own event processing:
    // user interaction from stdin?
    if (FD_ISSET(STDIN_FILENO, &rfds))
    {
        return ( numfd == 1 ) ? HAVESTDIN : ( HAVESTDIN | NEEDEXEC );
    }

    return NEEDEXEC;
}
} // namespace
