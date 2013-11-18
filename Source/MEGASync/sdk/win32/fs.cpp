/*

MEGA SDK 2013-11-17 - Win32 filesystem/directory access/notification (UNICODE)

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

#include <windows.h>
#include <shellapi.h>

#include "../megaclient.h"
#include "wait.h"
#include "fs.h"

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
	LARGE_INTEGER date;

	date.HighPart = ft->dwHighDateTime;
	date.LowPart = ft->dwLowDateTime;

	// removes the diff between 1970 and 1601
	date.QuadPart -= 11644473600000*10000;

	// converts back from 100-nanoseconds to seconds
	return date.QuadPart/10000000;
}

// emulates Linux open-directory-as-file semantics
// FIXME: use CreateFileW() to open the directory instead of FindFirstFileW()? How to read such a directory?
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
        cout << "INVALID HANDLE" << endl;
        wprintf(L"IIIII: %s\n", (LPCWSTR)name->data());
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

void WinFileSystemAccess::path2local(string* path, string* local)
{
    //return;

	local->resize((path->size()+1)*sizeof(wchar_t));

	local->resize(sizeof(wchar_t)*(MultiByteToWideChar(CP_UTF8,0,path->c_str(),-1,(wchar_t*)local->data(),local->size()/sizeof(wchar_t)+1)-1));
}

void WinFileSystemAccess::local2path(string* local, string* path)
{
    //return;

	path->resize((local->size()+1)*4/sizeof(wchar_t));

    path->resize(WideCharToMultiByte(CP_UTF8,0,(wchar_t*)local->data(),-1,(char*)path->data(),path->size()+1,NULL,NULL));
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
    //return;

	char c;
	string t = *filename;

	filename->resize((t.size()+1)*4/sizeof(wchar_t));

    filename->resize(WideCharToMultiByte(CP_UTF8,0,(wchar_t*)t.data(),-1,(char*)filename->data(),filename->size()+1,NULL,NULL));

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

bool WinFileSystemAccess::rubbishlocal(string* name)
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
}

bool WinFileSystemAccess::rmdirlocal(string* name)
{
	name->append("",1);
	int r = RemoveDirectoryW((LPCWSTR)name->data());
	name->resize(name->size()-1);

	return r;
}

bool WinFileSystemAccess::unlinklocal(string* name)
{
	name->append("",1);
	int r = DeleteFileW((LPCWSTR)name->data());
	name->resize(name->size()-1);

	return r;
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

VOID CALLBACK WinDirNotify::completion(DWORD dwErrorCode, DWORD dwBytes, LPOVERLAPPED lpOverlapped)
{
	if (dwErrorCode != ERROR_OPERATION_ABORTED && dwBytes) ((WinDirNotify*)lpOverlapped->hEvent)->process(dwBytes);
}

void WinDirNotify::process(DWORD dwBytes)
{
cout << "Notify completion routine with " << dwBytes << endl;

	assert(dwBytes >= offsetof(FILE_NOTIFY_INFORMATION,FileName)+sizeof(wchar_t));

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

// request change notifications on the subtree under hDirectory
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
