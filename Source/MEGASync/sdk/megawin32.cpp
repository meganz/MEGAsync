/*

MEGA SDK 2013-11-11 - sample application for the Win32 environment

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

#include <stdarg.h>
#include <fcntl.h>
#include <stdio.h>

#include <windows.h>
#include <winhttp.h>
#include <shellapi.h>

#include <time.h>
#include <conio.h>

#define USE_VARARGS
#define PREFER_STDARG
#include <readline/readline.h>

#include "mega.h"

#include "megacrypto.h"
#include "megaclient.h"
#include "megacli.h"
#include "megawin32.h"

WinWaiter::WinWaiter()
{
	DWORD dwMode;

	pGTC = (PGTC)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")),"GetTickCount64");
	
	if (!pGTC)
	{
		tickhigh = 0;
		prevt = 0;
	}
	
	if (!pGTC) cout << "Emulating GetTickCount64()" << endl;

	hWakeup[WAKEUP_CONSOLE] = GetStdHandle(STD_INPUT_HANDLE);

	GetConsoleMode(hWakeup[WAKEUP_CONSOLE],&dwMode);
	SetConsoleMode(hWakeup[WAKEUP_CONSOLE],dwMode & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
	FlushConsoleInputBuffer(hWakeup[WAKEUP_CONSOLE]);
}

void WinWaiter::init(dstime ds)
{
	maxds = ds;
}

// update monotonously increasing timestamp in deciseconds
dstime WinWaiter::getdstime()
{
	if (pGTC) return ds = pGTC()/100;
	else
	{
		// emulate GetTickCount64() on XP
		DWORD t = GetTickCount();
		
		if (t < prevt) tickhigh += 0x100000000;
		prevt = t;

		return (t+tickhigh)/100;
	}
}

// wait for events (sockets, timeout + application events)
// ds specifies the maximum amount of time to wait in deciseconds (or ~0 if no timeout scheduled)
int WinWaiter::wait()
{
	// only allow interaction of asynccallback() with the main process while waiting (because WinHTTP is threaded)
	if (pcsHTTP) LeaveCriticalSection(pcsHTTP);
	DWORD dwWaitResult = WaitForMultipleObjectsEx(sizeof hWakeup/sizeof *hWakeup,hWakeup,FALSE,maxds*100,TRUE);
	if (pcsHTTP) EnterCriticalSection(pcsHTTP);

	if (dwWaitResult == WAIT_OBJECT_0 || dwWaitResult == WAIT_TIMEOUT || dwWaitResult == WAIT_IO_COMPLETION) return NEEDEXEC;

	// FIXME: improve this gruesome nonblocking console read-simulating kludge
	if (_kbhit()) return HAVESTDIN;
	
	// this assumes that the user isn't typing too fast
	INPUT_RECORD ir[1024];
	DWORD dwNum;
	ReadConsoleInput(hWakeup[WAKEUP_CONSOLE],ir,1024,&dwNum);

	return 0;
}

void WinHttpIO::httpevent()
{
	SetEvent(hWakeupEvent);
}

void WinHttpIO::entercs()
{
	EnterCriticalSection(&csHTTP);
}

void WinHttpIO::leavecs()
{
	LeaveCriticalSection(&csHTTP);
}

// wake up from WinHttpIO events
void WinHttpIO::addevents(Waiter* w)
{
	WinWaiter* pw = (WinWaiter*)w;

	pw->hWakeup[WinWaiter::WAKEUP_HTTP] = hWakeupEvent;
	pw->pcsHTTP = &csHTTP;
}

// HttpIO implementation using WinHTTP
WinHttpIO::WinHttpIO()
{
    // create the session handle using the default settings.
    hSession = WinHttpOpen(L"MEGA Client Access Engine/1.0",WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,WINHTTP_NO_PROXY_NAME,WINHTTP_NO_PROXY_BYPASS,WINHTTP_FLAG_ASYNC);

	InitializeCriticalSection(&csHTTP);
	EnterCriticalSection(&csHTTP);

	hWakeupEvent = CreateEvent(NULL,FALSE,FALSE,NULL);

	completion = 0;
}

WinHttpIO::~WinHttpIO()
{
	WinHttpCloseHandle(hSession);
	LeaveCriticalSection(&csHTTP);
}

// handle WinHTTP callbacks (which can be in a worker thread context)
VOID CALLBACK WinHttpIO::asynccallback(HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength)
{
	WinHttpContext* httpctx = (WinHttpContext*)dwContext;
	WinHttpIO* httpio = (WinHttpIO*)httpctx->httpio;

	if (dwInternetStatus == WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING)
	{
if (httpctx->req)
{
	cout << "STATE ERROR - CLOSING UNCANCELED HTTPCTX" << endl;
	exit(0);
}
		delete httpctx;
		return;
	}

	httpio->entercs();

	HttpReq* req = httpctx->req;

	// FIXME: eliminate if not needed
	if (!req)
	{
cout << "STATE ERROR - EXECUTING CANCELED HTTPCTX hRequest=" << httpctx->hRequest << endl;
		httpio->leavecs();
		return;
	}

	switch (dwInternetStatus)
	{
		case WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE:
			{
				DWORD size = *(DWORD*)lpvStatusInformation;

				if (!size)
				{
					if (debug)
					{
						if (req->binary) cout << "[received " << req->bufpos << " bytes of raw data]" << endl;
						else cout << "Received: " << req->in.c_str() << endl;
					}

					req->status = req->httpstatus == 200 ? REQ_SUCCESS : REQ_FAILURE;
					httpio->httpevent();
				}
				else
				{
					char* ptr = (char*)req->reserveput((unsigned*)&size);

					if (!WinHttpReadData(hInternet,ptr,size,NULL)) httpio->cancel(req);
				}
			}

			httpio->httpevent();
			break;

		case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
			if (dwStatusInformationLength)
			{
				req->completeput(dwStatusInformationLength);

				if (!WinHttpQueryDataAvailable(httpctx->hRequest,NULL))
				{
					httpio->cancel(req);
					httpio->httpevent();
				}
			}
			break;

		case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:
			{
				DWORD statusCode = 0;
				DWORD statusCodeSize = sizeof(statusCode);

				if (!WinHttpQueryHeaders(((WinHttpContext*)req->httpiohandle)->hRequest,WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,WINHTTP_HEADER_NAME_BY_INDEX,&statusCode,&statusCodeSize,WINHTTP_NO_HEADER_INDEX))
				{
					httpio->cancel(req);
					httpio->httpevent();
				}
				else
				{
					req->httpstatus = statusCode;

					if (!WinHttpQueryDataAvailable(((WinHttpContext*)req->httpiohandle)->hRequest,NULL))
					{
						httpio->cancel(req);
						httpio->httpevent();
					}
				}
			}
			break;

		case WINHTTP_CALLBACK_STATUS_SECURE_FAILURE:
		case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
			httpio->cancel(req);
			httpio->httpevent();
			break;

		case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
		case WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE:
			if (httpctx->postpos < httpctx->postlen)
			{
				unsigned t = httpctx->postlen-httpctx->postpos;

				if (t > HTTP_POST_CHUNK_SIZE) t = HTTP_POST_CHUNK_SIZE;

				if (!WinHttpWriteData(httpctx->hRequest,(LPVOID)(httpctx->postdata+httpctx->postpos),t,NULL)) req->httpio->cancel(req);

				httpctx->postpos += t;
				httpio->httpevent();
			}
			else
			{
				if (!WinHttpReceiveResponse(httpctx->hRequest,NULL))
				{
					httpio->cancel(req);
					httpio->httpevent();
				}
			}
			break;
	}

	httpio->leavecs();
}

// POST request to URL
void WinHttpIO::post(HttpReq* req, const char* data, unsigned len)
{
	if (debug)
	{
		cout << "POST target URL: " << req->posturl << endl;

		if (req->binary) cout << "[sending " << req->out->size() << " bytes of raw data]" << endl;
		else cout << "Sending: " << *req->out << endl;
	}

	WinHttpContext* httpctx;

	WCHAR szURL[8192];
	WCHAR szHost[256];
	URL_COMPONENTS urlComp = { sizeof urlComp };

	urlComp.lpszHostName = szHost;
	urlComp.dwHostNameLength = sizeof szHost/sizeof *szHost;
    urlComp.dwUrlPathLength = (DWORD)-1;
    urlComp.dwSchemeLength = (DWORD)-1;

	httpctx = new WinHttpContext;

	httpctx->httpio = this;
	httpctx->req = req;
	req->httpiohandle = (void*)httpctx;

    if (MultiByteToWideChar(CP_UTF8,0,req->posturl.c_str(),-1,szURL,sizeof szURL/sizeof *szURL) && WinHttpCrackUrl(szURL,0,0,&urlComp))
    {
		if ((httpctx->hConnect = WinHttpConnect(hSession,szHost,urlComp.nPort,0)))
	    {
			httpctx->hRequest = WinHttpOpenRequest(httpctx->hConnect,L"POST",urlComp.lpszUrlPath,NULL,WINHTTP_NO_REFERER,WINHTTP_DEFAULT_ACCEPT_TYPES,(urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0);
			
			if (httpctx->hRequest)
			{
				WinHttpSetTimeouts(httpctx->hRequest,0,20000,20000,1800000);

				WinHttpSetStatusCallback(httpctx->hRequest,asynccallback,WINHTTP_CALLBACK_FLAG_DATA_AVAILABLE | WINHTTP_CALLBACK_FLAG_READ_COMPLETE | WINHTTP_CALLBACK_FLAG_HEADERS_AVAILABLE | WINHTTP_CALLBACK_FLAG_REQUEST_ERROR | WINHTTP_CALLBACK_FLAG_SECURE_FAILURE | WINHTTP_CALLBACK_FLAG_SENDREQUEST_COMPLETE | WINHTTP_CALLBACK_FLAG_WRITE_COMPLETE | WINHTTP_CALLBACK_FLAG_HANDLES ,0);

				LPCWSTR pwszHeaders = REQ_JSON ? L"Content-Type: application/json" : L"Content-Type: application/octet-stream";

				// data is sent in HTTP_POST_CHUNK_SIZE instalments
				httpctx->postlen = data ? len : req->out->size();
				httpctx->postdata = data ? data : req->out->data();
				httpctx->postpos = (httpctx->postlen < HTTP_POST_CHUNK_SIZE) ? httpctx->postlen : HTTP_POST_CHUNK_SIZE;
				
				if (WinHttpSendRequest(httpctx->hRequest,pwszHeaders,wcslen(pwszHeaders),(LPVOID)httpctx->postdata,httpctx->postpos,httpctx->postlen,(DWORD_PTR)httpctx))
				{
					req->status = REQ_INFLIGHT;
					return;
				}
				else if (debug) cout << "WinHTTPSendRequest() failed" << endl;
			}
			else if (debug) cout << "WinHttpOpenRequest() failed" << endl;
		}
		else
		{
			httpctx->hRequest = NULL;
			if (debug) cout << "WinHttpConnect() failed" << endl;
		}
	}
	else
	{
		httpctx->hRequest = NULL;
		httpctx->hConnect = NULL;		
		if (debug) cout << "WinHttpCrackUrl(" << req->posturl << ") failed" << endl;
	}

	req->status = REQ_FAILURE;
}

// cancel pending HTTP request
void WinHttpIO::cancel(HttpReq* req)
{
	WinHttpContext* httpctx;
	
	if ((httpctx = (WinHttpContext*)req->httpiohandle))
	{
		httpctx->req = NULL;

		req->httpstatus = 0;
		req->status = REQ_FAILURE;		
		req->httpiohandle = NULL;

		if (httpctx->hRequest) WinHttpCloseHandle(httpctx->hRequest);
	}
}

// real-time progress information on POST data
m_off_t WinHttpIO::postpos(void* handle)
{
	return ((WinHttpContext*)handle)->postpos;
}

// process events
bool WinHttpIO::doio()
{
	bool done;

	done = completion;
	completion = false;

	return done;
}

WinFileAccess::WinFileAccess()
{
	hFile = INVALID_HANDLE_VALUE;
	hFind = INVALID_HANDLE_VALUE;
}

WinFileAccess::~WinFileAccess()
{
	if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
	else if (hFind != INVALID_HANDLE_VALUE) FindClose(hFind);
}

bool WinFileAccess::fread(string* dst, unsigned len, unsigned pad, m_off_t pos)
{
	DWORD dwRead;

	if (!SetFilePointerEx(hFile,*(LARGE_INTEGER*)&pos,NULL,FILE_BEGIN)) return 0;

	dst->resize(len+pad);

	memset((char*)dst->data(),0,len+pad);

	if (ReadFile(hFile,(LPVOID)dst->data(),(DWORD)len,&dwRead,NULL) && dwRead == len)
	{
		memset((char*)dst->data()+len,0,pad);
		return 1;
	}

	return 0;
}

bool WinFileAccess::frawread(byte* dst, unsigned len, m_off_t pos)
{
	DWORD dwRead;

	if (!SetFilePointerEx(hFile,*(LARGE_INTEGER*)&pos,NULL,FILE_BEGIN)) return 0;

	return ReadFile(hFile,(LPVOID)dst,(DWORD)len,&dwRead,NULL) && dwRead == len;
}

bool WinFileAccess::fwrite(const byte* data, unsigned len, m_off_t pos)
{
	DWORD dwWritten;

	if (!SetFilePointerEx(hFile,*(LARGE_INTEGER*)&pos,NULL,FILE_BEGIN)) return 0;

	return WriteFile(hFile,(LPCVOID)data,(DWORD)len,&dwWritten,NULL) && dwWritten == len;
}

time_t FileTime_to_POSIX(FILETIME* ft)
{
	// takes the last modified date
	LARGE_INTEGER date, adjust;

	date.HighPart = ft->dwHighDateTime;
	date.LowPart = ft->dwLowDateTime;

	// 100-nanoseconds = milliseconds*10000
	adjust.QuadPart = 11644473600000*10000;

	// removes the diff between 1970 and 1601
	date.QuadPart -= adjust.QuadPart;

	// converts back from 100-nanoseconds to seconds
	return date.QuadPart/10000000;
}

// emulates Linux open-directory-as-file semantics
// FIXME: use CreateFileW() to open the directory instead of FindFirstFileW()?
bool WinFileAccess::fopen(string* name, bool read, bool write)
{
	name->append("",1);
	hFile = CreateFileW((LPCWSTR)name->data(),read ? GENERIC_READ : GENERIC_WRITE,FILE_SHARE_WRITE | FILE_SHARE_READ,NULL,read ? OPEN_EXISTING : OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	name->resize(name->size()-1);

	// (race condition between CreateFile() and FindFirstFile() possible - fixable with the current Win32 API?)

	if (hFile == INVALID_HANDLE_VALUE)
	{
		name->append((char*)L"\\*",5);

		hFind = FindFirstFileW((LPCWSTR)name->data(),&ffd);
		name->resize(name->size()-5);
	}

	if (hFind != INVALID_HANDLE_VALUE)
	{
		type = FOLDERNODE;
		return 1;
	}
	
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	if (read)
	{
		BY_HANDLE_FILE_INFORMATION fi;

		if (!GetFileInformationByHandle(hFile,&fi)) return 0;

		size = ((m_off_t)fi.nFileSizeHigh << 32)+(m_off_t)fi.nFileSizeLow;

		mtime = FileTime_to_POSIX(&fi.ftLastWriteTime);
	}
	else if (!GetFileSizeEx(hFile,(LARGE_INTEGER*)&size)) return 0;

	type = FILENODE;
	
	return 1;
}

WinFileSystemAccess::WinFileSystemAccess()
{
	pendingevents = 0;
	notifyerr = false;
	localseparator.assign((char*)L"\\",sizeof(wchar_t));
}

WinFileSystemAccess::~WinFileSystemAccess()
{
}

// wake up from filesystem updates
void WinFileSystemAccess::addevents(Waiter* w)
{
	// overlapped completion wakes up WaitForMultipleObjectsEx()
	((WinWaiter*)w)->pendingfsevents = pendingevents;
}

// generate unique local filename in the same fs as relatedpath
void WinFileSystemAccess::tmpnamelocal(string* filename, string* relatedpath)
{
	static unsigned tmpindex;
	char buf[256];
	
	sprintf(buf,".tmp.%lu.%u.mega",GetCurrentProcessId(),tmpindex++);
	*filename = buf;
	name2local(filename);
}

void WinFileSystemAccess::local2path(string* local, string* path)
{
	path->resize((local->size()+1)*4/sizeof(wchar_t));

	path->resize(WideCharToMultiByte(CP_UTF8,0,(wchar_t*)local->data(),local->size()/sizeof(wchar_t),(char*)path->data(),path->size()+1,NULL,NULL));
}

// use UTF-8 filenames directly, but escape forbidden characters
void WinFileSystemAccess::name2local(string* filename, const char* badchars)
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
	
	string t = *filename;
	
	// make space for the worst case
	filename->resize((t.size()+1)*sizeof(wchar_t));

	// resize to actual result
	filename->resize(sizeof(wchar_t)*(MultiByteToWideChar(CP_UTF8,0,t.c_str(),-1,(wchar_t*)filename->data(),filename->size()/sizeof(wchar_t)+1)-1));
}

// unescape escaped forbidden characters
// by replacing occurrences of %xx (x being a lowercase hex digit) with the encoded character
void WinFileSystemAccess::local2name(string* filename)
{
	char c;
	string t = *filename;
	
	filename->resize((t.size()+1)*4/sizeof(wchar_t));

	filename->resize(WideCharToMultiByte(CP_UTF8,0,(wchar_t*)t.data(),t.size()/sizeof(wchar_t),(char*)filename->data(),filename->size()+1,NULL,NULL));

	for (int i = filename->size()-2; i-- > 0; )
	{
		if ((*filename)[i] == '%' && islchex((*filename)[i+1]) && islchex((*filename)[i+2]))
		{
			c = (MegaClient::hexval((*filename)[i+1])<<4)+MegaClient::hexval((*filename)[i+2]);
			filename->replace(i,3,&c,1);
		}
	}
}

bool WinFileSystemAccess::renamelocal(string* oldname, string* newname)
{
	oldname->append("",1);
	newname->append("",1);
	bool r = !!MoveFileExW((LPCWSTR)oldname->data(),(LPCWSTR)newname->data(),MOVEFILE_REPLACE_EXISTING);
	newname->resize(newname->size()-1);
	oldname->resize(oldname->size()-1);

	return r;
}

bool WinFileSystemAccess::copylocal(string* oldname, string* newname)
{
	oldname->append("",1);
	newname->append("",1);
	bool r = !!CopyFileW((LPCWSTR)oldname->data(),(LPCWSTR)newname->data(),FALSE);
	newname->resize(newname->size()-1);
	oldname->resize(oldname->size()-1);
	
	return r;
}

static bool movetotrash(string* name)
{
	name->append("",1);
	string tmpname((MAX_PATH+1)*sizeof(wchar_t),0);
	
	int r, rr;

	r = tmpname.size()/sizeof(wchar_t);

	rr = GetFullPathNameW((LPCWSTR)name->data(),r,(LPWSTR)tmpname.data(),NULL);

	if (rr >= r)
	{
		tmpname.resize(rr*sizeof(wchar_t));
		rr = GetFullPathNameW((LPCWSTR)name->data(),rr,(LPWSTR)tmpname.data(),NULL);
	}

	if (!rr) return false;
	
	tmpname.append("\0\0",3);

	SHFILEOPSTRUCTW fileop;
	fileop.hwnd = NULL;
	fileop.wFunc = FO_DELETE;
	fileop.pFrom = (LPCWSTR)tmpname.data();
	fileop.pTo = NULL;
	fileop.fFlags = FOF_NOCONFIRMATION|FOF_SILENT|FOF_ALLOWUNDO|FOF_NOERRORUI|FOF_NOCONFIRMMKDIR;
	fileop.fAnyOperationsAborted = FALSE;
	fileop.lpszProgressTitle = NULL;
	fileop.hNameMappings = NULL;

	return !SHFileOperationW(&fileop);

	// FIXME: fall back to DeleteFile()/RemoveDirectory() if SHFileOperation() fails, e.g. because of excessive path length
	//int r = DeleteFileW((LPCWSTR)name->data());
	//int r = RemoveDirectoryW((LPCWSTR)name->data());
}

bool WinFileSystemAccess::unlinklocal(string* name)
{
	return movetotrash(name);
}

bool WinFileSystemAccess::rmdirlocal(string* name)
{
	return movetotrash(name);
}

bool WinFileSystemAccess::mkdirlocal(string* name)
{
	name->append("",1);
	int r = CreateDirectoryW((LPCWSTR)name->data(),NULL);
	name->resize(name->size()-1);
	
	return r;
}

bool WinFileSystemAccess::setmtimelocal(string* name, time_t mtime)
{
	FILETIME lwt;
	LONGLONG ll;
	HANDLE hFile;

	name->append("",1);
	hFile = CreateFileW((LPCWSTR)name->data(),FILE_WRITE_ATTRIBUTES,FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
	name->resize(name->size()-1);

	if (hFile == INVALID_HANDLE_VALUE) return 0;

	ll = Int32x32To64(mtime,10000000)+116444736000000000;
	lwt.dwLowDateTime = (DWORD)ll;
	lwt.dwHighDateTime = ll >> 32;

	int r = SetFileTime(hFile,NULL,NULL,&lwt);

	CloseHandle(hFile);

	return r;
}

bool WinFileSystemAccess::chdirlocal(string* name)
{
	name->append("",1);
	int r = SetCurrentDirectoryW((LPCWSTR)name->data());
	name->resize(name->size()-1);

	return r;
}

VOID CALLBACK WinDirNotify::completion(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
{
	if (dwErrorCode != ERROR_OPERATION_ABORTED && dwNumberOfBytesTransfered) ((WinDirNotify*)lpOverlapped->hEvent)->process(dwNumberOfBytesTransfered);
}

void WinDirNotify::process(DWORD dwNumberOfBytesTransfered)
{
	// Can't use sizeof(FILE_NOTIFY_INFORMATION) because
	// the structure is padded to 16 bytes.
	assert(dwNumberOfBytesTransfered >= offsetof(FILE_NOTIFY_INFORMATION, FileName) + sizeof(WCHAR));

	char* ptr = (char*)notifybuf[active].data();

	active ^= 1;

	readchanges();

	// we trust the OS to always return conformant data
	for (;;)
	{
		FILE_NOTIFY_INFORMATION* fni = (FILE_NOTIFY_INFORMATION*)ptr;

		// FIXME: perform GetLongPathName() here - the 8.3 short name plague still plagues Windows, apparently
#if 0
		fullpath.assign((char*)fni->FileName,fni->FileNameLength);
		
		if (*(wchar_t*)(fullpath.data()+fni->FileNameLength-sizeof(wchar_t)) != *(wchar_t*)fsaccess->localseparator.data()) fullpath.insert(0,fsaccess->localseparator);
		fullpath.insert(0,basepath);
		
		LPCWSTR wszFilename = PathFindFileNameW((LPCWSTR)fullpath.data());
		int len = wcslen(wszFilename);
		
		// The maximum length of an 8.3 filename is twelve, including the dot.
		if (len <= 12 && wcschr(wszFilename, '~'))
		{
			// Convert to the long filename form. Unfortunately, this
			// does not work for deletions, so it's an imperfect fix.
			wchar_t wbuf[MAX_PATH];
			if (GetLongPathName((LPCWSTR)fullpath.data(),wbuf,_countof(wbuf)) > 0) fullpath.assign(wbuf,wcslen(wbuf)*sizeof(wchar_t));
		}
#endif

		// FIXME: prevent unnecessary copying
		notifyq.insert(notifyq.end(),string((char*)fni->FileName,fni->FileNameLength));

		if (!fni->NextEntryOffset) break;

		ptr += fni->NextEntryOffset;
	}
}

void WinDirNotify::readchanges()
{
	if (!ReadDirectoryChangesW(hDirectory,(LPVOID)notifybuf[active].data(),notifybuf[active].size(),TRUE,FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_CREATION,&dwBytes,&overlapped,completion))
	{
		if (GetLastError() == ERROR_NOTIFY_ENUM_DIR) fsaccess->notifyerr = true;
		cout << "ReadDirectoryChangesW() failed: " << GetLastError() << endl;
	}
}

WinDirNotify::WinDirNotify(WinFileSystemAccess* cfsaccess, LocalNode* clocalnode, string* cpath)
{
	fsaccess = cfsaccess;
	basepath = *cpath;
	localnode = clocalnode;

	ZeroMemory(&overlapped,sizeof(overlapped));

	overlapped.hEvent = this;

	active = 0;
	
	notifybuf[0].resize(65536);
	notifybuf[1].resize(65536);

	if ((hDirectory = CreateFileW((LPCWSTR)basepath.data(),FILE_LIST_DIRECTORY,FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,NULL)) != INVALID_HANDLE_VALUE) readchanges();
	else if (debug) cout << "Creation of directory handle failed for " << basepath << ": " << GetLastError() << endl;
}

WinDirNotify::~WinDirNotify()
{
	if (hDirectory != INVALID_HANDLE_VALUE) CloseHandle(hDirectory);
}

void WinFileSystemAccess::addnotify(LocalNode* l, string* name)
{
	// Win32 directory change notification watches entire subtrees, hence we only care about the toplevel node
	if (!l->parent) l->notifyhandle = (void*)new WinDirNotify(this,l,name);
}

void WinFileSystemAccess::delnotify(LocalNode* l)
{
	if (!l->parent) delete (WinDirNotify*)l->notifyhandle;
}

// return next notified local name and corresponding parent node
bool WinFileSystemAccess::notifynext(sync_list* syncs, string* localname, LocalNode** localnodep)
{
	WinDirNotify* dn;
	LocalNode* ln;
	string name;
	localnode_map::iterator lit;

	pendingevents = 0;
	
	for (sync_list::iterator it = syncs->begin(); it != syncs->end(); it++)
	{
		if ((*it)->rootlocal && (ln = (LocalNode*)(*it)->rootlocal) && (dn = (WinDirNotify*)(*it)->rootlocal->notifyhandle))
		{
			if (dn->notifyq.size())
			{
				// find parent node of first stored notified path
				dn->notifyq[0].append("",1);
				
				wchar_t* wbase = (wchar_t*)dn->notifyq[0].data();
				
				// walk path components
				while (*wbase)
				{
					wchar_t* wptr = wbase;
					
					while (*++wptr && *wptr != '\\');

					if (*wptr)
					{
						name.assign((char*)wbase,(wptr-wbase)*sizeof(wchar_t));

						if ((lit = ln->children.find(&name)) != ln->children.end())
						{
							ln = lit->second;
							wbase = wptr+1;
						}
						else
						{
							// we don't have that child directory yet: defer until it has been created by the processing of previous updates
							pendingevents = 1;
							break;
						}
					}
					else
					{
						localname->assign((char*)wbase,(wptr-wbase)*sizeof(wchar_t));
						*localnodep = ln;
						
						dn->notifyq.pop_front();
						
						return true;
					}
				}
			}
		}
	}
	
	return false;
}

// true if notifications were unreliable and/or a full rescan is required
bool WinFileSystemAccess::notifyfailed()
{
	return notifyerr ? (notifyerr = false) || true : false;
}

// returns true for files that are not supposed to be synced
bool WinFileSystemAccess::localhidden(string*, string* filename)
{
	wchar_t c = *(wchar_t*)filename->data();

	return c == '.' || c == '~';
}

FileAccess* WinFileSystemAccess::newfileaccess()
{
	return new WinFileAccess();
}

DirAccess* WinFileSystemAccess::newdiraccess()
{
	return new WinDirAccess();
}

bool WinDirAccess::dopen(string* name, FileAccess* f, bool glob)
{
	if (f)
	{
		if ((hFind = ((WinFileAccess*)f)->hFind) != INVALID_HANDLE_VALUE)
		{
			ffd = ((WinFileAccess*)f)->ffd;
			((WinFileAccess*)f)->hFind = INVALID_HANDLE_VALUE;
		}
	}
	else
	{
		name->append("",1);
		hFind = FindFirstFileW((LPCWSTR)name->data(),&ffd);
		
		if (glob)
		{
			wchar_t* bp = (wchar_t*)name->data();

			// store base path for glob() emulation
			int p = wcslen(bp);

			while (p--) if (bp[p] == '/' || bp[p] == '\\') break;
		
			if (p >= 0) globbase.assign((char*)bp,(p+1)*sizeof(wchar_t));
			else globbase.clear();
		}
		
		name->resize(name->size()-1);
	}

	if (!(ffdvalid = (hFind != INVALID_HANDLE_VALUE))) return false;

	return true;
}

bool WinDirAccess::dnext(string* name, nodetype* type)
{
	for (;;)
	{
		if (ffdvalid && !(ffd.dwFileAttributes & (FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_OFFLINE | FILE_ATTRIBUTE_HIDDEN || FILE_ATTRIBUTE_HIDDEN)) && (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || *ffd.cFileName != '.' || (ffd.cFileName[1] && (ffd.cFileName[1] != '.' || ffd.cFileName[2]))))
		{
			name->assign((char*)ffd.cFileName,sizeof(wchar_t)*wcslen(ffd.cFileName));
			name->insert(0,globbase);
			
			if (type) *type = (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? FOLDERNODE : FILENODE;

			ffdvalid = false;
			return true;
		}

		
		if (!(ffdvalid = FindNextFileW(hFind,&ffd) != 0)) return false;
	}
}

WinDirAccess::WinDirAccess()
{
	ffdvalid = false;
	hFind = INVALID_HANDLE_VALUE;
}

WinDirAccess::~WinDirAccess()
{
	if (hFind != INVALID_HANDLE_VALUE) FindClose(hFind);
}

void term_init()
{
	// set up terminal
}

void term_restore()
{
	// restore startup config
}

void read_pw_char(char* pw_buf, int pw_buf_size, int* pw_buf_pos, char** line)
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

void term_echo(int echo)
{
	HANDLE hCon = GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode;

	GetConsoleMode(hCon,&mode);
	if (echo) mode |= ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT;
	else mode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
	SetConsoleMode(hCon,mode);
}

int main()
{
	// instantiate app components: the callback processor (DemoApp),
	// the HTTP I/O engine (WinHttpIO) and the MegaClient itself
	client = new MegaClient(new DemoApp,new WinWaiter,new WinHttpIO,new WinFileSystemAccess,NULL,"SDKSAMPLE");

	megacli();
}
