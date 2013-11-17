/*

MEGA SDK 2013-11-17 - Win32 event/timeout handling

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

#include <windows.h>
#include <conio.h>

#include "../megaclient.h"

#include "megaapiwait.h"

MegaApiWinWaiter::MegaApiWinWaiter()
{
	pGTC = (PGTC)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")),"GetTickCount64");

	if (!pGTC)
	{
		tickhigh = 0;
		prevt = 0;
	}

	if (!pGTC) cout << "Emulating GetTickCount64()" << endl;

    hWakeup[WAKEUP_CUSTOM] = CreateEvent(NULL,FALSE,FALSE,NULL);
}

void MegaApiWinWaiter::init(dstime ds)
{
	maxds = ds;
}

// update monotonously increasing timestamp in deciseconds
dstime MegaApiWinWaiter::getdstime()
{
	if (pGTC) return ds = pGTC()/100;
	else
	{
		// emulate GetTickCount64() on XP
		DWORD t = GetTickCount();

		if (t < prevt) tickhigh += 0x100000000;
		prevt = t;

		return (t+tickhigh)/100;
	}
}

// wait for events (socket, I/O completion, timeout + application events)
// ds specifies the maximum amount of time to wait in deciseconds (or ~0 if no timeout scheduled)
int MegaApiWinWaiter::wait()
{
	// only allow interaction of asynccallback() with the main process while waiting (because WinHTTP is threaded)
	if (pcsHTTP) LeaveCriticalSection(pcsHTTP);
    WaitForMultipleObjectsEx(sizeof hWakeup/sizeof *hWakeup,hWakeup,FALSE,maxds*100,TRUE);
	if (pcsHTTP) EnterCriticalSection(pcsHTTP);

    return NEEDEXEC;
}

void MegaApiWinWaiter::notify()
{
    SetEvent(hWakeup[WAKEUP_CUSTOM]);
}
