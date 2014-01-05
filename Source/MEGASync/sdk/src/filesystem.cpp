/**
 * @file filesystem.cpp
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

#include "mega/filesystem.h"
#include "mega/node.h"

namespace mega {

bool FileSystemAccess::islchex(char c)
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z');
}

// default DirNotify: no notification available
DirNotify::DirNotify(string* clocalbasepath)
{
	localbasepath = *clocalbasepath;
	
	failed = true;
	error = false;
}

// notify base LocalNode + relative path/filename
void DirNotify::notify(notifyqueue q, LocalNode* l, const char* localpath, size_t len)
{
	notifyq[q].resize(notifyq[q].size()+1);
	notifyq[q].back().localnode = l;
	notifyq[q].back().path.assign(localpath,len);
}

DirNotify* FileSystemAccess::newdirnotify(string* localpath)
{
	return new DirNotify(localpath);
}

// open file for reading
bool FileAccess::fopen(string* name)
{
	localname.resize(1);
	updatelocalname(name);
	
	return sysstat(&mtime,&size);
}

// check if size and mtime are unchanged, then open for reading
bool FileAccess::openf()
{
	if (!localname.size()) return true;
	
	time_t curr_mtime;
	m_off_t curr_size;
	
	if (!sysstat(&curr_mtime,&curr_size) || curr_mtime != mtime || curr_size != size) return false;

	return sysopen();
}

void FileAccess::closef()
{
	if (localname.size()) sysclose();
}

bool FileAccess::fread(string* dst, unsigned len, unsigned pad, m_off_t pos)
{
	if (!openf()) return false;

	bool r;
	
	dst->resize(len+pad);

	if ((r = sysread((byte*)dst->data(),len,pos))) memset((char*)dst->data()+len,0,pad);

	closef();
	
	return r;
}

bool FileAccess::frawread(byte* dst, unsigned len, m_off_t pos)
{
	if (!openf()) return false;
	
	bool r = sysread(dst,len,pos);
	
	closef();
	
	return r;
}

} // namespace
