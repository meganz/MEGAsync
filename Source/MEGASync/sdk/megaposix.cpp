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

#define _POSIX_SOURCE
#define _LARGE_FILES
#define _LARGEFILE64_SOURCE
#define _GNU_SOURCE 1
#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>

#include <glob.h>
#include <sys/select.h>
#include <sys/un.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <curl/curl.h>

#include <db_cxx.h>

#define USE_VARARGS
#define PREFER_STDARG
#include <readline/readline.h>
#include <readline/history.h>

#define __DARWIN_C_LEVEL 199506L

#ifdef __MACH__
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
ssize_t pread(int, void *, size_t, off_t) __DARWIN_ALIAS_C(pread);
ssize_t pwrite(int, const void *, size_t, off_t) __DARWIN_ALIAS_C(pwrite);
#endif

#include "mega.h"

#include "megacrypto.h"
#include "megaclient.h"
#include "megaposix.h"
#include "megacli.h"
#include "megabdb.h"

// HttpIO implementation using libcurl
CurlHttpIO::CurlHttpIO()
{
	curl_global_init(CURL_GLOBAL_DEFAULT);

	curlm = curl_multi_init();

	curlsh = curl_share_init();
	curl_share_setopt(curlsh,CURLSHOPT_SHARE,CURL_LOCK_DATA_DNS);
	curl_share_setopt(curlsh,CURLSHOPT_SHARE,CURL_LOCK_DATA_SSL_SESSION);

	contenttypejson = curl_slist_append(NULL,"Content-Type: application/json");
	contenttypejson = curl_slist_append(contenttypejson, "Expect:");

	contenttypebinary = curl_slist_append(NULL,"Content-Type: application/octet-stream");
	contenttypebinary = curl_slist_append(contenttypebinary, "Expect:");
}

CurlHttpIO::~CurlHttpIO()
{
	curl_global_cleanup();
}

// update monotonously increasing timestamp in deciseconds
void CurlHttpIO::updatedstime()
{
	timespec ts;

	clock_gettime(CLOCK_MONOTONIC,&ts);

	ds = ts.tv_sec*10+ts.tv_nsec/100000000;
}

// POST request to URL
void CurlHttpIO::post(HttpReq* req, const char* data, unsigned len)
{
	if (debug)
	{
		cout << "POST target URL: " << req->posturl << endl;

		if (req->binary) cout << "[sending " << req->out->size() << " bytes of raw data]" << endl;
		else cout << "Sending: " << *req->out << endl;
	}

	CURL* curl;

	req->in.clear();

	if ((curl = curl_easy_init()))
	{
		curl_easy_setopt(curl,CURLOPT_URL,req->posturl.c_str());
		curl_easy_setopt(curl,CURLOPT_POSTFIELDS,data ? data : req->out->data());
		curl_easy_setopt(curl,CURLOPT_POSTFIELDSIZE,data ? len : req->out->size());
		curl_easy_setopt(curl,CURLOPT_USERAGENT,"MEGA Client Access Engine/1.0");
		curl_easy_setopt(curl,CURLOPT_HTTPHEADER,req->type == REQ_JSON ? contenttypejson : contenttypebinary);
		curl_easy_setopt(curl,CURLOPT_SHARE,curlsh);
		curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,write_data);
		curl_easy_setopt(curl,CURLOPT_WRITEDATA,(void*)req);
		curl_easy_setopt(curl,CURLOPT_PRIVATE,(void*)req);

		curl_multi_add_handle(curlm,curl);

		req->status = REQ_INFLIGHT;

		req->httpiohandle = (void*)curl;
	}
	else req->status = REQ_FAILURE;
}

// cancel pending HTTP request
void CurlHttpIO::cancel(HttpReq* req)
{
	if (req->httpiohandle)
	{
		curl_multi_remove_handle(curlm,(CURL*)req->httpiohandle);
		curl_easy_cleanup((CURL*)req->httpiohandle);

		req->httpstatus = 0;
		req->status = REQ_FAILURE;
		req->httpiohandle = NULL;
	}
}

// real-time progress information on POST data
m_off_t CurlHttpIO::postpos(void* handle)
{
	double bytes;

	curl_easy_getinfo(handle,CURLINFO_SIZE_UPLOAD,&bytes);

	return (m_off_t)bytes;
}

// process events
int CurlHttpIO::doio()
{
	int done;

	done = 0;

	CURLMsg *msg;
	int dummy;

	curl_multi_perform(curlm,&dummy);

	while ((msg = curl_multi_info_read(curlm,&dummy)))
	{
		HttpReq* req;

		if (curl_easy_getinfo(msg->easy_handle,CURLINFO_PRIVATE,(char**)&req) == CURLE_OK && req)
		{
			req->httpio = NULL;

			if (msg->msg == CURLMSG_DONE)
			{
				curl_easy_getinfo(msg->easy_handle,CURLINFO_RESPONSE_CODE,&req->httpstatus);

				if (debug)
				{
					cout << "CURLMSG_DONE with HTTP status: " << req->httpstatus << endl;

					if (req->binary) cout << "[received " << req->in.size() << " bytes of raw data]" << endl;
					else cout << "Received: " << req->in.c_str() << endl;
				}

				req->status = req->httpstatus == 200 ? REQ_SUCCESS : REQ_FAILURE;
				done = 1;
			}
			else req->status = REQ_FAILURE;
		}

		curl_multi_remove_handle(curlm,msg->easy_handle);
		curl_easy_cleanup(msg->easy_handle);
	}

	return done;
}

