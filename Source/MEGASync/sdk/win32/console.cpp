/*

MEGA SDK Win32 console I/O

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

#include <windows.h>
#include <conio.h>

#include "megaclient.h"
#include "console.h"

#include <io.h>
#include <fcntl.h>

WinConsole::WinConsole()
{
	// FIXME: configure for UTF8
}

WinConsole::~WinConsole()
{
	// restore startup config
}

void WinConsole::readpwchar(char* pw_buf, int pw_buf_size, int* pw_buf_pos, char** line)
{
	char c;
	DWORD cread;

	if (ReadConsole(GetStdHandle(STD_INPUT_HANDLE),&c,1,&cread,NULL) == 1)
	{
		if (c == 8 && *pw_buf_pos) (*pw_buf_pos)--;
		else if (c == 13)
		{
			*line = (char*)malloc(*pw_buf_pos+1);
			memcpy(*line,pw_buf,*pw_buf_pos);
			(*line)[*pw_buf_pos] = 0;
		}
		else if (*pw_buf_pos < pw_buf_size) pw_buf[(*pw_buf_pos)++] = c;
	}
}

void WinConsole::setecho(bool echo)
{
	HANDLE hCon = GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode;

	GetConsoleMode(hCon,&mode);
	if (echo) mode |= ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT;
	else mode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
	SetConsoleMode(hCon,mode);
}
