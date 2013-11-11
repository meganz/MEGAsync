/*

MEGA SDK 2013-10-03 - sample application for the Win32 environment

Using WinHTTP for HTTP I/O and native Win32 file access with UTF-8 <-> WCHAR conversion

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

#ifdef _WIN32
#ifndef MEGAWIN32_H
#define MEGAWIN32_H

class WinHttpIO : public HttpIO
{
protected:
	HINTERNET hSession;
	int completion;

public:
	HANDLE hWakeup[2];
	CRITICAL_SECTION csHTTP;

	void updatedstime();

	void post(HttpReq*, const char* = 0, unsigned = 0);
	void cancel(HttpReq*);

	m_off_t postpos(void*);

	int doio(void);
	void waitio(uint32_t);

	WinHttpIO();
	~WinHttpIO();
};

struct WinHttpContext
{
    HINTERNET hConnect;       // connection handle
    HINTERNET hRequest;       // resource request handle
	unsigned postpos;
};

class WinFileAccess : public FileAccess
{
	HANDLE hFile;

public:
	int fopen(const char*, int, int);
	int fread(string*, unsigned, unsigned, m_off_t);
	int fwrite(const byte*, unsigned, m_off_t);

	WinFileAccess();
	~WinFileAccess();
};

#endif
#endif
