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
	localroot.init(this,crootpath,FOLDERNODE,NULL,crootpath);
	localroot.setnode(remotenode);

	queuescan(MAIN,NULL,NULL,NULL,NULL,true);
	procscanq(MAIN);

	sync_it = client->syncs.insert(client->syncs.end(),this);
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

// walk path and return state of the sync
// path must not start with a separator and be relative to the sync root
pathstate_t Sync::pathstate(string* localpath)
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
			t.assign(ptr,nptr-ptr);

			if ((it = l->children.find(&t)) == l->children.end() && (it = l->schildren.find(&t)) == l->schildren.end()) return PATHSTATE_NOTFOUND;

			l = it->second;

			if (nptr == end) break;

			ptr = nptr+separatorlen;
			nptr = ptr;
		}
		else nptr += separatorlen;
	}

	if (l->node) return PATHSTATE_SYNCED;
	if (l->transfer && l->transfer->slot) return PATHSTATE_SYNCING;
	return PATHSTATE_PENDING;
}


// scan rootpath, add or update child nodes, call recursively for folder nodes
void Sync::scan(string* localpath, FileAccess* fa, LocalNode* parent, bool fulltree)
{
	DirAccess* da;
	string localname;
	static handle scanseqno;
	localnode_map::iterator it;
	LocalNode* l;

	scanseqno++;

	da = client->fsaccess->newdiraccess();

	// scan the dir, mark all items with a unique identifier
	if (da->dopen(localpath,fa,false)) while (da->dnext(&localname)) if ((l = queuefsrecord(localpath,&localname,parent,fulltree))) l->scanseqno = scanseqno;

	// delete items that disappeared
	for (it = parent->children.begin(); it != parent->children.end(); )
	{
		if (scanseqno != it->second->scanseqno) delete (it++)->second;
		else it++;
	}

	delete da;
}

LocalNode* Sync::queuefsrecord(string* localpath, string* localname, LocalNode* parent, bool fulltree)
{
	localnode_map::iterator it;
	LocalNode* l;
	string name;

	name = *localname;
	client->fsaccess->local2name(&name);

	// check if this record is to be ignored
	if (client->app->sync_syncable(name.c_str(),localpath,localname))
	{
		if ((it = parent->children.find(localname)) != parent->children.end() || (it = parent->schildren.find(localname)) != parent->schildren.end()) l = it->second;
		else l = NULL;

		queuescan(MAIN,localpath,localname,l,parent,fulltree);

		return l;
	}

	return NULL;
}

void Sync::queuescan(int q, string* localpath, string* localname, LocalNode* localnode, LocalNode* parent, bool fulltree)
{
	// FIXME: efficient copy-free push_back? C++11 emplace()?
	scanq[q].resize(scanq[q].size()+1);

	ScanItem* si = &scanq[q].back();

	// FIXME: don't create mass copies of localpath
	if (localpath) si->localpath = *localpath;
	if (localname) si->localname = *localname;
	si->localnode = localnode;
	si->parent = parent;
	si->fulltree = fulltree;
	si->deleted = false;
}

// add or refresh local filesystem item from scan stack, add items to scan stack
// must be called with a scanq.siz() > 0
void Sync::procscanq(int q)
{
	ScanItem* si = &*scanq[q].begin();

	// ignore deleted ScanItems
	if (si->deleted)
	{
		scanq[q].pop_front();
		return;
	}

	string* localpath = &si->localpath;
	string* localname = &si->localname;

	bool fulltree = si->fulltree;

	FileAccess* fa;
	bool changed = false;

	string tmpname;

	LocalNode* l;

	// if localpath was not specified, construct based on parents & base sync path
	if (!localpath->size())
	{
		LocalNode* p = si->parent ? si->parent : &localroot;

		while (p)
		{
			localpath->insert(0,p->localname);
			if ((p = p->parent)) localpath->insert(0,client->fsaccess->localseparator);
		}
	}

	if (localname->size())
	{
		localpath->append(client->fsaccess->localseparator);
		localpath->append(*localname);
	}

	// check if a child by the same name already exists
	// (skip this check for localroot)
	if (si->parent)
	{
		// have we seen this item before?
		localnode_map::iterator it;

		// check if it exists as a child or an schild
		if ((it = si->parent->children.find(localname)) != si->parent->children.end() || (it = si->parent->schildren.find(localname)) != si->parent->schildren.end()) l = it->second;
		else l = NULL;
	}
	else l = NULL;

	// attempt to open/type this file, bail if unsuccessful
	fa = client->fsaccess->newfileaccess();

	if (fa->fopen(localpath,1,0))
	{
		if (si->parent)
		{
			if (l && l->type != fa->type)
			{
				// type change (a directory replaced a file of the same name or vice versa)
				delete l;
				l = NULL;
			}

			// new node
			if (!l)
			{
				// this is a new node: add
				l = new LocalNode;
				l->init(this,localname,fa->type,si->parent,localpath);

				changed = true;
			}
		}

		// detect file changes or recurse into new subfolders
		if (fa->type == FOLDERNODE)
		{
			if (fulltree) scan(localpath,fa,si->parent ? l : &localroot,fulltree);
		}
		else
		{
			if (!l) changestate(SYNC_FAILED);	// root node cannot be a file
			else
			{
				if (l->size > 0) localbytes -= l->size;
				if (l->genfingerprint(fa)) changed = true;
				if (l->size > 0) localbytes += l->size;
			}
		}

		if (changed) client->syncadded.insert(l->syncid);
	}
	else
	{
		if (fa->retry)
		{
			// fopen() signals that the failure is potentially transient - do nothing, but request a recheck
			localpath->resize(localpath->size()-client->fsaccess->localseparator.size()-localname->size());
			queuescan(RETRY,localpath,localname,l,si->parent,true);

			l = NULL;	// make no changes yet
		}
		else if (l)
		{
			// file gone
			client->fsaccess->local2path(localpath,&tmpname);
			if (l->type == FOLDERNODE) client->app->syncupdate_local_folder_deletion(this,tmpname.c_str());
			else client->app->syncupdate_local_file_deletion(this,tmpname.c_str());

			client->syncactivity = true;

			delete l;
			l = NULL;
		}
	}

	if (l)
	{
		if (changed)
		{
			client->syncactivity = true;
			client->fsaccess->local2path(localpath,&tmpname);
		}

		if (changed)
		{
			if (l->type == FILENODE) client->app->syncupdate_local_file_addition(this,tmpname.c_str());
			else client->app->syncupdate_local_folder_addition(this,tmpname.c_str());
		}
		client->syncactivity = true;
	}

	delete fa;

	scanq[q].pop_front();

	if (scanq[q].size()) client->syncactivity = true;
}

} // namespace
