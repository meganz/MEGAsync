/*

MEGA SDK 2013-11-17 - POSIX network access layer (using cURL)

(cURL *must* be configured with --enable-ares!)

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
#define HTTPIO_CLASS CurlHttpIO

#include <curl/curl.h>
#include <sys/select.h>

#include "megaclient.h"

#include "wait.h"

class CurlHttpIO : public HttpIO
{
protected:
	CURLM* curlm;
	CURLSH* curlsh;

	static size_t write_data(void *, size_t, size_t, void *);

	curl_slist* contenttypejson;
	curl_slist* contenttypebinary;

public:
	void post(HttpReq*, const char* = 0, unsigned = 0);
	void cancel(HttpReq*);

	m_off_t postpos(void*);

	bool doio(void);

	void addevents(Waiter*);

	CurlHttpIO();
	~CurlHttpIO();
};

#endif