// wait for events (sockets, timeout + application events)
// ds specifies the maximum amount of time to wait in deciseconds (or ~0 if no timeout scheduled)
void CurlHttpIO::waitio(dstime maxds)
{
	int maxfd;
	fd_set rfds, wfds, efds;
	timeval tv;

	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_ZERO(&efds);

	curl_multi_fdset(curlm,&rfds,&wfds,&efds,&maxfd);

	// unblock upon user input
	FD_SET(fileno(stdin),&rfds);

	if (maxfd < fileno(stdin)) maxfd = fileno(stdin);

	if (maxds+1)
	{
		dstime us = 1000000/10*maxds;

		tv.tv_sec = us/1000000;
		tv.tv_usec = us-tv.tv_sec*1000000;
	}

	select(maxfd+1,&rfds,&wfds,&efds,maxds+1 ? &tv : NULL);

	// user interaction from stdin?
	if (FD_ISSET(fileno(stdin),&rfds)) rl_callback_read_char();
	else redisplay = 1;
}

// callback for incoming HTTP payload
size_t CurlHttpIO::write_data(void *ptr, size_t size, size_t nmemb, void *target)
{
	size *= nmemb;

	((HttpReq*)target)->put(ptr,size);

	return size;
}

FileAccess* DemoApp::newfile()
{
	return new PosixFileAccess();
}

PosixFileAccess::PosixFileAccess()
{
	fd = -1;
}

PosixFileAccess::~PosixFileAccess()
{
	if (fd >= 0) close(fd);
}

int PosixFileAccess::fread(string* dst, unsigned len, unsigned pad, m_off_t pos)
{
	dst->resize(len+pad);

	if (pread(fd,(char*)dst->data(),len,pos) == len)
	{
		memset((char*)dst->data()+len,0,pad);
		return 1;
	}

	return 0;
}

int PosixFileAccess::fwrite(const byte* data, unsigned len, m_off_t pos)
{
	if (pwrite(fd,data,len,pos) == len) return 1;

	return 0;
}

int PosixFileAccess::fopen(const char* f, int read, int write)
{
	if ((fd = open(f,write ? (read ? O_RDWR : O_WRONLY|O_CREAT|O_TRUNC) : O_RDONLY,0500)) >= 0)
	{
		struct stat statbuf;

		if (!fstat(fd,&statbuf))
		{
			size = statbuf.st_size;
			mtime = statbuf.st_mtime;
			return 1;
		}

		close(fd);
	}

	return 0;
}

int rename_file(const char* oldname, const char* newname)
{
	return !rename(oldname,newname);
}

int unlink_file(const char* name)
{
	return !unlink(name);
}

int change_dir(const char* name)
{
	return !chdir(name);
}

int globenqueue(const char* path, const char* newname, handle target,  const char* targetuser)
{
	const char* filename;
	struct stat statbuf;
	glob_t globbuf;
	int total = 0;

	glob(path,GLOB_NOSORT,NULL,&globbuf);

	for (int i = 0; i < (int)globbuf.gl_pathc; i++)
	{
		if (!stat(globbuf.gl_pathv[i],&statbuf) && statbuf.st_mode & S_IFREG)
		{
			for (filename = strchr(globbuf.gl_pathv[i],0); filename > globbuf.gl_pathv[i] && *filename != ':' && *filename != '/'; filename--);

			if (*filename)
			{
				client->putq.push_back(new AppFilePut(filename,target,targetuser,newname));
				total++;
			}
		}
	}

	globfree(&globbuf);

	return total;
}

void term_init()
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

void term_restore()
{
	term.c_lflag = oldlflag;
	term.c_cc[VTIME] = oldvtime;

	if (tcsetattr(STDIN_FILENO,TCSANOW,&term) < 0)
	{
		perror("tcsetattr");
		exit(1);
	}
}

void term_echo(int echo)
{
	if (echo)
	{
		// enable echo
		tcsetattr(0,TCSANOW,&term);
	}
	else
	{
		// disable echo
		struct termios new_settings = term;
		new_settings.c_lflag &= ~ECHO;
		tcsetattr(0,TCSANOW,&new_settings);
	}
}

int main()
{
	// instantiate app components: the callback processor (DemoApp),
	// the cURL HTTP I/O engine (CurlHttpIO) and the MegaClient itself
	client = new MegaClient(new DemoApp,new CurlHttpIO,new BdbAccess,"SDKSAMPLE");

	megacli();
}
