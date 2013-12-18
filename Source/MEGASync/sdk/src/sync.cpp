/**
 * @file sync.cpp
 * @brief Class for synchronizing local and remote trees
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

#include "mega/sync.h"
#include "mega/megaapp.h"
#include "mega/transfer.h"

namespace mega {

// new Syncs are automatically inserted into the session's syncs list
// a full read of the subtree is initiated
Sync::Sync(MegaClient* cclient, string* crootpath, Node* remotenode, int ctag)
{
	client = cclient;
	tag = ctag;

	localbytes = 0;
	localnodes[FILENODE] = 0;
	localnodes[FOLDERNODE] = 0;

	state = SYNC_INITIALSCAN;

	localroot.init(this,FOLDERNODE,NULL,crootpath);
	localroot.setnode(remotenode);

	sync_it = client->syncs.insert(client->syncs.end(),this);

	dirnotify = client->fsaccess->newdirnotify(crootpath);
	scan(crootpath,NULL);
}

Sync::~Sync()
{
	// prevent remote mass deletion while rootlocal destructor runs
	state = SYNC_CANCELED;

	client->syncs.erase(sync_it);

	client->syncactivity = true;
}

void Sync::changestate(syncstate newstate)
{
	if (newstate != state)
	{
		client->app->syncupdate_state(this,newstate);

		state = newstate;
	}
}

// walk path and return corresponding LocalNode and its parent
// path must not start with a separator and be relative to the sync root
// NULL: no match
LocalNode* Sync::localnodebypath(string* localpath, LocalNode** parent)
{
	const char* ptr = localpath->data();
	const char* end = localpath->data()+localpath->size();
	const char* nptr = ptr;
	LocalNode* l = &localroot;
	size_t separatorlen = client->fsaccess->localseparator.size();
	localnode_map::iterator it;
	string t;

	for (;;)
	{
		if (nptr == end || !memcmp(nptr,client->fsaccess->localseparator.data(),separatorlen))
		{
			if (parent) *parent = l;

			t.assign(ptr,nptr-ptr);
			if ((it = l->children.find(&t)) == l->children.end() && (it = l->schildren.find(&t)) == l->schildren.end()) return NULL;

			l = it->second;

			if (nptr == end) return l;

			ptr = nptr+separatorlen;
			nptr = ptr;
		}
		else nptr += separatorlen;
	}
}

// determine sync state of path
pathstate_t Sync::pathstate(string* localpath)
{
	LocalNode* l = localnodebypath(localpath);

	if (!l) return PATHSTATE_NOTFOUND;
	if (l->node) return PATHSTATE_SYNCED;
	if (l->transfer && l->transfer->slot) return PATHSTATE_SYNCING;
	return PATHSTATE_PENDING;
}

// scan localpath, add or update child nodes, call recursively for folder nodes
// localpath must be prefixed with Sync
void Sync::scan(string* localpath, FileAccess* fa)
{
	DirAccess* da;
	string localname, name;
	size_t baselen;
	
	baselen = dirnotify->localbasepath.size()+client->fsaccess->localseparator.size();
	
	if (baselen > localpath->size()) baselen = localpath->size();

	da = client->fsaccess->newdiraccess();

	// scan the dir, mark all items with a unique identifier
	if (da->dopen(localpath,fa,false))
	{
		while (da->dnext(&localname))
		{
			name = localname;
			client->fsaccess->local2name(&name);

			// check if this record is to be ignored
			if (client->app->sync_syncable(name.c_str(),localpath,&localname))
			{
				// new or existing record: place at the end of the queue
				dirnotify->pathq[DirNotify::DIREVENTS].resize(dirnotify->pathq[DirNotify::DIREVENTS].size()+1);

				dirnotify->pathq[DirNotify::DIREVENTS].back().assign(*localpath,baselen,string::npos);
				if (dirnotify->pathq[DirNotify::DIREVENTS].back().size()) dirnotify->pathq[DirNotify::DIREVENTS].back().append(client->fsaccess->localseparator);
				dirnotify->pathq[DirNotify::DIREVENTS].back().append(localname);
			}
		}
	}

	delete da;
}

// check local path
// path references a new FOLDERNODE: returns created node
// path references a existing FILENODE: returns node
// otherwise, returns NULL
LocalNode* Sync::checkpath(string* localpath)
{
	FileAccess* fa;
	bool newnode = false, changed = false;
	bool isroot;

	LocalNode* l;
	LocalNode* parent;

	string tmppath;

	client->fsaccess->local2path(localpath,&tmppath);

	isroot = !localpath->size();

	l = isroot ? &localroot : localnodebypath(localpath,&parent);

	// prefix localpath with sync's base path
	if (!isroot) localpath->insert(0,client->fsaccess->localseparator);
	localpath->insert(0,dirnotify->localbasepath);

	// attempt to open/type this file, bail if unsuccessful
	fa = client->fsaccess->newfileaccess();

	if (fa->fopen(localpath,true,false))
	{
		if (!isroot)
		{
			// has the file or directory been overwritten since the last scan?
			if (l)
			{
				if ((fa->fsidvalid && l->fsid_it != client->fsidnode.end() && l->fsid != fa->fsid) || l->type != fa->type)
				{
					l->setnotseen(l->notseen+1);
					l = NULL;
				}
				else
				{
					l->setnotseen(0);
					l->scanseqno = scanseqno;
				}
			}

			// new node
			if (!l)
			{
				// rename or move of existing node?
				handlelocalnode_map::iterator it;

				if (fa->fsidvalid && (it = client->fsidnode.find(fa->fsid)) != client->fsidnode.end())
				{
					client->app->syncupdate_local_move(this,it->second->name.c_str(),tmppath.c_str());

					// (in case of a move, this synchronously updates l->parent and l->node->parent)
					it->second->setnameparent(parent,localpath);

					// unmark possible deletion
					it->second->setnotseen(0);
				}
				else
				{
					// this is a new node: add
					l = new LocalNode;
					l->init(this,fa->type,parent,localpath);
					if (fa->fsidvalid) l->setfsid(fa->fsid);
					newnode = true;
				}
			}
		}

		if (l)
		{
			// detect file changes or recurse into new subfolders
			if (l->type == FOLDERNODE)
			{
				if (newnode)
				{
					scan(localpath,fa);
					client->app->syncupdate_local_folder_addition(this,tmppath.c_str());
				}
				else l = NULL;
			}
			else
			{
				if (isroot) changestate(SYNC_FAILED);	// root node cannot be a file
				else
				{
					if (l->size > 0) localbytes -= l->size;
					if (l->genfingerprint(fa)) changed = true;
					if (l->size > 0) localbytes += l->size;
					
					if (newnode) client->app->syncupdate_local_file_addition(this,tmppath.c_str());
					else if (changed) client->app->syncupdate_local_file_change(this,tmppath.c_str());
				}
			}
		}

		if (changed || newnode)
		{
			client->syncadded.insert(l->syncid);
			client->syncactivity = true;
		}
	}
	else
	{
		if (fa->retry)
		{
			// fopen() signals that the failure is potentially transient - do nothing, but request a recheck
			dirnotify->pathq[DirNotify::RETRY].resize(dirnotify->pathq[DirNotify::RETRY].size()+1);
			dirnotify->pathq[DirNotify::RETRY].back().assign(localpath->data()+dirnotify->localbasepath.size()+client->fsaccess->localseparator.size(),localpath->size()-dirnotify->localbasepath.size()-client->fsaccess->localseparator.size());
		}
		else if (l)
		{
			// immediately stop outgoing transfer, if any
			if (l->transfer) client->stopxfer(l);
			
			client->syncactivity = true;

			l->setnotseen(1);
		}
		
		l = NULL;
	}

	delete fa;
	
	return l;
}

// add or refresh local filesystem item from scan stack, add items to scan stack
void Sync::procscanq(int q)
{
	while (dirnotify->pathq[q].size())
	{
		string* localpath = &dirnotify->pathq[q].front();

		LocalNode* l = checkpath(localpath);

		dirnotify->pathq[q].pop_front();
		
		// we return control to the application in case a filenode was encountered
		// (in order to avoid lengthy blocking episodes due to multiple fingerprint calculations)
		if (l && l->type == FILENODE) break;
	}

	if (dirnotify->pathq[q].size()) client->syncactivity = true;
	else if (!dirnotify->pathq[!q].size()) scanseqno++;	// all queues empty: new scan sweep begins
}

} // namespace
