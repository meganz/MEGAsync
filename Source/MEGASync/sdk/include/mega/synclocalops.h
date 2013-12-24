/**
 * @file mega/synclocalops.h
 * @brief Various sync-related local filesystem operations
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

#if 0
#ifndef MEGA_LOCALSYNCOPS_H
#define MEGA_LOCALSYNCOPS_H 1

#include "types.h"
#include "megaclient.h"

namespace mega {

// local file/folder rename/deletion
class SyncLocalOp
{
protected:
	MegaClient* client;
	nodetype type;
	string to;

public:
	string from;

	bool recurse(nodetype, string*, string*);
	bool exec();

	SyncLocalOp(MegaClient*, nodetype, string*, string* = NULL);
};

} // namespace

#endif
#endif
