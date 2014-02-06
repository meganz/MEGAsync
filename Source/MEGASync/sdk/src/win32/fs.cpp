/**
 * @file win32/fs.cpp
 * @brief Win32 filesystem/directory access/notification (UNICODE)
 *
 * (c) 2013 by Mega Limited, Wellsford, New Zealand
 *
 * This file is part of the MEGA SDK - Client Access Engine.
 *
 * Applications using the MEGA API must present a valid application key
 * and comply with the the rules set forth in the Terms of Service.
 *
 * The MEGA SDK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * @copyright Simplified (2-clause) BSD License.
 *
 * You should have received a copy of the license along with this
 * program.
 */

#include "mega.h"

namespace mega {
WinFileAccess::WinFileAccess()
{
    hFile = INVALID_HANDLE_VALUE;
    hFind = INVALID_HANDLE_VALUE;

    fsidvalid = false;
}

WinFileAccess::~WinFileAccess()
{
    if (hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
    }
    else if (hFind != INVALID_HANDLE_VALUE)
    {
        FindClose(hFind);
    }
}

bool WinFileAccess::sysread(byte* dst, unsigned len, m_off_t pos)
{
    DWORD dwRead;

    if (!SetFilePointerEx(hFile, *(LARGE_INTEGER*)&pos, NULL, FILE_BEGIN))
    {
        return false;
    }

    return ReadFile(hFile, (LPVOID)dst, (DWORD)len, &dwRead, NULL) && dwRead == len;
}

bool WinFileAccess::fwrite(const byte* data, unsigned len, m_off_t pos)
{
    DWORD dwWritten;

    if (!SetFilePointerEx(hFile, *(LARGE_INTEGER*)&pos, NULL, FILE_BEGIN))
    {
        return false;
    }

    return WriteFile(hFile, (LPCVOID)data, (DWORD)len, &dwWritten, NULL) && dwWritten == len;
}

time_t FileTime_to_POSIX(FILETIME* ft) // NS (suppress style error)
{
    // takes the last modified date
    LARGE_INTEGER date;

    date.HighPart = ft->dwHighDateTime;
    date.LowPart = ft->dwLowDateTime;

    // removes the diff between 1970 and 1601
    date.QuadPart -= 11644473600000 * 10000;

    // converts back from 100-nanoseconds to seconds
    return date.QuadPart / 10000000;
}

bool WinFileAccess::sysstat(time_t* mtime, m_off_t* size)
{
    WIN32_FILE_ATTRIBUTE_DATA fad;

    if (!GetFileAttributesExW((LPCWSTR)localname.data(), GetFileExInfoStandard, (LPVOID)&fad))
    {
        retry = WinFileSystemAccess::istransient(GetLastError());
        return false;
    }

    if (fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        return false;
    }

    *mtime = FileTime_to_POSIX(&fad.ftLastWriteTime);
    *size = ((m_off_t)fad.nFileSizeHigh << 32 ) + (m_off_t)fad.nFileSizeLow;

    return true;
}

bool WinFileAccess::sysopen()
{
    hFile = CreateFileW((LPCWSTR)localname.data(), GENERIC_READ,
                        FILE_SHARE_WRITE | FILE_SHARE_READ,
                        NULL, OPEN_EXISTING, 0, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        retry = WinFileSystemAccess::istransient(GetLastError());
        return false;
    }

    return true;
}

void WinFileAccess::sysclose()
{
    if (localname.size())
    {
        // hFile will always be valid at this point
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }
}

// update local name
void WinFileAccess::updatelocalname(string* name)
{
    if (localname.size())
    {
        localname = *name;
        localname.append("", 1);
    }
}

// emulates Linux open-directory-as-file semantics
// FIXME #1: How to open files and directories with a single atomic
// CreateFile() operation without first looking at the attributes?
// FIXME #2: How to convert a CreateFile()-opened directory directly to a hFind
// without doing a FindFirstFile()?
bool WinFileAccess::fopen(string* name, bool read, bool write)
{
    WIN32_FILE_ATTRIBUTE_DATA fad;
    BY_HANDLE_FILE_INFORMATION bhfi;

    name->append("", 1);

    if (write)
    {
        type = FILENODE;
    }
    else
    {
        if (!GetFileAttributesExW((LPCWSTR)name->data(), GetFileExInfoStandard, (LPVOID)&fad))
        {
            name->resize(name->size() - 1);
            retry = WinFileSystemAccess::istransient(GetLastError());
            return false;
        }

        type = ( fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ? FOLDERNODE : FILENODE;
    }

    // (race condition between GetFileAttributesEx()/FindFirstFile() possible -
    // fixable with the current Win32 API?)

    hFile = CreateFileW((LPCWSTR)name->data(),
                        read ? GENERIC_READ : GENERIC_WRITE,
                        FILE_SHARE_WRITE | FILE_SHARE_READ,
                        NULL,
                        read ? OPEN_EXISTING : OPEN_ALWAYS,
                        (( type == FOLDERNODE ) ? FILE_FLAG_BACKUP_SEMANTICS : 0 ),
                        NULL);

    name->resize(name->size() - 1);

    // FIXME: verify that keeping the directory opened quashes the possibility
    // of a race condition between CreateFile() and FindFirstFile()

    if (hFile == INVALID_HANDLE_VALUE)
    {
        retry = WinFileSystemAccess::istransient(GetLastError());
        return false;
    }

    mtime = FileTime_to_POSIX(&fad.ftLastWriteTime);

    if (read && ( fsidvalid = !!GetFileInformationByHandle(hFile, &bhfi)))
    {
        fsid = ((handle)bhfi.nFileIndexHigh << 32 ) | (handle)bhfi.nFileIndexLow;
    }

    if (type == FOLDERNODE)
    {
        // enumerate directory
        name->append((char*)L"\\*", 5);

        hFind = FindFirstFileW((LPCWSTR)name->data(), &ffd);
        name->resize(name->size() - 5);

        if (hFind == INVALID_HANDLE_VALUE)
        {
            retry = WinFileSystemAccess::istransient(GetLastError());
            return false;
        }

        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
        retry = false;
        return true;
    }

    if (read)
    {
        size = ((m_off_t)fad.nFileSizeHigh << 32 ) + (m_off_t)fad.nFileSizeLow;
    }

    return true;
}

WinFileSystemAccess::WinFileSystemAccess()
{
    pendingevents = 0;
    localseparator.assign((char*)L"\\", sizeof( wchar_t ));
}

WinFileSystemAccess::~WinFileSystemAccess()
{}

bool WinFileSystemAccess::istransient(DWORD e)
{
    return e == ERROR_ACCESS_DENIED
           || e == ERROR_TOO_MANY_OPEN_FILES
           || e == ERROR_NOT_ENOUGH_MEMORY
           || e == ERROR_OUTOFMEMORY
           || e == ERROR_WRITE_PROTECT
           || e == ERROR_LOCK_VIOLATION
           || e == ERROR_SHARING_VIOLATION;
}

bool WinFileSystemAccess::istransientorexists(DWORD e)
{
    target_exists = (e == ERROR_FILE_EXISTS || e == ERROR_ALREADY_EXISTS);

    return istransient(e);
}

// wake up from filesystem updates
void WinFileSystemAccess::addevents(Waiter* w, int)
{
    // overlapped completion wakes up WaitForMultipleObjectsEx()
    ((WinWaiter*)w )->pendingfsevents = pendingevents;
}

// generate unique local filename in the same fs as relatedpath
void WinFileSystemAccess::tmpnamelocal(string* localname)
{
    static unsigned tmpindex;
    char buf[128];

    sprintf(buf, ".getxfer.%lu.%u.mega", GetCurrentProcessId(), tmpindex++);
    *localname = buf;
    name2local(localname);
}

void WinFileSystemAccess::path2local(string* path, string* local)
{
    local->resize(( path->size() + 1 ) * sizeof( wchar_t ));

    local->resize(sizeof( wchar_t ) * ( MultiByteToWideChar(CP_UTF8, 0,
                                                            path->c_str(),
                                                            -1,
                                                            (wchar_t*)local->data(),
                                                            local->size() / sizeof( wchar_t ) + 1) - 1 ));
}

void WinFileSystemAccess::local2path(string* local, string* path)
{
    path->resize(( local->size() + 1 ) * 4 / sizeof( wchar_t ));

    path->resize(WideCharToMultiByte(CP_UTF8, 0, (wchar_t*)local->data(),
                                     local->size() / sizeof( wchar_t ),
                                     (char*)path->data(),
                                     path->size() + 1,
                                     NULL, NULL));
}

//  escape forbidden characters, then convert UTF-8 to Windows 16-bit UNICODE
void WinFileSystemAccess::name2local(string* filename, const char* badchars)
{
    char buf[4];

    if (!badchars)
    {
        badchars = "\\/:?\"<>|*\1\2\3\4\5\6\7\10\11\12\13\14\15\16\17\20\21\22\23\24\25\26\27\30\31\32\33\34\35\36\37";
    }

    // replace all occurrences of a badchar with %xx
    for (int i = filename->size(); i--; )
    {
        if (((unsigned char)( *filename )[i] < ' ' ) || strchr(badchars, ( *filename )[i]))
        {
            sprintf(buf, "%%%02x", (unsigned char)( *filename )[i]);
            filename->replace(i, 1, buf);
        }
    }

    string t = *filename;

    // make space for the worst case
    filename->resize(( t.size() + 1 ) * sizeof( wchar_t ));

    // resize to actual result
    filename->resize(sizeof( wchar_t ) * ( MultiByteToWideChar(CP_UTF8, 0, t.c_str(),
                                                               -1,
                                                               (wchar_t*)filename->data(),
                                                               filename->size() / sizeof( wchar_t ) + 1) - 1 ));
}

// convert Windows 16-bit UNICODE to UTF-8, then unescape escaped forbidden
// characters
// by replacing occurrences of %xx (x being a lowercase hex digit) with the
// encoded character
void WinFileSystemAccess::local2name(string* filename)
{
    char c;
    string t = *filename;

    filename->resize(( t.size() + 1 ) * 4 / sizeof( wchar_t ));

    filename->resize(WideCharToMultiByte(CP_UTF8, 0, (wchar_t*)t.data(),
                                         t.size() / sizeof( wchar_t ),
                                         (char*)filename->data(),
                                         filename->size() + 1,
                                         NULL, NULL));

    for (int i = filename->size() - 2; i-- > 0; )
    {
        if ((( *filename )[i] == '%' ) && islchex(( *filename )[i + 1]) && islchex(( *filename )[i + 2]))
        {
            c = ( MegaClient::hexval(( *filename )[i + 1]) << 4 ) + MegaClient::hexval(( *filename )[i + 2]);
            filename->replace(i, 3, &c, 1);
        }
    }
}

// write short name of the last path component to sname
bool WinFileSystemAccess::getsname(string* name, string* sname)
{
    int r, rr;

    name->append("", 1);

    r = name->size() / sizeof( wchar_t ) + 1;

    sname->resize(r * sizeof( wchar_t ));
    rr = GetShortPathNameW((LPCWSTR)name->data(), (LPWSTR)sname->data(), r);

    sname->resize(rr * sizeof( wchar_t ));

    if (rr >= r)
    {
        rr = GetShortPathNameW((LPCWSTR)name->data(), (LPWSTR)sname->data(), rr);
        sname->resize(rr * sizeof( wchar_t ));
    }

    name->resize(name->size() - 1);

    if (!rr)
    {
        sname->clear();
        return false;
    }

    // we are only interested in the path's last component
    wchar_t* ptr;

    if (( ptr = wcsrchr((wchar_t*)sname->data(), '\\')) || ( ptr = wcsrchr((wchar_t*)sname->data(), ':')))
    {
        sname->erase(0, (char*)ptr - sname->data() + sizeof( wchar_t ));
    }

    return true;
}

// FIXME: if a folder rename fails because the target exists, do a top-down
// recursive copy/delete
bool WinFileSystemAccess::renamelocal(string* oldname, string* newname, bool replace)
{
    oldname->append("", 1);
    newname->append("", 1);
    bool r = !!MoveFileExW((LPCWSTR)oldname->data(), (LPCWSTR)newname->data(), replace ? MOVEFILE_REPLACE_EXISTING : 0);
    newname->resize(newname->size() - 1);
    oldname->resize(oldname->size() - 1);

    if (!r)
    {
        transient_error = istransientorexists(GetLastError());
    }

    return r;
}

bool WinFileSystemAccess::copylocal(string* oldname, string* newname)
{
    oldname->append("", 1);
    newname->append("", 1);
    bool r = !!CopyFileW((LPCWSTR)oldname->data(), (LPCWSTR)newname->data(), FALSE);
    newname->resize(newname->size() - 1);
    oldname->resize(oldname->size() - 1);

    if (!r)
    {
        transient_error = istransientorexists(GetLastError());
    }

    return r;
}

bool WinFileSystemAccess::rmdirlocal(string* name)
{
    name->append("", 1);
    int r = !!RemoveDirectoryW((LPCWSTR)name->data());
    name->resize(name->size() - 1);

    if (!r)
    {
        transient_error = istransient(GetLastError());
    }

    return r;
}

bool WinFileSystemAccess::unlinklocal(string* name)
{
    name->append("", 1);
    int r = !!DeleteFileW((LPCWSTR)name->data());
    name->resize(name->size() - 1);

    if (!r)
    {
        transient_error = istransient(GetLastError());
    }

    return r;
}

bool WinFileSystemAccess::mkdirlocal(string* name, bool hidden)
{
    name->append("", 1);
    int r = !!CreateDirectoryW((LPCWSTR)name->data(), NULL);

    if (!r)
    {
        transient_error = istransientorexists(GetLastError());
    }
    else if (hidden)
    {
        DWORD a = GetFileAttributesW((LPCWSTR)name->data());
        if (a != INVALID_FILE_ATTRIBUTES)
        {
            SetFileAttributesW((LPCWSTR)name->data(), a | FILE_ATTRIBUTE_HIDDEN);
        }
    }

    name->resize(name->size() - 1);

    return r;
}

bool WinFileSystemAccess::setmtimelocal(string* name, time_t mtime)
{
    FILETIME lwt;
    LONGLONG ll;
    HANDLE hFile;

    name->append("", 1);
    hFile = CreateFileW((LPCWSTR)name->data(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    name->resize(name->size() - 1);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    ll = Int32x32To64(mtime, 10000000) + 116444736000000000;
    lwt.dwLowDateTime = (DWORD)ll;
    lwt.dwHighDateTime = ll >> 32;

    int r = !!SetFileTime(hFile, NULL, NULL, &lwt);

    CloseHandle(hFile);

    return r;
}

bool WinFileSystemAccess::chdirlocal(string* name)
{
    name->append("", 1);
    int r = SetCurrentDirectoryW((LPCWSTR)name->data());
    name->resize(name->size() - 1);

    return r;
}

size_t WinFileSystemAccess::lastpartlocal(string* name)
{
    for (size_t i = name->size() / sizeof( wchar_t ); i--; )
    {
        if ((((wchar_t*)name->data())[i] == '\\' ) || (((wchar_t*)name->data())[i] == ':' ))
        {
            return ( i + 1 ) * sizeof( wchar_t );
        }
    }

    return 0;
}

void WinFileSystemAccess::osversion(string* u)
{
    char buf[128];
    DWORD dwVersion = GetVersion();

    sprintf(buf, "Windows %d.%d", (int)( LOBYTE(LOWORD(dwVersion))), (int)( HIBYTE(LOWORD(dwVersion))));

    u->append(buf);
}

// set DirNotify's root LocalNode
void WinDirNotify::addnotify(LocalNode* l, string*)
{
    if (!l->parent)
    {
        localrootnode = l;
    }
}

VOID CALLBACK WinDirNotify::completion(DWORD dwErrorCode, DWORD dwBytes, LPOVERLAPPED lpOverlapped)
{
    if (dwErrorCode != ERROR_OPERATION_ABORTED)
    {
        ((WinDirNotify*)lpOverlapped->hEvent )->process(dwBytes);
    }
}

void WinDirNotify::process(DWORD dwBytes)
{
    if (!dwBytes)
    {
        // empty notification: re-read all trees
        readchanges();
        error = true;
    }
    else
    {
        assert(dwBytes >= offsetof(FILE_NOTIFY_INFORMATION, FileName) + sizeof( wchar_t ));

        char* ptr = (char*)notifybuf[active].data();

        active ^= 1;

        readchanges();

        // we trust the OS to always return conformant data
        for (; ; )
        {
            FILE_NOTIFY_INFORMATION* fni = (FILE_NOTIFY_INFORMATION*)ptr;

            if (( fni->FileNameLength < ignore.size())
                    || memcmp((char*)fni->FileName, ignore.data(), ignore.size())
                    || (( fni->FileNameLength > ignore.size())
                            && memcmp((char*)fni->FileName + ignore.size(), (char*)L"\\", sizeof( wchar_t ))))
            {
                notify(DIREVENTS, localrootnode, (char*)fni->FileName, fni->FileNameLength);
            }

            if (!fni->NextEntryOffset)
            {
                break;
            }

            ptr += fni->NextEntryOffset;
        }
    }
}

// request change notifications on the subtree under hDirectory
void WinDirNotify::readchanges()
{
    if (ReadDirectoryChangesW(hDirectory, (LPVOID)notifybuf[active].data(),
                              notifybuf[active].size(), TRUE,
                              FILE_NOTIFY_CHANGE_FILE_NAME
                              | FILE_NOTIFY_CHANGE_DIR_NAME
                              | FILE_NOTIFY_CHANGE_LAST_WRITE
                              | FILE_NOTIFY_CHANGE_SIZE
                              | FILE_NOTIFY_CHANGE_CREATION,
                              &dwBytes, &overlapped, completion))
    {
        failed = false;
    }
    else
    {
        if (GetLastError() == ERROR_NOTIFY_ENUM_DIR)
        {
            error = true;                                           //
                                                                    //
                                                                    // notification
                                                                    // buffer
                                                                    // overflow
        }
        else
        {
            failed = true;  // permanent failure
        }
    }
}

WinDirNotify::WinDirNotify(string* localbasepath, string* ignore) : DirNotify(localbasepath, ignore)
{
    ZeroMemory(&overlapped, sizeof( overlapped ));

    overlapped.hEvent = this;

    active = 0;
    notifybuf[0].resize(65534);
    notifybuf[1].resize(65534);

    localbasepath->append("", 1);
    if (( hDirectory = CreateFileW((LPCWSTR)localbasepath->data(),
                                   FILE_LIST_DIRECTORY,
                                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                                   NULL,
                                   OPEN_EXISTING,
                                   FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                                   NULL)) != INVALID_HANDLE_VALUE)
    {
        readchanges();
    }
    localbasepath->resize(localbasepath->size() - 1);
}

WinDirNotify::~WinDirNotify()
{
    if (hDirectory != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hDirectory);
    }
}

FileAccess* WinFileSystemAccess::newfileaccess()
{
    return new WinFileAccess();
}

DirAccess* WinFileSystemAccess::newdiraccess()
{
    return new WinDirAccess();
}

DirNotify* WinFileSystemAccess::newdirnotify(string* localpath, string* ignore)
{
    return new WinDirNotify(localpath, ignore);
}

bool WinDirAccess::dopen(string* name, FileAccess* f, bool glob)
{
    if (f)
    {
        if (( hFind = ((WinFileAccess*)f )->hFind ) != INVALID_HANDLE_VALUE)
        {
            ffd = ((WinFileAccess*)f )->ffd;
            ((WinFileAccess*)f )->hFind = INVALID_HANDLE_VALUE;
        }
    }
    else
    {
        if (!glob)
        {
            name->append((char*)L"\\*", 5);
        }

        hFind = FindFirstFileW((LPCWSTR)name->data(), &ffd);

        if (glob)
        {
            wchar_t* bp = (wchar_t*)name->data();

            // store base path for glob() emulation
            int p = wcslen(bp);

            while (p--)
            {
                if (( bp[p] == '/' ) || ( bp[p] == '\\' ))
                {
                    break;
                }
            }

            if (p >= 0)
            {
                globbase.assign((char*)bp, ( p + 1 ) * sizeof( wchar_t ));
            }
            else
            {
                globbase.clear();
            }
        }

        name->resize(name->size() - 5);
    }

    if (!( ffdvalid = ( hFind != INVALID_HANDLE_VALUE )))
    {
        return false;
    }

    return true;
}

bool WinDirAccess::dnext(string* name, nodetype_t* type)
{
    for (; ; )
    {
        if (ffdvalid
                && !( ffd.dwFileAttributes & ( FILE_ATTRIBUTE_TEMPORARY
                                             | FILE_ATTRIBUTE_SYSTEM
                                             | FILE_ATTRIBUTE_OFFLINE
                                             | FILE_ATTRIBUTE_HIDDEN ))
                && ( !( ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
                        || ( *ffd.cFileName != '.' )
                        || ( ffd.cFileName[1] && (( ffd.cFileName[1] != '.' ) || ffd.cFileName[2] ))))
        {
            name->assign((char*)ffd.cFileName, sizeof( wchar_t ) * wcslen(ffd.cFileName));
            name->insert(0, globbase);

            if (type)
            {
                *type = ( ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ? FOLDERNODE : FILENODE;
            }

            ffdvalid = false;
            return true;
        }

        if (!( ffdvalid = FindNextFileW(hFind, &ffd) != 0 ))
        {
            return false;
        }
    }
}

WinDirAccess::WinDirAccess()
{
    ffdvalid = false;
    hFind = INVALID_HANDLE_VALUE;
}

WinDirAccess::~WinDirAccess()
{
    if (hFind != INVALID_HANDLE_VALUE)
    {
        FindClose(hFind);
    }
}
} // namespace
