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

#ifndef MEGA_API_WAIT_CLASS
#define MEGA_API_WAIT_CLASS MegaApiLinuxWaiter

#include "mega/types.h"
#include "mega.h"
#include "mega/waiter.h"
#include "mega/posix/megawaiter.h"

namespace mega {

class MegaApiLinuxWaiter : public PosixWaiter
{

public:
    MegaApiLinuxWaiter();
    int wait();
    void notify();

protected:
    int m_pipe[2];
};

}

#endif
