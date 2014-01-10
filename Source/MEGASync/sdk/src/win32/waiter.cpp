/**
 * @file win32/wait.cpp
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

#include "mega.h"
#include "megawaiter.h"

namespace mega {

WinWaiter::WinWaiter()
{
	pGTC = (PGTC)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")),"GetTickCount64");

	if (!pGTC)
	{
		tickhigh = 0;
		prevt = 0;
	}
}

void WinWaiter::init(dstime ds)
{
	maxds = ds;
}

// update monotonously increasing timestamp in deciseconds
dstime WinWaiter::getdstime()
{
	if (pGTC) return ds = pGTC()/100;
	else
	{
		// emulate GetTickCount64() on XP
		DWORD t = GetTickCount();

		if (t < prevt) tickhigh += 0x100000000;
		prevt = t;

		return ds = (t+tickhigh)/100;
	}
}

// wait for events (socket, I/O completion, timeout + application events)
// ds specifies the maximum amount of time to wait in deciseconds (or ~0 if no timeout scheduled)
// (this assumes that the second call to addhandle() was coming from the network layer)
int WinWaiter::wait()
{
	int r = 0;

	// only allow interaction of asynccallback() with the main process while waiting (because WinHTTP is threaded)
	if (pcsHTTP) LeaveCriticalSection(pcsHTTP);
	DWORD dwWaitResult = ::WaitForMultipleObjectsEx((DWORD)handles.size(), &handles.front(),FALSE,maxds*100,TRUE);
	if (pcsHTTP) EnterCriticalSection(pcsHTTP);

	if (dwWaitResult == WAIT_TIMEOUT || dwWaitResult == WAIT_IO_COMPLETION) r = NEEDEXEC;
	else if (dwWaitResult >= WAIT_OBJECT_0 && dwWaitResult < WAIT_OBJECT_0+flags.size()) r = flags[dwWaitResult-WAIT_OBJECT_0];

	handles.clear();
	flags.clear();
	
	return r;
}

// add handle to the list - must not be called twice with the same handle
// return true if handle added
bool WinWaiter::addhandle(HANDLE handle, int flag)
{
    handles.push_back(handle);
	flags.push_back(flag);

    return true;
}

} // namespace
