/**
 * @file localsyncops.cpp
 * @brief Implementation of various sync-related local filesystem operations
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

#include "mega/synclocalops.h"
#include "mega/megaapp.h"

namespace mega {

// (recursively) rename file/folder to target name, or delete if NULL
SyncLocalOp::SyncLocalOp(MegaClient* cclient, nodetype ctype, string* cfrom, string* cto)
{
	client = cclient;
	type = ctype;
	from = *cfrom;
	if (cto) to = *cto;
}

// rename or delete local filesystem object
// if target exists, recursively rename or copy/delete
// if target is temporarily locked, abort and repeat
bool SyncLocalOp::recurse(nodetype type, string* from, string* to)
{
	client->fsaccess->target_exists = false;

	if (to)
	{
		if (client->fsaccess->renamelocal(from,to)) return true;

		// cross-device renames have to be executed as copy-delete sequence
		if (type == FILENODE)
		{
			if (!client->fsaccess->copylocal(from,to)) return false;
			
			if (client->fsaccess->rubbishlocal(from)) return true;
			
			if (client->fsaccess->unlinklocal(from)) return true;
		}
		else
		{
			if (!client->fsaccess->mkdirlocal(to) && !client->fsaccess->target_exists) return false;
		}
	}
	else
	{
		if (client->fsaccess->rubbishlocal(from)) return true;
		
		if (client->fsaccess->transient_error) return false;
		
		if (type == FILENODE)
		{
			if (client->fsaccess->unlinklocal(from)) return true;
		}
		else 
		{
			if (client->fsaccess->rmdirlocal(from)) return true;
		}
	}
	
	if (type == FOLDERNODE)
	{
		DirAccess* da = client->fsaccess->newdiraccess();
		string entry;
		nodetype entrytype;
		size_t lfrom, lto;
		bool allgood = true;
		
		if (da->dopen(from,NULL,false))
		{
			while (da->dnext(&entry,&entrytype))
			{
				lfrom = from->size();
				from->append(client->fsaccess->localseparator);
				from->append(entry);
				
				if (to)
				{
					lto = to->size();
					to->append(client->fsaccess->localseparator);
					to->append(entry);
				}

				if (!recurse(entrytype,from,to) && allgood) allgood = false;
			
				if (to) to->resize(lto);
			
				from->resize(lfrom);
			}			
		}
		
		delete da;

		if (allgood && !client->fsaccess->rmdirlocal(from)) allgood = false;

		return allgood;
	}
	
	return false;
}

bool SyncLocalOp::exec()
{
	return recurse(type,&from,to.size() ? &to : NULL);
}

} // namespace
