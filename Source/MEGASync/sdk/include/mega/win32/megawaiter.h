/**
 * @file mega/win32/megawaiter.h
 * @brief Win32 event/timeout handling
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

#ifndef WAIT_CLASS
#define WAIT_CLASS WinWaiter

namespace mega {
class MEGA_API WinWaiter : public Waiter
{
    typedef ULONGLONG ( WINAPI * PGTC )();
    PGTC pGTC;
    ULONGLONG tickhigh;
    DWORD prevt;

    vector<HANDLE> handles;
    vector<int> flags;

public:
    PCRITICAL_SECTION pcsHTTP;
    unsigned pendingfsevents;

    dstime getdstime();

    void init(dstime);
    int wait();

    bool addhandle(HANDLE handle, int);

    WinWaiter();
};
} // namespace

#endif
