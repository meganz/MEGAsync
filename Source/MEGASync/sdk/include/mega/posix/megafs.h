/**
 * @file mega/posix/megafs.h
 * @brief POSIX filesystem/directory access/notification
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
#define FSACCESS_CLASS PosixFileSystemAccess

#include "mega.h"

namespace mega {

struct PosixDirAccess : public DirAccess
{
	DIR* dp;
	bool globbing;
	glob_t globbuf;
	unsigned globindex;

	bool dopen(string*, FileAccess*, bool);
	bool dnext(string*, nodetype* = NULL);

	PosixDirAccess();
	virtual ~PosixDirAccess();
};

class PosixFileSystemAccess : public FileSystemAccess
{
public:
#ifdef USE_INOTIFY
	int notifyfd;

	bool notifyerr, notifyfailed;

	typedef map<int,LocalNode*> wdlocalnode_map;
	wdlocalnode_map wdnodes;
#endif

	FileAccess* newfileaccess();
	DirAccess* newdiraccess();
	DirNotify* newdirnotify(string*);

	void tmpnamelocal(string*, string* = NULL);

	void local2path(string*, string*);
	void path2local(string*, string*);

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

	void addevents(Waiter*, int);
	int checkevents(Waiter*);

	PosixFileSystemAccess();
	~PosixFileSystemAccess();
};

class PosixFileAccess : public FileAccess
{
public:
	int fd;

#ifndef USE_FDOPENDIR
	DIR* dp;
#endif

	bool fopen(string*, bool, bool);
	void updatelocalname(string*);
	bool fread(string*, unsigned, unsigned, m_off_t);
	bool frawread(byte*, unsigned, m_off_t);
	bool fwrite(const byte*, unsigned, m_off_t);

	bool sysread(byte*, unsigned, m_off_t);
	bool sysstat(time_t*, m_off_t*);
	bool sysopen();
	void sysclose();

	PosixFileAccess();
	~PosixFileAccess();
};

class PosixDirNotify : public DirNotify
{
public:
	PosixFileSystemAccess* fsaccess;

	void addnotify(LocalNode*, string*);
	void delnotify(LocalNode*);

	PosixDirNotify(string*);
};

} // namespace

#endif
