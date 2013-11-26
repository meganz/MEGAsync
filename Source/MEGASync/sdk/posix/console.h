/*

MEGA SDK POSIX console/terminal control

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

#ifndef CONSOLE_CLASS
#define CONSOLE_CLASS PosixConsole

#include <termios.h>

struct PosixConsole : public Console
{
	tcflag_t oldlflag;
	cc_t oldvtime;
	struct termios term;

	void readpwchar(char*, int, int* pw_buf_pos, char**);
	void setecho(bool);

	PosixConsole();
	~PosixConsole();
};

#endif