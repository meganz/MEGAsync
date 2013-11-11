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

#include <stdarg.h>
#include <fcntl.h>
#include <stdio.h>

#include <windows.h>
#include <winhttp.h>
#include <wchar.h>

#include <time.h>
#include <conio.h>

#define USE_VARARGS
#define PREFER_STDARG
//#include <readline/readline.h>

#include "mega.h"

#include "megacrypto.h"
#include "megaclient.h"
#include "megacli.h"
#include "megaposix.h"
#include "megawin32.h"
#include "megabdb.h"

int debug = 0;

// HttpIO implementation using WinHTTP
WinHttpIO::WinHttpIO()
{
	InitializeCriticalSection(&csHTTP);
	EnterCriticalSection(&csHTTP);

    // create the session handle using the default settings.
    hSession = WinHttpOpen(L"MEGA Client Access Engine/1.0",WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,WINHTTP_NO_PROXY_NAME,WINHTTP_NO_PROXY_BYPASS,WINHTTP_FLAG_ASYNC);

	hWakeup[0] = CreateEvent(NULL,FALSE,FALSE,NULL);
	hWakeup[1] = GetStdHandle(STD_INPUT_HANDLE);
	DWORD dwMode;

	GetConsoleMode(hWakeup[1],&dwMode);
	SetConsoleMode(hWakeup[1],dwMode & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
	FlushConsoleInputBuffer(hWakeup[1]);

	completion = 0;
}

WinHttpIO::~WinHttpIO()
{
	WinHttpCloseHandle(hSession);
	LeaveCriticalSection(&csHTTP);
}

// update monotonously increasing timestamp in deciseconds
void WinHttpIO::updatedstime()
{
	ds = (dstime)(GetTickCount()/100);	// FIXME: Use GetTickCount64() instead
}

// handle WinHTTP callbacks (which can be in a worker thread context)
static void CALLBACK AsyncCallback(HINTERNET hInternet, DWORD_PTR dwContext,  DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength)
{
	HttpReq* req = (HttpReq*)dwContext;

	EnterCriticalSection(&((WinHttpIO*)req->httpio)->csHTTP);

	switch (dwInternetStatus)
	{
		case WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE:
			{
				DWORD size =  *(DWORD*)lpvStatusInformation;

				if (!size)
				{
                    if (debug)
					{
						if (req->binary) cout << "[received " << req->bufpos << " bytes of raw data]" << endl;
						else cout << "Received: " << req->in.c_str() << endl;
					}

					req->status = req->httpstatus == 200 ? REQ_SUCCESS : REQ_FAILURE;

					SetEvent(((WinHttpIO*)req->httpio)->hWakeup[0]);
				}
				else
				{
					char* ptr = (char*)req->reserveput((unsigned*)&size);

					if (!WinHttpReadData(hInternet,ptr,size,NULL)) req->httpio->cancel(req);
				}
			}
			SetEvent(((WinHttpIO*)req->httpio)->hWakeup[0]);
			break;

		case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
			if (dwStatusInformationLength)
			{
				req->completeput(dwStatusInformationLength);

				if (!WinHttpQueryDataAvailable(((WinHttpContext*)req->httpiohandle)->hRequest,NULL))
				{
					req->httpio->cancel(req);
					SetEvent(((WinHttpIO*)req->httpio)->hWakeup[0]);
				}
			}
			break;

		case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:
			{
				DWORD statusCode = 0;
				DWORD statusCodeSize = sizeof(statusCode);

				if (!WinHttpQueryHeaders(((WinHttpContext*)req->httpiohandle)->hRequest,WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,WINHTTP_HEADER_NAME_BY_INDEX,&statusCode,&statusCodeSize,WINHTTP_NO_HEADER_INDEX))
				{
					req->httpio->cancel(req);
					SetEvent(((WinHttpIO*)req->httpio)->hWakeup[0]);
				}
				else
				{
					req->httpstatus = statusCode;

					if (!WinHttpQueryDataAvailable(((WinHttpContext*)req->httpiohandle)->hRequest,NULL))
					{
						req->httpio->cancel(req);
						SetEvent(((WinHttpIO*)req->httpio)->hWakeup[0]);
					}
				}
			}
			break;

		case WINHTTP_CALLBACK_STATUS_SECURE_FAILURE:
		case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
			req->httpio->cancel(req);
			SetEvent(((WinHttpIO*)req->httpio)->hWakeup[0]);
			break;

		case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
            if ((WinHttpContext*)req->httpiohandle && WinHttpReceiveResponse(((WinHttpContext*)req->httpiohandle)->hRequest,NULL) == FALSE)
            {
				req->httpio->cancel(req);
				SetEvent(((WinHttpIO*)req->httpio)->hWakeup[0]);
            }
	}

	LeaveCriticalSection(&((WinHttpIO*)req->httpio)->csHTTP);
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

	WinHttpContext* cpContext = new WinHttpContext;
	WCHAR szHost[256];
	URL_COMPONENTS urlComp;

	req->httpiohandle = (void*)cpContext;

	ZeroMemory(cpContext,sizeof(WinHttpContext));
	ZeroMemory(&urlComp,sizeof urlComp);
	urlComp.dwStructSize = sizeof urlComp;

	urlComp.lpszHostName = szHost;
	urlComp.dwHostNameLength = sizeof szHost/sizeof *szHost;
    urlComp.dwUrlPathLength = (DWORD)-1;
    urlComp.dwSchemeLength = (DWORD)-1;

    // Crack the URL.
	WCHAR szURL[1024];

    if (MultiByteToWideChar(CP_UTF8,0,req->posturl.c_str(),-1,szURL,sizeof szURL/sizeof *szURL) && WinHttpCrackUrl(szURL,0,0,&urlComp))
    {
		if ((cpContext->hConnect = WinHttpConnect(hSession,szHost,urlComp.nPort,0)))
	    {
			cpContext->hRequest = WinHttpOpenRequest(cpContext->hConnect,L"POST", urlComp.lpszUrlPath,NULL,WINHTTP_NO_REFERER,WINHTTP_DEFAULT_ACCEPT_TYPES,(urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0);

			if (cpContext->hRequest)
			{
				WinHttpSetStatusCallback(cpContext->hRequest,AsyncCallback,WINHTTP_CALLBACK_FLAG_DATA_AVAILABLE | WINHTTP_CALLBACK_FLAG_READ_COMPLETE | WINHTTP_CALLBACK_FLAG_HEADERS_AVAILABLE | WINHTTP_CALLBACK_FLAG_REQUEST_ERROR | WINHTTP_CALLBACK_FLAG_SECURE_FAILURE | WINHTTP_CALLBACK_FLAG_SENDREQUEST_COMPLETE,0);

				LPCWSTR pwszHeaders = REQ_JSON ? L"Content-Type: application/json" : L"Content-Type: application/octet-stream";

				if (WinHttpSendRequest(cpContext->hRequest,pwszHeaders, wcslen(pwszHeaders),(LPVOID)(data ? data : req->out->data()), data ? len : req->out->size(), data ? len : req->out->size(),(DWORD_PTR)req))
				{
				   req->status = REQ_INFLIGHT;
				   return;
				}
				else if (debug) cout << "WinHTTPSendRequest() failed" << endl;
			}
			else if (debug) cout << "WinHttpOpenRequest() failed" << endl;
		}
		else if (debug) cout << "WinHttpConnect() failed" << endl;
	}
	else if (debug) cout << "WinHttpCrackUrl(" << req->posturl << ") failed" << endl;

	req->status = REQ_FAILURE;
}

// cancel pending HTTP request
void WinHttpIO::cancel(HttpReq* req)
{
	if (req->httpiohandle)
	{
		WinHttpContext* cpContext = (WinHttpContext*)req->httpiohandle;

		if (cpContext->hRequest)
		{
			WinHttpSetStatusCallback(cpContext->hRequest,
					NULL,
					0,
					0);

			WinHttpCloseHandle(cpContext->hRequest);
		}

		if (cpContext->hConnect) WinHttpCloseHandle(cpContext->hConnect);

		req->httpstatus = 0;
		req->status = REQ_FAILURE;

		delete (WinHttpContext*)req->httpiohandle;
		req->httpiohandle = NULL;
	}
}

// real-time progress information on POST data
m_off_t WinHttpIO::postpos(void* handle)
{
	return 0;
}

// process events
int WinHttpIO::doio()
{
	int done;

	done = completion;
	completion = 0;

	return done;
}

// wait for events (sockets, timeout + application events)
// ds specifies the maximum amount of time to wait in deciseconds (or ~0 if no timeout scheduled)
void WinHttpIO::waitio(dstime maxds)
{
	LeaveCriticalSection(&csHTTP);
	DWORD dwWaitResult = WaitForMultipleObjects(sizeof hWakeup/sizeof *hWakeup,hWakeup,FALSE,maxds*100);
	EnterCriticalSection(&csHTTP);
/*
    if (dwWaitResult == WAIT_OBJECT_0 || dwWaitResult == WAIT_TIMEOUT) redisplay = 1;
	else
	{
		// FIXME: improve this gruesome nonblocking console read-simulating kludge
        if (_kbhit()) rl_callback_read_char();
        else
		{
			// this assumes that the user isn't typing too fast
			INPUT_RECORD ir[1024];
			DWORD dwNum;
			ReadConsoleInput(hWakeup[1],ir,1024,&dwNum);
		}
    }*/
}

WinFileAccess::WinFileAccess()
{
	hFile = INVALID_HANDLE_VALUE;
}

WinFileAccess::~WinFileAccess()
{
	if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
}

int WinFileAccess::fread(string* dst, unsigned len, unsigned pad, m_off_t pos)
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

int WinFileAccess::fwrite(const byte* data, unsigned len, m_off_t pos)
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

int WinFileAccess::fopen(const char* f, int read, int write)
{
	WCHAR wszFileName[MAX_PATH+1];

    if (!MultiByteToWideChar(CP_UTF8,0,f,-1,wszFileName,sizeof wszFileName/sizeof *wszFileName)) return 0;

	hFile = CreateFileW(wszFileName,read ? GENERIC_ALL : GENERIC_WRITE,FILE_SHARE_WRITE | FILE_SHARE_READ,NULL,read ? OPEN_EXISTING : OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);

	if (hFile == INVALID_HANDLE_VALUE) return 0;

	if (read)
	{
		BY_HANDLE_FILE_INFORMATION fi;

		if (!GetFileInformationByHandle(hFile,&fi)) return 0;

		size = ((m_off_t)fi.nFileSizeHigh << 32)+(m_off_t)fi.nFileSizeLow;

		mtime = FileTime_to_POSIX(&fi.ftLastWriteTime);
	}
	else if (!GetFileSizeEx(hFile,(LARGE_INTEGER*)&size)) return 0;

	return 1;
}

int rename_file(const char* oldname, const char* newname)
{
	WCHAR wszOldFileName[MAX_PATH+1], wszNewFileName[MAX_PATH+1];

	if (!MultiByteToWideChar(CP_UTF8,0,oldname,-1,wszOldFileName,sizeof wszOldFileName/sizeof *wszOldFileName)) return 0;
	if (!MultiByteToWideChar(CP_UTF8,0,newname,-1,wszNewFileName,sizeof wszNewFileName/sizeof *wszNewFileName)) return 0;

	return MoveFileExW(wszOldFileName,wszNewFileName,MOVEFILE_REPLACE_EXISTING);
}

int unlink_file(const char* name)
{
	WCHAR wszFileName[MAX_PATH+1];

    if (!MultiByteToWideChar(CP_UTF8,0,name,-1,wszFileName,sizeof wszFileName/sizeof *wszFileName)) return 0;

	return DeleteFileW(wszFileName);
}

int change_dir(const char* name)
{
	WCHAR wszPath[MAX_PATH+1];

    if (!MultiByteToWideChar(CP_UTF8,0,name,-1,wszPath,sizeof wszPath/sizeof *wszPath)) return 0;

	return _wchdir(wszPath);
}

FileAccess* DemoApp::newfile()
{
	return new WinFileAccess();
}

int globenqueue(const char* path, const char* newname, handle target,  const char* targetuser)
{
	WIN32_FIND_DATAW FindFileData;
	int pathlen = strlen(path)+1;
	WCHAR* szPath = new WCHAR[pathlen];
	char utf8buf[MAX_PATH*4+1];
	HANDLE hFindFile;
	int total = 0;

    if (MultiByteToWideChar(CP_UTF8,0,path,-1,szPath,pathlen))
	{
		hFindFile = FindFirstFileW(szPath,&FindFileData);

		if (hFindFile != INVALID_HANDLE_VALUE)
		{
			do {
				if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					if (WideCharToMultiByte(CP_UTF8,0,FindFileData.cFileName,-1,utf8buf,sizeof utf8buf,NULL,NULL))
					{
                        //client->putq.push_back(new AppFilePut(utf8buf,target,targetuser,newname));
						total++;
					}
				}
			} while (FindNextFileW(hFindFile,&FindFileData));
		}
	}

	FindClose(hFindFile);

	return total;
}

// FIXME: implement terminal echo control
void term_init()
{
	// get terminal config at startup
}

void term_restore()
{
	// restore startup config
}

void term_echo(int echo)
{
	if (echo)
	{
		// FIXME: enable echo
	}
	else
	{
		// FIXME: disable echo
	}
}

/*int main()
{
	// instantiate app components: the callback processor (DemoApp),
	// the HTTP I/O engine (WinHttpIO) and the MegaClient itself
    client = new MegaClient(new DemoApp,new WinHttpIO,new BdbAccess,"SDKSAMPLE");

	megacli();
}
*/
