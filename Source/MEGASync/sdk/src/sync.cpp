/**
 * @file sync.cpp
 * @brief Class for synchronizing local and remote trees
 *
 * (c) 2013-2014 by Mega Limited, Wellsford, New Zealand
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
#include "mega/megaclient.h"
#include "mega/base64.h"

namespace mega {
// new Syncs are automatically inserted into the session's syncs list
// and a full read of the subtree is initiated
Sync::Sync(MegaClient* cclient, string* crootpath, const char* cdebris,
           string* clocaldebris, Node* remotenode, int ctag)
{

    string dbname;

    client = cclient;
    tag = ctag;

    tmpfa = NULL;

    localbytes = 0;
    localnodes[FILENODE] = 0;
    localnodes[FOLDERNODE] = 0;

    state = SYNC_INITIALSCAN;

    if (cdebris)
    {
        debris = cdebris;
        client->fsaccess->path2local(&debris, &localdebris);

        dirnotify = client->fsaccess->newdirnotify(crootpath, &localdebris);

        localdebris.insert(0, client->fsaccess->localseparator);
        localdebris.insert(0, *crootpath);
    }
    else
    {
        localdebris = *clocaldebris;

        // FIXME: pass last segment of localebris
        dirnotify = client->fsaccess->newdirnotify(crootpath, &localdebris);
    }

    localroot.init(this, FOLDERNODE, NULL, crootpath, crootpath);
    localroot.setnode(remotenode);

    //client->current_email isn't valid until loggedin is called
    //It's not called if the MEGA filesystem is loaded from the local storage
    //so I call it here
    client->loggedin();

    // state cache table
    // I use currentmail_localid+remoteid instead of using the local path
    // because this name is used by SQLite to create a file and paths contain
    // non valid characters for file names.
    char local_id[12];
    FileAccess *fas = client->fsaccess->newfileaccess();
    fas->fopen(crootpath, true, false);
    Base64::btoa((byte *)&fas->fsid, MegaClient::NODEHANDLE, local_id);
    delete fas;

    char remote_id[12];
    Base64::btoa((byte *)&remotenode->nodehandle, MegaClient::NODEHANDLE, remote_id);

    dbname = client->current_email + "_" + local_id + remote_id;

    //dbaccess doesn't use local names, it uses UTF8 (see megaclient.cpp:3628)
    //client->fsaccess->name2local( &dbname );
    statecachetable = client->dbaccess->open( client->fsaccess, &dbname );

    sync_it = client->syncs.insert(client->syncs.end(), this);

    loadFromCache();
}

Sync::~Sync()
{
    // must be set to prevent remote mass deletion while rootlocal destructor
    // runs
    assert(state == SYNC_CANCELED);

    // unlock tmp lock
    delete tmpfa;

    // stop all active and pending downloads
    if (localroot.node)
    {
        TreeProcDelSyncGet tdsg;
        client->proctree(localroot.node, &tdsg);
    }

    if( statecachetable ) {
        delete statecachetable;
    }

    client->syncs.erase(sync_it);

    client->syncactivity = true;
}

bool Sync::loadFromCache() {

    if( NULL != statecachetable && SYNC_INITIALSCAN == state ) {

        statecachetable->rewind();
        string cachedata;
        map<int32_t, LocalNode*> tempMap;
        uint32_t cid;

        // Loads all cached nodes
        while( statecachetable->next( &cid, &cachedata, &client->key ) ) {
            LocalNode* cur = LocalNode::unserialize(this, &cachedata);
            if( cur ) {
                cur->dbid           = cid;
                tempMap[cur->dbid]  = cur;
            }
        }

        // Restores parent relationship between nodes
        for( map<int32_t, LocalNode*>::iterator it = tempMap.begin(); it != tempMap.end(); ++it ) {
            LocalNode* cur = it->second;
            LocalNode* pNode = NULL;

            if(cur->parent_dbid)
            {
                map<int32_t, LocalNode*>::iterator pit = tempMap.find(cur->parent_dbid);
                if((pit != tempMap.end()) && (pit->second != cur))
                    pNode = pit->second;
            }
            else pNode = &localroot;

            if(pNode) //Adding nodes without a parent breaks filesystem notifications
            {
                Node *node = cur->node;
                int32_t pdbid = cur->parent_dbid;
                treestate_t ts = cur->ts;
                handle fsid = cur->fsid;
                m_off_t size = cur->size;

                string tmppath;
                pNode->getlocalpath(&tmppath, true);
                tmppath.append(client->fsaccess->localseparator);
                tmppath.append(cur->localname);

                //Clear localname to force newnode = true in setnameparent
                //otherwise, setnameparent could trigger node moves
                cur->localname.clear();

                cur->init(this, cur->type, pNode, NULL, &tmppath);
                cur->parent_dbid = pdbid;
                cur->size = size;
                cur->setfsid(fsid);
                cur->setnode(node);
                cur->treestate(ts);
            }

            cur->setnotseen(1);
        }

        return true;
    }

    return false;
}

void Sync::addToDeleteQueue( LocalNode* toDelete ) {
    if( SYNC_CANCELED == state ) {
        return;
    }
    insertq.remove( toDelete );
    if( toDelete->dbid ) {
        deleteq.push_back(toDelete->dbid);
    }
}

void Sync::addToInsertQueue( LocalNode* toInsert ) {
    if( SYNC_CANCELED == state ) {
        return;
    }
    if( toInsert->dbid ) {
        deleteq.remove( toInsert->dbid );
    }
    insertq.push_back(toInsert);
}

void Sync::cachenodes() {
    if( statecachetable && (SYNC_ACTIVE == state) && ( deleteq.size() || insertq.size() ) ) {

        statecachetable->begin();

        // Process delete queue
        while( deleteq.size() ) {
            int32_t dbid = deleteq.front();
            statecachetable->del(dbid);
            deleteq.pop_front();
        }

        // Process insert queue
        while( insertq.size() ) {
            LocalNode* cur = insertq.front();
            statecachetable->put( MegaClient::CACHEDLOCALNODE, cur, &client->key );
            insertq.pop_front();
        }

        statecachetable->commit();
    }
}

void Sync::changestate(syncstate_t newstate)
{
    if (newstate != state)
    {
        client->app->syncupdate_state(this, newstate);

        state = newstate;
    }
}

// walk path and return corresponding LocalNode and its parent
// path must be relative to l or start with the root prefix if l == NULL
// path must be a full sync path, i.e. start with localroot->localname
// NULL: no match, optionally returns residual path
LocalNode* Sync::localnodebypath(LocalNode* l, string* localpath, LocalNode** parent, string* rpath)
{
    const char* ptr = localpath->data();
    const char* end = ptr + localpath->size();
    size_t separatorlen = client->fsaccess->localseparator.size();

    if (rpath)
    {
        assert(!rpath->size());
    }

    if (!l)
    {
        // verify matching localroot prefix - this should always succeed for
        // internal use
        if (memcmp(ptr, localroot.localname.data(), localroot.localname.size())
            || memcmp(ptr + localroot.localname.size(),
                      client->fsaccess->localseparator.data(),
                      separatorlen))
        {
            if (parent)
            {
                *parent = NULL;
            }
            return NULL;
        }

        l = &localroot;
        ptr += l->localname.size() + client->fsaccess->localseparator.size();
    }

    const char* nptr = ptr;
    localnode_map::iterator it;
    string t;

    for (;;)
    {
        if ((nptr == end) || !memcmp(nptr, client->fsaccess->localseparator.data(), separatorlen))
        {
            if (parent)
            {
                *parent = l;
            }

            t.assign(ptr, nptr - ptr);
            if (((it = l->children.find(&t)) == l->children.end())
                && ((it = l->schildren.find(&t)) == l->schildren.end()))
            {
                // no full match: store residual path, return NULL with the
                // matching component LocalNode in parent
                if (rpath)
                {
                    rpath->assign(ptr, localpath->data() - ptr + localpath->size());
                }
                return NULL;
            }

            l = it->second;

            if (nptr == end)
            {
                // full match: no residual path, return corresponding LocalNode
                if (rpath)
                {
                    rpath->clear();
                }
                return l;
            }

            ptr = nptr + separatorlen;
            nptr = ptr;
        }
        else
        {
            nptr += separatorlen;
        }
    }
}

// scan localpath, add or update child nodes, call recursively for folder nodes
// localpath must be prefixed with Sync
bool Sync::scan(string* localpath, FileAccess* fa)
{
	if ((localpath->size() < localdebris.size())
		|| memcmp(localpath->data(), localdebris.data(), localdebris.size())
		|| ((localpath->size() != localdebris.size())
			&& memcmp(localpath->data() + localdebris.size(),
					  client->fsaccess->localseparator.data(),
					  client->fsaccess->localseparator.size())))
	{
		DirAccess* da;
		string localname, name;
		size_t baselen;
		bool success;

		baselen = localroot.localname.size() + client->fsaccess->localseparator.size();

		if (baselen > localpath->size())
		{
			baselen = localpath->size();
		}

		da = client->fsaccess->newdiraccess();

		// scan the dir, mark all items with a unique identifier
		if ((success = da->dopen(localpath, fa, false)))
		{
			size_t t = localpath->size();

			while (da->dnext(&localname))
			{
				name = localname;
				client->fsaccess->local2name(&name);

				// check if this record is to be ignored
				if (client->app->sync_syncable(name.c_str(), localpath, &localname))
				{
					if (t)
					{
						localpath->append(client->fsaccess->localseparator);
					}
					localpath->append(localname);

					// skip the sync's debris folder
					if ((localpath->size() < localdebris.size())
						|| memcmp(localpath->data(), localdebris.data(), localdebris.size())
						|| ((localpath->size() != localdebris.size())
							&& memcmp(localpath->data() + localdebris.size(),
									  client->fsaccess->localseparator.data(),
									  client->fsaccess->localseparator.size())))
					{
						// new or existing record: place scan result in
						// notification queue
						dirnotify->notify(DirNotify::DIREVENTS, NULL, localpath->data(), localpath->size());
					}

					localpath->resize(t);
				}
			}
		}

		delete da;

		return success;
	}
	else return false;
}


// check local path - if !localname, localpath is relative to l, with l == NULL
// being the root of the sync
// if localname is set, localpath is absolute and localname its last component
// path references a new FOLDERNODE: returns created node
// path references a existing FILENODE: returns node
// otherwise, returns NULL
LocalNode* Sync::checkpath(LocalNode* l, string* localpath, string* localname)
{
    LocalNode* ll = l;
    FileAccess* fa;
    bool newnode = false, changed = false;
    bool isroot;

    LocalNode* parent;
    string path;        // UTF-8 representation of tmppath
    string tmppath;     // full path represented by l + localpath
    string newname;     // portion of tmppath not covered by the existing
                        // LocalNode structure (always the last path component
                        // that does not have a corresponding LocalNode yet)

    if (localname)
    {
        // shortcut case (from within syncdown())
        isroot = false;
        parent = l;
        l = NULL;

        client->fsaccess->local2path(localpath, &path);
    }
    else
    {
        // construct full filesystem path in tmppath
        if (l)
        {
            l->getlocalpath(&tmppath);
        }

        if (localpath->size())
        {
            if (tmppath.size())
            {
                tmppath.append(client->fsaccess->localseparator);
            }

            tmppath.append(*localpath);
        }

        // look up deepest existing LocalNode by path, store remainder (if any)
        // in newname
        l = localnodebypath(l, localpath, &parent, &newname);

        // path invalid?
        if (!l && !newname.size())
        {
            return NULL;
        }

        string name = newname;
        client->fsaccess->local2name(&name);

        if (!client->app->sync_syncable(name.c_str(), &tmppath, &newname))
        {
            return NULL;
        }

        isroot = (l == &localroot && !newname.size());

        client->fsaccess->local2path(&tmppath, &path);
    }

    // attempt to open/type this file
    fa = client->fsaccess->newfileaccess();

    if (fa->fopen(localname ? localpath : &tmppath, true, false))
    {
        // Tries to load local nodes from cache
        if( SYNC_INITIALSCAN == state ) {
            LocalNode* tmpL = NULL;

            //Fixed extraction of file names (compatible with UTF16)
            int lastpart = client->fsaccess->lastpartlocal(localname ? localpath : &tmppath);
            string fname = string( localname ? *localpath : tmppath,
                                   lastpart,
                                   (localname ? *localpath : tmppath).size()-lastpart );

            if( parent ) {
                tmpL = parent->childbyname( &fname );
            } else {
                tmpL = localroot.childbyname( &fname );
            }

            if( tmpL
                && ( FOLDERNODE == tmpL->type || FILENODE == tmpL->type )
                // fsid changes => ignore cache
                && fa->fsid == tmpL->fsid
            ) {

                l           = tmpL;
                l->deleted  = false;
                l->setnotseen(0);

                if( FOLDERNODE == l->type
                        || ( FILENODE == l->type
                             && fa->size == l->size
                             && fa->mtime == l->mtime
                           )
                ) {
                    l->scanseqno = scanseqno;
                    localbytes  += l->size;

                    if( FOLDERNODE == l->type ) {
                        scan(localname ? localpath : &tmppath, fa);
                    }

                    delete fa;
                    return l;
                }
            }

        }

        if (!isroot)
        {
            if (l)
            {
                // mark as present
                l->setnotseen(0);

                if (fa->type == FILENODE)
                {
                    // has the file been overwritten or changed since the last
                    // scan?
                    // or did the size or mtime change?
                    if (fa->fsidvalid)
                    {
                        // if fsid has changed, the file was overwritten
                        // (FIXME: handle type changes)
                        if (l->fsid != fa->fsid)
                        {
                            handlelocalnode_map::iterator it;

                            // was the file overwritten by moving an existing
                            // file over it?
                            if ((it = client->fsidnode.find(fa->fsid)) != client->fsidnode.end())
                            {
                                client->app->syncupdate_local_move(this, it->second->name.c_str(), path.c_str());

                                // immediately delete existing LocalNode and
                                // replace with moved one
                                delete l;

                                // (in case of a move, this synchronously
                                // updates l->parent and l->node->parent)
                                it->second->setnameparent(parent, localname ? localpath : &tmppath);

                                // unmark possible deletion
                                it->second->setnotseen(0);

                                addToInsertQueue( it->second );

                                delete fa;
                                return it->second;
                            }
                            else
                            {
                                l->mtime = -1;  // trigger change detection
                            }
                        }
                    }

                    // no fsid change detected or overwrite with unknown file:
                    if ((fa->mtime != l->mtime) || (fa->size != l->size))
                    {
                        if (fa->fsidvalid && (l->fsid != fa->fsid))
                        {
                            l->setfsid(fa->fsid);
                        }

                        m_off_t dsize = l->size;

                        if (l->genfingerprint(fa))
                        {
                            localbytes -= dsize - l->size;
                        }

                        client->app->syncupdate_local_file_change(this, path.c_str());

                        client->stopxfer(l);
                        l->bumpnagleds();
                        l->deleted = false;

                        client->syncactivity = true;

                        addToInsertQueue( l );

                        delete fa;
                        return l;
                    }
                }
                else
                {
                    // (we tolerate overwritten folders, because we do a
                    // content scan anyway)
                    if (fa->fsidvalid)
                    {
                        l->setfsid(fa->fsid);
                    }
                }
            }

            // new node
            if (!l)
            {
                // rename or move of existing node?
                handlelocalnode_map::iterator it;

                if (fa->fsidvalid && ((it = client->fsidnode.find(fa->fsid)) != client->fsidnode.end()))
                {
                    client->app->syncupdate_local_move(this, it->second->name.c_str(), path.c_str());

                    // (in case of a move, this synchronously updates l->parent
                    // and l->node->parent)
                    it->second->setnameparent(parent, localname ? localpath : &tmppath);

                    // make sure that active PUTs receive their updated
                    // filenames
                    client->updateputs();

                    addToInsertQueue( it->second );

                    // unmark possible deletion
                    it->second->setnotseen(0);

                    // Immediately scans folder to avoid fake deletion of children
                    if( SYNC_INITIALSCAN == state ) {
                        scan(localname ? localpath : &tmppath, fa);
                    }
                }
                else
                {
                    // this is a new node: add
                    l = new LocalNode;
                    l->init(this, fa->type, parent, localname ? localname : &newname, localname ? localpath : &tmppath);

                    if (fa->fsidvalid)
                    {
                        l->setfsid(fa->fsid);
                    }

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
                    scan(localname ? localpath : &tmppath, fa);
                    client->app->syncupdate_local_folder_addition(this, path.c_str());
                    if( !isroot ) {
                        addToInsertQueue( l );
                    }
                }
                else
                {
                    l = NULL;
                }
            }
            else
            {
                if (isroot)
                {
                    changestate(SYNC_FAILED);           // root node cannot be
                                                        // a file
                }
                else 
                {
                    if (l->size > 0)
                    {
                        localbytes -= l->size;
                    }

                    if (l->genfingerprint(fa))
                    {
                        changed = true;
                        l->bumpnagleds();
                        l->deleted = 0;
                    }

                    if (l->size > 0)
                    {
                        localbytes += l->size;
                    }

                    if (newnode)
                    {
                        client->app->syncupdate_local_file_addition(this, path.c_str());
                    }
                    else if (changed)
                    {
                        client->app->syncupdate_local_file_change(this, path.c_str());
                    }

                    if( newnode || changed ) {
                        addToInsertQueue( l );
                    }

                }
            }
        }

        if (changed || newnode)
        {
            client->syncactivity = true;
        }
    }
    else
    {
        if (fa->retry)
        {
            // fopen() signals that the failure is potentially transient - do
            // nothing and request a recheck
            dirnotify->notify(DirNotify::RETRY, ll, localpath->data(), localpath->size());
        }
        else if (l)
        {
            // immediately stop outgoing transfer, if any
            if (l->transfer)
            {
                client->stopxfer(l);
            }

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
    size_t t = dirnotify->notifyq[q].size();

    while (t-- && (state != SYNC_ACTIVE
                   || q != DirNotify::DIREVENTS
                   || Waiter::ds - dirnotify->notifyq[DirNotify::DIREVENTS].front().timestamp > 4))
    {

        LocalNode* l = checkpath(dirnotify->notifyq[q].front().localnode, &dirnotify->notifyq[q].front().path);

        dirnotify->notifyq[q].pop_front();

        // we return control to the application in case a filenode was added
        // (in order to avoid lengthy blocking episodes due to multiple
        // consecutive fingerprint calculations)
        if (l && (l->type == FILENODE))
        {
            break;
        }
    }

    if (dirnotify->notifyq[q].size())
    {
        client->syncactivity = true;
    }
    else if (!dirnotify->notifyq[!q].size())
    {
        cachenodes();
        scanseqno++;    // all queues empty: new scan sweep begins
    }

}

bool Sync::movetolocaldebris(string* localpath)
{
    size_t t = localdebris.size();
    char buf[32];
    time_t ts = time(NULL);
    struct tm* ptm = localtime(&ts);
    string day, localday;

    for (int i = -3; i < 100; i++)
    {
        if ((i == -2) || (i > 95))
        {
            client->fsaccess->mkdirlocal(&localdebris);
        }

        sprintf(buf, "%04d-%02d-%02d", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday);

        if (i >= 0)
        {
            sprintf(strchr(buf, 0), " %02d.%02d.%02d.%02d", ptm->tm_hour,  ptm->tm_min, ptm->tm_sec, i);
        }

        day = buf;
        client->fsaccess->path2local(&day, &localday);

        localdebris.append(client->fsaccess->localseparator);
        localdebris.append(localday);

        if (i > -3)
        {
            client->fsaccess->mkdirlocal(&localdebris, true);
        }

        localdebris.append(client->fsaccess->localseparator);
        localdebris.append(*localpath, client->fsaccess->lastpartlocal(localpath), string::npos);

        if (client->fsaccess->renamelocal(localpath, &localdebris, false))
        {
            localdebris.resize(t);
            return true;
        }

        localdebris.resize(t);

        if (client->fsaccess->transient_error)
        {
            return false;
        }
    }

    return false;
}
} // namespace
