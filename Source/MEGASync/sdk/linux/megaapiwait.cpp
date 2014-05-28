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

#include "megaapiwait.h"

namespace mega {

MegaApiLinuxWaiter::MegaApiLinuxWaiter() : PosixWaiter()
{
    //Pipe to be able to leave select() call
    if (pipe(m_pipe) < 0)
        cout << "Error creating pipe" << endl;

    if (fcntl(m_pipe[0], F_SETFL, O_NONBLOCK) < 0)
        cout << "fcntl error" << endl;
}

int MegaApiLinuxWaiter::wait()
{
    //Pipe added to rfds to be able to leave select() when needed
    FD_SET(m_pipe[0], &rfds);
    bumpmaxfd(m_pipe[0]);

    int r = PosixWaiter::wait();

    //Empty pipe
    uint8_t buf;
    while (read(m_pipe[0], &buf, 1) == 1);

    return r;
}

void MegaApiLinuxWaiter::notify()
{
    write(m_pipe[1], "0", 1);
}

}
