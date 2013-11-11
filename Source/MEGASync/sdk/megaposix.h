/*

MEGA SDK 2013-10-03 - sample application for the gcc/POSIX environment 

Using cURL for HTTP I/O, GNU Readline for console I/O and FreeImage for thumbnail creation

cURL *must* be configured with --enable-ares!

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

#ifndef _WIN32
#ifndef MEGAPOSIX_H
#define MEGAPOSIX_H

class CurlHttpIO : public HttpIO
{
protected:
	CURLM* curlm;
	CURLSH* curlsh;

	static size_t write_data(void *, size_t, size_t, void *);

	curl_slist* contenttypejson;
	curl_slist* contenttypebinary;

public:
	void updatedstime();

	void post(HttpReq*, const char* = 0, unsigned = 0);
	void cancel(HttpReq*);

	m_off_t postpos(void*);

	int doio(void);
	void waitio(uint32_t);

	CurlHttpIO();
	~CurlHttpIO();
};

class PosixFileAccess : public FileAccess
{
	int fd;

public:
	int fopen(const char*, int, int);
	int fread(string*, unsigned, unsigned, m_off_t);
	int fwrite(const byte*, unsigned, m_off_t);

	static int rename(const char*, const char*);
	static int unlink(const char*);

	PosixFileAccess();
	~PosixFileAccess();
};

// terminal handling
static tcflag_t oldlflag;
static cc_t oldvtime;
static struct termios term;

#endif
#endif
