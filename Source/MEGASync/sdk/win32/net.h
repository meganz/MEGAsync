/*

MEGA SDK 2013-11-17 - Win32 network access layer (using WinHTTP)

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

#ifndef HTTPIO_CLASS
#define HTTPIO_CLASS WinHttpIO

#include "megaclient.h"

#include "wait.h"

#include <windows.h>
#include <winhttp.h>
#include <shellapi.h>

class WinHttpIO : public HttpIO
{
	CRITICAL_SECTION csHTTP;
	HANDLE hWakeupEvent;

	protected:
	WinWaiter* waiter;
	HINTERNET hSession;
	bool completion;

public:
	static const unsigned HTTP_POST_CHUNK_SIZE = 16384;

	static VOID CALLBACK asynccallback(HINTERNET, DWORD_PTR, DWORD, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength);

	void updatedstime();

	void post(HttpReq*, const char* = 0, unsigned = 0);
	void cancel(HttpReq*);

	m_off_t postpos(void*);

	bool doio(void);

	void addevents(Waiter*);

	void entercs();
	void leavecs();

	void httpevent();

	WinHttpIO();
	~WinHttpIO();
};

struct WinHttpContext
{
	HINTERNET hRequest;
	HINTERNET hConnect;

	HttpReq* req;                   // backlink to underlying HttpReq
	WinHttpIO* httpio;              // backlink to application-wide WinHttpIO object

	unsigned postpos;
	unsigned postlen;
	const char* postdata;
};

#endif
