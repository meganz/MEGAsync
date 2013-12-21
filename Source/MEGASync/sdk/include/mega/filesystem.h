/**
 * @file mega/filesystem.h
 * @brief Generic host filesystem access interfaces
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

#ifndef MEGA_FILESYSTEM_H
#define MEGA_FILESYSTEM_H 1

#include "types.h"
#include "waiter.h"

namespace mega {

// generic host filesystem node ID interface
struct FsNodeId
{
	virtual bool isequalto(FsNodeId*) = 0;
};

// generic host file/directory access interface
struct FileAccess
{
	// file size
	m_off_t size;

	// mtime of a file opened for reading
	time_t mtime;

	// local filesystem record id (survives renames & moves)
	handle fsid;
	bool fsidvalid;
	
	// type of opened path
	nodetype type;

	// if the open failed, retry indicates a potentially transient reason
	bool retry;

	// open for reading, writing or reading and writing
	virtual bool fopen(string*, bool, bool) = 0;

	// absolute position read, with NUL padding
	virtual bool fread(string*, unsigned, unsigned, m_off_t) = 0;

	// absolute position read to byte buffer
	virtual bool frawread(byte*, unsigned, m_off_t) = 0;

	// absolute position write
	virtual bool fwrite(const byte*, unsigned, m_off_t) = 0;

	virtual ~FileAccess() { }
};

// generic host directory enumeration
struct DirAccess
{
	// open for scanning
	virtual bool dopen(string*, FileAccess*, bool) = 0;

	// get next record
	virtual bool dnext(string*, nodetype* = NULL) = 0;

	virtual ~DirAccess() { }
};

// generic filesystem change notification
struct DirNotify
{
	typedef enum { DIREVENTS, RETRY, NUMQUEUES } notifyqueue;

	// notifyq[DIREVENTS] is fed with filesystem changes
	// notifyq[RETRY] receives transient errors that need to be retried
	notify_deque notifyq[NUMQUEUES];
	
	// set if no notification available on this platform or a permanent failure occurred
	bool failed;
	
	// set if a temporary error occurred
	bool error;

	// base path
	string localbasepath;

	virtual void addnotify(LocalNode*, string*) { }
	virtual void delnotify(LocalNode*) { }

	void notify(notifyqueue, LocalNode*, const char*, size_t);

	DirNotify(string*);
};

// generic host filesystem access interface
struct FileSystemAccess : public EventTrigger
{
	// local path separator, e.g. "/"
	string localseparator;

	// instantiate FileAccess object
	virtual FileAccess* newfileaccess() = 0;

	// instantiate DirAccess object
	virtual DirAccess* newdiraccess() = 0;

	// instantiate DirNotify object (default to periodic scanning handler if no notification configured) with given root path
	virtual DirNotify* newdirnotify(string*);

	// check if character is lowercase hex ASCII
	bool islchex(char);

	// convert MEGA path (UTF-8) to local format
	virtual void path2local(string*, string*) = 0;

	// convert local path to MEGA format (UTF-8)
	virtual void local2path(string*, string*) = 0;

	// convert MEGA-formatted filename (UTF-8) to local filesystem name; escape forbidden characters using urlencode
	virtual void name2local(string*, const char* = NULL) = 0;

	// reverse local2name()
	virtual void local2name(string*) = 0;

	// generate local temporary file name
	virtual void tmpnamelocal(string*, string* = NULL) = 0;

	// obtain local secondary name
	virtual bool getsname(string*, string*) = 0;
	
	// rename file, overwrite target
	virtual bool renamelocal(string*, string*) = 0;

	// copy file, overwrite target
	virtual bool copylocal(string*, string*) = 0;

	// move file or folder tree to OS-managed local rubbish bin
	virtual bool rubbishlocal(string*) = 0;

	// delete file
	virtual bool unlinklocal(string*) = 0;

	// delete empty directory
	virtual bool rmdirlocal(string*) = 0;

	// create directory
	virtual bool mkdirlocal(string*) = 0;

	// set mtime
	virtual bool setmtimelocal(string*, time_t) = 0;

	// change working directory
	virtual bool chdirlocal(string*) = 0;

	// add notification (has to be called for all directories in tree for full crossplatform support)
	virtual void addnotify(LocalNode*, string*) { }

	// delete notification
	virtual void delnotify(LocalNode*) { }

	// set whenever an operation fails due to a transient condition (e.g. locking violation)
	bool transient_error;

	// set whenever an operation fails because the target already exists
	bool target_exists;
	
	virtual ~FileSystemAccess() { }
};

} // namespace

#endif
