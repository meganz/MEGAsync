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

namespace mega {

bool FileSystemAccess::islchex(char c)
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z');
}

// default DirNotify: no notification available
DirNotify::DirNotify(string* clocalpath)
{
	localbasepath = *clocalpath;
	
	failed = true;
	error = false;
}

DirNotify* FileSystemAccess::newdirnotify(string* localpath)
{
	return new DirNotify(localpath);
}

} // namespace
