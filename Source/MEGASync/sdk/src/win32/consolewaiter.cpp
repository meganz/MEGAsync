/**
 * @file win32/consolewaiter.cpp
 * @brief Win32 event/timeout handling, listens for console input
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

#include "mega.h"
#include "megaconsolewaiter.h"

namespace mega {
WinConsoleWaiter::WinConsoleWaiter()
{
    DWORD dwMode;

    hInput = GetStdHandle(STD_INPUT_HANDLE);

    GetConsoleMode(hInput, &dwMode);
    SetConsoleMode(hInput, dwMode & ~( ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT ));
    FlushConsoleInputBuffer(hInput);
}

// wait for events (socket, I/O completion, timeout + application events)
// ds specifies the maximum amount of time to wait in deciseconds (or ~0 if no
// timeout scheduled)
int WinConsoleWaiter::wait()
{
    int r;

    addhandle(hInput, 0);

    // aggregated wait
    r = WinWaiter::wait();

    // is it a network- or filesystem-triggered wakeup?
    if (r)
    {
        return r;
    }

    // FIXME: improve this gruesome nonblocking console read-simulating kludge
    if (_kbhit())
    {
        return HAVESTDIN;
    }

    // this assumes that the user isn't typing too fast
    INPUT_RECORD ir[1024];
    DWORD dwNum;
    ReadConsoleInput(hInput, ir, 1024, &dwNum);

    return 0;
}
} // namespace
