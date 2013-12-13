/**
 * @file posix/fs.cpp
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

#include "mega.h"

namespace mega {

PosixFileAccess::PosixFileAccess()
{
	fd = -1;
#ifndef USE_FDOPENDIR
	dp = NULL;
#endif
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
#ifndef USE_FDOPENDIR
	if ((dp = opendir(f->c_str())))
	{
		type = FOLDERNODE;
		return true;
	}
#endif

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

	retry = false;
	return false;
}

PosixFileSystemAccess::PosixFileSystemAccess()
{
	localseparator = "/";

#ifdef USE_INOTIFY
	notifyfd = inotify_init1(IN_NONBLOCK);
	notifyerr = false;
	notifyleft = 0;
#endif
}

PosixFileSystemAccess::~PosixFileSystemAccess()
{
#ifdef USE_INOTIFY
	if (notifyfd >= 0) close(notifyfd);
#endif
}

// wake up from filesystem updates
void PosixFileSystemAccess::addevents(Waiter* w)
{
#ifdef USE_INOTIFY
	PosixWaiter* pw = (PosixWaiter*)w;

	FD_SET(notifyfd,&pw->rfds);

	pw->bumpmaxfd(notifyfd);
#endif
}

// generate unique local filename in the same fs as relatedpath
void PosixFileSystemAccess::tmpnamelocal(string* filename, string* relatedpath)
{
	*filename = tmpnam(NULL);
}

void PosixFileSystemAccess::path2local(string* local, string* path)
{
	*path = *local;
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

// no legacy DOS garbage here...
bool PosixFileSystemAccess::getsname(string*, string*)
{
	return false;
}

bool PosixFileSystemAccess::renamelocal(string* oldname, string* newname)
{
	return !rename(oldname->c_str(),newname->c_str());
}

bool PosixFileSystemAccess::copylocal(string* oldname, string* newname)
{
	int sfd, tfd;
	ssize_t t = -1;

#ifdef USE_INOTIFY
#include <sys/sendfile.h>

	// Linux-specific - kernel 2.6.33+ required
	if ((sfd = open(oldname->c_str(),O_RDONLY|O_DIRECT)) >= 0)
	{
		if ((tfd = open(newname->c_str(),O_WRONLY|O_CREAT|O_TRUNC|O_DIRECT,0600)) >= 0)
		{
			while ((t = sendfile(tfd,sfd,NULL,1024*1024*1024)) > 0);
#else
	char buf[16384];

	if ((sfd = open(oldname->c_str(),O_RDONLY)) >= 0)
	{
		if ((tfd = open(newname->c_str(),O_WRONLY|O_CREAT|O_TRUNC,0600)) >= 0)
		{
			while (((t = read(sfd,buf,sizeof buf)) > 0) && write(tfd,buf,t) == t);
#endif
			close(tfd);
		}

		close(sfd);
	}

	return !t;
}

// FIXME: add platform support for recycle bins
bool PosixFileSystemAccess::rubbishlocal(string* name)
{
	return false;
}

bool PosixFileSystemAccess::unlinklocal(string* name)
{
	return !unlink(name->c_str());
}

bool PosixFileSystemAccess::rmdirlocal(string* name)
{
	return !rmdir(name->c_str());
}

bool PosixFileSystemAccess::mkdirlocal(string* name)
{
	bool r = !mkdir(name->c_str(),0700);
	
	if (!r) target_exists = errno == EEXIST;
	
	return r;
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
#ifdef USE_INOTIFY
	int wd;

	wd = inotify_add_watch(notifyfd,path->c_str(),IN_CREATE|IN_DELETE|IN_MOVED_FROM|IN_MOVED_TO|IN_CLOSE_WRITE|IN_EXCL_UNLINK|IN_ONLYDIR);

	if (wd >= 0)
	{
		l->notifyhandle = (void*)(long)wd;
		wdnodes[wd] = l;
	}
#endif
}

void PosixFileSystemAccess::delnotify(LocalNode* l)
{
#ifdef USE_INOTIFY
	if (wdnodes.erase((int)(long)l->notifyhandle)) inotify_rm_watch(notifyfd,(int)(long)l->notifyhandle);
#endif
}

// return next notified local name and corresponding parent node
bool PosixFileSystemAccess::notifynext(sync_list*, string* localname, LocalNode** localnodep, bool* fulltreep)
{
#ifdef USE_INOTIFY
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

		if (in->mask & (IN_Q_OVERFLOW | IN_UNMOUNT)) break;

		if (in->mask & (IN_CREATE|IN_DELETE|IN_MOVED_FROM|IN_MOVED_TO|IN_CLOSE_WRITE|IN_EXCL_UNLINK))
		{
			if ((in->mask & (IN_CREATE|IN_ISDIR)) != IN_CREATE)
			{
				it = wdnodes.find(in->wd);

				if (it != wdnodes.end())
				{
					*localname = in->name;
					*localnodep = it->second;

					if (fulltreep) *fulltreep = true;

					return true;
				}
				else cout << "Unknown wd handle" << endl;
			}
			else cout << "File creation ignored" << endl;
		}
		else cout << "Unknown mask: " << in->mask << endl;
	}

	cout << "Notify Overflow" << endl;
#endif
	notifyerr = true;
	return false;
}

// true if notifications were unreliable and/or a full rescan is required
bool PosixFileSystemAccess::notifyfailed()
{
	return notifyerr ? (notifyerr = false) || true : false;
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
#ifdef USE_FDOPENDIR
		dp = fdopendir(((PosixFileAccess*)f)->fd);
		((PosixFileAccess*)f)->fd = -1;
#else
		dp = ((PosixFileAccess*)f)->dp;
		((PosixFileAccess*)f)->dp = NULL;
#endif
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

} // namespace
