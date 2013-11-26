/*

MEGA SDK POSIX event/timeout handling

(c) 2013 by Mega Limited, Wellsford, New Zealand

Author: mo

Applications using the MEGA API must present a valid application key
and comply with the the rules set forth in the Terms of Service.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#include <sys/select.h>
#include <sys/inotify.h>

#define __DARWIN_C_LEVEL 199506L

#ifdef __MACH__

// FIXME: revisit OS X support
#include <machine/endian.h>
#include <strings.h>
#include <sys/time.h>
#define CLOCK_MONOTONIC 0
int clock_gettime(int, struct timespec* t)
{
    struct timeval now;
    int rv = gettimeofday(&now,NULL);
    if (rv) return rv;
    t->tv_sec  = now.tv_sec;
    t->tv_nsec = now.tv_usec*1000;
    return 0;
}

#endif

#include "megaclient.h"

#include "wait.h"

void PosixWaiter::init(dstime ds)
{
	maxds = ds;

	maxfd = -1;

	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_ZERO(&efds);
}

// update monotonously increasing timestamp in deciseconds
dstime PosixWaiter::getdstime()
{
	timespec ts;

	clock_gettime(CLOCK_MONOTONIC,&ts);

	return ds = ts.tv_sec*10+ts.tv_nsec/100000000;
}

// update maxfd for select()
void PosixWaiter::bumpmaxfd(int fd)
{
	if (fd > maxfd) maxfd = fd;
}

// wait for supplied events (sockets, filesystem changes), plus timeout + application events
// maxds specifies the maximum amount of time to wait in deciseconds (or ~0 if no timeout scheduled)
// returns application-specific bitmask. bit 0 set indicates that exec() needs to be called.
// this implementation returns the presence of pending stdin data in bit 1.
int PosixWaiter::wait()
{
	timeval tv;
	int numfd;

	// application's own wakeup criteria:
	// wake up upon user input
	FD_SET(STDIN_FILENO,&rfds);

	bumpmaxfd(STDIN_FILENO);

	if (maxds+1)
	{
		dstime us = 1000000/10*maxds;

		tv.tv_sec = us/1000000;
		tv.tv_usec = us-tv.tv_sec*1000000;
	}

	numfd = select(maxfd+1,&rfds,&wfds,&efds,maxds+1 ? &tv : NULL);

	if (numfd <= 0) return NEEDEXEC;

	// application's own event processing:
	// user interaction from stdin?
	if (FD_ISSET(STDIN_FILENO,&rfds)) return (numfd == 1) ? HAVESTDIN : (HAVESTDIN | NEEDEXEC);

	return NEEDEXEC;
}
