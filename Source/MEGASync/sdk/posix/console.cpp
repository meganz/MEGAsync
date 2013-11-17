/*

MEGA SDK 2013-11-17 - POSIX console/terminal control

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

#include "megaclient.h"
#include "console.h"

PosixConsole::PosixConsole()
{
	// set up the console
    if (tcgetattr(STDIN_FILENO,&term) < 0)
	{
        perror("tcgetattr");
        exit(1);
    }

    oldlflag = term.c_lflag;
    oldvtime = term.c_cc[VTIME];
    term.c_lflag &= ~ICANON;
    term.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO,TCSANOW,&term) < 0)
	{
        perror("tcsetattr");
        exit(1);
    }
}

PosixConsole::~PosixConsole()
{
	term.c_lflag = oldlflag;
	term.c_cc[VTIME] = oldvtime;

	if (tcsetattr(STDIN_FILENO,TCSANOW,&term) < 0)
	{
		perror("tcsetattr");
		exit(1);
	}
}

// FIXME: UTF-8 compatibility
void PosixConsole::readpwchar(char* pw_buf, int pw_buf_size, int* pw_buf_pos, char** line)
{
	char c;

	if (read(STDIN_FILENO,&c,1) == 1)
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

void PosixConsole::setecho(bool echo)
{
}
