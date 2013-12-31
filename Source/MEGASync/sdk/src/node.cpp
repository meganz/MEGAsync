/**
 * @file node.cpp
 * @brief Classes for accessing local and remote nodes
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

#include "mega/node.h"
#include "mega/megaclient.h"
#include "mega/share.h"
#include "mega/base64.h"
#include "mega/sync.h"
#include "mega/transfer.h"

namespace mega {

Node::Node(MegaClient* cclient, node_vector* dp, handle h, handle ph, nodetype t, m_off_t s, handle u, const char* fa, time_t ts, time_t tm)
{
	client = cclient;

	syncdeleted = false;

	tag = 0;

	nodehandle = h;
	parenthandle = ph;

	parent = NULL;

	localnode = NULL;
	syncget = NULL;

	type = t;

	size = s;
	owner = u;

	copystring(&fileattrstring,fa);

	clienttimestamp = tm;
	ctime = ts;

	inshare = NULL;
	sharekey = NULL;

	removed = 0;

	if (client)
	{
		Node* p;

		client->nodes[h] = this;

		// folder link access: first returned record defines root node and identity
		if (ISUNDEF(*client->rootnodes)) *client->rootnodes = h;

		if (t >= ROOTNODE && t <= MAILNODE) client->rootnodes[t-ROOTNODE] = h;

		// set parent linkage or queue for delayed parent linkage in case of out-of-order delivery
		if ((p = client->nodebyhandle(ph))) setparent(p);
		else dp->push_back(this);

		if (type == FILENODE) fingerprint_it = client->fingerprints.end();
	}
}

Node::~Node()
{
	// remove node's fingerprint from hash
	if (type == FILENODE && fingerprint_it != client->fingerprints.end()) client->fingerprints.erase(fingerprint_it);

	// delete outshares, including pointers from users for this node
	for (share_map::iterator it = outshares.begin(); it != outshares.end(); it++) delete it->second;

	// remove from parent's children
	if (parent) parent->children.erase(child_it);

	// delete child-parent associations (normally not used, as nodes are deleted bottom-up)
	for (node_list::iterator it = children.begin(); it != children.end(); it++) (*it)->parent = NULL;

	delete inshare;
	delete sharekey;

	// sync: remove reference from local filesystem node
	if (localnode) localnode->node = NULL;

	// in case this node is currently being transferred for syncing: abort transfer
	delete syncget;
}

// parse serialized node and return Node object - updates nodes hash and parent mismatch vector
Node* Node::unserialize(MegaClient* client, string* d, node_vector* dp)
{
	handle h, ph;
	nodetype t;
	m_off_t s;
	handle u;
	const byte* k = NULL;
	const char* fa;
	time_t tm;
	time_t ts;
	const byte* skey;
	const char* ptr = d->data();
	const char* end = ptr+d->size();
	unsigned short ll;
	Node* n;
	int i;

	if (ptr+sizeof s+2*MegaClient::NODEHANDLE+MegaClient::USERHANDLE+2*sizeof tm+sizeof ll > end) return NULL;

	s = *(m_off_t*)ptr;
	ptr += sizeof s;

	if (s < 0 && s >= -MAILNODE) t = (nodetype)-s;
	else t = FILENODE;

	h = 0;
	memcpy((char*)&h,ptr,MegaClient::NODEHANDLE);
	ptr += MegaClient::NODEHANDLE;

	ph = 0;
	memcpy((char*)&ph,ptr,MegaClient::NODEHANDLE);
	ptr += MegaClient::NODEHANDLE;

	if (!ph) ph = UNDEF;

	memcpy((char*)&u,ptr,MegaClient::USERHANDLE);
	ptr += MegaClient::USERHANDLE;

	tm = *(time_t*)ptr;
	ptr += sizeof tm;

	ts = *(time_t*)ptr;
	ptr += sizeof ts;

	if (t == FILENODE || t == FOLDERNODE)
	{
		int keylen = ((t == FILENODE) ? FILENODEKEYLENGTH+0 : FOLDERNODEKEYLENGTH+0);

		if (ptr+keylen+8+sizeof(short) > end) return NULL;

		k = (const byte*)ptr;
		ptr += keylen;
	}

	if (t == FILENODE)
	{
		ll = *(unsigned short*)ptr;
		ptr += sizeof ll;
		if (ptr+ll > end || ptr[ll]) return NULL;
		fa = ptr;
		ptr += ll;
	}
	else fa = NULL;

	for (i = 8; i--; ) if (ptr+*(unsigned char*)ptr < end) ptr += *(unsigned char*)ptr+1;

	if (i >= 0) return NULL;

	short numshares = *(short*)ptr;
	ptr += sizeof(numshares);

	if (numshares)
	{
		if (ptr+SymmCipher::KEYLENGTH > end) return 0;
		skey = (const byte*)ptr;
		ptr += SymmCipher::KEYLENGTH;
	}
	else skey = NULL;

	n = new Node(client,dp,h,ph,t,s,u,fa,ts,tm);

	if (k) n->setkey(k);

	if (numshares)
	{
		// read inshare or outshares
		while (Share::unserialize(client,(numshares > 0) ? -1 : 0,h,skey,&ptr,end) && numshares > 0 && --numshares);
	}

	ptr = n->attrs.unserialize(ptr,end-ptr);

	n->setfingerprint();

	if (ptr == end)	return n;
	else return NULL;
}

// serialize node - nodes with pending or RSA keys are unsupported
bool Node::serialize(string* d)
{
	// do not update state if undecrypted nodes are present
	if (attrstring.size()) return false;

	switch (type)
	{
		case FILENODE:
			if ((int)nodekey.size() != FILENODEKEYLENGTH) return false;
			break;
		case FOLDERNODE:
			if ((int)nodekey.size() != FOLDERNODEKEYLENGTH) return false;
			break;
		default:
			if (nodekey.size()) return false;
	}

	unsigned short ll;
	short numshares;
	string t;
	m_off_t s;

	s = type ? -type : size;

	d->append((char*)&s,sizeof s);

	d->append((char*)&nodehandle,MegaClient::NODEHANDLE);

	if (parent) d->append((char*)&parent->nodehandle,MegaClient::NODEHANDLE);
	else d->append("\0\0\0\0\0",MegaClient::NODEHANDLE);

	d->append((char*)&owner,MegaClient::USERHANDLE);

	d->append((char*)&clienttimestamp,sizeof(clienttimestamp));
	d->append((char*)&ctime,sizeof(ctime));

	d->append(nodekey);

	if (type == FILENODE)
	{
		ll = (short)fileattrstring.size()+1;
		d->append((char*)&ll,sizeof ll);
		d->append(fileattrstring.c_str(),ll);
	}

	d->append("\0\0\0\0\0\0\0",8);

	if (inshare) numshares = -1;
	else numshares = (short)outshares.size();

	d->append((char*)&numshares,sizeof numshares);

	if (numshares)
	{
		d->append((char*)sharekey->key,SymmCipher::KEYLENGTH);

		if (inshare) inshare->serialize(d);
		else for (share_map::iterator it = outshares.begin(); it != outshares.end(); it++) it->second->serialize(d);
	}

	attrs.serialize(d);

	return true;
};

// copy remainder of quoted string (no unescaping, use for base64 data only)
void Node::copystring(string* s, const char* p)
{
	if (p)
	{
		const char* pp;

		if ((pp = strchr(p,'"'))) s->assign(p,pp-p);
		else *s = p;
	}
	else s->clear();
}

// decrypt attrstring and check magic number prefix
byte* Node::decryptattr(SymmCipher* key, const char* attrstring, int attrstrlen)
{
	if (attrstrlen)
	{
		int l = attrstrlen*3/4+3;
		byte* buf = new byte[l];

		l = Base64::atob(attrstring,buf,l);

		if (!(l & (SymmCipher::BLOCKSIZE-1)))
		{
			key->cbc_decrypt(buf,l);

			if (!memcmp(buf,"MEGA{\"",6)) return buf;
		}

		delete[] buf;
	}

	return NULL;
}

// decrypt attributes and build attribute hash
void Node::setattr()
{
	byte* buf;

	if (attrstring.size() && (buf = decryptattr(&key,attrstring.c_str(),attrstring.size())))
	{
		JSON json;
		nameid name;
		string* t;

		json.begin((char*)buf+5);

		while ((name = json.getnameid()) != EOO && json.storeobject((t = &attrs.map[name]))) JSON::unescape(t);

		setfingerprint();

		delete[] buf;

		attrstring.clear();
	}
}

// if present, configure FileFingerprint from attributes
// otherwise, the file's fingerprint is derived from the file's mtime/size/key
void Node::setfingerprint()
{
	if (type == FILENODE)
	{
		if (fingerprint_it != client->fingerprints.end()) client->fingerprints.erase(fingerprint_it);

		attr_map::iterator it = attrs.map.find('c');

		if (it != attrs.map.end()) unserializefingerprint(&it->second);

		// if we lack a valid FileFingerprint for this file, use file's key, size and client timestamp instead
		if (!isvalid)
		{
			memcpy(crc,nodekey.data(),sizeof crc);
			mtime = clienttimestamp;
		}

		fingerprint_it = client->fingerprints.insert((FileFingerprint*)this);
	}
}

// return file/folder name or special status strings
const char* Node::displayname()
{
	// not yet decrypted
	if (attrstring.size()) return "NO_KEY";

	attr_map::iterator it;

	it = attrs.map.find('n');

	if (it == attrs.map.end()) return "CRYPTO_ERROR";
	if (!it->second.size()) return "BLANK";
	return it->second.c_str();
}

// returns position of file attribute or 0 if not present
int Node::hasfileattribute(fatype t) const
{
	char buf[16];

	sprintf(buf,":%u*",t);
	return fileattrstring.find(buf)+1;
}

// attempt to apply node key - clears keystring if successful
bool Node::applykey()
{
	if (!keystring.length()) return false;

	int l = -1, t = 0;
	handle h;
	const char* k = NULL;
	SymmCipher* sc = &client->key;
	handle me = client->loggedin() ? client->me : *client->rootnodes;

	while ((t = keystring.find_first_of(':',t)) != (int)string::npos)
	{
		// compound key: locate suitable subkey (always symmetric)
		h = 0;

		l = Base64::atob(keystring.c_str()+(keystring.find_last_of('/',t)+1),(byte*)&h,sizeof h);
		t++;

		if (l == MegaClient::USERHANDLE)
		{
			// this is a user handle - reject if it's not me
			if (h != me) continue;
		}
		else
		{
			// look for share key if not folder access with folder master key
			if (h != me)
			{
				Node* n;

				// this is a share node handle - check if we have node and the share key
				if (!(n = client->nodebyhandle(h)) || !n->sharekey) continue;
				sc = n->sharekey;
			}
		}

		k = keystring.c_str()+t;
		break;
	}

	// no: found => personal key, use directly
	// otherwise, no suitable key available yet - bail (it might arrive soon)
	if (!k)
	{
		if (l < 0) k = keystring.c_str();
		else return false;
	}

	byte key[FILENODEKEYLENGTH];

	if (client->decryptkey(k,key,(type == FILENODE) ? FILENODEKEYLENGTH+0 : FOLDERNODEKEYLENGTH+0,sc,0,nodehandle))
	{
		keystring.clear();
		setkey(key);
	}

	return true;
}

// update node key and decrypt attributes
void Node::setkey(const byte* newkey)
{
	if (newkey) nodekey.assign((char*)newkey,(type == FILENODE) ? FILENODEKEYLENGTH+0 : FOLDERNODEKEYLENGTH+0);
	key.setkey((const byte*)nodekey.data(),type);
	setattr();
}

// returns whether node was moved
bool Node::setparent(Node* p)
{
	if (p == parent) return false;

	if (parent) parent->children.erase(child_it);

	parent = p;

	child_it = parent->children.insert(parent->children.end(),this);

	return true;
}

// returns 1 if n is under p, 0 otherwise
bool Node::isbelow(Node* p) const
{
	const Node* n = this;

	for (;;)
	{
		if (!n) return false;
		if (n == p) return true;
		n = n->parent;
	}
}

// enqueue in master queue for a class of remote operations
void LocalNode::enqremote(syncremote r)
{
	if (remoteq >= SYNCREMOTEAFFECTED) sync->client->syncremoteq[r].erase(remoteq_it);

	if (r >= SYNCREMOTEAFFECTED) remoteq_it = sync->client->syncremoteq[r].insert(sync->client->syncremoteq[r].end(),this);
}

// set, change or remove LocalNode's parent and name/localname/slocalname.
// clocalpath must be a full path and must not point to an empty string.
// no shortname allowed as the last path component.
void LocalNode::setnameparent(LocalNode* newparent, string* newlocalpath)
{
	bool newnode = !localname.size();

	if (parent)
	{
		// remove existing child linkage
		parent->children.erase(&localname);
		if (slocalname.size()) parent->schildren.erase(&slocalname);
	}

	if (newlocalpath)
	{
		// extract name component from localpath, check for rename unless newnode
		int p;
		
		for (p = newlocalpath->size(); p -= sync->client->fsaccess->localseparator.size(); )
		{
			if (!memcmp(newlocalpath->data()+p,sync->client->fsaccess->localseparator.data(),sync->client->fsaccess->localseparator.size()))
			{
				p += sync->client->fsaccess->localseparator.size();
				break;
			}
		}
	
		// has the name changed?
		if (localname.size() != newlocalpath->size()-p || memcmp(localname.data(),newlocalpath->data()+p,localname.size()))
		{
			// set new name
			localname.assign(newlocalpath->data()+p,newlocalpath->size()-p);

			name = localname;
			sync->client->fsaccess->local2name(&name);

			if (node)
			{
				if (name != node->attrs.map['n'])
				{
					// set new name
					node->attrs.map['n'] = name;
					sync->client->setattr(node);
				}
			}
		}
	}

	if (newparent)
	{
		if (newparent != parent)
		{
			parent = newparent;

			if (!newnode && node && parent->node)
			{
				// FIXME: detect if rename permitted, copy/delete if not
				sync->client->rename(node,parent->node);
			}
		}

		// (we don't construct a UTF-8 or sname for the root path)
		parent->children[&localname] = this;
		if (sync->client->fsaccess->getsname(newlocalpath,&slocalname)) parent->schildren[&slocalname] = this;
	}
}

// initialize fresh LocalNode object - must be called exactly once
void LocalNode::init(Sync* csync, nodetype ctype, LocalNode* cparent, string* clocalpath, string* cfullpath)
{
	sync = csync;
	parent = NULL;
	node = NULL;
	notseen = 0;
	remoteq = SYNCREMOTENOTSET;
	syncxfer = true;

	type = ctype;
	syncid = sync->client->nextsyncid();

	if (cparent) setnameparent(cparent,cfullpath);
	else localname = *cfullpath;

	scanseqno = sync->scanseqno;

	// mark fsid as not valid
	fsid_it	= sync->client->fsidnode.end();

	// enable folder notification
	if (type == FOLDERNODE) sync->dirnotify->addnotify(this,cfullpath);

	sync->client->syncactivity = true;

	sync->localnodes[type]++;
}

void LocalNode::setnode(Node* cnode)
{
	node = cnode;
	node->localnode = this;

	sync->client->syncidhandles[syncid] = node->nodehandle;
}

void LocalNode::setnotseen(int newnotseen)
{
	if (!newnotseen)
	{
		if (notseen) sync->client->localsyncnotseen.erase(notseen_it);
		notseen = 0;
	}
	else
	{
		if (!notseen) notseen_it = sync->client->localsyncnotseen.insert(this).first;
		notseen = newnotseen;
	}
}

void LocalNode::setfsid(handle newfsid)
{
	if ((fsid_it != sync->client->fsidnode.end()))
	{
		if (newfsid == fsid) return;

		sync->client->fsidnode.erase(fsid_it);
	}
	
	fsid = newfsid;
	
	pair<handlelocalnode_map::iterator,bool> r = sync->client->fsidnode.insert(pair<handle,LocalNode*>(fsid,this));

	// FIXME: alert if inode dupe detected
	if (r.second) fsid_it = r.first;
}

LocalNode::~LocalNode()
{
	setnotseen(0);

	// remove from fsidnode map, if present
	if (fsid_it != sync->client->fsidnode.end()) sync->client->fsidnode.erase(fsid_it);

	sync->localnodes[type]--;
	if (type == FILENODE && size > 0) sync->localbytes -= size;

	if (sync->state >= SYNC_INITIALSCAN)
	{
		// record deletion unless local node already marked as removed
		if (node && !node->removed) sync->client->syncdeleted[type].insert(syncid);
	}

	if (type == FOLDERNODE) sync->dirnotify->delnotify(this);

	// remove parent association
	if (parent) setnameparent(NULL,NULL);

	// remove from remoteq
	enqremote(SYNCREMOTENOTSET);
	
	for (localnode_map::iterator it = children.begin(); it != children.end(); ) delete it++->second;

	if (node)
	{
		node->syncdeleted = true;
		node->localnode = NULL;
	}
}

void LocalNode::getlocalpath(string* path, bool sdisable)
{
	LocalNode* l = this;

	path->erase();

	while (l)
	{
		// use short name, if available (less likely to overflow MAXPATH, perhaps faster?) and sdisable not set
		if (!sdisable && l->slocalname.size()) path->insert(0,l->slocalname);
		else path->insert(0,l->localname);

		if ((l = l->parent)) path->insert(0,sync->client->fsaccess->localseparator);
		
		if (sdisable) sdisable = false;
	}
}

void LocalNode::getlocalsubpath(string* path)
{
	LocalNode* l = this;

	path->erase();
	
	for (;;)
	{
		path->insert(0,l->localname);
		
		if (!(l = l->parent) || !l->parent) break;
		
		path->insert(0,sync->client->fsaccess->localseparator);
	}
}

// locate child by localname or slocalname
LocalNode* LocalNode::childbyname(string* localname)
{
	localnode_map::iterator it;

	if ((it = children.find(localname)) == children.end() && (it = schildren.find(localname)) == schildren.end()) return NULL;

	return it->second;
}

void LocalNode::prepare()
{
	getlocalpath(&transfer->localfilename,true);
}

void LocalNode::completed(Transfer* t, LocalNode*)
{
	if (parent)
	{
		if (!t->client->syncdeleted[type].count(syncid))
		{
			// if parent node exists, complete directly - otherwise, complete to SyncDebris or rubbish bin for later retrieval
			syncidhandle_map::iterator it = t->client->syncidhandles.find(parent->syncid);

			if (it != t->client->syncidhandles.end()) h = it->second;	// existing parent: synchronous completioh
			else
			{
				Node* p;

				// complete to //bin by default
				h = t->client->rootnodes[RUBBISHNODE-ROOTNODE];

				if ((p = t->client->nodebyhandle(h)))
				{
					// or to //bin/SyncDebris, if it exists
					if ((p = t->client->childnodebyname(p,MegaClient::SYNCDEBRISFOLDERNAME))) h = p->nodehandle;
				}
			}

			File::completed(t,this);
		}
	}
}

} // namespace
