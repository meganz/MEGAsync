/**
 * @file mega/win32/megafs.h
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

#ifndef FSACCESS_CLASS
#define FSACCESS_CLASS WinFileSystemAccess

namespace mega {

struct WinDirAccess : public DirAccess
{
	bool ffdvalid;
	WIN32_FIND_DATAW ffd;
	HANDLE hFind;
	string globbase;

public:
	bool dopen(string*, FileAccess*, bool);
	bool dnext(string*, nodetype* = NULL);

	WinDirAccess();
	virtual ~WinDirAccess();
};

class WinFileSystemAccess : public FileSystemAccess
{
public:
	unsigned pendingevents;

	FileAccess* newfileaccess();
	DirAccess* newdiraccess();
	DirNotify* newdirnotify(string*);

	void tmpnamelocal(string*, string* = NULL);

	void path2local(string*, string*);
	void local2path(string*, string*);

	void name2local(string*, const char* = NULL);
	void local2name(string*);

	bool getsname(string*, string*);

	bool renamelocal(string*, string*);
	bool copylocal(string*, string*);
	bool rubbishlocal(string*);
	bool unlinklocal(string*);
	bool rmdirlocal(string*);
	bool mkdirlocal(string*);
	bool setmtimelocal(string*, time_t);
	bool chdirlocal(string*);

	void addevents(Waiter*);

	static bool istransient(DWORD);
	bool istransientorexists(DWORD);
	
	WinFileSystemAccess();
	~WinFileSystemAccess();
};

struct WinDirNotify : public DirNotify
{
	WinFileSystemAccess* fsaccess;

	HANDLE hDirectory;

	int active;
	string notifybuf[2];

	DWORD dwBytes;
	OVERLAPPED overlapped;

	static VOID CALLBACK completion(DWORD, DWORD, LPOVERLAPPED);

	void process(DWORD wNumberOfBytesTransfered);
	void readchanges();

	WinDirNotify(string* basepath);
	~WinDirNotify();
};

class WinFileAccess : public FileAccess
{
	HANDLE hFile;

public:
	HANDLE hFind;
	WIN32_FIND_DATAW ffd;

	bool fopen(string*, bool, bool);
	bool fread(string*, unsigned, unsigned, m_off_t);
	bool frawread(byte*, unsigned, m_off_t);
	bool fwrite(const byte*, unsigned, m_off_t);

	WinFileAccess();
	~WinFileAccess();
};

} // namespace

#endif
