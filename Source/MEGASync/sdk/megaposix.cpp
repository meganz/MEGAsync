/*

MEGA SDK 2013-11-11 - sample application for the gcc/POSIX environment 

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

// FIXME: removal of last inotify continuously triggers select()

#define _POSIX_SOURCE
#define _LARGE_FILES
#define _LARGEFILE64_SOURCE
#define _GNU_SOURCE 1
#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>

#include <glob.h>
#include <dirent.h>
#include <sys/select.h>
#include <sys/inotify.h>
#include <sys/sendfile.h>
#include <sys/un.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <utime.h>

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

void PosixWaiter::init(dstime ds)
{
	maxds = ds;

	maxfd = -1;

	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_ZERO(&efds);
}

// update monotonously increasing timestamp in deciseconds
dstime PosixWaiter::getdstime()
{
	timespec ts;

	clock_gettime(CLOCK_MONOTONIC,&ts);

	return ds = ts.tv_sec*10+ts.tv_nsec/100000000;
}

// update maxfd for select()
void PosixWaiter::bumpmaxfd(int fd)
{
	if (fd > maxfd) maxfd = fd;
}

// wait for supplied events (sockets, filesystem changes), plus timeout + application events
// maxds specifies the maximum amount of time to wait in deciseconds (or ~0 if no timeout scheduled)
// returns application-specific bitmask. bit 0 set indicates that exec() needs to be called.
// this implementation returns the presence of pending stdin data in bit 1.
int PosixWaiter::wait()
{
	timeval tv;
	int numfd;

	// application's own wakeup criteria:
	// wake up upon user input
	FD_SET(STDIN_FILENO,&rfds);

	bumpmaxfd(STDIN_FILENO);

	if (maxds+1)
	{
		dstime us = 1000000/10*maxds;

		tv.tv_sec = us/1000000;
		tv.tv_usec = us-tv.tv_sec*1000000;
	}

	numfd = select(maxfd+1,&rfds,&wfds,&efds,maxds+1 ? &tv : NULL);

	if (numfd <= 0) return NEEDEXEC;

	// application's own event processing:
	// user interaction from stdin?
	if (FD_ISSET(STDIN_FILENO,&rfds)) return (numfd == 1) ? HAVESTDIN : (HAVESTDIN | NEEDEXEC);

	return NEEDEXEC;
}

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

// wake up from cURL I/O
void CurlHttpIO::addevents(Waiter* w)
{
	int t;
	PosixWaiter* pw = (PosixWaiter*)w;

	curl_multi_fdset(curlm,&pw->rfds,&pw->wfds,&pw->efds,&t);
	
	pw->bumpmaxfd(t);
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
bool CurlHttpIO::doio()
{
	bool done;

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
				done = true;
			}
			else req->status = REQ_FAILURE;
		}

		curl_multi_remove_handle(curlm,msg->easy_handle);
		curl_easy_cleanup(msg->easy_handle);
	}

	return done;
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

bool PosixFileAccess::fread(string* dst, unsigned len, unsigned pad, m_off_t pos)
{
	dst->resize(len+pad);

	if (pread(fd,(char*)dst->data(),len,pos) == len)
	{
		memset((char*)dst->data()+len,0,pad);
		return true;
	}

	return false;
}

bool PosixFileAccess::frawread(byte* dst, unsigned len, m_off_t pos)
{
	return pread(fd,(char*)dst,len,pos) == len;
}

bool PosixFileAccess::fwrite(const byte* data, unsigned len, m_off_t pos)
{
	return pwrite(fd,data,len,pos) == len;
}

bool PosixFileAccess::fopen(string* f, bool read, bool write)
{
	if ((fd = open(f->c_str(),write ? (read ? O_RDWR : O_WRONLY|O_CREAT|O_TRUNC) : O_RDONLY,0600)) >= 0)
	{
		struct stat statbuf;

		if (!fstat(fd,&statbuf))
		{
			size = statbuf.st_size;
			mtime = statbuf.st_mtime;
			type = S_ISDIR(statbuf.st_mode) ? FOLDERNODE : FILENODE;

			return true;
		}

		close(fd);
	}

	return false;
}

PosixFileSystemAccess::PosixFileSystemAccess()
{
	localseparator = "/";

	notifyfd = inotify_init1(IN_NONBLOCK);
	notifyerr = false;
	notifyleft = 0;
}

PosixFileSystemAccess::~PosixFileSystemAccess()
{
	if (notifyfd >= 0) close(notifyfd);
}

// wake up from filesystem updates
void PosixFileSystemAccess::addevents(Waiter* w)
{
	PosixWaiter* pw = (PosixWaiter*)w;

	FD_SET(notifyfd,&pw->rfds);
	
	pw->bumpmaxfd(notifyfd);
}

// generate unique local filename in the same fs as relatedpath
void PosixFileSystemAccess::tmpnamelocal(string* filename, string* relatedpath)
{
	*filename = tmpnam(NULL);
}

void PosixFileSystemAccess::local2path(string* local, string* path)
{
	*path = *local;
}

// use UTF-8 filenames directly, but escape forbidden characters
void PosixFileSystemAccess::name2local(string* filename, const char* badchars)
{
	char buf[4];

	if (!badchars) badchars = "\\/:?\"<>|*\1\2\3\4\5\6\7\10\11\12\13\14\15\16\17\20\21\22\23\24\25\26\27\30\31\32\33\34\35\36\37";

	// replace all occurrences of a badchar with %xx
	for (int i = filename->size(); i--; )
	{
		if ((unsigned char)(*filename)[i] < ' ' || strchr(badchars,(*filename)[i]))
		{
			sprintf(buf,"%%%02x",(unsigned char)(*filename)[i]);
			filename->replace(i,1,buf);
		}
	}
}

// use UTF-8 filenames directly, but unescape escaped forbidden characters
// by replacing occurrences of %xx (x being a lowercase hex digit) with the encoded character
void PosixFileSystemAccess::local2name(string* filename)
{
	char c;

	for (int i = filename->size()-2; i-- > 0; )
	{
		if ((*filename)[i] == '%' && islchex((*filename)[i+1]) && islchex((*filename)[i+2]))
		{
			c = (MegaClient::hexval((*filename)[i+1])<<4)+MegaClient::hexval((*filename)[i+2]);
			filename->replace(i,3,&c,1);
		}
	}
}

bool PosixFileSystemAccess::renamelocal(string* oldname, string* newname)
{
	return !rename(oldname->c_str(),newname->c_str());
}

bool PosixFileSystemAccess::copylocal(string* oldname, string* newname)
{
	int sfd, tfd;
	ssize_t t = -1;

	// Linux-specific - kernel 2.6.33+ required
	if ((sfd = open(oldname->c_str(),O_RDONLY|O_DIRECT)) >= 0)
	{
		if ((tfd = open(newname->c_str(),O_WRONLY|O_CREAT|O_TRUNC|O_DIRECT,0644)) >= 0)
		{
			while ((t = sendfile(tfd,sfd,NULL,1024*1024*1024)) > 0);
			close(tfd);
		}

		close(sfd);
	}

	return !t;
}

bool PosixFileSystemAccess::unlinklocal(string* name)
{
	return !unlink(name->c_str());
}

// FIXME: *** must delete subtree recursively ***
bool PosixFileSystemAccess::rmdirlocal(string* name)
{
	return !rmdir(name->c_str());
}

bool PosixFileSystemAccess::mkdirlocal(string* name)
{
	return !mkdir(name->c_str(),0700);
}

bool PosixFileSystemAccess::setmtimelocal(string* name, time_t mtime)
{
	struct utimbuf times = { mtime, mtime };
	
	return !utime(name->c_str(),&times);
}

bool PosixFileSystemAccess::chdirlocal(string* name)
{
	return !chdir(name->c_str());
}

void PosixFileSystemAccess::addnotify(LocalNode* l, string* path)
{
	int wd;

	wd = inotify_add_watch(notifyfd,path->c_str(),IN_CREATE|IN_DELETE|IN_MOVED_FROM|IN_MOVED_TO|IN_CLOSE_WRITE|IN_EXCL_UNLINK|IN_ONLYDIR);
	
	if (wd >= 0)
	{
		l->notifyhandle = (void*)(long)wd;
		wdnodes[wd] = l;
	}
}

void PosixFileSystemAccess::delnotify(LocalNode* l)
{
	if (wdnodes.erase((int)(long)l->notifyhandle)) inotify_rm_watch(notifyfd,(int)(long)l->notifyhandle);
}

// return next notified local name and corresponding parent node
bool PosixFileSystemAccess::notifynext(sync_list*, string* localname, LocalNode** localnodep)
{
	inotify_event* in;
	wdlocalnode_map::iterator it;

	for (;;)
	{
		if (notifyleft <= 0)
		{
			if ((notifyleft = read(notifyfd,notifybuf,sizeof notifybuf)) <= 0) return false;
			notifypos = 0;
		}

		in = (inotify_event*)(notifybuf+notifypos);

		notifyleft += notifypos;
		notifypos = in->name-notifybuf+in->len;
		notifyleft -= notifypos;

		if (in->mask & (IN_Q_OVERFLOW | IN_UNMOUNT))
		{
cout << "Notify Overflow" << endl;
			notifyerr = true;
			return false;
		}
		
		if (in->mask & (IN_CREATE|IN_DELETE|IN_MOVED_FROM|IN_MOVED_TO|IN_CLOSE_WRITE|IN_EXCL_UNLINK))
		{
			if ((in->mask & (IN_CREATE|IN_ISDIR)) != IN_CREATE)
			{
				if (*in->name) cout << "name=" << in->name << endl;
				else cout << "name=EMPTY!" << endl;

				it = wdnodes.find(in->wd);

				if (it != wdnodes.end())
				{
					*localname = in->name;
					*localnodep = it->second;

					return true;
				}
				else cout << "Unknown wd handle" << endl;
			}
			else cout << "File creation ignored" << endl;
		}
		else cout << "Unknown mask: " << in->mask << endl;			
	}
	
	return false;
}

// true if notifications were unreliable and/or a full rescan is required
bool PosixFileSystemAccess::notifyfailed()
{
	return notifyerr ? (notifyerr = false) || true : false;
}

bool PosixFileSystemAccess::localhidden(string*, string* filename)
{
	char c = *filename->c_str();
	return c == '.' || c == '~';
}

FileAccess* PosixFileSystemAccess::newfileaccess()
{
	return new PosixFileAccess();
}

DirAccess* PosixFileSystemAccess::newdiraccess()
{
	return new PosixDirAccess();
}

bool PosixDirAccess::dopen(string* path, FileAccess* f, bool doglob)
{
	if (doglob)
	{
		if (glob(path->c_str(),GLOB_NOSORT,NULL,&globbuf)) return false;

		globbing = true;
		globindex = 0;

		return true;
	}

	if (f)
	{
		dp = fdopendir(((PosixFileAccess*)f)->fd);
		((PosixFileAccess*)f)->fd = -1;
	}
	else dp = opendir(path->c_str());

	return dp != NULL;
}

bool PosixDirAccess::dnext(string* name, nodetype* type)
{
	if (globbing)
	{
		struct stat statbuf;

		while (globindex < globbuf.gl_pathc)
		{
			if (!stat(globbuf.gl_pathv[globindex++],&statbuf))
			{
				if (statbuf.st_mode & (S_IFREG | S_IFDIR))
				{
					*name = globbuf.gl_pathv[globindex-1];
					*type = (statbuf.st_mode & S_IFREG) ? FILENODE : FOLDERNODE;

					return true;
				}
			}
		}
	
		return false;
	}
	
	dirent* d;

	while ((d = readdir(dp)))
	{
		if ((d->d_type == DT_DIR || d->d_type == DT_REG) && (d->d_type != DT_DIR || *d->d_name != '.' || (d->d_name[1] && (d->d_name[1] != '.' || d->d_name[2]))))
		{
			*name = d->d_name;
			if (type) *type = d->d_type == DT_DIR ? FOLDERNODE : FILENODE;

			return true;
		}
	}

	return false;
}

PosixDirAccess::PosixDirAccess()
{
	dp = NULL;
	globbing = false;
}

PosixDirAccess::~PosixDirAccess()
{
	if (dp) closedir(dp);
	if (globbing) globfree(&globbuf);
}

// terminal handling
static tcflag_t oldlflag;
static cc_t oldvtime;
static struct termios term;

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

void term_echo(int echo)
{
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

// FIXME: UTF-8 compatibility
void read_pw_char(char* pw_buf, int pw_buf_size, int* pw_buf_pos, char** line)
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

int main()
{
	// instantiate app components: the callback processor (DemoApp),
	// the cURL HTTP I/O engine (CurlHttpIO) and the MegaClient itself
	client = new MegaClient(new DemoApp,new PosixWaiter,new CurlHttpIO,new PosixFileSystemAccess,new BdbAccess,"SDKSAMPLE");

	megacli();
}
