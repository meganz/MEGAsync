/**
 * @file treeproc.cpp
 * @brief Node tree processor
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

#include "mega/treeproc.h"
#include "mega/megaclient.h"

namespace mega {

// create share keys
TreeProcShareKeys::TreeProcShareKeys(Node* n)
{
	sn = n;
}

void TreeProcShareKeys::proc(MegaClient*, Node* n)
{
	snk.add(n,sn,sn != NULL);
}

void TreeProcShareKeys::get(Command* c)
{
	snk.get(c);
}

// total disk space / node count
TreeProcDU::TreeProcDU()
{
	numbytes = 0;
	numfiles = 0;
	numfolders = 0;
}

void TreeProcDU::proc(MegaClient*, Node* n)
{
	if (n->type == FILENODE)
	{
		numbytes += n->size;
		numfiles++;
	}
	else numfolders++;
}

// mark node as removed and notify
void TreeProcDel::proc(MegaClient* client, Node* n)
{
	n->removed = 1;
	client->notifynode(n);
}

// stop sync get
void TreeProcDelSyncGet::proc(MegaClient*, Node* n)
{
	if (n->syncget)
	{
		delete n->syncget;
		n->syncget = NULL;
	}
}

TreeProcSyncStatus::TreeProcSyncStatus()
{
	state = PATHSTATE_SYNCED;
}

// determine subtree's sync status
void TreeProcSyncStatus::proc(MegaClient* client, Node* n)
{
	if (state == PATHSTATE_SYNCING) return;

	if (!n->localnode)
	{
		if (n->type == FILENODE)
		{
			// SYNCING > PENDING > SYNCED, but we ignore nodes with no associated GET transfer (might be dupes or filtered)
			if (n->syncget && n->syncget->transfer)
			{
				if (n->syncget->transfer->slot) state = PATHSTATE_SYNCING;
				else if (state == PATHSTATE_SYNCED) state = PATHSTATE_PENDING;
			}
		}
	}
	else
	{
		if (n->type == FILENODE)
		{
			// missing remote file - uploading already?
			if (n->localnode->transfer)
			{
				if (n->localnode->transfer->slot) state = PATHSTATE_SYNCING;
				else if (state == PATHSTATE_SYNCED) state = PATHSTATE_PENDING;
			}
		}
		else
		{
			// find and check LocalNodes without corresponding Node, but don't recurse
			for (localnode_map::iterator it = n->localnode->children.begin(); it != n->localnode->children.end(); it++)
			{
				if (!it->second->node)
				{
					if (it->second->type == FILENODE)
					{
						// missing remote file - uploading already?
						if (it->second->transfer)
						{
							if (it->second->transfer->slot) state = PATHSTATE_SYNCING;
							else if (state == PATHSTATE_SYNCED) state = PATHSTATE_PENDING;
						}
					}
					else
					{
						// missing remote folders are subject to immediate creation, so always show SYNCING
						state = PATHSTATE_SYNCING;
					}
				}
			}
		}
	}
}

} // namespace
