/*

MEGA SDK - Client Access Engine Core Logic

(c) 2013 by Mega Limited, Wellsford, New Zealand

Author: mo
Bugfixing: js, mr

Applications using the MEGA API must present a valid application key
and comply with the the rules set forth in the Terms of Service.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#include "megaclient.h"

// FIXME: recreate filename after sync transfer completes to shortcut in-transfer rename handling
// FIXME: generate cr element for file imports
// FIXME: support invite links (including responding to sharekey requests)
// FIXME: Sync: recognize folder renames and use setattr() instead of potentially huge delete/putnodes sequence
// FIXME: instead of copying nodes, move if the source is in the rubbish to prevent unnecessary node creation

// root URL for API access
const char* const MegaClient::APIURL = "https://g.api.mega.co.nz/";

// //bin/SyncDebris/yyyy-mm-dd folder name
const char* const MegaClient::SYNCDEBRISFOLDERNAME = "SyncDebris";

// exported link marker
const char* const MegaClient::EXPORTEDLINK = "EXP";

// modified base64 conversion (no trailing '=' and '-_' instead of '+/')
unsigned char Base64::to64(byte c)
{
	c &= 63;
	if (c < 26) return c+'A';
	if (c < 52) return c-26+'a';
	if (c < 62) return c-52+'0';
	if (c == 62) return '-';
	return '_';
}

unsigned char Base64::from64(byte c)
{
	if (c >= 'A' && c <= 'Z') return c-'A';
	if (c >= 'a' && c <= 'z') return c-'a'+26;
	if (c >= '0' && c <= '9') return c-'0'+52;
	if (c == '-') return 62;
	if (c == '_') return 63;
	return 255;
}

int Base64::atob(const char* a, byte* b, int blen)
{
	byte c[4];
	int i;
	int p = 0;

	c[3] = 0;

	for (;;)
	{
		for (i = 0; i < 4; i++) if ((c[i] = from64(*a++)) == 255) break;

		if (p >= blen || !i) return p;
		b[p++] = (c[0] << 2) | ((c[1] & 0x30) >> 4);
		if (p >= blen || i < 3) return p;
		b[p++] = (c[1] << 4) | ((c[2] & 0x3c) >> 2);
		if (p >= blen || i < 4) return p;
		b[p++] = (c[2] << 6) | c[3];
	}

	return p;
}

int Base64::btoa(const byte* b, int blen, char* a)
{
	int p = 0;

	for (;;)
	{
		if (blen <= 0) break;
		a[p++] = to64(*b >> 2);
		a[p++] = to64((*b << 4) | (((blen > 1) ? b[1] : 0) >> 4));
		if (blen < 2) break;
		a[p++] = to64(b[1] << 2 | (((blen > 2) ? b[2] : 0) >> 6));
		if (blen < 3) break;
		a[p++] = to64(b[2]);

		blen -= 3;
		b += 3;
	}

	a[p] = 0;

	return p;
}

int Serialize64::serialize(byte* b, int64_t v)
{
	int p = 0;

	while (v)
	{
		b[++p] = (byte)v;
		v >>= 8;
	}

	return (*b = p)+1;
}

int Serialize64::unserialize(byte* b, int blen, int64_t* v)
{
	byte p = *b;

	if (p > sizeof(*v) || p >= blen) return -1;

	*v = 0;
	while (p) *v = (*v<<8)+b[(int)p--];

	return *b+1;
}

// pad and CBC-encrypt
void PaddedCBC::encrypt(string* data, SymmCipher* key)
{
	// pad to blocksize and encrypt
	data->append("E");
	data->resize((data->size()+key->BLOCKSIZE-1)&-key->BLOCKSIZE,'P');
	key->cbc_encrypt((byte*)data->data(),data->size());
}

// CBC-decrypt and unpad
bool PaddedCBC::decrypt(string* data, SymmCipher* key)
{
	if ((data->size() & (key->BLOCKSIZE-1))) return false;

	// decrypt and unpad
	key->cbc_decrypt((byte*)data->data(),data->size());

	size_t p = data->find_last_of('E');

	if (p == string::npos) return false;

	data->resize(p);

	return true;
}

// add or update record from string
bool DbTable::put(uint32_t index, string* data)
{
	return put(index,(char*)data->data(),data->size());
}

// add or update record with padding and encryption
bool DbTable::put(uint32_t type, Cachable* record, SymmCipher* key)
{
	string data;

	if (!record->serialize(&data)) return -1;

	PaddedCBC::encrypt(&data,key);

	if (!record->dbid) record->dbid = (nextid += IDSPACING) | type;

	return put(record->dbid,&data);
}

// get next record, decrypt and unpad
bool DbTable::next(uint32_t* type, string* data, SymmCipher* key)
{
	if (next(type,data))
	{
		if (!*type) return true;

		if (*type > nextid) nextid = *type & -IDSPACING;

		return PaddedCBC::decrypt(data,key);
	}

	return false;
}

DbTable::DbTable()
{
	nextid = 0;
}

Cachable::Cachable()
{
	dbid = 0;
	notified = 0;
}

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
	if (type == FILENODE && fingerprint_it != client->fingerprints.end()) client->fingerprints.erase(fingerprint_it);

	// delete outshares, including pointers from users for this node
	for (share_map::iterator it = outshares.begin(); it != outshares.end(); it++) delete it->second;

	// remove from parent's children
	if (parent) parent->children.erase(child_it);

	// delete child-parent associations (normally not used, as nodes are deleted bottom-up)
	for (node_list::iterator it = children.begin(); it != children.end(); it++) (*it)->parent = NULL;

	delete inshare;
	delete sharekey;

	if (localnode) localnode->node = NULL;

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

// decrypt key (symmetric or asymmetric), rewrite asymmetric to symmetric key
bool MegaClient::decryptkey(const char* sk, byte* tk, int tl, SymmCipher* sc, int type, handle node)
{
	int sl;
	const char* ptr = sk;

	// measure key length
	while (*ptr)
	{
		if (*ptr == '"' || *ptr == '/') break;
		ptr++;
	}

	sl = ptr-sk;

	if (sl > 4*Node::FILENODEKEYLENGTH/3+1)
	{
		// RSA-encrypted key - decrypt and update on the server to save CPU time next time
		sl = sl/4*3+3;

		if (sl > 4096) return false;

		byte* buf = new byte[sl];

		sl = Base64::atob(sk,buf,sl);

		// decrypt and set session ID for subsequent API communication
		if (!asymkey.decrypt(buf,sl,tk,tl))
		{
			delete[] buf;
			app->debug_log("Corrupt or invalid RSA node key detected");
			return false;
		}

		delete[] buf;

		if (!ISUNDEF(node))
		{
			if (type) sharekeyrewrite.push_back(node);
			else nodekeyrewrite.push_back(node);
		}
	}
	else
	{
		if (Base64::atob(sk,tk,tl) != tl)
		{
			app->debug_log("Corrupt or invalid symmetric node key");
			return false;
		}

		sc->ecb_decrypt(tk,tl);
	}

	return true;
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

void Node::setkey(const byte* newkey)
{
	if (newkey) nodekey.assign((char*)newkey,(type == FILENODE) ? FILENODEKEYLENGTH+0 : FOLDERNODEKEYLENGTH+0);
	key.setkey((const byte*)nodekey.data(),type);
	setattr();
}

Share::Share(User* u, accesslevel a, time_t t)
{
	user = u;
	access = a;
	ts = t;
}

void Share::serialize(string* d)
{
	handle uh = user ? user->userhandle : 0;
	char a = (char)access;

	d->append((char*)&uh,sizeof uh);
	d->append((char*)&ts,sizeof ts);
	d->append((char*)&a,1);
	d->append("",1);
}

bool Share::unserialize(MegaClient* client, int direction, handle h, const byte* key, const char** ptr, const char* end)
{
	if (*ptr+sizeof(handle)+sizeof(time_t)+2 > end) return 0;

	client->newshares.push_back(new NewShare(h,direction,*(handle*)*ptr,(accesslevel)(*ptr)[sizeof(handle)+sizeof(time_t)],*(time_t*)(*ptr+sizeof(handle)),key));

	*ptr += sizeof(handle)+sizeof(time_t)+2;

	return true;
}

// modify share
void Share::update(accesslevel a, time_t t)
{
	access = a;
	ts = t;
}

// coutgoing: < 0 - don't authenticate, > 0 - authenticate
NewShare::NewShare(handle ch, int coutgoing, handle cpeer, accesslevel caccess, time_t cts, const byte* ckey, const byte* cauth)
{
	h = ch;
	outgoing = coutgoing;
	peer = cpeer;
	access = caccess;
	ts = cts;

	if (ckey)
	{
		memcpy(key,ckey,sizeof key);
		have_key = 1;
	}
	else have_key = 0;

	if (outgoing > 0 && cauth)
	{
		memcpy(auth,cauth,sizeof auth);
		have_auth = 1;
	}
	else have_auth = 0;
}

// apply queued new shares
void MegaClient::mergenewshares(bool notify)
{
	newshare_list::iterator it;

	for (it = newshares.begin(); it != newshares.end(); )
	{
		Node* n;
		NewShare* s = *it;

		if ((n = nodebyhandle(s->h)))
		{
			if (!n->sharekey && s->have_key)
			{
				// setting an outbound sharekey requires node authentication unless coming from a trusted source (the local cache)
				bool auth = true;

				if (s->outgoing > 0)
				{
					if (!checkaccess(n,OWNERPRELOGIN))
					{
						app->debug_log("Attempt to create dislocated outbound share foiled");
						auth = false;
					}
					else
					{
						byte buf[SymmCipher::KEYLENGTH];

						handleauth(s->h,buf);

						if (memcmp(buf,s->auth,sizeof buf))
						{
							app->debug_log("Attempt to create forged outbound share foiled");
							auth = false;
						}
					}
				}

				if (auth) n->sharekey = new SymmCipher(s->key);
			}

			if (s->access == ACCESS_UNKNOWN && !s->have_key)
			{
				// share was deleted
				if (s->outgoing)
				{
					// outgoing share to user u deleted
					if (n->outshares.erase(s->peer) && notify) notifynode(n);

					// if no other outgoing shares remain on this node, erase sharekey
					if (!n->outshares.size())
					{
						delete n->sharekey;
						n->sharekey = NULL;
					}
				}
				else
				{
					// incoming share deleted - remove tree
					TreeProcDel td;
					proctree(n,&td);
					notifypurge();
				}
			}
			else
			{
				if (!ISUNDEF(s->peer))
				{
					if (s->outgoing)
					{
						// perform mandatory verification of outgoing shares: only on own nodes and signed unless read from cache
						if (checkaccess(n,OWNERPRELOGIN))
						{
							Share** sharep = &n->outshares[s->peer];

							// modification of existing share or new share
							if (*sharep) (*sharep)->update(s->access,s->ts);
							else *sharep = new Share(finduser(s->peer,1),s->access,s->ts);

							if (notify) notifynode(n);
						}
					}
					else
					{
						if (s->peer)
						{
							if (!checkaccess(n,OWNERPRELOGIN))
							{
								// modification of existing share or new share
								if (n->inshare) n->inshare->update(s->access,s->ts);
								else
								{
									n->inshare = new Share(finduser(s->peer,1),s->access,s->ts);
									n->inshare->user->sharing.insert(n->nodehandle);
								}

								if (notify) notifynode(n);
							}
							else app->debug_log("Invalid inbound share location");
						}
						else app->debug_log("Invalid null peer on inbound share");
					}
				}
			}

		}

		delete s;
		newshares.erase(it++);
	}
}

// approximate raw storage size of serialized AttrMap, not taking JSON escaping or name length into account
unsigned AttrMap::storagesize(int perrecord)
{
	unsigned t = 0;

	for (attr_map::iterator it = map.begin(); it != map.end(); it++) t += perrecord+it->second.size();

	return t;
}

int AttrMap::nameid2string(nameid id, char* buf)
{
	char* ptr = buf;

	for (int i = 64; (i -= 8) >= 0; ) if ((*ptr = ((id >> i) & 0xff))) ptr++;

	return ptr-buf;
}

// generate binary serialize of attr_map name-value pairs
void AttrMap::serialize(string* d)
{
	char buf[8];
	unsigned char l;
	unsigned short ll;

	for (attr_map::iterator it = map.begin(); it != map.end(); it++)
	{
		if ((l = nameid2string(it->first,buf)))
		{
			d->append((char*)&l,sizeof l);
			d->append(buf,l);
			ll = it->second.size();
			d->append((char*)&ll,sizeof ll);
			d->append(it->second.data(),ll);
		}
	}

	d->append("",1);
}

// read binary serialize, return final offset
const char* AttrMap::unserialize(const char* ptr, unsigned len)
{
	unsigned char l;
	unsigned short ll;
	nameid id;

	while ((l = *ptr++))
	{
		id = 0;
		while (l--) id = (id<<8)+(unsigned char)*ptr++;

		ll = *(short*)ptr;
		ptr += sizeof ll;
		map[id].assign(ptr,ll);
		ptr += ll;
	}

	return ptr;
}

// generate JSON object containing attr_map
void AttrMap::getjson(string* s)
{
	nameid id;
	char buf[8];
	const char* ptr;
	const char* pptr;

	// reserve estimated size of final string
	s->erase();
	s->reserve(storagesize(20));

	for (attr_map::iterator it = map.begin(); it != map.end(); it++)
	{
		s->append(s->size() ? ",\"" : "\"");

		if ((id = it->first))
		{
			// no JSON escaping here, as no escape characters are allowed in attribute names
			s->append(buf,nameid2string(id,buf));
			s->append("\":\"");

			// JSON-escape value
			pptr = ptr = it->second.c_str();

			for (int i = it->second.size(); i-- >= 0; ptr++)
			{
				if (i < 0 || (*ptr >= 0 && *ptr < ' ') || *ptr == '"' || *ptr == '\\')
				{
					if (ptr > pptr) s->append(pptr,ptr-pptr);

					if (i >= 0)
					{
						s->append("\\");

						switch (*ptr)
						{
							case '"':
								s->append("\"");
								break;
							case '\\':
								s->append("\\");
								break;
							case '\n':
								s->append("n");
								break;
							case '\r':
								s->append("r");
								break;
							case '\b':
								s->append("b");
								break;
							case '\f':
								s->append("f");
								break;
							case '\t':
								s->append("t");
								break;
							default:
								s->append("u00");
								sprintf(buf,"%02x",(unsigned char)*ptr);
								s->append(buf);
						}

						pptr = ptr+1;
					}
				}
			}

			s->append("\"");
		}
	}
}

User::User(const char* cemail)
{
	userhandle = UNDEF;
	show = VISIBILITY_UNKNOWN;
	ctime = 0;
	pubkrequested = 0;
	if (cemail) email = cemail;
}

bool User::serialize(string* d)
{
	unsigned char l;
	attr_map::iterator it;

	d->reserve(d->size()+100+attrs.storagesize(10));

	d->append((char*)&userhandle,sizeof userhandle);
	d->append((char*)&ctime,sizeof ctime);
	d->append((char*)&show,sizeof show);

	l = email.size();
	d->append((char*)&l,sizeof l);
	d->append(email.c_str(),l);

	d->append("\0\0\0\0\0\0\0",8);

	attrs.serialize(d);

	if (pubk.isvalid()) pubk.serializekey(d,AsymmCipher::PUBKEY);

	return true;
}

User* User::unserialize(MegaClient* client, string* d)
{
	handle uh;
	time_t ts;
	visibility v;
	unsigned char l;
	string m;
	User* u;
	const char* ptr = d->data();
	const char* end = ptr+d->size();
	int i;

	if (ptr+sizeof(handle)+sizeof(time_t)+sizeof(visibility)+2 > end) return NULL;

	uh = *(handle*)ptr;
	ptr += sizeof uh;

	ts = *(time_t*)ptr;
	ptr += sizeof ts;

	v = *(visibility*)ptr;
	ptr += sizeof v;

	l = *ptr++;
	if (l) m.assign(ptr,l);
	ptr += l;

	for (i = 8; i--; ) if (ptr+*(unsigned char*)ptr < end) ptr += *(unsigned char*)ptr+1;

	if (i >= 0 || !(u = client->finduser(uh,1))) return NULL;

	if (v == ME) client->me = uh;
	client->mapuser(uh,m.c_str());
	u->set(v,ts);

	if (ptr < end && !(ptr = u->attrs.unserialize(ptr,end-ptr))) return NULL;

	if (ptr < end && !u->pubk.setkey(AsymmCipher::PUBKEY,(byte*)ptr,end-ptr)) return NULL;

	return u;
}

// update user attributes
void User::set(visibility v, time_t ct)
{
	show = v;
	ctime = ct;
}

// store array or object in string s
// reposition after object
bool JSON::storeobject(string* s)
{
	int openobject[2] = { 0 };
	const char* ptr;

	while (*pos > 0 && *pos <= ' ') pos++;

	if (*pos == ',') pos++;

	ptr = pos;

	for (;;)
	{
		if (*ptr == '[' || *ptr == '{') openobject[*ptr == '[']++;
		else if (*ptr == ']' || *ptr == '}') openobject[*ptr == ']']--;
		else if (*ptr == '"')
		{
			ptr++;
			while (*ptr != '"' || ptr[-1] == '\\') ptr++;
		}
		else if ((*ptr >= '0' && *ptr <= '9') || *ptr == '-' || *ptr == '.')
		{
			ptr++;
			while ((*ptr >= '0' && *ptr <= '9') || *ptr == '.' || *ptr == 'e' || *ptr == 'E') ptr++;
			ptr--;
		}
		else if (*ptr != ':' && *ptr != ',') return false;

		ptr++;

		if (!openobject[0] && !openobject[1])
		{
			if (s)
			{
				if (*pos == '"') s->assign(pos+1,ptr-pos-2);
				else s->assign(pos,ptr-pos);
			}
			pos = ptr;
			return true;
		}
	}
}

bool JSON::isnumeric()
{
	if (*pos == ',') pos++;

	const char* ptr = pos;

	if (*ptr == '-') ptr++;

	return *ptr >= '0' && *ptr <= '9';
}

nameid JSON::getnameid(const char* ptr)
{
	nameid id = EOO;

	while (*ptr && *ptr != '"') id = (id<<8)+*ptr++;

	return id;
}

// pos points to [,]"name":...
// returns nameid and repositons pos after :
// no unescaping supported
nameid JSON::getnameid()
{
	const char* ptr = pos;
	nameid id = 0;

	if (*ptr == ',' || *ptr == ':') ptr++;

	if (*ptr++ == '"')
	{
		while (*ptr && *ptr != '"') id = (id<<8)+*ptr++;
		pos = ptr+2;
	}

	return id;
}

// specific string comparison/skipping
bool JSON::is(const char* value)
{
	if (*pos == ',') pos++;

	if (*pos != '"') return false;

	int t = strlen(value);

	if (memcmp(pos+1,value,t) || pos[t+1] != '"') return false;

	pos += t+2;

	return true;
}

// base64-decode binary value to designated fixed-length buffer
int JSON::storebinary(byte* dst, int dstlen)
{
	int l = 0;

	if (*pos == ',') pos++;

	if (*pos == '"')
	{
		l = Base64::atob(pos+1,dst,dstlen);

		// skip string
		storeobject();
	}

	return l;
}

// base64-decode binary value to designated string
bool JSON::storebinary(string* dst)
{
	if (*pos == ',') pos++;

	if (*pos == '"')
	{
		const char* ptr;

		if (!(ptr = strchr(pos+1,'"'))) return false;

		dst->resize((ptr-pos-1)/4*3+3);
		dst->resize(Base64::atob(pos+1,(byte*)dst->data(),dst->size()));

		// skip string
		storeobject();
	}

	return true;
}

// decode handle
handle JSON::gethandle(int size)
{
	byte buf[9] = { 0 };

	// no arithmetic or semantic comparisons will be performed on handles, so no endianness issues
	if (storebinary(buf,sizeof buf) == size) return *(handle*)buf;

	return UNDEF;
}

// decode integer
m_off_t JSON::getint()
{
	const char* ptr;

	if (*pos == ':' || *pos == ',') pos++;

	ptr = pos;
	if (*ptr == '"') ptr++;

	if ((*ptr < '0' || *ptr > '9') && *ptr != '-') return -1;

	handle r = atoll(ptr);
	storeobject();

	return r;
}

// decode float
double JSON::getfloat()
{
	if (*pos == ':' || *pos == ',') pos++;

	if ((*pos < '0' || *pos > '9') && *pos != '-' && *pos != '.') return -1;

	double r = atof(pos);

	storeobject();

	return r;
}

// return pointer to JSON payload data
const char* JSON::getvalue()
{
	const char* r;

	if (*pos == ':' || *pos == ',') pos++;

	if (*pos == '"') r = pos+1;
	else r = pos;

	storeobject();

	return r;
}

// try to to enter array
bool JSON::enterarray()
{
	if (*pos == ',') pos++;

	if (*pos == '[')
	{
		pos++;
		return true;
	}

	return false;
}

// leave array (must be at end of array)
bool JSON::leavearray()
{
	if (*pos == ']')
	{
		pos++;
		return true;
	}

	return false;
}

// try to enter object
bool JSON::enterobject()
{
	if (*pos == '}') pos++;
	if (*pos == ',') pos++;

	if (*pos == '{')
	{
		pos++;
		return true;
	}

	return false;
}

// leave object (skip remainder)
bool JSON::leaveobject()
{
	for (;;)
	{
		if (*pos == ':' || *pos == ',') pos++;
		else if (*pos == '"' || (*pos >= '0' && *pos <= '9') || *pos == '-' || *pos == '[' || *pos == '{') storeobject();
		else break;
	}

	if (*pos == '}')
	{
		pos++;
		return true;
	}

	return false;
}

// unescape JSON string (non-strict)
void JSON::unescape(string* s)
{
	char c;
	int l;

	for (unsigned i = 0; i+1 < s->size(); i++)
	{
		if ((*s)[i] == '\\')
		{
			switch ((*s)[i+1])
			{
				case 'n':
					c = '\n';
					l = 2;
					break;
				case 'r':
					c = '\r';
					l = 2;
					break;
				case 'b':
					c = '\b';
					l = 2;
					break;
				case 'f':
					c = '\f';
					l = 2;
					break;
				case 't':
					c = '\t';
					l = 2;
					break;
				case 'u':
					c = (MegaClient::hexval((*s)[i+4])<<4) | MegaClient::hexval((*s)[i+5]);
					l = 6;
					break;
				default:
					c = (*s)[i+1];
					l = 2;
			}

			s->replace(i,l,&c,1);
		}
	}
}

// position at start of object
void JSON::begin(const char* json)
{
	pos = json;
}

// timer with capped exponential backoff
BackoffTimer::BackoffTimer()
{
	reset();
}

void BackoffTimer::reset()
{
	next = 0;
	delta = 1;
}

void BackoffTimer::backoff(dstime ds)
{
	next = ds+delta;
	delta <<= 1;
	if (delta > 36000) delta = 36000;
}

void BackoffTimer::backoff(dstime ds, dstime newdelta)
{
	next = ds+newdelta;
	delta = newdelta;
}

void BackoffTimer::freeze()
{
	delta = next = ~(dstime)0;
}

bool BackoffTimer::armed(dstime ds) const
{
	return !next || ds >= next;
}

bool BackoffTimer::arm(dstime ds)
{
	if (next+delta > ds)
	{
		next = ds;
		delta = 1;

		return true;
	}

	return false;
}

dstime BackoffTimer::retryin(dstime ds)
{
	if (armed(ds)) return 0;

	return next-ds;
}

dstime BackoffTimer::backoff()
{
	return delta;
}

dstime BackoffTimer::nextset() const
{
	return (int)next;
}

// event in the future: potentially updates waituntil
// event in the past: zeros out waituntil and clears event
void BackoffTimer::update(dstime ds, dstime* waituntil)
{
	if (next)
	{
		if (next <= ds)
		{
			*waituntil = 0;
			next = 1;
		}
		else if (next < *waituntil) *waituntil = next;
	}
}

// configure for full account session access
void MegaClient::setsid(const byte* sid, unsigned len)
{
	auth = "&sid=";

	int t = auth.size();
	auth.resize(t+len*4/3+4);
	auth.resize(t+Base64::btoa(sid,len,(char*)(auth.c_str()+t)));
}

// configure for exported folder links access
void MegaClient::setrootnode(handle h)
{
	char buf[12];

	Base64::btoa((byte*)&h,NODEHANDLE,buf);

	auth = "&n=";
	auth.append(buf);
}

// set server-client sequence number
bool MegaClient::setscsn(JSON* j)
{
	handle t;

	if (j->storebinary((byte*)&t,sizeof t) != sizeof t) return false;

	Base64::btoa((byte*)&t,sizeof t,scsn);

	return true;
}

int MegaClient::nextreqtag()
{
	return ++reqtag;
}

int MegaClient::hexval(char c)
{
	return c > '9' ? c-'a'+10 : c-'0';
}

bool FileSystemAccess::islchex(char c)
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z');
}

// set warn level
void MegaClient::warn(const char* msg)
{
	app->debug_log(msg);
	warned = 1;
}

// reset and return warnlevel
bool MegaClient::warnlevel()
{
	return warned ? (warned = 0) | 1 : 0;
}

// returns the first matching child node by UTF-8 name (does not resolve name clashes)
Node* MegaClient::childnodebyname(Node* p, const char* name)
{
	for (node_list::iterator it = p->children.begin(); it != p->children.end(); it++) if (!strcmp(name,(*it)->displayname())) return *it;

	return NULL;
}

void MegaClient::init()
{
	for (int i = sizeof rootnodes/sizeof *rootnodes; i--; ) rootnodes[i] = UNDEF;

	pendingcs = NULL;
	pendingsc = NULL;

	btcs.reset();
	btsc.reset();
	btpfa.reset();

	me = UNDEF;

	syncadding = 0;
	movedebrisinflight = 0;
	syncdebrisadding = false;
	syncscanfailed = false;
	
	putmbpscap = 0;
}

MegaClient::MegaClient(MegaApp* a, Waiter* w, HttpIO* h, FileSystemAccess* f, DbAccess* d, const char* k)
{
	sctable = NULL;

	init();

	app = a;
	waiter = w;
	httpio = h;
	fsaccess = f;
	dbaccess = d;

	a->client = this;

	*scsn = 0;
	scsn[sizeof scsn-1] = 0;

	connections[PUT] = 3;
	connections[GET] = 4;

	int i;

	// initialize random client application instance ID
	for (i = sizeof sessionid; i--; ) sessionid[i] = 'a'+PrnGen::genuint32(26);

	// initialize random API request sequence ID
	for (i = sizeof reqid; i--; ) reqid[i] = 'a'+PrnGen::genuint32(26);

	warned = 0;

	userid = 0;

	r = 0;

	curfa = newfa.end();

	nextuh = 0;

	currsyncid = 0;

	syncactivity = false;

	snprintf(appkey,sizeof appkey,"&ak=%s",k);
}

MegaClient::~MegaClient()
{
	logout();

	delete pendingcs;
	delete pendingsc;
	delete sctable;
	delete dbaccess;
}

// nonblocking state machine executing all operations currently in progress
void MegaClient::exec()
{
	waiter->getdstime();

	do {
		// file attribute puts (handled sequentially, newest-to-oldest)
		if (curfa != newfa.end())
		{
			HttpReqCommandPutFA* fa = *curfa;

			switch (fa->status)
			{
				case REQ_SUCCESS:
					if (fa->in.size() == sizeof(handle))
					{
						// successfully wrote file attribute - store handle & remove from list
						handle fah = *(handle*)fa->in.data();

						Node* n;
						handle h;
						handlepair_set::iterator it;

						// do we have a valid upload handle?
						h = fa->th;

						it = uhnh.lower_bound(pair<handle,handle>(h,0));

						if (it != uhnh.end() && it->first == h) h = it->second;

						// are we updating a live node? issue command directly. otherwise, queue for processing upon upload completion.
						if ((n = nodebyhandle(h)) || (n = nodebyhandle(fa->th))) reqs[r].add(new CommandAttachFA(n->nodehandle,fa->type,fah,fa->tag));
						else pendingfa[pair<handle,fatype>(fa->th,fa->type)] = pair<handle,int>(fah,fa->tag);

						delete fa;
						newfa.erase(curfa);
					}

					btpfa.reset();
					curfa = newfa.end();
					break;

				case REQ_FAILURE:
					// repeat request with exponential backoff
					curfa = newfa.end();
					btpfa.backoff(waiter->ds);

				default:;
			}
		}

		if (newfa.size() && curfa == newfa.end() && btpfa.armed(waiter->ds))
		{
			// dispatch most recent file attribute put
			curfa = newfa.begin();

			(*curfa)->status = REQ_INFLIGHT;
			reqs[r].add(*curfa);
		}

		// file attribute fetching (handled in parallel on a per-cluster basis)
		fafc_map::iterator it;

		for (it = fafcs.begin(); it != fafcs.end(); )
		{
			// is this request currently in flight?
			switch (it->second->req.status)
			{
				case REQ_READY:
				case REQ_INFLIGHT:
					// implement timeout?
					break;

				case REQ_SUCCESS:
					it->second->parse(this,it->first,&it->second->req.in);
					it->second->bt.reset();
					break;

				case REQ_FAILURE:
					faf_failed(it->first);
					it->second->bt.backoff(waiter->ds);

				default:;
			}

			if (it->second->req.status != REQ_INFLIGHT && it->second->bt.armed(waiter->ds))
			{
				// no request in flight, but ready for next request - check for remaining fetches for this cluster
				faf_map::iterator itf;

				for (itf = fafs.begin(); itf != fafs.end(); itf++) if (itf->second->fac == it->first) break;

				if (itf != fafs.end())
				{
					// pending fetches present - dispatch
					reqs[r].add(new CommandGetFA(it->first,it->second->fahref));
					it->second->req.status = REQ_INFLIGHT;
					it++;
				}
				else
				{
					delete it->second;
					fafcs.erase(it++);
				}
			}
			else it++;
		}

		// API client-server requests
		for (;;)
		{
			// do we have an API request outstanding?
			if (pendingcs)
			{
				switch (pendingcs->status)
				{
					case REQ_READY:
					case REQ_INFLIGHT:
						// implement timeout?
						break;

					case REQ_SUCCESS:
						if (pendingcs->in != "-3")
						{
							if (*pendingcs->in.c_str() == '[')
							{
								// request succeeded, process result array
								json.begin(pendingcs->in.c_str());
								reqs[r^1].procresult(this);

								delete pendingcs;
								pendingcs = NULL;

								// increment unique request ID
								for (int i = sizeof reqid; i--; ) if (reqid[i]++ < 'z') break;
								else reqid[i] = 'a';
							}
							else
							{
								// request failed
								error e = (error)atoi(pendingcs->in.c_str());
								if (!e) e = API_EINTERNAL;
								app->request_error(e);
								break;
							}

							btcs.reset();
							break;
						}
						// fall through
					case REQ_FAILURE:	// failure, repeat with capped exponential backoff
						delete pendingcs;
						pendingcs = NULL;

						btcs.backoff(waiter->ds);
						app->notify_retry(btcs.retryin(waiter->ds));

					default:;
				}

				if (pendingcs) break;
			}

			if (btcs.armed(waiter->ds))
			{
				if (btcs.nextset()) r ^= 1;

				if (reqs[r].cmdspending())
				{
					pendingcs = new HttpReq();

					reqs[r].get(pendingcs->out);

					pendingcs->posturl = APIURL;

					pendingcs->posturl.append("cs?id=");
					pendingcs->posturl.append(reqid,sizeof reqid);
					pendingcs->posturl.append(auth);
					pendingcs->posturl.append(appkey);

					pendingcs->type = REQ_JSON;

					pendingcs->post(this);

					r ^= 1;
					continue;
				}
				else btcs.reset();
			}
			break;
		}

		// API server-client requests
		for (;;)
		{
			if (pendingsc)
			{
				if (scnotifyurl.size())
				{
					// pendingsc is a scnotifyurl connection
					if (pendingsc->status == REQ_SUCCESS || pendingsc->status == REQ_FAILURE)
					{
						delete pendingsc;
						pendingsc = NULL;

						scnotifyurl.clear();
					}
				}
				else
				{
					// pendingsc is a server-client API request
					switch (pendingsc->status)
					{
						case REQ_READY:
						case REQ_INFLIGHT:
							// TODO: timeout
							break;

						case REQ_SUCCESS:
							if (*pendingsc->in.c_str() == '{')
							{
								// FIXME: reload in case of bad JSON
								jsonsc.begin(pendingsc->in.c_str());
								procsc();

								delete pendingsc;
								pendingsc = NULL;

								btsc.reset();
								break;
							}
							else
							{
								error e = (error)atoi(pendingsc->in.c_str());

								if (e == API_ESID)
								{
									app->request_error(API_ESID);
									*scsn = 0;
								}
								else if (e == API_ETOOMANY)
								{
									app->debug_log("Too many pending updates - reloading local state");
									fetchnodes();
								}
							}
							// fall through
						case REQ_FAILURE:	// failure, repeat with capped exponential backoff
							delete pendingsc;
							pendingsc = NULL;

							btsc.backoff(waiter->ds);

						default:;
					}
				}

				if (pendingsc) break;
			}

			if (*scsn && btsc.armed(waiter->ds))
			{
				pendingsc = new HttpReq();

				if (scnotifyurl.size()) pendingsc->posturl = scnotifyurl;
				else
				{
					pendingsc->posturl = APIURL;
					pendingsc->posturl.append("sc?sn=");
					pendingsc->posturl.append(scsn);
					pendingsc->posturl.append(auth);
				}

				pendingsc->type = REQ_JSON;

				pendingsc->post(this);
			}
			break;
		}

		// fill transfer slots from the queue
		dispatchmore(PUT);
		dispatchmore(GET);

		// handle active transfers
		for (transferslot_list::iterator it = tslots.begin(); it != tslots.end(); ) (*it++)->doio(this);
	} while (httpio->doio() || (!pendingcs && reqs[r].cmdspending() && btcs.armed(waiter->ds)));

	if (syncscanfailed && syncscanbt.armed(waiter->ds)) syncscanfailed = false;

	// process active syncs
	if (syncs.size() || syncactivity)
	{
		syncactivity = false;

		bool syncscanning = false;
		sync_list::iterator it;

		// process pending scanstacks
		for (it = syncs.begin(); it != syncs.end(); )
		{
			// make sure that the remote synced folder still exists
			if (!(*it)->localroot.node) (*it)->changestate(SYNC_FAILED);

			if ((*it)->state == SYNC_FAILED) it++;
			else
			{
				// process items from the scanstack until depleted
				if ((*it)->scanstack.size()) (*it)->procscanstack();
			
				if ((*it)->scanstack.size())
				{
					syncscanning = true;
					it++;
				}
				else
				{
					if ((*it)->state == SYNC_INITIALSCAN)
					{
						// initial scan of this synced folder is now complete
						syncdown(&(*it)->localroot,&(*it)->localroot.localname);
						(*it++)->changestate(SYNC_ACTIVE);
					}
					else it++;
				}
			}
		}

		// process filesystem notifications for active syncs unless node addition currently in flight
		if (!syncscanning && !syncadding)
		{
			string localname;
			LocalNode* l;

			// check for filesystem changes
			while (fsaccess->notifynext(&syncs,&localname,&l)) l->sync->queuefsrecord(NULL,&localname,l,false);
						
			// rescan full trees in case fs notification is currently unreliable or unavailable
			if (fsaccess->notifyfailed())
			{
				unsigned totalnodes = 0;

				for (it = syncs.begin(); it != syncs.end(); it++) if ((*it)->state == SYNC_ACTIVE)
				{
					(*it)->queuescan(NULL,NULL,NULL,NULL,true);
					totalnodes += (*it)->localnodes[FILENODE]+(*it)->localnodes[FOLDERNODE];
				}
				
				// rescan periodically (interval depends on total tree size)
				syncscanfailed = true;
				syncscanbt.backoff(waiter->ds,10+totalnodes/128);
			}
			else syncscanfailed = false;

			// FIXME: only syncup for subtrees that were actually updated (to reduce CPU load)
			for (it = syncs.begin(); it != syncs.end(); it++) if ((*it)->state == SYNC_ACTIVE) syncup(&(*it)->localroot);

			syncupdate();
		}
	}
}

// determine what to wait for and for how long, and invoke the app's blocking facility if needed
// optional parameter: maximum number of deciseconds to wait
// returns true if an engine-relevant event has occurred, false otherwise
int MegaClient::wait()
{
	dstime ds, nds;

	// get current dstime and clear wait events
	ds = waiter->getdstime();

	// sync directory scans in progress? don't wait.
	if (syncactivity) nds = ds;
	else
	{
		// next retry of a failed transfer
		nds = nexttransferretry(PUT,~(dstime)0);
		nds = nexttransferretry(GET,nds);

		// retry failed client-server requests
		if (!pendingcs) btcs.update(ds,&nds);

		// retry failed server-client requests
		if (!pendingsc && *scsn) btsc.update(ds,&nds);

		// retry failed file attribute puts
		if (curfa == newfa.end()) btpfa.update(ds,&nds);

		// retry failed file attribute gets
		for (fafc_map::iterator it = fafcs.begin(); it != fafcs.end(); it++) if (it->second->req.status != REQ_INFLIGHT) it->second->bt.update(ds,&nds);
		
		// sync rescan
		if (syncscanfailed) syncscanbt.update(ds,&nds);
	}

	if (nds)
	{
		// nds is either MAX_INT (== no pending events) or > ds
		if (nds+1) nds -= ds;

		waiter->init(nds);
		waiter->wakeupby(httpio);
		waiter->wakeupby(fsaccess);

		return waiter->wait();
	}

	return Waiter::NEEDEXEC;
}

// add events to wakeup criteria
void Waiter::wakeupby(EventTrigger* et)
{
	et->addevents(this);
}

// reset all backoff timers
bool MegaClient::abortbackoff()
{
	bool r = false;
	dstime ds = waiter->getdstime();

	for (transfer_map::iterator it = transfers[GET].begin(); it != transfers[GET].end(); it++) if (it->second->bt.arm(ds)) r = 1;
	for (transfer_map::iterator it = transfers[PUT].begin(); it != transfers[PUT].end(); it++) if (it->second->bt.arm(ds)) r = 1;

	if (btcs.arm(ds)) r = true;

	if (!pendingsc && btsc.arm(ds)) r = true;

	if (curfa == newfa.end() && btpfa.arm(ds)) r = true;

	for (fafc_map::iterator it = fafcs.begin(); it != fafcs.end(); it++) if (it->second->req.status != REQ_INFLIGHT && it->second->bt.arm(ds)) r = true;

	return r;
}

// this will dispatch the next queued transfer unless one is already in progress and force isn't set
// returns true if dispatch occurred, false otherwise
bool MegaClient::dispatch(direction d)
{
	// do we have any transfer tslots available?
	if (!slotavail()) return false;

	transfer_map::iterator nextit;
	dstime ds = waiter->getdstime();
	TransferSlot *ts = NULL;

	for (;;)
	{
		nextit = transfers[d].end();

		for (transfer_map::iterator it = transfers[d].begin(); it != transfers[d].end(); it++)
		{
			if (!it->second->slot && it->second->bt.armed(ds) && (nextit == transfers[d].end() || it->second->bt.retryin(ds) < nextit->second->bt.retryin(ds))) nextit = it;
		}

		// no inactive transfers ready?
		if (nextit == transfers[d].end()) return false;

		if (!nextit->second->localfilename.size())
		{
			// this is a fresh transfer rather than the resumption of a partly completed and deferred one
			if (d == PUT)
			{
				// generate fresh random encryption key/CTR IV for this file
				byte keyctriv[SymmCipher::KEYLENGTH+sizeof(int64_t)];
				PrnGen::genblock(keyctriv,sizeof keyctriv);
				nextit->second->key.setkey(keyctriv);
				nextit->second->ctriv = *(uint64_t*)(keyctriv+SymmCipher::KEYLENGTH);
			}
			else
			{
				// set up keys for the decryption of this file (k == NULL => private node)
				Node* n;
				const byte* k;

				// locate suitable template file
				for (file_list::iterator it = nextit->second->files.begin(); it != nextit->second->files.end(); it++)
				{
					// FIXME: add support for exported file links
					if ((n = nodebyhandle((*it)->h)) && n->type == FILENODE)
					{
						k = (const byte*)n->nodekey.data();

						nextit->second->key.setkey(k,FILENODE);
						nextit->second->ctriv = *(int64_t*)(k+SymmCipher::KEYLENGTH);
						nextit->second->metamac = *(int64_t*)(k+SymmCipher::KEYLENGTH+sizeof(int64_t));

						// FIXME: re-add support for partial transfers

						// the size field must be valid right away for MegaClient::moretransfers()
						nextit->second->size = n->size;

						break;
					}
				}
			}

			// set localname
			for (file_list::iterator it = nextit->second->files.begin(); !nextit->second->localfilename.size() && it != nextit->second->files.end(); it++) (*it)->prepare();

			// app-side transfer preparations (populate localname, create thumbnail...)
			app->transfer_prepare(nextit->second);
		}

		// verify that a local path was given and start/resume transfer
		if (nextit->second->localfilename.size())
		{
			// allocate transfer slot
			ts = new TransferSlot(nextit->second);

			// try to open file
			if (ts->file->fopen(&nextit->second->localfilename,d == PUT,d == GET))
			{
				handle h = UNDEF;

				nextit->second->pos = 0;

				// always (re)start upload from scratch
				if (d == PUT)
				{
					nextit->second->size = ts->file->size;
					nextit->second->chunkmacs.clear();
				}
				else
				{
					// downloads resume at the end of the last contiguous completed block
					for (chunkmac_map::iterator it = nextit->second->chunkmacs.begin(); it != nextit->second->chunkmacs.end(); it++)
					{
						if (nextit->second->pos != it->first) break;
						nextit->second->pos = ChunkedHash::chunkceil(nextit->second->pos);
					}

					for (file_list::iterator it = nextit->second->files.begin(); it != nextit->second->files.end(); it++) if (nodebyhandle((*it)->h))
					{
						h = (*it)->h;
						break;
					}
				}

				// uploads always start at position 0, downloads resume at the p
				// dispatch request for temporary source/target URL
				reqs[r].add((ts->pendingcmd = (d == PUT) ? (Command*)new CommandPutFile(ts,putmbpscap) : (Command*)new CommandGetFile(ts,h,1)));

				ts->slots_it = tslots.insert(tslots.begin(),ts);

				// notify the app about the starting transfer
				for (file_list::iterator it = nextit->second->files.begin(); it != nextit->second->files.end(); it++) (*it)->start();

				return true;
			}
		}

		// file didn't open - fail & defer
		nextit->second->failed(API_EREAD);
	}
}

Transfer::Transfer(MegaClient* cclient, direction ctype)
{
	type = ctype;
	client = cclient;

	failcount = 0;
	uploadhandle = 0;
	slot = NULL;
}

// delete transfer with underlying slot, notify files
Transfer::~Transfer()
{
	client->transfers[type].erase(transfers_it);
	delete slot;
}

// transfer attempt failed, notify all related files, collect request on whether to abort the transfer, kill transfer if unanimous
void Transfer::failed(error e)
{
	bool defer;

	bt.backoff(client->waiter->ds);
	client->app->transfer_failed(this,e);

	for (file_list::iterator it = files.begin(); it != files.end(); it++) if ((*it)->failed(e) && !defer) defer = true;

	if (defer)
	{
		failcount++;
		delete slot;
	}
	else delete this;
}

File::File()
{
	transfer = NULL;
}

File::~File()
{
	// if transfer currently running, stop
	if (transfer) transfer->client->stopxfer(this);
}

void File::prepare()
{
	transfer->localfilename = localname;
}

// generate upload handle for this upload
// (after 65536 uploads, a node handle clash is possible, but far too unlikely to be of concern)
handle MegaClient::getuploadhandle()
{
	byte* ptr = (byte*)(&nextuh+1);

	while (!++(*--ptr));

	return nextuh;
}

// clear transfer queue
void MegaClient::freeq(direction d)
{
	for (transfer_map::iterator it = transfers[d].begin(); it != transfers[d].end(); it++) delete it->second;

	transfers[d].clear();
}

// time at which next undispatched transfer retry occurs
dstime MegaClient::nexttransferretry(direction d, dstime dsmin)
{
	for (transfer_map::iterator it = transfers[d].begin(); it != transfers[d].end(); it++)
	{
		if (!it->second->slot && it->second->bt.nextset() && it->second->bt.nextset() < dsmin) dsmin = it->second->bt.nextset();
	}

	return dsmin;
}

// disconnect all HTTP connections (slows down operations, but is semantically neutral)
void MegaClient::disconnect()
{
	if (pendingcs) pendingcs->disconnect();
	if (pendingsc) pendingsc->disconnect();

	for (transferslot_list::iterator it = tslots.begin(); it != tslots.end(); it++) (*it)->disconnect();
	for (putfa_list::iterator it = newfa.begin(); it != newfa.end(); it++) (*it)->disconnect();
	for (fafc_map::iterator it = fafcs.begin(); it != fafcs.end(); it++) it->second->req.disconnect();
}

void MegaClient::logout()
{
	int i;

	disconnect();

	delete sctable;
	sctable = NULL;

	me = UNDEF;

	cachedscsn = UNDEF;

	freeq(GET);
	freeq(PUT);

	purgenodesusersabortsc();

	for (i = 2; i--; ) reqs[i].clear();

	for (putfa_list::iterator it = newfa.begin(); it != newfa.end(); ) delete *it;
	newfa.clear();

	for (faf_map::iterator it = fafs.begin(); it != fafs.end(); it++) delete it->second;
	fafs.clear();

	for (fafc_map::iterator it = fafcs.begin(); it != fafcs.end(); it++) delete it->second;
	fafcs.clear();

	pendingfa.clear();

	// erase master key & session ID
	key.setkey(SymmCipher::zeroiv);

	for (i = auth.size(); i--; ) ((char*)auth.c_str())[i] = 0;
	auth.clear();

	init();
}

// process server-client request
void MegaClient::procsc()
{
	nameid name;

	jsonsc.enterobject();

	for (;;)
	{
		switch (jsonsc.getnameid())
		{
			case 'w':
				if (!jsonsc.storeobject(&scnotifyurl)) return;
				break;

			case 'a':
				if (jsonsc.enterarray())
				{
					while (jsonsc.enterobject())
					{
						// the "a" attribute is guaranteed to be first in the object
						if (jsonsc.getnameid() == 'a')
						{
							name = jsonsc.getnameid();

							// only process action if not marked as self-originated (marker guaranteed to be next in sequence if present)
							if (memcmp(jsonsc.pos,"\"i\":\"",5) || memcmp(jsonsc.pos+5,sessionid,sizeof sessionid) || jsonsc.pos[5+sizeof sessionid] != '"')
							{
								switch (name)
								{
									case 'u':
										// node update
										sc_updatenode();
										break;

									case 't':
										// node addition
										sc_newnodes();
										mergenewshares(1);
										break;

									case 'd':
										// node deletion
										sc_deltree();
										break;

									case 's':
										// share addition/update/revocation
										if (sc_shares()) mergenewshares(1);
										break;

									case 'c':
										// contact addition/update
										sc_contacts();
										break;

									case 'k':
										// crypto key request
										sc_keys();
										break;

									case MAKENAMEID2('f','a'):
										// file attribute update
										sc_fileattr();
										break;

									case MAKENAMEID2('u','a'):
										// user attribtue update
										sc_userattr();

								}
							}
						}

						jsonsc.leaveobject();
					}

					jsonsc.leavearray();
				}
				break;

			case MAKENAMEID2('s','n'):
				setscsn(&jsonsc);
				break;

			case EOO:
				mergenewshares(1);
				applykeys();
				notifypurge();
				return;

			default:
				if (!jsonsc.storeobject()) return;
		}
	}
}

// update the user's local state cache
// (note that if immediate-completion commands have been issued in the meantime, the state of the affected nodes
// may be ahead of the recorded scsn - their consistency will be checked by subsequent server-client commands.)
// initsc() is called after all initial decryption has been performed, so we are tolerant towards incomplete/faulty nodes.
void MegaClient::initsc()
{
	if (sctable)
	{
		sctable->begin();

		sctable->truncate();

		bool complete;

		// 1. write current scsn
		handle tscsn;
		Base64::atob(scsn,(byte*)&tscsn,sizeof tscsn);
		complete = sctable->put(CACHEDSCSN,(char*)&tscsn,sizeof tscsn);

		if (complete)
		{
			// 2. write all users
			for (user_map::iterator it = users.begin(); it != users.end(); it++) if (!(complete = sctable->put(CACHEDUSER,&it->second,&key))) break;
		}

		if (complete)
		{
			// 3. write new or modified nodes, purge deleted nodes
			for (node_map::iterator it = nodes.begin(); it != nodes.end(); it++) if (!(complete = sctable->put(CACHEDNODE,it->second,&key))) break;
		}

		finalizesc(complete);
	}
}

// erase and and fill user's local state cache
void MegaClient::updatesc()
{
	if (sctable)
	{
		string t;

		sctable->get(CACHEDSCSN,&t);

		if (t.size() != sizeof cachedscsn) return;

		sctable->begin();

		bool complete;

		// 1. update associated scsn
		handle tscsn;
		Base64::atob(scsn,(byte*)&tscsn,sizeof tscsn);
		complete = sctable->put(CACHEDSCSN,(char*)&tscsn,sizeof tscsn);

		if (complete)
		{
			// 2. write new or update modified users
			for (user_vector::iterator it = usernotify.begin(); it != usernotify.end(); it++) if (!(complete = sctable->put(CACHEDUSER,*it,&key))) break;
		}

		if (complete)
		{
			// 3. write new or modified nodes, purge deleted nodes
			for (node_vector::iterator it = nodenotify.begin(); it != nodenotify.end(); it++)
			{
				if ((*it)->removed && (*it)->dbid)
				{
					if (!(complete = sctable->del((*it)->dbid))) break;
				}
				else if (!(complete = sctable->put(CACHEDNODE,*it,&key))) break;
			}
		}

		finalizesc(complete);
	}
}

// commit or purge local state cache
void MegaClient::finalizesc(bool complete)
{
	if (complete) sctable->commit();
	else
	{
		sctable->abort();
		sctable->truncate();

		app->debug_log("Cache update DB write error - disabling caching");

		delete sctable;
		sctable = NULL;
	}
}

HttpReqCommandPutFA::HttpReqCommandPutFA(MegaClient* client, handle cth, fatype ctype, byte* cdata, unsigned clen)
{
	cmd("ufa");
	arg("s",clen);

	persistent = 1;	// object will be recycled either for retry or for posting to the file attribute server

	th = cth;
	type = ctype;
	data = cdata;
	len = clen;

	binary = 1;

	tag = client->reqtag;
}

HttpReqCommandPutFA::~HttpReqCommandPutFA()
{
	delete[] data;
}

void HttpReqCommandPutFA::procresult()
{
	if (client->json.isnumeric()) status = REQ_FAILURE;
	else
	{
		const char* p = NULL;

		for (;;)
		{
			switch (client->json.getnameid())
			{
				case 'p':
					p = client->json.getvalue();
					break;

				case EOO:
					if (!p) status = REQ_FAILURE;
					else
					{
						Node::copystring(&posturl,p);
						post(client,(char*)data,len);
					}
					return;

				default:
					if (!client->json.storeobject()) return client->app->putfa_result(th,type,API_EINTERNAL);
			}
		}
	}
}

// post pending requests for this cluster to supplied URL
void FileAttributeFetchChannel::dispatch(MegaClient* client, int fac, const char* targeturl)
{
	// dispatch all pending fetches for this channel's cluster
	for (faf_map::iterator it = client->fafs.begin(); it != client->fafs.end(); it++)
	{
		if (it->second->fac == fac)
		{
			req.out->reserve(client->fafs.size()*sizeof(handle));	// prevent reallocations

			it->second->dispatched = 1;
			req.out->append((char*)&it->first,sizeof(handle));
		}
	}

	req.posturl = targeturl;
	req.post(client);
}

// communicate the result of a file attribute fetch to the application and remove completed records
void FileAttributeFetchChannel::parse(MegaClient* client, int fac, string* data)
{
	// data is structured (handle.8.le / position.4.le)* attribute data
	// attributes are CBC-encrypted with the file's key

	// we must have received at least one full header to continue
	if (data->size() < sizeof(handle)+sizeof(uint32_t)) return client->faf_failed(fac);

	uint32_t bod = *(uint32_t*)(data->data()+sizeof(handle));

	if (bod > data->size()) return client->faf_failed(fac);

	handle fah;
	const char* fadata;
	uint32_t falen, fapos;
	Node* n;
	faf_map::iterator it;

	fadata = data->data();

	for (unsigned h = 0; h < bod; h += sizeof(handle)+sizeof(uint32_t))
	{
		fah = *(handle*)(fadata+h);

		it = client->fafs.find(fah);

		// locate fetch request (could have been deleted by the application in the meantime)
		if (it != client->fafs.end())
		{
			// locate related node (could have been deleted)
			if ((n = client->nodebyhandle(it->second->nodehandle)))
			{
				fapos = *(uint32_t*)(fadata+h+sizeof(handle));
				falen = ((h+sizeof(handle)+sizeof(uint32_t) < bod) ? *(uint32_t*)(fadata+h+2*sizeof(handle)+sizeof(uint32_t)) : data->size())-fapos;

				if (!(falen & (SymmCipher::BLOCKSIZE-1)))
				{
					n->key.cbc_decrypt((byte*)fadata+fapos,falen);

					client->restag = it->second->tag;

					client->app->fa_complete(n,it->second->type,fadata+fapos,falen);
					delete it->second;
					client->fafs.erase(it);
				}
			}
		}
	}
}

// notify the application of the request failure and remove records no longer needed
void MegaClient::faf_failed(int fac)
{
	for (faf_map::iterator it = fafs.begin(); it != fafs.end(); )
	{
		if (it->second->fac == fac)
		{
			restag = it->second->tag;

			if (it->second->dispatched && app->fa_failed(it->second->nodehandle,it->second->type,it->second->retries))
			{
				delete it->second;
				fafs.erase(it++);
			}
			else
			{
				it->second->retries++;
				it++;
			}
		}
		else it++;
	}
}

FileAttributeFetchChannel::FileAttributeFetchChannel()
{
	req.binary = 1;
}

FileAttributeFetch::FileAttributeFetch(handle h, fatype t, int c, int ctag)
{
	nodehandle = h;
	type = t;
	fac = c;
	retries = 0;
	tag = ctag;
}

// queue node file attribute for retrieval or cancel retrieval
error MegaClient::getfa(Node* n, fatype t, int cancel)
{
	// locate this file attribute type in the nodes's attribute string
	handle fah;
	int p, pp;

	if (!(p = n->hasfileattribute(t))) return API_ENOENT;

	pp = p-1;
	while (pp && n->fileattrstring[pp-1] >= '0' && n->fileattrstring[pp-1] <= '9') pp--;

	if (p == pp) return API_ENOENT;

	if (Base64::atob(strchr(n->fileattrstring.c_str()+p,'*')+1,(byte*)&fah,sizeof(fah)) != sizeof(fah)) return API_ENOENT;

	if (cancel)
	{
		// cancel pending request
		faf_map::iterator it = fafs.find(fah);

		if (it != fafs.end())
		{
			delete it->second;
			fafs.erase(it);
			return API_OK;
		}

		return API_ENOENT;
	}
	else
	{
		int c = atoi(n->fileattrstring.c_str()+pp);

		// add file atttribute cluster channel and set cluster reference node handle
		FileAttributeFetchChannel** fafcp = &fafcs[c];
		if (!*fafcp) *fafcp = new FileAttributeFetchChannel();
		(*fafcp)->fahref = fah;

		// map returned handle to type/node upon retrieval response
		FileAttributeFetch** fafp = &fafs[fah];
		if (!*fafp) *fafp = new FileAttributeFetch(n->nodehandle,t,c,reqtag);

		return API_OK;
	}
}

CommandGetFA::CommandGetFA(int p, handle fahref)
{
	part = p;

	cmd("ufa");
	arg("fah",(byte*)&fahref,sizeof fahref);
}

void CommandGetFA::procresult()
{
	fafc_map::iterator it = client->fafcs.find(part);

	// (can never happen)
	if (it == client->fafcs.end()) return;

	if (client->json.isnumeric())
	{
		it->second->req.status = REQ_FAILURE;
		return;
	}

	const char* p = NULL;

	for (;;)
	{
		switch (client->json.getnameid())
		{
			case 'p':
				p = client->json.getvalue();
				break;

			case EOO:
				if (p)
				{
					string url;

					Node::copystring(&url,p);
					it->second->dispatch(client,part,url.c_str());
				}
				else it->second->req.status = REQ_FAILURE;
				return;

			default:
				if (!client->json.storeobject())
				{
					it->second->req.status = REQ_FAILURE;
					return;
				}
		}
	}
}

// build pending attribute string for this handle and remove
void MegaClient::pendingattrstring(handle h, string* fa)
{
	char buf[128];

	for (fa_map::iterator it = pendingfa.lower_bound(pair<handle,fatype>(h,0)); it != pendingfa.end() && it->first.first == h; )
	{
		sprintf(buf,"/%u*",(unsigned)it->first.second);
		Base64::btoa((byte*)&it->second.first,sizeof(it->second.first),strchr(buf+3,0));
		pendingfa.erase(it++);
		fa->append(buf+!fa->size());
	}
}

CommandAttachFA::CommandAttachFA(handle nh, fatype t, handle ah, int ctag)
{
	cmd("pfa");
	arg("n",(byte*)&nh,MegaClient::NODEHANDLE);

	char buf[64];

	sprintf(buf,"%u*",t);
	Base64::btoa((byte*)&ah,sizeof(ah),strchr(buf+2,0));
	arg("fa",buf);

	h = nh;
	type = t;
	tag = ctag;
}

void CommandAttachFA::procresult()
{
	client->app->putfa_result(h,type,client->json.isnumeric() ? (error)client->json.getint() : API_EINTERNAL);
}

// attach file attribute to a file
// FIXME: to avoid unnecessary roundtrips to the attribute servers, also store locally
void MegaClient::putfa(Transfer* transfer, fatype t, const byte* data, unsigned len)
{
	// build encrypted file attribute data block
	byte* cdata;
	unsigned clen = (len+SymmCipher::BLOCKSIZE-1)&-SymmCipher::BLOCKSIZE;

	cdata = new byte[clen];

	memcpy(cdata,data,len);
	memset(cdata+len,0,clen-len);

	transfer->key.cbc_encrypt(cdata,clen);

	if (!transfer->uploadhandle) transfer->uploadhandle = getuploadhandle();

	newfa.push_back(new HttpReqCommandPutFA(this,transfer->uploadhandle,t,cdata,clen));

	// no other file attribute storage request currently in progress? POST this one.
	if (curfa == newfa.end())
	{
		curfa = newfa.begin();
		reqs[r].add(*curfa);
	}
}

// request upload target URL
CommandPutFile::CommandPutFile(TransferSlot* cts, int ms)
{
	ts = cts;

	cmd("u");
	arg("s",ts->file->size);
	arg("ms",ms);
}

// set up file transfer with returned target URL
void CommandPutFile::procresult()
{
	ts->pendingcmd = NULL;
	if (canceled) return;

	if (client->json.isnumeric()) return ts->transfer->failed((error)client->json.getint());

	for (;;)
	{
		switch (client->json.getnameid())
		{
			case 'p':
				client->json.storeobject(&ts->tempurl);
				break;

			case EOO:
				if (ts->tempurl.size())
				{
					ts->starttime = ts->lastdata = client->waiter->ds;
					return ts->progress();
				}
				else return ts->transfer->failed(API_EINTERNAL);

			default:
				if (!client->json.storeobject()) return ts->transfer->failed(API_EINTERNAL);
		}
	}
}

// has the limit of concurrent transfer tslots been reached?
bool MegaClient::slotavail()
{
	return tslots.size() < MAXTRANSFERS;
}

// returns 1 if more transfers of the requested type can be dispatched (back-to-back overlap pipelining)
// FIXME: support overlapped partial reads (and support partial reads in the first place)
bool MegaClient::moretransfers(direction d)
{
	m_off_t c = 0, r = 0;
	dstime t = 0;
	int total = 0;

	// don't dispatch if all tslots busy
	if (!slotavail()) return false;

	// determine average speed and total amount of data remaining for the given direction
	for (transferslot_list::iterator it = tslots.begin(); it != tslots.end(); it++)
	{
		if ((*it)->transfer->type == d)
		{
			if ((*it)->starttime) t += waiter->ds-(*it)->starttime;
			c += (*it)->progressreported;
			r += (*it)->transfer->size-(*it)->progressreported;
			total++;
		}
	}

	// always blindly dispatch transfers up to MINPIPELINE
	if (r < MINPIPELINE) return true;

	// otherwise, don't allow more than two concurrent transfers
	if (t >= 2) return false;

	// dispatch more if less than two seconds of transfers left (at least 5 seconds must have elapsed for precise speed indication)
	if (t > 50)
	{
		int bpds = (int)(c/t);
		if (bpds > 100 && r/bpds < 20) return true;
	}

	return false;
}

void MegaClient::dispatchmore(direction d)
{
	// keep pipeline full by dispatching additional queued transfers, if appropriate and available
	while (moretransfers(d) && dispatch(d));
}

// request temporary source URL
// p == private node
CommandGetFile::CommandGetFile(TransferSlot* cts, handle h, int p)
{
	cmd("g");
	arg(p ? "n" : "p",(byte*)&h,MegaClient::NODEHANDLE);
	arg("g",1);

	ts = cts;
}

// decrypt returned attributes, open local file for writing and start transfer
void CommandGetFile::procresult()
{
	ts->pendingcmd = NULL;
	if (canceled) return;

	if (client->json.isnumeric()) return ts->transfer->failed((error)client->json.getint());

	const char* at = NULL;
	error e = API_EINTERNAL;
	m_off_t s = -1;
	int d = 0;
	byte* buf;

	for (;;)
	{
		switch (client->json.getnameid())
		{
			case 'g':
				client->json.storeobject(&ts->tempurl);
				break;

			case 's':
				s = client->json.getint();
				break;

			case 'd':
				d = 1;
				break;

			case MAKENAMEID2('a','t'):
				at = client->json.getvalue();
				break;

			case MAKENAMEID2('f','a'):
				client->json.storeobject(&ts->fileattrstring);
				break;

			case MAKENAMEID3('p','f','a'):
				ts->fileattrsmutable = (int)client->json.getint();
				break;

			case 'e':
				e = (error)client->json.getint();
				break;

			case EOO:
				if (d) return ts->transfer->failed(API_EBLOCKED);
				else
				{
					if (ts->tempurl.size() && s >= 0)
					{
						// decrypt at and set filename
						const char* eos = strchr(at,'"');
						string tmpfilename;

						if ((buf = Node::decryptattr(&ts->transfer->key,at,eos ? eos-at : strlen(at))))
						{
							JSON json;

							json.begin((char*)buf+5);

							for (;;)
							{
								switch (json.getnameid())
								{
									case 'n':
										if (!json.storeobject(&tmpfilename))
										{
											delete[] buf;
											return ts->transfer->failed(API_EINTERNAL);
										}
										break;

									case EOO:
										delete[] buf;
										ts->starttime = ts->lastdata = client->waiter->ds;
										return ts->progress();

									default:
										if (!json.storeobject())
										{
											delete[] buf;
											return ts->transfer->failed(API_EINTERNAL);
										}
								}
							}
						}
					}
				}

				return ts->transfer->failed(e);

			default:
				if (!client->json.storeobject()) return ts->transfer->failed(API_EINTERNAL);
		}
	}
}

// server-client node update processing
void MegaClient::sc_updatenode()
{
	handle h = UNDEF;
	handle u = 0;
//	const char* k = NULL;
	const char* a = NULL;
	time_t ts = -1, tm = -1;

	for (;;)
	{
		switch (jsonsc.getnameid())
		{
			case 'n':
				h = jsonsc.gethandle();
				break;

			case 'u':
				u = jsonsc.gethandle(USERHANDLE);
				break;

//			case 'k':
//				k = jsonsc.getvalue();
//				break;

			case MAKENAMEID2('a','t'):
				a = jsonsc.getvalue();
				break;

			case MAKENAMEID2('t','s'):
				ts = jsonsc.getint();
				break;

			case MAKENAMEID3('t','m','d'):
				tm = ts+jsonsc.getint();
				break;

			case EOO:
				if (!ISUNDEF(h))
				{
					Node* n;

					if ((n = nodebyhandle(h)))
					{
						if (u) n->owner = u;
						if (a) Node::copystring(&n->attrstring,a);
//						if (k) Node::copystring(&n->keystring,k);
						if (tm+1) n->clienttimestamp = tm;
						if (ts+1) n->ctime = ts;

						n->applykey();
						n->setattr();

						notifynode(n);
					}
				}
				return;

			default:
				if (!jsonsc.storeobject()) return;
		}
	}
}

// read tree object (nodes and users)
void MegaClient::readtree(JSON* j)
{
	if (j->enterobject())
	{
		for (;;)
		{
			switch (jsonsc.getnameid())
			{
				case 'f':
					readnodes(j,1);
					break;

				case 'u':
					readusers(j);
					break;

				case EOO:
					j->leaveobject();
					return;

				default:
					if (!jsonsc.storeobject()) return;
			}
		}
	}
}

// server-client newnodes processing
void MegaClient::sc_newnodes()
{
	for (;;)
	{
		switch (jsonsc.getnameid())
		{
			case 't':
				readtree(&jsonsc);
				break;

			case 'u':
				readusers(&jsonsc);
				break;

			case EOO:
				return;

			default:
				if (!jsonsc.storeobject()) return;
		}
	}
}

// share requests come in the following flavours:
// - n/k (set share key) (always symmetric)
// - n/o/u (share deletion)
// - n/o/u/k/r/ts[/ok][/ha] (share addition) (k can be asymmetric)
// returns 0 in case of a share addition or error, 1 otherwise
bool MegaClient::sc_shares()
{
	handle h = UNDEF;
	handle oh = UNDEF;
	handle uh = UNDEF;
	const char* k = NULL;
	const char* ok = NULL;
	byte ha[SymmCipher::BLOCKSIZE];
	byte sharekey[SymmCipher::BLOCKSIZE];
	int have_ha = 0;
	accesslevel r = ACCESS_UNKNOWN;
	time_t ts = 0;
	int outbound;

	for (;;)
	{
		switch (jsonsc.getnameid())
		{
			case 'n':	// share node
				h = jsonsc.gethandle();
				break;

			case 'o':	// owner user
				oh = jsonsc.gethandle(USERHANDLE);
				break;

			case 'u':	// target user
				uh = jsonsc.is(EXPORTEDLINK) ? 0 : jsonsc.gethandle(USERHANDLE);
				break;

			case MAKENAMEID2('o','k'):	// owner key
				ok = jsonsc.getvalue();
				break;

			case MAKENAMEID2('h','a'):	// outgoing share signature
				have_ha = Base64::atob(jsonsc.getvalue(),ha,sizeof ha) == sizeof ha;
				break;

			case 'r':	// share access level
				r = (accesslevel)jsonsc.getint();
				break;

			case MAKENAMEID2('t','s'):	// share timestamp
				ts = jsonsc.getint();
				break;

			case 'k':	// share key
				k = jsonsc.getvalue();
				break;

			case EOO:
				// we do not process share commands unless logged into a full account
				if (!loggedin() < FULLACCOUNT) return false;

				// need a share node
				if (ISUNDEF(h)) return false;

				// ignore unrelated share packets (should never be triggered)
				if (!ISUNDEF(oh) && !(outbound = oh == me) && uh != me) return false;

				// am I the owner of the share? use ok, otherwise k.
				if (ok && oh == me) k = ok;

				if (k)
				{
					if (!decryptkey(k,sharekey,sizeof sharekey,&key,1,h)) return false;

					if (ISUNDEF(oh) && ISUNDEF(uh))
					{
						// share key update on inbound share
						newshares.push_back(new NewShare(h,0,UNDEF,ACCESS_UNKNOWN,0,sharekey));
						return true;
					}

					if (!ISUNDEF(oh) && !ISUNDEF(uh))
					{
						// new share - can be inbound or outbound
						newshares.push_back(new NewShare(h,outbound,outbound ? uh : oh,r,ts,sharekey,have_ha ? ha : NULL));
						return false;
					}
				}
				else
				{
					if (!ISUNDEF(oh) && !ISUNDEF(uh))
					{
						// share revocation
						newshares.push_back(new NewShare(h,outbound,outbound ? uh : oh,ACCESS_UNKNOWN,0,NULL));
						return true;
					}
				}

				return false;

			default:
				if (!jsonsc.storeobject()) return false;
		}
	}
}

// user/contact updates come in the following format:
// u:[{c/m/ts}*] - Add/modify user/contact
void MegaClient::sc_contacts()
{
	for (;;)
	{
		switch (jsonsc.getnameid())
		{
			case 'u':
				readusers(&jsonsc);
				break;

			case EOO:
				return;

			default:
				if (!jsonsc.storeobject()) return;
		}
	}
}

// server-client key requests/responses
void MegaClient::sc_keys()
{
	handle h;
	Node* n = NULL;
	node_vector kshares;
	node_vector knodes;

	for (;;)
	{
		switch (jsonsc.getnameid())
		{
			case MAKENAMEID2('s','r'):
				procsr(&jsonsc);
				break;

			case 'h':
				h = jsonsc.gethandle();

				// we only distribute node keys for our own outgoing shares
				if (!ISUNDEF(h = jsonsc.gethandle()) && (n = nodebyhandle(h)) && n->sharekey && !n->inshare) kshares.push_back(n);
				break;

			case 'n':
				if (jsonsc.enterarray())
				{
					while (!ISUNDEF(h = jsonsc.gethandle()) && (n = nodebyhandle(h))) knodes.push_back(n);

					jsonsc.leavearray();
				}
				break;

			case MAKENAMEID2('c','r'):
				proccr(&jsonsc);
				break;

			case EOO:
				cr_response(&kshares,&knodes,NULL);
				return;

			default:
				if (!jsonsc.storeobject()) return;
		}
	}
}

// server-client file attribute update
void MegaClient::sc_fileattr()
{
	Node* n = NULL;
	const char* fa = NULL;

	for (;;)
	{
		switch (jsonsc.getnameid())
		{
			case MAKENAMEID2('f','a'):
				fa = jsonsc.getvalue();
				break;

			case 'n':
				handle h;
				if (!ISUNDEF(h = jsonsc.gethandle())) n = nodebyhandle(h);
				break;

			case EOO:
				if (fa && n)
				{
					Node::copystring(&n->fileattrstring,fa);
					notifynode(n);
				}
				return;

			default:
				if (!jsonsc.storeobject()) return;
		}
	}
}

// server-client user attribute update notification
void MegaClient::sc_userattr()
{
	string ua;
	handle uh = UNDEF;

	for (;;)
	{
		switch (jsonsc.getnameid())
		{
			case 'u':
				uh = jsonsc.gethandle(USERHANDLE);
				break;

			case MAKENAMEID2('u','a'):
				if (!ISUNDEF(uh))
				{
					User* u;

					if ((u = finduser(uh)))
					{
						if (jsonsc.enterarray())
						{
							while (jsonsc.storeobject(&ua))
							{
								if (ua[0] == '+') app->userattr_update(u,0,ua.c_str()+1);
								else if (ua[0] == '*') app->userattr_update(u,1,ua.c_str()+1);
							}

							jsonsc.leavearray();
							return;
						}
					}
				}

				jsonsc.storeobject();
				break;

			case EOO:
				return;

			default:
				if (!jsonsc.storeobject()) return;
		}
	}
}

// scan notified nodes for
// - name differences with an existing LocalNode
// - appearance of new folders
// - (re)appearance of files
// - deletions
// purge removed nodes after notification
void MegaClient::notifypurge(void)
{
	int i, t;

	updatesc();

	if ((t = nodenotify.size()))
	{
		applykeys();

		app->nodes_updated(&nodenotify[0],t);

		if (syncs.size())
		{
			attr_map::iterator ait;
			string localpath, newlocalpath;
			bool is_rename, is_move;

			// execute renames/moves locally
			for (i = 0; i < t; i++)
			{
				Node* n = nodenotify[i];

				if (!n->removed && n->localnode && !n->attrstring.size() && (ait = n->attrs.map.find('n')) != n->attrs.map.end())
				{
					// FIXME: handle moves into and out of the synced subtree (and between distinct synced subtrees)
					is_rename = ait->second != n->localnode->name;
					is_move = n->parent && n->localnode->parent && n->parent->localnode && n->localnode->parent != n->parent->localnode;

					if (is_rename || is_move)
					{
						if (n->localnode->parent) n->localnode->parent->children.erase(&n->localnode->localname);

						n->localnode->getlocalpath(this,&localpath);

						if (is_rename)
						{
							// file or folder was renamed
							n->localnode->name = ait->second;
							n->localnode->localname = ait->second;
							fsaccess->name2local(&n->localnode->localname);
						}

						if (is_move)
						{
							// file or folder was moved
							n->localnode->parent = n->parent->localnode;
						}

						n->localnode->parent->children[&n->localnode->localname] = n->localnode;
						n->localnode->getlocalpath(this,&newlocalpath);

						if (!fsaccess->renamelocal(&localpath,&newlocalpath) && fsaccess->copylocal(&localpath,&newlocalpath)) fsaccess->unlinklocal(&localpath);
					}
				}
			}

			// add new local files and folders
			// FIXME: call for topmost nodes only to eliminate scanning overhead
			for (i = 0; i < t; i++)
			{
				Node* n = nodenotify[i];

				if (!n->removed)
				{
					if (n->parent && n->parent->localnode)
					{
						n->parent->localnode->getlocalpath(this,&localpath);
						syncdown(n->parent->localnode,&localpath);
					}
				}
			}

			// delete deleted and removed local files, leaves first
			for (i = 0; i < t; i++)
			{
				Node* n = nodenotify[i];

				if (n->localnode && (n->removed || (n->parent && !n->parent->localnode)))
				{
					n->localnode->getlocalpath(this,&localpath);

					if (n->type == FILENODE)
					{
						FileAccess* fa = fsaccess->newfileaccess();

						if (fa->fopen(&localpath,1,0))
						{
							if (fa->mtime == n->mtime)
							{
								if (fa->size == n->size)
								{
									if (!n->genfingerprint(fa))
									{
										delete fa;
										fa = NULL;

										// make sure that the file we are deleting is actually the one we want to delete
										app->syncupdate_remote_unlink(n);
										fsaccess->rubbishlocal(&localpath);
										syncactivity = true;
									}
									// FIXME: else log "not deleting because of fingerprint mismatch"
								}
								// FIXME: else log "not deleting because of size mismatch"
							}
							// FIXME: else log "not deleting because of mtime mismatch"
						}

						delete fa;
					}
					else
					{
						app->syncupdate_remote_rmdir(n);
						fsaccess->rmdirlocal(&localpath);
						syncactivity = true;
					}
				}
			}
		}

		// check all notified nodes for removed status and purge
		for (i = 0; i < t; i++)
		{
			Node* n = nodenotify[i];

			if (n->removed)
			{
				// remove inbound share
				if (n->inshare)
				{
					n->inshare->user->sharing.erase(n->nodehandle);
					notifyuser(n->inshare->user);
				}

				nodes.erase(n->nodehandle);

				delete n;
			}
			else n->notified = 0;
		}

		nodenotify.clear();
	}

	// users are never deleted
	if ((t = usernotify.size()))
	{
		app->users_updated(&usernotify[0],t);
		usernotify.clear();
	}
}

// return node pointer derived from node handle
Node* MegaClient::nodebyhandle(handle h)
{
	node_map::iterator it;

	if ((it = nodes.find(h)) != nodes.end()) return it->second;

	return NULL;
}

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

// server-client deletion
void MegaClient::sc_deltree()
{
	Node* n = NULL;

	for (;;)
	{
		switch (jsonsc.getnameid())
		{
			case 'n':
				handle h;
				if (!ISUNDEF(h = jsonsc.gethandle())) n = nodebyhandle(h);
				break;

			case EOO:
				if (n)
				{
					TreeProcDel td;
					proctree(n,&td);
				}
				return;

			default:
				if (!jsonsc.storeobject()) return;
		}
	}
}

// generate handle authentication token
void MegaClient::handleauth(handle h, byte* auth)
{
	Base64::btoa((byte*)&h,NODEHANDLE,(char*)auth);
	memcpy(auth+sizeof h,auth,sizeof h);
	key.ecb_encrypt(auth);
}

// make attribute string; add magic number prefix
void MegaClient::makeattr(SymmCipher* key, string* attrstring, const char* json, int l)
{
	if (l < 0) l = strlen(json);
	int ll = (l+6+SymmCipher::KEYLENGTH-1)&-SymmCipher::KEYLENGTH;
	byte* buf = new byte[ll];

	memcpy(buf,"MEGA{",5);	// magic number
	memcpy(buf+5,json,l);
	buf[l+5] = '}';
	memset(buf+6+l,0,ll-l-6);

	key->cbc_encrypt(buf,ll);

	attrstring->assign((char*)buf,ll);

	delete[] buf;
}

// update node attributes - optional newattr is { name, value, name, value, ..., NULL }
// (with speculative instant completion)
error MegaClient::setattr(Node* n, const char** newattr)
{
	if (!checkaccess(n,FULL)) return API_EACCESS;

	if (newattr)
	{
		while (*newattr)
		{
			n->attrs.map[nameid(*newattr)] = newattr[1];
			newattr += 2;
		}
	}

	notifynode(n);
	notifypurge();

	reqs[r].add(new CommandSetAttr(this,n));

	return API_OK;
}

CommandSetAttr::CommandSetAttr(MegaClient* client, Node* n)
{
	cmd("a");
	notself(client);

	string at;

	n->attrs.getjson(&at);
	client->makeattr(&n->key,&at,at.c_str(),at.size());

	arg("n",(byte*)&n->nodehandle,MegaClient::NODEHANDLE);
	arg("at",(byte*)at.c_str(),at.size());

	byte key[Node::FILENODEKEYLENGTH];
	client->key.ecb_encrypt((byte*)n->nodekey.data(),key,n->nodekey.size());
	arg("k",key,n->nodekey.size());

	h = n->nodehandle;
	tag = client->reqtag;
}

void CommandSetAttr::procresult()
{
	if (client->json.isnumeric()) client->app->setattr_result(h,(error)client->json.getint());
	else
	{
		client->json.storeobject();
		client->app->setattr_result(h,API_EINTERNAL);
	}
}

// send new nodes to API for processing
void MegaClient::putnodes(handle h, NewNode* newnodes, int numnodes)
{
	reqs[r].add(new CommandPutNodes(this,h,NULL,newnodes,numnodes,reqtag));
}

// (the result is not processed directly - we rely on the server-client response)
CommandPutNodes::CommandPutNodes(MegaClient* client, handle th, const char* userhandle, NewNode* newnodes, int numnodes, int ctag, putsource csource)
{
	byte key[Node::FILENODEKEYLENGTH];
	int i;

	nn = newnodes;
	type = userhandle ? USER_HANDLE : NODE_HANDLE;
	source = csource;

	cmd("p");
	notself(client);

	if (userhandle) arg("t",userhandle);
	else arg("t",(byte*)&th,MegaClient::NODEHANDLE);

	beginarray("n");

	for (i = 0; i < numnodes; i++)
	{
		beginobject();

		switch (nn[i].source)
		{
			case NEW_NODE:
				arg("h",(byte*)&nn[i].nodehandle,MegaClient::NODEHANDLE);
				break;

			case NEW_PUBLIC:
				arg("ph",(byte*)&nn[i].nodehandle,MegaClient::NODEHANDLE);
				break;

			case NEW_UPLOAD:
				arg("h",nn[i].uploadtoken,sizeof nn->uploadtoken);

				// include pending file attributes for this upload
				string s;
				client->pendingattrstring(nn[i].uploadhandle,&s);
				if (s.size()) arg("fa",s.c_str(),1);
		}

		if (!ISUNDEF(nn[i].parenthandle)) arg("p",(byte*)&nn[i].parenthandle,MegaClient::NODEHANDLE);

		arg("t",nn[i].type);
		arg("a",(byte*)nn[i].attrstring.data(),nn[i].attrstring.size());

		if (nn[i].nodekey.size() <= sizeof key)
		{
			client->key.ecb_encrypt((byte*)nn[i].nodekey.data(),key,nn[i].nodekey.size());
			arg("k",key,nn[i].nodekey.size());
		}
		else arg("k",(const byte*)nn[i].nodekey.data(),nn[i].nodekey.size());

		arg("ts",nn[i].clienttimestamp);

		endobject();
	}

	endarray();

	// add cr element for new nodes, if applicable
	if (type == NODE_HANDLE)
	{
		Node* tn;

		if ((tn = client->nodebyhandle(th)))
		{
			ShareNodeKeys snk;

			for (i = 0; i < numnodes; i++)
			{
				switch (nn[i].source)
				{
					case NEW_NODE:
						snk.add((NodeCore*)(nn+i),tn,0);
						break;

					case NEW_UPLOAD:
						snk.add((NodeCore*)(nn+i),tn,0,nn[i].uploadtoken,(int)sizeof nn->uploadtoken);
						break;

					case NEW_PUBLIC:
						break;
				}
			}

			snk.get(this);
		}
	}

	tag = ctag;
}

// add new nodes and handle->node handle mapping
void CommandPutNodes::procresult()
{
	error e;

	if (client->json.isnumeric())
	{
		e = (error)client->json.getint();

		if (source == PUTNODES_SYNC) return client->putnodes_sync_result(e,nn);
		else if (source == PUTNODES_APP) return client->app->putnodes_result(e,type,nn);
		else return client->putnodes_syncdebris_result(e,nn);
	}

	e = API_EINTERNAL;

	for (;;)
	{
		switch (client->json.getnameid())
		{
			case 'f':
				if (client->readnodes(&client->json,1,source,nn,tag)) e = API_OK;
				break;

			default:
				if (client->json.storeobject()) continue;
				e = API_EINTERNAL;
				// fall through
			case EOO:
				client->applykeys();
				if (source == PUTNODES_SYNC) client->putnodes_sync_result(e,nn);
				else if (source == PUTNODES_APP) client->app->putnodes_result(e,type,nn);
				else client->putnodes_syncdebris_result(e,nn);
				client->notifypurge();
				return;
		}
	}
}

// drop nodes into a user's inbox (must have RSA keypair)
void MegaClient::putnodes(const char* user, NewNode* newnodes, int numnodes)
{
	User* u;

	restag = reqtag;

	if (!(u = finduser(user,1))) return app->putnodes_result(API_EARGS,USER_HANDLE,newnodes);

	queuepubkeyreq(u,new PubKeyActionPutNodes(newnodes,numnodes,reqtag));
}

PubKeyActionPutNodes::PubKeyActionPutNodes(NewNode* newnodes, int numnodes, int ctag)
{
	nn = newnodes;
	nc = numnodes;
	tag = ctag;
}

void PubKeyActionPutNodes::proc(MegaClient* client, User* u)
{
	if (u)
	{
		byte buf[AsymmCipher::MAXKEYLENGTH];
		int t;

		// re-encrypt all node keys to the user's public key
		for (int i = nc; i--; )
		{
			if (!(t = u->pubk.encrypt((const byte*)nn[i].nodekey.data(),nn[i].nodekey.size(),buf,sizeof buf))) return client->app->putnodes_result(API_EINTERNAL,USER_HANDLE,nn);

			nn[i].nodekey.assign((char*)buf,t);
		}

		client->reqs[client->r].add(new CommandPutNodes(client,UNDEF,u->uid.c_str(),nn,nc,tag));
	}
	else client->app->putnodes_result(API_ENOENT,USER_HANDLE,nn);
}

// returns 1 if node has accesslevel a or better, 0 otherwise
int MegaClient::checkaccess(Node* n, accesslevel a)
{
	// folder link access is always read-only - ignore login status during initial tree fetch
	if (a < OWNERPRELOGIN && !loggedin()) return a == RDONLY;

	// trace back to root node (always full access) or share node
	while (n)
	{
		if (n->inshare) return n->inshare->access >= a;
		if (!n->parent) return n->type > FOLDERNODE;

		n = n->parent;
	}

	return 0;
}

// returns API_OK if a move operation is permitted, API_EACCESS or API_ECIRCULAR otherwise
error MegaClient::checkmove(Node* fn, Node* tn)
{
	// condition 1: cannot move top-level node, must have full access to fn's parent
	if (!fn->parent || !checkaccess(fn->parent,FULL)) return API_EACCESS;

	// condition 2: target must be folder
	if (tn->type == FILENODE) return API_EACCESS;

	// condition 3: must have write access to target
	if (!checkaccess(tn,RDWR)) return API_EACCESS;

	// condition 4: tn must not be below fn (would create circular linkage)
	for (;;)
	{
		if (tn == fn) return API_ECIRCULAR;
		if (tn->inshare || !tn->parent) break;
		tn = tn->parent;
	}

	// condition 5: fn and tn must be in the same tree (same ultimate parent node or shared by the same user)
	for (;;)
	{
		if (fn->inshare || !fn->parent) break;
		fn = fn->parent;
	}

	// moves within the same tree or between the user's own trees are permitted
	if (fn == tn || (!fn->inshare && !tn->inshare)) return API_OK;

	// moves between inbound shares from the same user are permitted
	if (fn->inshare && tn->inshare && fn->inshare->user == tn->inshare->user) return API_OK;

	return API_EACCESS;
}

// move node to new parent node (for changing the filename, use setattr and modify the 'n' attribute)
error MegaClient::rename(Node* n, Node* p)
{
	error e;

	if ((e = checkmove(n,p))) return e;

	if (n->setparent(p))
	{
		notifynode(n);
		notifypurge();

		reqs[r].add(new CommandMoveNode(this,n,p));
	}

	return API_OK;
}

// returns 1 if new parent differs, 0 otherwise
bool Node::setparent(Node* p)
{
	if (p == parent) return false;

	if (parent) parent->children.erase(child_it);

	parent = p;

	child_it = parent->children.insert(parent->children.end(),this);

	return true;
}

CommandMoveNode::CommandMoveNode(MegaClient* client, Node* n, Node* t)
{
	cmd("m");
	notself(client);

	h = n->nodehandle;

	arg("n",(byte*)&h,MegaClient::NODEHANDLE);
	arg("t",(byte*)&t->nodehandle,MegaClient::NODEHANDLE);

	TreeProcShareKeys tpsk;
	client->proctree(n,&tpsk);
	tpsk.get(this);

	tag = client->reqtag;
}

void CommandMoveNode::procresult()
{
	if (client->json.isnumeric()) client->app->rename_result(h,(error)client->json.getint());
	else
	{
		client->json.storeobject();
		client->app->rename_result(h,API_EINTERNAL);
	}
}

// delete node tree
error MegaClient::unlink(Node* n)
{
	if (!checkaccess(n,FULL)) return API_EACCESS;

	reqs[r].add(new CommandDelNode(this,n->nodehandle));

	mergenewshares(1);

	TreeProcDel td;
	proctree(n,&td);
	notifypurge();

	return API_OK;
}

CommandDelNode::CommandDelNode(MegaClient* client, handle th)
{
	cmd("d");
	notself(client);

	arg("n",(byte*)&th,MegaClient::NODEHANDLE);

	h = th;
	tag = client->reqtag;
}

void CommandDelNode::procresult()
{
	if (client->json.isnumeric()) client->app->unlink_result(h,(error)client->json.getint());
	else
	{
		client->json.storeobject();
		client->app->unlink_result(h,API_EINTERNAL);
	}
}

// emulates the semantics of its JavaScript counterpart
// (returns NULL if the input is invalid UTF-8)
// unfortunately, discards bits 8-31 of multibyte characters
char* MegaClient::str_to_a32(const char* str, int* len)
{
	if (!str) return NULL;

	int t = strlen(str);
	int t2 = 4*((t+3)>>2);
	char* result = new char[t2]();
	uint32_t* a32 = (uint32_t*)result;
	uint32_t unicode;

	int i = 0;
	int j = 0;

	while (i < t)
	{
		char c = str[i++] & 0xff;

		if (!(c & 0x80)) unicode = c & 0xff;
		else if ((c & 0xe0) == 0xc0)
		{
			if (i >= t || (str[i] & 0xc0) != 0x80)
			{
				delete[] result;
				return NULL;
			}

			unicode = (c & 0x1f) << 6;
			unicode |= str[i++] & 0x3f;
		}
		else if ((c & 0xf0) == 0xe0)
		{
			if (i+2 > t || (str[i] & 0xc0) != 0x80 || (str[i+1] & 0xc0) != 0x80)
			{
				delete[] result;
				return NULL;
			}

			unicode = (c & 0x0f) << 12;
			unicode |= (str[i++] & 0x3f) << 6;
			unicode |= str[i++] & 0x3f;
		}
		else if ((c & 0xf8) == 0xf0)
		{
			if (i+3 > t || (str[i] & 0xc0) != 0x80 || (str[i+1] & 0xc0) != 0x80 || (str[i+2] & 0xc0) != 0x80)
			{
				delete[] result;
				return NULL;
			}

			unicode = (c & 0x07) << 18;
			unicode |= (str[i++] & 0x3f) << 12;
			unicode |= (str[i++] & 0x3f) << 6;
			unicode |= str[i++] & 0x3f;

			// management of surrogate pairs like the JavaScript code
			uint32_t hi = 0xd800 | ((unicode >> 10) & 0x3F) | (((unicode >> 16)-1) << 6);
			uint32_t low = 0xdc00 | (unicode & 0x3ff);

			a32[j>>2] |= htonl(hi << (24-(j & 3)*8));
			j++;

			unicode = low;
		}
		else
		{
			delete[] result;
			return NULL;
		}

		a32[j>>2] |= htonl(unicode << (24-(j & 3)*8));
		j++;
	}

	*len = j;
	return result;
}

// compute UTF-8 password hash
error MegaClient::pw_key(const char* utf8pw, byte* key)
{
	int t;
	char* pw;

	if (!(pw = str_to_a32(utf8pw,&t))) return API_EARGS;

	int n = (t+15)/16;
	SymmCipher* keys = new SymmCipher[n];

	for (int i = 0; i < n; i++)
	{
		int valid = (i != (n-1)) ? SymmCipher::BLOCKSIZE : (t-SymmCipher::BLOCKSIZE*i);
		memcpy(key,pw+i*SymmCipher::BLOCKSIZE,valid);
		memset(key+valid,0,SymmCipher::BLOCKSIZE-valid);
		keys[i].setkey(key);
	}

	memcpy(key,"\x93\xC4\x67\xE3\x7D\xB0\xC7\xA4\xD1\xBE\x3F\x81\x01\x52\xCB\x56",SymmCipher::BLOCKSIZE);

	for (int r = 65536; r--; )
		for (int i = 0; i < n; i++)
			keys[i].ecb_encrypt(key);

	delete[] keys;
	delete[] pw;

	return API_OK;
}

// compute generic string hash
void MegaClient::stringhash(const char* s, byte* hash, SymmCipher* cipher)
{
	int t;

	t = strlen(s) & -SymmCipher::BLOCKSIZE;

	strncpy((char*)hash,s+t,SymmCipher::BLOCKSIZE);

	while (t)
	{
		t -= SymmCipher::BLOCKSIZE;
		SymmCipher::xorblock((byte*)s+t,hash);
	}

	for (t = 16384; t--; ) cipher->ecb_encrypt(hash);

	memcpy(hash+4,hash+8,4);
}

// (transforms s to lowercase)
uint64_t MegaClient::stringhash64(string* s, SymmCipher* c)
{
	byte hash[SymmCipher::KEYLENGTH];

	transform(s->begin(),s->end(),s->begin(),::tolower);
	stringhash(s->c_str(),hash,c);

	return *(uint64_t*)hash;
}

// read and add/verify node array
int MegaClient::readnodes(JSON* j, int notify, putsource source, NewNode* nn, int tag)
{
	if (!j->enterarray()) return 0;

	node_vector dp;
	Node* n;
	int i = 0;

	while (j->enterobject())
	{
		handle h = UNDEF, ph = UNDEF;
		handle u = 0, su = UNDEF;
		nodetype t = TYPE_UNKNOWN;
		const char* a = NULL;
		const char* k = NULL;
		const char* fa = NULL;
		const char *sk = NULL;
		accesslevel rl = ACCESS_UNKNOWN;
		m_off_t s = ~(m_off_t)0;
		time_t ts = -1, tmd = 0, sts = -1;
		nameid name;

		while ((name = j->getnameid()) != EOO)
		{
			switch (name)
			{
				case 'h':	// new node: handle
					h = j->gethandle();
					break;

				case 'p':	// parent node
					ph = j->gethandle();
					break;

				case 'u': 	// owner user
					u = j->gethandle(USERHANDLE);
					break;

				case 't':	// type
					t = (nodetype)j->getint();
					break;

				case 'a':	// attributes
					a = j->getvalue();
					break;

				case 'k':	// key(s)
					k = j->getvalue();
					break;

				case 's':	// file size
					s = j->getint();
					break;

				case MAKENAMEID2('t','s'):	// actual creation timestamp
					ts = j->getint();
					break;

				case MAKENAMEID3('t','m','d'):	// user-controlled last modified time
					tmd = j->getint();
					break;

				case MAKENAMEID2('f','a'):	// file attributes
					fa = j->getvalue();
					break;

					// inbound share attributes
				case 'r':	// share access level
					rl = (accesslevel)j->getint();
					break;

				case MAKENAMEID2('s','k'):	// share key
					sk = j->getvalue();
					break;

				case MAKENAMEID2('s','u'):	// sharing user
					su = j->gethandle(USERHANDLE);
					break;
					
				case MAKENAMEID3('s','t','s'): 	// share timestamp
					sts = j->getint();
					break;
					
				default:
					if (!j->storeobject()) return 0;
			}
		}

		if (ISUNDEF(h)) warn("Missing node handle");
		else
		{
			if (t == TYPE_UNKNOWN) warn("Unknown node type");
			else if (t == FILENODE || t == FOLDERNODE)
			{
				if (ISUNDEF(ph)) warn("Missing parent");
				else if (!a) warn("Missing node attributes");
				else if (!k) warn("Missing node key");

				if (t == FILENODE && ISUNDEF(s)) warn("File node without file size");
			}
		}

		if (fa && t != FILENODE) warn("Spurious file attributes");

		if (!warnlevel())
		{
			if ((n = nodebyhandle(h)))
			{
				if (n->removed)
				{
					// node marked for deletion is being resurrected, possibly with a new parent (server-client move operation)
					n->removed = 0;

					if (!ISUNDEF(ph))
					{
						Node* p;

						if ((p = nodebyhandle(ph))) n->setparent(p);
						else
						{
							n->parenthandle = ph;
							dp.push_back(n);
						}
					}
				}
				else
				{
					// node already present - check for race condition
					if ((n->parent && ph != n->parent->nodehandle) || n->type != t) app->reload("Node inconsistency (parent linkage)");
				}
			}
			else
			{
				byte buf[SymmCipher::KEYLENGTH];

				if (!ISUNDEF(su))
				{
					if (t != FOLDERNODE) warn("Invalid share node type");
					if (rl == ACCESS_UNKNOWN) warn("Missing access level");
					if (!sk) warn("Missing share key for inbound share");

					if (warnlevel()) su = UNDEF;
					else decryptkey(sk,buf,sizeof buf,&key,1,h);
				}

				string fas;

				Node::copystring(&fas,fa);

				// fallback timestamps
				if (!(ts+1)) ts = time(NULL);
				if (!(sts+1)) sts = ts;
				
				n = new Node(this,&dp,h,ph,t,s,u,fas.c_str(),ts,ts+tmd);

				n->tag = tag;

				Node::copystring(&n->attrstring,a);
				Node::copystring(&n->keystring,k);

				if (!ISUNDEF(su)) newshares.push_back(new NewShare(h,0,su,rl,ts,buf));

				if (source == PUTNODES_SYNC)
				{
					// FIXME: add purging of orphaned syncidhandles
					if ((t == FOLDERNODE || t == FILENODE) && !syncdeleted[t].count(nn[i].syncid))
					{
						nn[i].localnode->setnode(n);
//						syncidhandles[nn[i].localnode->syncid] = h;
//						nn[i].localnode->node = n;
//						n->localnode = nn[i].localnode;
					}
				}

				// map upload handle to node handle for pending file attributes
				if (nn && nn[i].source == NEW_UPLOAD)
				{
					handle uh = nn[i].uploadhandle;

					// do we have pending file attributes for this upload? set them.
					for (fa_map::iterator it = pendingfa.lower_bound(pair<handle,fatype>(uh,0)); it != pendingfa.end() && it->first.first == uh; )
					{
						reqs[r].add(new CommandAttachFA(h,it->first.second,it->second.first,it->second.second));
						pendingfa.erase(it++);
					}
					
					// FIXME: only do this for in-flight FA writes
					uhnh.insert(pair<handle,handle>(uh,h));
				}

				i++;
			}

			if (notify) notifynode(n);
		}
	}

	// any child nodes arrived before their parents?
	for (i = dp.size(); i--; ) if ((n = nodebyhandle(dp[i]->parenthandle))) dp[i]->setparent(n);

	return j->leavearray();
}

// decrypt and set encrypted sharekey
void MegaClient::setkey(SymmCipher* c, const char* k)
{
	byte newkey[SymmCipher::KEYLENGTH];

	if (Base64::atob(k,newkey,sizeof newkey) == sizeof newkey)
	{
		key.ecb_decrypt(newkey);
		c->setkey(newkey);
	}
}

// read outbound share keys
void MegaClient::readok(JSON* j)
{
	if (j->enterarray())
	{
		while (j->enterobject()) readokelement(j);
		j->leavearray();

		mergenewshares(0);
	}
}

// - h/ha/k (outbound sharekeys, always symmetric)
void MegaClient::readokelement(JSON* j)
{
	handle h = UNDEF;
	byte ha[SymmCipher::BLOCKSIZE];
	byte buf[SymmCipher::BLOCKSIZE];
	int have_ha = 0;
	const char* k = NULL;

	for (;;)
	{
		switch (j->getnameid())
		{
			case 'h':
				h = j->gethandle();
				break;

			case MAKENAMEID2('h','a'):		// share authentication tag
				have_ha = Base64::atob(j->getvalue(),ha,sizeof ha) == sizeof ha;
				break;

			case 'k':			// share key(s)
				k = j->getvalue();
				break;

			case EOO:
				if (ISUNDEF(h))
				{
					app->debug_log("Missing outgoing share handle in ok element");
					return;
				}

				if (!k)
				{
					app->debug_log("Missing outgoing share key in ok element");
					return;
				}

				if (!have_ha)
				{
					app->debug_log("Missing outbound share signature");
					return;
				}

				if (decryptkey(k,buf,SymmCipher::KEYLENGTH,&key,1,h)) newshares.push_back(new NewShare(h,1,UNDEF,ACCESS_UNKNOWN,0,buf,ha));
				return;

			default:
				if (!j->storeobject()) return;
		}
	}
}

// read outbound shares
void MegaClient::readoutshares(JSON* j)
{
	if (j->enterarray())
	{
		while (j->enterobject()) readoutshareelement(j);
		j->leavearray();

		mergenewshares(0);
	}
}

// - h/u/r/ts (outbound share)
void MegaClient::readoutshareelement(JSON* j)
{
	handle h = UNDEF;
	handle uh = UNDEF;
	accesslevel r = ACCESS_UNKNOWN;
	time_t ts = 0;

	for (;;)
	{
		switch (j->getnameid())
		{
			case 'h':
				h = j->gethandle();
				break;

			case 'u':			// share target user
				uh = j->is(EXPORTEDLINK) ? 0 : j->gethandle(USERHANDLE);
				break;

			case 'r':			// access
				r = (accesslevel)j->getint();
				break;

			case MAKENAMEID2('t','s'):		// timestamp
				ts = j->getint();
				break;

			case EOO:
				if (ISUNDEF(h))
				{
					app->debug_log("Missing outgoing share node");
					return;
				}

				if (ISUNDEF(uh))
				{
					app->debug_log("Missing outgoing share user");
					return;
				}

				if (r == ACCESS_UNKNOWN)
				{
					app->debug_log("Missing outgoing share access");
					return;
				}

				newshares.push_back(new NewShare(h,1,uh,r,ts,NULL,NULL));
				return;

			default:
				if (!j->storeobject()) return;
		}
	}
}

int MegaClient::applykeys()
{
	int t = 0;

	// FIXME: rather than iterating through the whole node set, maintain subset with missing keys
	for (node_map::iterator it = nodes.begin(); it != nodes.end(); it++) if (it->second->applykey()) t++;

	if (sharekeyrewrite.size())
	{
		reqs[r].add(new CommandShareKeyUpdate(this,&sharekeyrewrite));
		sharekeyrewrite.clear();
	}

	if (nodekeyrewrite.size())
	{
		reqs[r].add(new CommandNodeKeyUpdate(this,&nodekeyrewrite));
		nodekeyrewrite.clear();
	}

	return t;
}

// user/contact list
bool MegaClient::readusers(JSON* j)
{
	if (!j->enterarray()) return 0;

	while (j->enterobject())
	{
		handle uh = 0;
		visibility v = VISIBILITY_UNKNOWN;	// new share objects do not override existing visibility
		time_t ts = 0;
		const char* m = NULL;
		nameid name;

		while ((name = j->getnameid()) != EOO)
		{
			switch (name)
			{
				case 'u':	// new node: handle
					uh = j->gethandle(USERHANDLE);
					break;

				case 'c': 	// visibility
					v = (visibility)j->getint();
					break;

				case 'm':	// attributes
					m = j->getvalue();
					break;

				case MAKENAMEID2('t','s'):
					ts = j->getint();
					break;

				default:
					if (!j->storeobject()) return false;
			}
		}

		if (ISUNDEF(uh)) warn("Missing contact user handle");

		if (!m) warn("Unknown contact user e-mail address");

		if (!warnlevel())
		{
			User* u;

			if (v == ME) me = uh;

			if ((u = finduser(uh,1)))
			{
				mapuser(uh,m);

				if (v != VISIBILITY_UNKNOWN) u->set(v,ts);

				notifyuser(u);
			}
		}
	}

	return j->leavearray();
}

Command::Command()
{
	persistent = 0;
	level = -1;
	canceled = false;
}

void Command::cancel()
{
	canceled = true;
}

// returns completed command JSON string
const char* Command::getstring()
{
	return json.c_str();
}

// add opcode
void Command::cmd(const char* cmd)
{
	json.append("\"a\":\"");
	json.append(cmd);
	json.append("\"");
}

void Command::notself(MegaClient *client)
{
	json.append(",\"i\":\"");
	json.append(client->sessionid,sizeof client->sessionid);
	json.append("\"");
}

// add comma separator unless first element
void Command::addcomma()
{
	if (json.size() && !strchr("[{",json[json.size()-1])) json.append(",");
}

// add command argument name:value pair (FIXME: add proper JSON escaping)
void Command::arg(const char* name, const char* value, int quotes)
{
	addcomma();
	json.append("\"");
	json.append(name);
	json.append(quotes ? "\":\"" : "\":");
	json.append(value);
	if (quotes) json.append("\"");
}

// binary data
void Command::arg(const char* name, const byte* value, int len)
{
	char* buf = new char[len*4/3+4];

	Base64::btoa(value,len,buf);

	arg(name,buf);

	delete[] buf;
}

// 64-bit signed integer
void Command::arg(const char* name, m_off_t n)
{
	char buf[32];

	sprintf(buf,"%" PRId64,n);

	arg(name,buf,0);
}

// raw JSON data
void Command::appendraw(const char* s)
{
	json.append(s);
}

// raw JSON data with length specifier
void Command::appendraw(const char* s, int len)
{
	json.append(s,len);
}

// open anonymous array
void Command::beginarray()
{
	addcomma();
	json.append("[");
	openobject();
}

// open named array
void Command::beginarray(const char* name)
{
	addcomma();
	json.append("\"");
	json.append(name);
	json.append("\":[");
	openobject();
}

// close array
void Command::endarray()
{
	json.append("]");
	closeobject();
}

// open anonymous object
void Command::beginobject()
{
	addcomma();
	json.append("{");
}

// close anonymous object
void Command::endobject()
{
	json.append("}");
}

// add integer
void Command::element(int n)
{
	char buf[24];

	sprintf(buf,"%d",n);

	if (elements()) json.append(",");
	json.append(buf);
}

// add handle (with size specifier)
void Command::element(handle h, int len)
{
	char buf[12];

	Base64::btoa((const byte*)&h,len,buf);

	json.append(elements() ? ",\"" : "\"");
	json.append(buf);
	json.append("\"");
}

// add binary data
void Command::element(const byte* data, int len)
{
	char* buf = new char[len*4/3+4];

	len = Base64::btoa(data,len,buf);

	json.append(elements() ? ",\"" : "\"");
	json.append(buf,len);

	delete[] buf;

	json.append("\"");
}

// open object
void Command::openobject()
{
	levels[(int)++level] = 0;
}

// close object
void Command::closeobject()
{
	level--;
}

// number of elements present in this level
int Command::elements()
{
	if (!levels[(int)level])
	{
		levels[(int)level] = 1;
		return 0;
	}

	return 1;
}

// default command result handler: ignore & skip
void Command::procresult()
{
	if (client->json.isnumeric())
	{
		client->json.getint();
		return;
	}

	for (;;)
	{
		switch (client->json.getnameid())
		{
			case EOO:
				return;

			default:
				if (!client->json.storeobject()) return;
		}
	}
}

error MegaClient::folderaccess(const char* f, const char* k)
{
	handle h = 0;
	byte folderkey[SymmCipher::KEYLENGTH];

	if (Base64::atob(f,(byte*)&h,NODEHANDLE) != NODEHANDLE) return API_EARGS;
	if (Base64::atob(k,folderkey,sizeof folderkey) != sizeof folderkey) return API_EARGS;

	setrootnode(h);
	key.setkey(folderkey);

	return API_OK;
}

void MegaClient::login(const char* email, const byte* pwkey, bool nocache)
{
	logout();

	string t;
	string lcemail(email);

	key.setkey((byte*)pwkey);
	uint64_t emailhash = stringhash64(&lcemail,&key);

	if (!nocache && dbaccess && (sctable = dbaccess->open(fsaccess,&lcemail)) && sctable->get(CACHEDSCSN,&t))
	{
		if (t.size() == sizeof cachedscsn) cachedscsn = *(handle*)t.data();
		else cachedscsn = UNDEF;
	}

	reqs[r].add(new CommandLogin(this,email,emailhash));
}

// login request with user e-mail address and user hash
CommandLogin::CommandLogin(MegaClient* client, const char* e, uint64_t emailhash)
{
	cmd("us");
	arg("user",e);
	arg("uh",(byte*)&emailhash,sizeof emailhash);

	if (client->cachedscsn != UNDEF) arg("sn",(byte*)&client->cachedscsn,sizeof client->cachedscsn);

	tag = client->reqtag;
}

// process login result
void CommandLogin::procresult()
{
	if (client->json.isnumeric()) return client->app->login_result((error)client->json.getint());

	byte hash[SymmCipher::KEYLENGTH];
	byte sidbuf[AsymmCipher::MAXKEYLENGTH];
	byte privkbuf[AsymmCipher::MAXKEYLENGTH*2];
	int len_k = 0, len_privk = 0, len_csid = 0, len_tsid = 0;
	handle me = UNDEF;

	for (;;)
	{
		switch (client->json.getnameid())
		{
			case 'k':
				len_k = client->json.storebinary(hash,sizeof hash);
				break;

			case 'u':
				me = client->json.gethandle(MegaClient::USERHANDLE);
				break;

			case MAKENAMEID4('t','s','i','d'):
				len_tsid = client->json.storebinary(sidbuf,sizeof sidbuf);
				break;

			case MAKENAMEID4('c','s','i','d'):
				len_csid = client->json.storebinary(sidbuf,sizeof sidbuf);
				break;

			case MAKENAMEID5('p','r','i','v','k'):
				len_privk = client->json.storebinary(privkbuf,sizeof privkbuf);
				break;

			case MAKENAMEID2('s','n'):
				if (!client->json.getint())
				{
					// local state cache continuity rejected: read state from server instead
					delete client->sctable;
					client->sctable = NULL;
				}
				break;

			case EOO:
				if (ISUNDEF(me) || len_k != sizeof hash) client->app->login_result(API_EINTERNAL);

				// decrypt and set master key
				client->key.ecb_decrypt(hash);
				client->key.setkey(hash);

				if (len_tsid)
				{
					client->setsid(sidbuf,MegaClient::SIDLEN);

					// account does not have an RSA keypair set: verify password using symmetric challenge
					if (!client->checktsid(sidbuf,len_tsid)) return client->app->login_result(API_EKEY);

					// add missing RSA keypair
					client->app->debug_log("Generating and adding missing RSA keypair");
					client->setkeypair();
				}
				else
				{
					// account has RSA keypair: decrypt server-provided session ID
					if (len_csid < 32 || len_privk < 256) return client->app->login_result(API_EINTERNAL);

					// decrypt and set private key
					client->key.ecb_decrypt(privkbuf,len_privk);
					if (!client->asymkey.setkey(AsymmCipher::PRIVKEY,privkbuf,len_privk)) return client->app->login_result(API_EKEY);

					// decrypt and set session ID for subsequent API communication
					if (!client->asymkey.decrypt(sidbuf,len_csid,sidbuf,MegaClient::SIDLEN)) return client->app->login_result(API_EINTERNAL);
					client->setsid(sidbuf,MegaClient::SIDLEN);
				}

				client->me = me;

				return client->app->login_result(API_OK);

			default:
				if (!client->json.storeobject()) return client->app->login_result(API_EINTERNAL);
		}
	}
}

// verify a static symmetric password challenge
int MegaClient::checktsid(byte* sidbuf, unsigned len)
{
	if (len != SIDLEN) return 0;

	key.ecb_encrypt(sidbuf);

	return !memcmp(sidbuf,sidbuf+SIDLEN-SymmCipher::KEYLENGTH,SymmCipher::KEYLENGTH);
}

// locate user by e-mail address or ASCII handle
User* MegaClient::finduser(const char* uid, int add)
{
	// null user for folder links?
	if (!uid || !*uid) return NULL;

	if (!strchr(uid,'@'))
	{
		// not an e-mail address: must be ASCII handle
		handle uh;

		if (Base64::atob(uid,(byte*)&uh,sizeof uh) == sizeof uh) return finduser(uh,add);
		return NULL;
	}

	string nuid;
	User* u;

	// convert e-mail address to lowercase (ASCII only)
	Node::copystring(&nuid,uid);
	transform(nuid.begin(), nuid.end(), nuid.begin(), ::tolower);

	um_map::iterator it = umindex.find(nuid);

	if (it == umindex.end())
	{
		if (!add) return NULL;

		// add user by lowercase e-mail address
		u = &users[++userid];
		u->uid = nuid;
		Node::copystring(&u->email,uid);
		umindex[nuid] = userid;

		return u;
	}
	else return &users[it->second];
}

// locate user by binary handle
User* MegaClient::finduser(handle uh, int add)
{
	if (!uh) return NULL;

	char uid1[12];
	Base64::btoa((byte*)&uh,sizeof uh,uid1);
	uid1[11] = 0;

	User* u;
	uh_map::iterator it = uhindex.find(uh);

	if (it == uhindex.end())
	{
		if (!add) return NULL;

		// add user by binary handle
		u = &users[++userid];

		char uid[12];
		Base64::btoa((byte*)&uh,sizeof uh,uid);
		u->uid.assign(uid,11);

		uhindex[uh] = userid;
		u->userhandle = uh;

		return u;
	}
	else return &users[it->second];
}

// add missing mapping (handle or email)
// reduce uid to ASCII uh if only known by email
void MegaClient::mapuser(handle uh, const char* email)
{
	if (!*email) return;

	User* u;
	string nuid;

	Node::copystring(&nuid,email);
	transform(nuid.begin(),nuid.end(),nuid.begin(),::tolower);

	// does user uh exist?
	uh_map::iterator hit = uhindex.find(uh);

	if (hit != uhindex.end())
	{
		// yes: add email reference
		u = &users[hit->second];

		if (!u->email.size())
		{
			Node::copystring(&u->email,email);
			umindex[nuid] = hit->second;
		}

		return;
	}

	// does user email exist?
	um_map::iterator mit = umindex.find(nuid);

	if (mit != umindex.end())
	{
		// yes: add uh reference
		u = &users[mit->second];

		uhindex[uh] = mit->second;
		u->userhandle = uh;

		char uid[12];
		Base64::btoa((byte*)&uh,sizeof uh,uid);
		u->uid.assign(uid,11);
	}
}

// sharekey distribution request for handle h
PubKeyActionSendShareKey::PubKeyActionSendShareKey(handle h)
{
	sh = h;
}

void PubKeyActionSendShareKey::proc(MegaClient* client, User* u)
{
	Node* n;

	// only the share owner distributes share keys
	if (u && (n = client->nodebyhandle(sh)) && n->sharekey && client->checkaccess(n,OWNER))
	{
		int t;
		byte buf[AsymmCipher::MAXKEYLENGTH];

		if ((t = u->pubk.encrypt(n->sharekey->key,SymmCipher::KEYLENGTH,buf,sizeof buf))) client->reqs[client->r].add(new CommandShareKeyUpdate(client,sh,u->uid.c_str(),buf,t));
	}
}

// sharekey distribution request - walk array consisting of node/user handles and submit public key requests
void MegaClient::procsr(JSON* j)
{
	handle sh, uh;

	if (!j->enterarray()) return;

	for (;;)
	{
		if (!ISUNDEF(sh = j->gethandle()) && !ISUNDEF(uh = j->gethandle()))
		{
			User* u;

			if (nodebyhandle(sh) && (u = finduser(uh))) queuepubkeyreq(u,new PubKeyActionSendShareKey(sh));
		}
		else break;
	}

	j->leavearray();
}

// process node tree (bottom up)
void MegaClient::proctree(Node* n, TreeProc* tp)
{
	if (n->type != FILENODE) for (node_list::iterator it = n->children.begin(); it != n->children.end(); ) proctree(*it++,tp);

	tp->proc(this,n);
}

void PubKeyActionCreateShare::proc(MegaClient* client, User* u)
{
	Node* n;
	int newshare;

	// node vanished: bail
	if (!(n = client->nodebyhandle(h))) return client->app->share_result(API_ENOENT);

	// do we already have a share key for this node?
	if ((newshare = !n->sharekey))
	{
		// no: create
		byte key[SymmCipher::KEYLENGTH];

		PrnGen::genblock(key,sizeof key);

		n->sharekey = new SymmCipher(key);
	}

	// we have all ingredients ready: the target user's public key, the share key and all nodes to share
	client->restag = tag;
	client->reqs[client->r].add(new CommandSetShare(client,n,u,a,newshare));
}

// add share node and return its index
int ShareNodeKeys::addshare(Node* sn)
{
	for (int i = shares.size(); i--; ) if (shares[i] == sn) return i;

	shares.push_back(sn);

	return shares.size()-1;
}

void ShareNodeKeys::add(Node* n, Node* sn, int specific)
{
	if (!sn) sn = n;

	add((NodeCore*)n,sn,specific);
}

// add a nodecore (!sn: all relevant shares, otherwise starting from sn, fixed: only sn)
void ShareNodeKeys::add(NodeCore* n, Node* sn, int specific, const byte* item, int itemlen)
{
	char buf[96];
	char* ptr;
	byte key[Node::FILENODEKEYLENGTH];

	int addnode = 0;

	// emit all share nodekeys for known shares
	do {
		if (sn->sharekey)
		{
			sprintf(buf,",%d,%d,\"",addshare(sn),(int)items.size());

			sn->sharekey->ecb_encrypt((byte*)n->nodekey.data(),key,n->nodekey.size());

			ptr = strchr(buf+5,0);
			ptr += Base64::btoa(key,n->nodekey.size(),ptr);
			*ptr++ = '"';

			keys.append(buf,ptr-buf);
			addnode = 1;
		}
	} while (!specific && (sn = sn->parent));

	if (addnode)
	{
		items.resize(items.size()+1);

		if (item) items[items.size()-1].assign((const char*)item,itemlen);
		else items[items.size()-1].assign((const char*)&n->nodehandle,MegaClient::NODEHANDLE);
	}
}

void ShareNodeKeys::get(Command* c)
{
	if (keys.size())
	{
		c->beginarray("cr");

		// emit share node handles
		c->beginarray();
		for (unsigned i = 0; i < shares.size(); i++) c->element((const byte*)&shares[i]->nodehandle,MegaClient::NODEHANDLE);
		c->endarray();

		// emit item handles (can be node handles or upload tokens)
		c->beginarray();
		for (unsigned i = 0; i < items.size(); i++) c->element((const byte*)items[i].c_str(),items[i].size());
		c->endarray();

		c->beginarray();
		c->appendraw(keys.c_str()+1,keys.size()-1);
		c->endarray();

		c->endarray();
	}
}

CommandShareKeyUpdate::CommandShareKeyUpdate(MegaClient* client, handle sh, const char* uid, const byte* key, int len)
{
	cmd("k");
	beginarray("sr");

	element(sh,MegaClient::NODEHANDLE);
	element((byte*)uid,strlen(uid));
	element(key,len);

	endarray();
}

CommandShareKeyUpdate::CommandShareKeyUpdate(MegaClient* client, handle_vector* v)
{
	Node* n;
	byte sharekey[SymmCipher::KEYLENGTH];

	cmd("k");
	beginarray("sr");

	for (int i = v->size(); i--; )
	{
		handle h = (*v)[i];

		if ((n = client->nodebyhandle(h)) && n->sharekey)
		{
			client->key.ecb_encrypt(n->sharekey->key,sharekey,SymmCipher::KEYLENGTH);

			element(h,MegaClient::NODEHANDLE);
			element(client->me,8);
			element(sharekey,SymmCipher::KEYLENGTH);
		}
	}

	endarray();
}

// add/remove share; include node share keys if new share
CommandSetShare::CommandSetShare(MegaClient* client, Node* n, User* u, accesslevel a, int newshare)
{
	byte auth[SymmCipher::BLOCKSIZE];
	byte key[SymmCipher::KEYLENGTH];
	byte asymmkey[AsymmCipher::MAXKEYLENGTH];
	string uid;
	int t;

	tag = client->restag;

	sh = n->nodehandle;
	user = u;
	access = a;

	cmd("s");
	arg("n",(byte*)&sh,MegaClient::NODEHANDLE);

	if (a != ACCESS_UNKNOWN)
	{
		// securely store/transmit share key
		// by creating a symmetrically (for the sharer) and an asymmetrically (for the sharee) encrypted version
		memcpy(key,n->sharekey->key,sizeof key);
		memcpy(asymmkey,key,sizeof key);

		client->key.ecb_encrypt(key);
		arg("ok",key,sizeof key);

		if (u) t = u->pubk.encrypt(asymmkey,SymmCipher::KEYLENGTH,asymmkey,sizeof asymmkey);

		// outgoing handle authentication
		client->handleauth(sh,auth);
		arg("ha",auth,sizeof auth);
	}

	beginarray("s");
	beginobject();

	arg("u",u ? u->uid.c_str() : MegaClient::EXPORTEDLINK);

	if (a != ACCESS_UNKNOWN)
	{
		arg("r",a);
		if (u) arg("k",asymmkey,t);
	}

	endobject();
	endarray();

	// only for a fresh share: add cr element with all node keys encrypted to the share key
	if (newshare)
	{
		// the new share's nodekeys for this user: generate node list
		TreeProcShareKeys tpsk(n);
		client->proctree(n,&tpsk);
		tpsk.get(this);
	}
}

// process user element (email/handle pairs)
bool CommandSetShare::procuserresult(MegaClient* client)
{
	while (client->json.enterobject())
	{
		handle uh = UNDEF;
		const char* m = NULL;

		for (;;)
		{
			switch (client->json.getnameid())
			{
				case 'u':
					uh = client->json.gethandle(MegaClient::USERHANDLE);
					break;

				case 'm':
					m = client->json.getvalue();
					break;

				case EOO:
					if (!ISUNDEF(uh) && m) client->mapuser(uh,m);
					return true;

				default:
					if (!client->json.storeobject()) return false;
			}
		}
	}

	return false;
}

// process result of share addition/modification
void CommandSetShare::procresult()
{
	if (client->json.isnumeric()) return client->app->share_result((error)client->json.getint());

	for (;;)
	{
		switch (client->json.getnameid())
		{
			byte key[SymmCipher::KEYLENGTH+1];

			case MAKENAMEID2('o','k'):	// an owner key response will only occur if the same share was created concurrently with a different key
				if (client->json.storebinary(key,sizeof key+1) == SymmCipher::KEYLENGTH)
				{
					Node* n;

					if ((n = client->nodebyhandle(sh)) && n->sharekey)
					{
						client->key.ecb_decrypt(key);
						n->sharekey->setkey(key);

						// repeat attempt with corrected share key
						client->restag = tag;
						client->reqs[client->r].add(new CommandSetShare(client,n,user,access,0));
						return;
					}
				}
				break;

			case 'u':	// user/handle confirmation
				if (client->json.enterarray()) while (procuserresult(client));
				break;

			case 'r':
				if (client->json.enterarray())
				{
					int i = 0;

					while (client->json.isnumeric()) client->app->share_result(i++,(error)client->json.getint());

					client->json.leavearray();
				}
				break;

			case MAKENAMEID3('s','n','k'):
				client->procsnk(&client->json);
				break;

			case MAKENAMEID3('s','u','k'):
				client->procsuk(&client->json);
				break;

			case MAKENAMEID2('c','r'):
				client->proccr(&client->json);
				break;

			case EOO:
				client->app->share_result(API_OK);
				return;

			default:
				if (!client->json.storeobject()) return;
		}
	}
}

// share node sh with access level sa
PubKeyActionCreateShare::PubKeyActionCreateShare(handle sh, accesslevel sa, int ctag)
{
	h = sh;
	a = sa;
	tag = ctag;
}

// queue PubKeyAction request to be triggered upon availability of the user's public key
void MegaClient::queuepubkeyreq(User* u, PubKeyAction* pka)
{
	if (!u || u->pubk.isvalid())
	{
		restag = pka->tag;
		pka->proc(this,u);
		delete pka;
	}
	else
	{
		u->pkrs.push_back(pka);
	 	if (!u->pubkrequested) reqs[r].add(new CommandPubKeyRequest(this,u));
	}
}

// if user has a known public key, complete instantly
// otherwise, queue and request public key if not already pending
void MegaClient::setshare(Node* n, const char* user, accesslevel a)
{
	queuepubkeyreq(finduser(user,1),new PubKeyActionCreateShare(n->nodehandle,a,reqtag));
}

class CommandEnumerateQuotaItems : public Command
{
public:
	void procresult();

	CommandEnumerateQuotaItems(MegaClient*);
};

// enumerate Pro account purchase options (not fully implemented)
void MegaClient::purchase_enumeratequotaitems()
{
	reqs[r].add(new CommandEnumerateQuotaItems(this));
}

CommandEnumerateQuotaItems::CommandEnumerateQuotaItems(MegaClient* client)
{
	cmd("utqa");

	tag = client->reqtag;
}

void CommandEnumerateQuotaItems::procresult()
{
	if (client->json.isnumeric()) return client->app->enumeratequotaitems_result((error)client->json.getint());

	handle product;
	int prolevel, gbstorage, gbtransfer, months;
	unsigned amount;
	const char* a;
	const char* c;
	string currency;

	while (client->json.enterarray())
	{
		if (ISUNDEF((product = client->json.gethandle()))
			|| (prolevel = client->json.getint()) < 0
			|| (gbstorage = client->json.getint()) < 0
			|| (gbtransfer = client->json.getint()) < 0
			|| (months = client->json.getint()) < 0
			|| !(a = client->json.getvalue())
			|| !(c = client->json.getvalue())) return client->app->enumeratequotaitems_result(API_EINTERNAL);


		Node::copystring(&currency,c);

		amount = atoi(a)*100;
		if ((c = strchr(a,'.')))
		{
			c++;
			if (*c >= '0' && *c <= '9') amount += (*c-'0')*10;
			c++;
			if (*c >= '0' && *c <= '9') amount += *c-'0';
		}

		client->app->enumeratequotaitems_result(product,prolevel,gbstorage,gbtransfer,months,amount,currency.c_str());
		client->json.leavearray();
	}

	client->app->enumeratequotaitems_result(API_OK);
}

// begin a new purchase (not fully implemented)
void MegaClient::purchase_begin()
{
	purchase_basket.clear();
}

// submit purchased product for payment
void MegaClient::purchase_additem(int itemclass, handle item, unsigned price, char* currency, unsigned tax, char* country, char* affiliate)
{
	reqs[r].add(new CommandPurchaseAddItem(this,itemclass,item,price,currency,tax,country,affiliate));
}

CommandPurchaseAddItem::CommandPurchaseAddItem(MegaClient* chan, int itemclass, handle item, unsigned price, char* curreny, unsigned tax, char* country, char* affiliate)
{
	cmd("uts");

	// FIXME: implement
}

void CommandPurchaseAddItem::procresult()
{
	// FIXME: implement
}

// obtain payment URL for given provider
void MegaClient::purchase_checkout(int gateway)
{
	reqs[r].add(new CommandPurchaseCheckout(this,gateway));
}

CommandPurchaseCheckout::CommandPurchaseCheckout(MegaClient* client, int gateway)
{
	cmd("utc");

	beginarray("s");
	for (handle_vector::iterator it = client->purchase_basket.begin(); it != client->purchase_basket.end(); it++) element((byte*)&*it,sizeof(handle));
	endarray();

	arg("m",gateway);

	// empty basket
	client->purchase_begin();

	tag = client->reqtag;
}

void CommandPurchaseCheckout::procresult()
{
	if (client->json.isnumeric()) return client->app->checkout_result((error)client->json.getint());

	string response;

	client->json.storeobject(&response);

	client->app->checkout_result(response.c_str());
}

// add new contact (by e-mail address)
error MegaClient::invite(const char* email, visibility show)
{
	if (!strchr(email,'@')) return API_EARGS;

	reqs[r].add(new CommandUserRequest(this,email,show));

	return API_OK;
}

CommandUserRequest::CommandUserRequest(MegaClient* client, const char* m, visibility show)
{
	cmd("ur");
	arg("u",m);
	arg("l",(int)show);

	tag = client->reqtag;
}

void CommandUserRequest::procresult()
{
	error e;

	if (client->json.isnumeric()) e = (error)client->json.getint();
	else
	{
		client->json.storeobject();
		e = API_OK;
	}

	client->app->invite_result(e);
}

// attach/update/delete user attribute
// attributes are stored as base64-encoded binary blobs
// internal attribute name prefixes:
// * - private and CBC-encrypted
// + - public and plaintext
void MegaClient::putua(const char* an, const byte* av, unsigned avl, int priv)
{
	string name = priv ? "*" : "+";
	string data;

	name.append(an);

	if (priv)
	{
		if (av) data.assign((const char*)av,avl);
		PaddedCBC::encrypt(&data,&key);
	}
	else if (!av) av = (const byte*)"";

	reqs[r].add(new CommandPutUA(this,name.c_str(),priv ? (const byte*)data.data() : av, priv ? data.size() : avl));
}

CommandPutUA::CommandPutUA(MegaClient* client, const char *an, const byte* av, unsigned avl)
{
	cmd("up");
	arg(an,av,avl);

	tag = client->reqtag;
}

void CommandPutUA::procresult()
{
	error e;

	if (client->json.isnumeric()) e = (error)client->json.getint();
	else
	{
		client->json.storeobject();
		e = API_OK;
	}

	client->app->putua_result(e);
}

// queue user attribute retrieval
void MegaClient::getua(User* u, const char* an, int p)
{
	if (an)
	{
		string name = p ? "*" : "+";

		name.append(an);

		reqs[r].add(new CommandGetUA(this,u->uid.c_str(),name.c_str(),p));
	}
}

CommandGetUA::CommandGetUA(MegaClient* client, const char* uid, const char* an, int p)
{
	priv = p;

	cmd("uga");
	arg("u",uid);
	arg("ua",an);

	tag = client->reqtag;
}

void CommandGetUA::procresult()
{
	if (client->json.isnumeric()) return client->app->getua_result((error)client->json.getint());
	else
	{
		string d;
		const char* ptr;
		const char* end;

		if (!(ptr = client->json.getvalue()) || !(end = strchr(ptr,'"'))) return client->app->getua_result(API_EINTERNAL);

		int l = (end-ptr)/4*3+3;

		byte* data = new byte[l];

		l = Base64::atob(ptr,data,l);

		if (priv)
		{
			d.assign((char*)data,l);
			delete[] data;

			if (!PaddedCBC::decrypt(&d,&client->key)) return client->app->getua_result(API_EINTERNAL);

			return client->app->getua_result((byte*)d.data(),d.size());
		}

		client->app->getua_result(data,l);

		delete[] data;
	}
}

// queue node for notification
void MegaClient::notifynode(Node* n)
{
	if (!n->notified)
	{
		n->notified = 1;
		nodenotify.push_back(n);
	}
}

// queue user for notification
void MegaClient::notifyuser(User* u)
{
	usernotify.push_back(u);
}

// set node keys (e.g. to convert asymmetric keys to symmetric ones)
CommandNodeKeyUpdate::CommandNodeKeyUpdate(MegaClient* client, handle_vector* v)
{
	byte nodekey[Node::FILENODEKEYLENGTH];
	cmd("k");
	beginarray("nk");

	for (int i = v->size(); i--; )
	{
		handle h = (*v)[i];

		Node* n;

		if ((n = client->nodebyhandle(h)))
		{
			client->key.ecb_encrypt((byte*)n->nodekey.data(),nodekey,n->nodekey.size());

			element(h,MegaClient::NODEHANDLE);
			element(nodekey,n->nodekey.size());
		}
	}

	endarray();
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

// process request for share node keys
// builds & emits k/cr command
// returns 1 in case of a valid response, 0 otherwise
void MegaClient::proccr(JSON* j)
{
	node_vector shares, nodes;
	handle h;

	if (j->enterobject())
	{
		for (;;)
		{
			switch (j->getnameid())
			{
				case MAKENAMEID3('s','n','k'):
					procsnk(j);
					break;

				case MAKENAMEID3('s','u','k'):
					procsuk(j);
					break;

				case EOO:
					j->leaveobject();
					return;

				default:
					if (!j->storeobject()) return;
			}
		}

		return;
	}

	if (!j->enterarray())
	{
		app->debug_log("Malformed CR - outer array");
		return;
	}

	if (j->enterarray())
	{
		while (!ISUNDEF(h = j->gethandle())) shares.push_back(nodebyhandle(h));
		j->leavearray();

		if (j->enterarray())
		{
			while (!ISUNDEF(h = j->gethandle())) nodes.push_back(nodebyhandle(h));
			j->leavearray();
		}
		else
		{
			app->debug_log("Malformed SNK CR - nodes part");
			return;
		}

		if (j->enterarray())
		{
			cr_response(&shares,&nodes,j);
			j->leavearray();
		}
		else
		{
			app->debug_log("Malformed CR - linkage part");
			return;
		}
	}

	j->leavearray();
}

CommandSingleKeyCR::CommandSingleKeyCR(handle sh, handle nh, const byte* key, unsigned keylen)
{
	cmd("k");
	beginarray("cr");

	beginarray();
	element(sh,MegaClient::NODEHANDLE);
	endarray();

	beginarray();
	element(nh,MegaClient::NODEHANDLE);
	endarray();

	beginarray();
	element(0);
	element(0);
	element(key,keylen);
	endarray();

	endarray();
}

// share nodekey delivery
void MegaClient::procsnk(JSON* j)
{
	if (j->enterarray())
	{
		handle sh, nh;

		while (j->enterarray())
		{
			if (ISUNDEF((sh = j->gethandle()))) return;
			if (ISUNDEF((nh = j->gethandle()))) return;

			Node* sn = nodebyhandle(sh);

			if (sn && sn->sharekey && checkaccess(sn,OWNER))
			{
				Node* n = nodebyhandle(nh);

				if (n && n->isbelow(sn))
				{
					byte keybuf[Node::FILENODEKEYLENGTH];

					sn->sharekey->ecb_encrypt((byte*)n->nodekey.data(),keybuf,n->nodekey.size());

					reqs[r].add(new CommandSingleKeyCR(sh,nh,keybuf,n->nodekey.size()));
				}
			}

			j->leavearray();
		}

		j->leavearray();
	}
}

// share userkey delivery
void MegaClient::procsuk(JSON* j)
{
	if (j->enterarray())
	{
		while (j->enterarray())
		{
			handle sh, uh;

			sh = j->gethandle();

			if (!ISUNDEF(sh))
			{
				uh = j->gethandle();

				if (!ISUNDEF(uh))
				{
					// FIXME: add support for share user key delivery
				}
			}

			j->leavearray();
		}

		j->leavearray();
	}
}


// add node to vector, return position, deduplicate
unsigned MegaClient::addnode(node_vector* v, Node* n)
{
	// linear search not particularly scalable, but fine for the relatively small real-world requests
	for (int i = v->size(); i--; ) if ((*v)[i] == n) return i;
	v->push_back(n);
	return v->size()-1;
}

// generate crypto key response
// if !selector, generate all shares*nodes tuples
void MegaClient::cr_response(node_vector* shares, node_vector* nodes, JSON* selector)
{
	node_vector rshares, rnodes;
	unsigned si, ni;
	Node* sn;
	Node* n;
	string crkeys;
	byte keybuf[Node::FILENODEKEYLENGTH];
	char buf[128];
	int setkey = -1;

	// for security reasons, we only respond to key requests affecting our own shares
	for (si = shares->size(); si--; ) if ((*shares)[si] && ((*shares)[si]->inshare || !(*shares)[si]->sharekey))
	{
		app->debug_log("Attempt to obtain node key for invalid/third-party share foiled");
		(*shares)[si] = NULL;
	}

	if (!selector) si = ni = 0;

	crkeys.reserve(shares->size()*nodes->size()*(32*4/3+10));

	for (;;)
	{
		if (selector)
		{
			// walk selector, detect errors/end by checking if the JSON position advanced
			const char* p = selector->pos;
			si = (unsigned)selector->getint();
			if (p == selector->pos) break;
			ni = (unsigned)selector->getint();

			if (si >= shares->size())
			{
				app->debug_log("Share index out of range");
				return;
			}

			if (ni >= nodes->size())
			{
				app->debug_log("Node index out of range");
				return;
			}

			if (selector->pos[1] == '"') setkey = selector->storebinary(keybuf,sizeof keybuf);
		}
		else
		{
			// no selector supplied
			ni++;

			if (ni >= nodes->size())
			{
				ni = 0;
				if (++si >= shares->size()) break;
			}
		}

		if ((sn = (*shares)[si]) && (n = (*nodes)[ni]))
		{
		 	if (n->isbelow(sn))
			{
				if (setkey >= 0)
				{
					if (setkey == (int)n->nodekey.size())
					{
						sn->sharekey->ecb_decrypt(keybuf,n->nodekey.size());
						n->setkey(keybuf);
						setkey = -1;
					}
				}
				else
				{
					unsigned nsi, nni;

					nsi = addnode(&rshares,sn);
					nni = addnode(&rnodes,n);

					sprintf(buf,"\",%u,%u,\"",nsi,nni);

					// generate & queue share nodekey
					sn->sharekey->ecb_encrypt((byte*)n->nodekey.data(),keybuf,n->nodekey.size());
					Base64::btoa(keybuf,n->nodekey.size(),strchr(buf+7,0));
					crkeys.append(buf);
				}
			}
			else app->debug_log("Attempt to obtain key of node outside share foiled");
		}
	}

	if (crkeys.size())
	{
		crkeys.append("\"");
		reqs[r].add(new CommandKeyCR(this,&rshares,&rnodes,crkeys.c_str()+2));
	}
}

CommandKeyCR::CommandKeyCR(MegaClient* client, node_vector* rshares, node_vector* rnodes, const char* keys)
{
	cmd("k");
	beginarray("cr");

	beginarray();
	for (int i = 0; i < (int)rshares->size(); i++) element((*rshares)[i]->nodehandle,MegaClient::NODEHANDLE);
	endarray();

	beginarray();
	for (int i = 0; i < (int)rnodes->size(); i++) element((*rnodes)[i]->nodehandle,MegaClient::NODEHANDLE);
	endarray();

	beginarray();
	appendraw(keys);
	endarray();

	endarray();
}

// a == ACCESS_UNKNOWN: request public key for user handle and respond with share key for sn
// otherwise: request public key for user handle and continue share creation for node sn to user u with access a
CommandPubKeyRequest::CommandPubKeyRequest(MegaClient* client, User* user)
{
	cmd("uk");
	arg("u",user->uid.c_str());

	u = user;
	tag = client->reqtag;
}

void CommandPubKeyRequest::procresult()
{
	byte pubkbuf[AsymmCipher::MAXKEYLENGTH];
	int len_pubk = 0;
	handle uh = UNDEF;

	if (client->json.isnumeric())
	{
		// FIXME: handle users without public key / unregistered users
	}

	for (;;)
	{
		switch (client->json.getnameid())
		{
			case 'u':
				uh = client->json.gethandle(MegaClient::USERHANDLE);
				break;

			case MAKENAMEID4('p','u','b','k'):
				len_pubk = client->json.storebinary(pubkbuf,sizeof pubkbuf);
				break;

			case EOO:
				if (!ISUNDEF(uh)) client->mapuser(uh,u->email.c_str());

				if (len_pubk && !u->pubk.setkey(AsymmCipher::PUBKEY,pubkbuf,len_pubk)) len_pubk = 0;

				if (0)
				{
			default:
					if (client->json.storeobject()) continue;
					len_pubk = 0;
				}

				// satisfy all pending PubKeyAction requests for this user
				// if no valid public key was received, satisfy them with a NULL user pointer
				while (u->pkrs.size())
				{
					client->restag = tag;
					u->pkrs[0]->proc(client,len_pubk ? u : NULL);
					delete u->pkrs[0];
					u->pkrs.pop_front();
				}

				if (len_pubk) client->notifyuser(u);
				return;
		}
	}
}

void MegaClient::getaccountdetails(AccountDetails* ad, bool storage, bool transfer, bool pro, bool transactions, bool purchases, bool sessions)
{
	reqs[r].add(new CommandGetUserQuota(this,ad,storage,transfer,pro));
	if (transactions) reqs[r].add(new CommandGetUserTransactions(this,ad));
	if (purchases) reqs[r].add(new CommandGetUserPurchases(this,ad));
	if (sessions) reqs[r].add(new CommandGetUserSessions(this,ad));
}

CommandGetUserQuota::CommandGetUserQuota(MegaClient* client, AccountDetails* ad, bool storage, bool transfer, bool pro)
{
	details = ad;

	cmd("uq");
	if (storage) arg("strg","1",0);
	if (transfer) arg("xfer","1",0);
	if (pro) arg("pro","1",0);

	tag = client->reqtag;
}

void CommandGetUserQuota::procresult()
{
	short td;
	bool got_storage = false;
	bool got_transfer = false;
	bool got_pro = false;

	if (client->json.isnumeric()) return client->app->account_details(details,(error)client->json.getint());

	details->pro_level = 0;
	details->subscription_type = 0;

	details->pro_until = 0;

	details->storage_used = 0;
	details->storage_max = 0;
	details->transfer_own_used = 0;
	details->transfer_srv_used = 0;
	details->transfer_max = 0;
	details->transfer_own_reserved = 0;
	details->transfer_srv_reserved = 0;
	details->srv_ratio = 0;

	details->transfer_hist_starttime = 0;
	details->transfer_hist_interval = 3600;
	details->transfer_hist.clear();

	details->transfer_reserved = 0;

	details->transfer_limit = 0;

	for (;;)
	{
		switch (client->json.getnameid())
		{
			case MAKENAMEID2('b','t'):					// age of transfer window start
				td = (short)client->json.getint();
				if (td != -1) details->transfer_hist_starttime = time(NULL)-(unsigned short)td;
				break;

			case MAKENAMEID3('b','t','i'):
				details->transfer_hist_interval = client->json.getint();
				break;

			case MAKENAMEID3('t','a','h'):
				if (client->json.enterarray())
				{
					m_off_t t;

					while ((t = client->json.getint()) >= 0) details->transfer_hist.push_back(t);

					client->json.leavearray();
				}
				break;

			case MAKENAMEID3('t','a','r'):
				details->transfer_reserved = client->json.getint();
				break;

			case MAKENAMEID3('t','a','l'):
				details->transfer_limit = client->json.getint();
				got_transfer = true;
				break;

			case MAKENAMEID3('t','u','a'):
				details->transfer_own_used += client->json.getint();
				break;

			case MAKENAMEID3('t','u','o'):
				details->transfer_srv_used += client->json.getint();
				break;

			case MAKENAMEID3('r','u','a'):
				details->transfer_own_reserved += client->json.getint();
				break;

			case MAKENAMEID3('r','u','o'):
				details->transfer_srv_reserved += client->json.getint();
				break;

			case MAKENAMEID5('c','s','t','r','g'):		// storage used
				details->storage_used = client->json.getint();
				break;

			case MAKENAMEID5('m','s','t','r','g'):		// total storage quota
				details->storage_max = client->json.getint();
				got_storage = true;
				break;

			case MAKENAMEID6('c','a','x','f','e','r'):	// own transfer quota used
				details->transfer_own_used += client->json.getint();
				break;

			case MAKENAMEID6('c','s','x','f','e','r'):		// third-party transfer quota used
				details->transfer_srv_used += client->json.getint();
				break;

			case MAKENAMEID5('m','x','f','e','r'):		// total transfer quota
				details->transfer_max = client->json.getint();
				got_transfer = true;
				break;

			case MAKENAMEID8('s','r','v','r','a','t','i','o'):		// percentage of transfer allocated for serving
				details->srv_ratio = client->json.getfloat();
				break;

			case MAKENAMEID5('u','t','y','p','e'):			// Pro level (0 == none)
				details->pro_level = (int)client->json.getint();
				got_pro = 1;
				break;

			case MAKENAMEID5('s','t','y','p','e'):			// subscription type
				const char* ptr;
				if ((ptr = client->json.getvalue())) details->subscription_type = *ptr;
				break;

			case MAKENAMEID6('s','u','n','t','i','l'):			// Pro level until
				details->pro_until = client->json.getint();
				break;

			case MAKENAMEID7('b','a','l','a','n','c','e'):		// account balances
				if (client->json.enterarray())
				{
					const char* cur;
					const char* amount;

					while (client->json.enterarray())
					{
						if ((amount = client->json.getvalue()) && (cur = client->json.getvalue()))
						{
							int t = details->balances.size();
							details->balances.resize(t+1);
							details->balances[t].amount = atof(amount);
							memcpy(details->balances[t].currency,cur,3);
							details->balances[t].currency[3] = 0;
						}

						client->json.leavearray();
					}

					client->json.leavearray();
				}
				break;

			case EOO:
				client->app->account_details(details,got_storage,got_transfer,got_pro,false,false,false);
				return;

			default:
				if (!client->json.storeobject()) return client->app->account_details(details,API_EINTERNAL);
		}
	}
}

CommandGetUserTransactions::CommandGetUserTransactions(MegaClient* client, AccountDetails* ad)
{
	cmd("utt");

	details = ad;
	tag = client->reqtag;
}

void CommandGetUserTransactions::procresult()
{
	details->transactions.clear();

	while (client->json.enterarray())
	{
		const char* handle = client->json.getvalue();
		time_t ts = client->json.getint();
		const char* delta = client->json.getvalue();
		const char* cur = client->json.getvalue();

		if (handle && ts > 0 && delta && cur)
		{
			int t = details->transactions.size();
			details->transactions.resize(t+1);
			memcpy(details->transactions[t].handle,handle,11);
			details->transactions[t].handle[11] = 0;
			details->transactions[t].timestamp = ts;
			details->transactions[t].delta = atof(delta);
			memcpy(details->transactions[t].currency,cur,3);
			details->transactions[t].currency[3] = 0;
		}

		client->json.leavearray();
	}

	client->app->account_details(details,false,false,false,false,true,false);
}

CommandGetUserPurchases::CommandGetUserPurchases(MegaClient* client, AccountDetails* ad)
{
	cmd("utp");

	details = ad;
	tag = client->reqtag;
}

void CommandGetUserPurchases::procresult()
{
	client->restag = tag;

	details->purchases.clear();

	while (client->json.enterarray())
	{
		const char* handle = client->json.getvalue();
		const time_t ts = client->json.getint();
		const char* amount = client->json.getvalue();
		const char* cur = client->json.getvalue();
		int method = (int)client->json.getint();

		if (handle && ts > 0 && amount && cur && method >= 0)
		{
			int t = details->purchases.size();
			details->purchases.resize(t+1);
			memcpy(details->purchases[t].handle,handle,11);
			details->purchases[t].handle[11] = 0;
			details->purchases[t].timestamp = ts;
			details->purchases[t].amount = atof(amount);
			memcpy(details->purchases[t].currency,cur,3);
			details->purchases[t].currency[3] = 0;
			details->purchases[t].method = method;
		}

		client->json.leavearray();
	}

	client->app->account_details(details,false,false,false,true,false,false);
}

CommandGetUserSessions::CommandGetUserSessions(MegaClient* client, AccountDetails* ad)
{
	cmd("usl");

	details = ad;
	tag = client->reqtag;
}

void CommandGetUserSessions::procresult()
{
	details->sessions.clear();

	while (client->json.enterarray())
	{
		int t = details->sessions.size();
		details->sessions.resize(t+1);

		details->sessions[t].timestamp = client->json.getint();
		details->sessions[t].mru = client->json.getint();
		client->json.storeobject(&details->sessions[t].useragent);
		client->json.storeobject(&details->sessions[t].ip);

		const char* country = client->json.getvalue();
		memcpy(details->sessions[t].country,country ? country : "\0\0",2);
		details->sessions[t].country[2] = 0;

		details->sessions[t].current = (int)client->json.getint();

		client->json.leavearray();
	}

	client->app->account_details(details,false,false,false,false,false,true);
}

void Request::add(Command* c)
{
	cmds.push_back(c);
}

int Request::cmdspending()
{
	return cmds.size();
}

int Request::get(string* req)
{
	// concatenate all command objects, resulting in an API request
	*req = "[";

	for (int i = 0; i < (int)cmds.size(); i++)
	{
		req->append(i ? ",{" : "{");
		req->append(cmds[i]->getstring());
		req->append("}");
	}

	req->append("]");

	return 1;
}

void Request::procresult(MegaClient* client)
{
	client->json.enterarray();

	for (int i = 0; i < (int)cmds.size(); i++)
	{
		client->restag = cmds[i]->tag;

		cmds[i]->client = client;

		if (client->json.enterobject())
		{
			cmds[i]->procresult();
			client->json.leaveobject();
		}
		else if (client->json.enterarray())
		{
			cmds[i]->procresult();
			client->json.leavearray();
		}
		else cmds[i]->procresult();

		if (!cmds[i]->persistent) delete cmds[i];
	}

	cmds.clear();
}

void Request::clear()
{
	cmds.clear();
}

// export node link
error MegaClient::exportnode(Node* n, int del)
{
	if (!checkaccess(n,OWNER)) return API_EACCESS;

	// exporting folder - create share
	if (n->type == FOLDERNODE) setshare(n,NULL,del ? ACCESS_UNKNOWN : RDONLY);

	// export node
	if (n->type == FOLDERNODE || n->type == FILENODE) reqs[r].add(new CommandSetPH(this,n,del));
	else return API_EACCESS;

	return API_OK;
}

CommandSetPH::CommandSetPH(MegaClient* client, Node* n, int del)
{
	cmd("l");
	arg("n",(byte*)&n->nodehandle,MegaClient::NODEHANDLE);
	if (del) arg("d",1);

	h = n->nodehandle;
	tag = client->reqtag;
}

void CommandSetPH::procresult()
{
	if (client->json.isnumeric()) return client->app->exportnode_result((error)client->json.getint());

	handle ph = client->json.gethandle();

	if (ISUNDEF(ph)) return client->app->exportnode_result(API_EINTERNAL);

	client->app->exportnode_result(h,ph);
}

// open exported file link
// formats supported: ...#!publichandle#key or publichandle#key
error MegaClient::openfilelink(const char* link)
{
	const char* ptr;
	handle ph = 0;
	byte key[Node::FILENODEKEYLENGTH];

	if ((ptr = strstr(link,"#!"))) ptr += 2;
	else ptr = link;

	if (Base64::atob(ptr,(byte*)&ph,NODEHANDLE) == NODEHANDLE)
	{
		ptr += 8;

		if (*ptr++ == '!')
		{
			if (Base64::atob(ptr,key,sizeof key) == sizeof key)
			{
				reqs[r].add(new CommandGetPH(this,ph,key));
				return API_OK;
			}
		}
	}

	return API_EARGS;
}

CommandGetPH::CommandGetPH(MegaClient* client, handle cph, const byte* ckey)
{
	cmd("g");
	arg("p",(byte*)&cph,MegaClient::NODEHANDLE);

	ph = cph;
	memcpy(key,ckey,sizeof key);
	tag = client->reqtag;
}

void CommandGetPH::procresult()
{
	if (client->json.isnumeric()) return client->app->openfilelink_result((error)client->json.getint());

	m_off_t s = -1;
	time_t ts = 0, tm = 0;
	string a, fa;

	for (;;)
	{
		switch (client->json.getnameid())
		{
			case 's':
				s = client->json.getint();
				break;

			case MAKENAMEID2('a','t'):
				client->json.storeobject(&a);
				break;

			case MAKENAMEID2('f','a'):
				client->json.storeobject(&fa);
				break;

			case MAKENAMEID2('t','s'):
				ts = client->json.getint();
				break;

			case MAKENAMEID3('t','m','d'):
				tm = ts+client->json.getint();
				break;

			case EOO:
				// we want at least the attributes
				if (s >= 0)
				{
					a.resize(Base64::atob(a.c_str(),(byte*)a.data(),a.size()));
					client->app->openfilelink_result(ph,key,s,&a,fa.c_str(),ts,tm);
				}
				else client->app->openfilelink_result(API_EINTERNAL);
				return;

			default:
				if (!client->json.storeobject()) client->app->openfilelink_result(API_EINTERNAL);
		}
	}
}

sessiontype MegaClient::loggedin()
{
	if (ISUNDEF(me)) return NOTLOGGEDIN;

	User* u = finduser(me);

	if (u && !u->email.size()) return EPHEMERALACCOUNT;

	if (!asymkey.isvalid()) return CONFIRMEDACCOUNT;

	return FULLACCOUNT;
}

error MegaClient::changepw(const byte* oldpwkey, const byte* newpwkey)
{
	User* u;

	if (!loggedin() || !(u = finduser(me))) return API_EACCESS;

	byte oldkey[SymmCipher::KEYLENGTH];
	byte newkey[SymmCipher::KEYLENGTH];

	SymmCipher pwcipher;

	memcpy(oldkey,key.key,sizeof oldkey);
	memcpy(newkey,oldkey,sizeof newkey);

	pwcipher.setkey(oldpwkey);
	pwcipher.ecb_encrypt(oldkey);

	pwcipher.setkey(newpwkey);
	pwcipher.ecb_encrypt(newkey);

	string email = u->email;

	reqs[r].add(new CommandSetMasterKey(this,oldkey,newkey,stringhash64(&email,&pwcipher)));

	return API_OK;
}

CommandSetMasterKey::CommandSetMasterKey(MegaClient* client, const byte* oldkey, const byte* newkey, uint64_t hash)
{
	cmd("up");
	arg("currk",oldkey,SymmCipher::KEYLENGTH);
	arg("k",newkey,SymmCipher::KEYLENGTH);
	arg("uh",(byte*)&hash,sizeof hash);

	tag = client->reqtag;
}

void CommandSetMasterKey::procresult()
{
	if (client->json.isnumeric()) client->app->changepw_result((error)client->json.getint());
	else client->app->changepw_result(API_OK);
}

// create ephemeral session
void MegaClient::createephemeral()
{
	byte keybuf[SymmCipher::KEYLENGTH];
	byte pwbuf[SymmCipher::KEYLENGTH];
	byte sscbuf[2*SymmCipher::KEYLENGTH];

	PrnGen::genblock(keybuf,sizeof keybuf);
	PrnGen::genblock(pwbuf,sizeof pwbuf);
	PrnGen::genblock(sscbuf,sizeof sscbuf);

	key.setkey(keybuf);
	key.ecb_encrypt(sscbuf,sscbuf+SymmCipher::KEYLENGTH,SymmCipher::KEYLENGTH);

	key.setkey(pwbuf);
	key.ecb_encrypt(keybuf);

	reqs[r].add(new CommandCreateEphemeralSession(this,keybuf,pwbuf,sscbuf));
}

CommandCreateEphemeralSession::CommandCreateEphemeralSession(MegaClient* client, const byte* key, const byte* cpw, const byte* ssc)
{
	memcpy(pw,cpw,sizeof pw);

	cmd("up");
	arg("k",key,SymmCipher::KEYLENGTH);
	arg("ts",ssc,2*SymmCipher::KEYLENGTH);

	tag = client->reqtag;
}

void CommandCreateEphemeralSession::procresult()
{
	if (client->json.isnumeric()) client->app->ephemeral_result((error)client->json.getint());
	else client->resumeephemeral(client->json.gethandle(MegaClient::USERHANDLE),pw);
}

void MegaClient::resumeephemeral(handle uh, const byte* pw)
{
	reqs[r].add(new CommandResumeEphemeralSession(this,uh,pw));
}

CommandResumeEphemeralSession::CommandResumeEphemeralSession(MegaClient* client, handle cuh, const byte* cpw)
{
	memcpy(pw,cpw,sizeof pw);
	uh = cuh;

	cmd("us");
	arg("user",(byte*)&uh,MegaClient::USERHANDLE);

	tag = client->reqtag;
}

void CommandResumeEphemeralSession::procresult()
{
	byte keybuf[SymmCipher::KEYLENGTH];
	byte sidbuf[MegaClient::SIDLEN];
	int havek = 0, havecsid = 0;

	if (client->json.isnumeric()) return client->app->ephemeral_result((error)client->json.getint());

	for (;;)
	{
		switch (client->json.getnameid())
		{
			case 'k':
				havek = client->json.storebinary(keybuf,sizeof keybuf) == sizeof keybuf;
				break;

			case MAKENAMEID4('t','s','i','d'):
				havecsid = client->json.storebinary(sidbuf,sizeof sidbuf) == sizeof sidbuf;
				break;

			case EOO:
				if (!havek || !havecsid) return client->app->ephemeral_result(API_EINTERNAL);

				client->setsid(sidbuf,sizeof sidbuf);

				client->key.setkey(pw);
				client->key.ecb_decrypt(keybuf);

				client->key.setkey(keybuf);

				if (!client->checktsid(sidbuf,sizeof sidbuf)) return client->app->ephemeral_result(API_EKEY);

				client->me = uh;

				return client->app->ephemeral_result(uh,pw);

			default:
				if (!client->json.storeobject()) return client->app->ephemeral_result(API_EINTERNAL);
		}
	}
}

void MegaClient::sendsignuplink(const char* email, const char* name, const byte* pwhash)
{
	SymmCipher pwcipher(pwhash);
	byte c[2*SymmCipher::KEYLENGTH];

	memcpy(c,key.key,sizeof key.key);
	PrnGen::genblock(c+SymmCipher::KEYLENGTH,SymmCipher::KEYLENGTH/4);
	memset(c+SymmCipher::KEYLENGTH+SymmCipher::KEYLENGTH/4,0,SymmCipher::KEYLENGTH/2);
	PrnGen::genblock(c+2*SymmCipher::KEYLENGTH-SymmCipher::KEYLENGTH/4,SymmCipher::KEYLENGTH/4);

	pwcipher.ecb_encrypt(c,c,sizeof c);

	reqs[r].add(new CommandSendSignupLink(this,email,name,c));
}

CommandSendSignupLink::CommandSendSignupLink(MegaClient* client, const char* email, const char* name, byte* c)
{
	cmd("uc");
	arg("c",c,2*SymmCipher::KEYLENGTH);
	arg("n",(byte*)name,strlen(name));
	arg("m",(byte*)email,strlen(email));

	tag = client->reqtag;
}

void CommandSendSignupLink::procresult()
{
	if (client->json.isnumeric()) return client->app->sendsignuplink_result((error)client->json.getint());

	client->json.storeobject();

	client->app->sendsignuplink_result(API_EINTERNAL);
}

// if query is 0, actually confirm account; just decode/query signup link details otherwise
void MegaClient::querysignuplink(const byte* code, unsigned len)
{
	reqs[r].add(new CommandQuerySignupLink(this,code,len));
}

CommandQuerySignupLink::CommandQuerySignupLink(MegaClient* client, const byte* code, unsigned len)
{
	confirmcode.assign((char*)code,len);

	cmd("ud");
	arg("c",code,len);

	tag = client->reqtag;
}

void CommandQuerySignupLink::procresult()
{
	string name;
	string email;
	handle uh;
	const char* kc;
	const char* pwcheck;
	string namebuf, emailbuf;
	byte pwcheckbuf[SymmCipher::KEYLENGTH];
	byte kcbuf[SymmCipher::KEYLENGTH];

	if (client->json.isnumeric()) return client->app->querysignuplink_result((error)client->json.getint());

	if (client->json.storebinary(&name) && client->json.storebinary(&email) && (uh = client->json.gethandle(MegaClient::USERHANDLE)) && (kc = client->json.getvalue()) && (pwcheck = client->json.getvalue()))
	{
		if (!ISUNDEF(uh) && Base64::atob(pwcheck,pwcheckbuf,sizeof pwcheckbuf) == sizeof pwcheckbuf && Base64::atob(kc,kcbuf,sizeof kcbuf) == sizeof kcbuf)
		{
			client->json.leavearray();

			return client->app->querysignuplink_result(uh,name.c_str(),email.c_str(),pwcheckbuf,kcbuf,(const byte*)confirmcode.data(),confirmcode.size());
		}
	}

	client->app->querysignuplink_result(API_EINTERNAL);
}

void MegaClient::confirmsignuplink(const byte* code, unsigned len, uint64_t emailhash)
{
	reqs[r].add(new CommandConfirmSignupLink(this,code,len,emailhash));
}

CommandConfirmSignupLink::CommandConfirmSignupLink(MegaClient* client, const byte* code, unsigned len, uint64_t emailhash)
{
	cmd("up");
	arg("c",code,len);
	arg("uh",(byte*)&emailhash,sizeof emailhash);

	tag = client->reqtag;
}

void CommandConfirmSignupLink::procresult()
{
	if (client->json.isnumeric()) return client->app->confirmsignuplink_result((error)client->json.getint());

	client->json.storeobject();

	client->app->confirmsignuplink_result(API_OK);
}

// generate and configure encrypted private key, plaintext public key
void MegaClient::setkeypair()
{
	CryptoPP::Integer pubk[AsymmCipher::PUBKEY];

	string privks, pubks;

	asymkey.genkeypair(asymkey.key,pubk,2048);

	AsymmCipher::serializeintarray(pubk,AsymmCipher::PUBKEY,&pubks);
	AsymmCipher::serializeintarray(asymkey.key,AsymmCipher::PRIVKEY,&privks);

	// add random padding and ECB-encrypt with master key
	unsigned t = privks.size();

	privks.resize((t+SymmCipher::BLOCKSIZE-1)&-SymmCipher::BLOCKSIZE);
	PrnGen::genblock((byte*)(privks.data()+t),privks.size()-t);

	key.ecb_encrypt((byte*)privks.data(),(byte*)privks.data(),(unsigned)privks.size());

	reqs[r].add(new CommandSetKeyPair(this,(const byte*)privks.data(),privks.size(),(const byte*)pubks.data(),pubks.size()));
}

CommandSetKeyPair::CommandSetKeyPair(MegaClient* client, const byte* privk, unsigned privklen, const byte* pubk, unsigned pubklen)
{
	cmd("up");
	arg("privk",privk,privklen);
	arg("pubk",pubk,pubklen);

	tag = client->reqtag;
}

void CommandSetKeyPair::procresult()
{
	if (client->json.isnumeric()) return client->app->setkeypair_result((error)client->json.getint());

	client->json.storeobject();

	client->app->setkeypair_result(API_OK);
}

bool MegaClient::fetchsc(DbTable* sctable)
{
	uint32_t id;
	string data;
	Node* n;
	User* u;
	node_vector dp;

	app->debug_log("Loading session from local cache");

	sctable->rewind();

	while (sctable->next(&id,&data,&key))
	{
		switch (id & 15)
		{
			case CACHEDSCSN:
				if (data.size() != sizeof cachedscsn || *(handle*)data.data() != cachedscsn) return 0;
				break;

			case CACHEDNODE:
				if ((n = Node::unserialize(this,&data,&dp))) n->dbid = id;
				else
				{
					app->debug_log("Failed - node record read error");
					return false;
				}
				break;

			case CACHEDUSER:
				if ((u = User::unserialize(this,&data))) u->dbid = id;
				else
				{
					app->debug_log("Failed - user record read error");
					return false;
				}
		}
	}

	// any child nodes arrived before their parents?
	for (int i = dp.size(); i--; ) if ((n = nodebyhandle(dp[i]->parenthandle))) dp[i]->setparent(n);

	mergenewshares(0);

	return true;
}

void MegaClient::fetchnodes()
{
	// only initial load from local cache
	if (!nodes.size() && sctable && !ISUNDEF(cachedscsn) && fetchsc(sctable))
	{
		app->debug_log("Session loaded from local cache");

		restag = reqtag;

		app->fetchnodes_result(API_OK);
		app->nodes_updated(NULL,nodes.size());

		Base64::btoa((byte*)&cachedscsn,sizeof cachedscsn,scsn);
	}
	else
	{
		// clear everything in case this is a reload
		purgenodesusersabortsc();
		reqs[r].add(new CommandFetchNodes(this));
	}
}

// fetch full node tree
CommandFetchNodes::CommandFetchNodes(MegaClient* client)
{
	cmd("f");
	arg("c","1",0);
	arg("r","1",0);

	tag = client->reqtag;
}

void MegaClient::purgenodesusersabortsc()
{
	app->clearing();

	for (sync_list::iterator it = syncs.begin(); it != syncs.end(); ) delete *(it++);
	syncs.clear();

	newsyncdebris.clear();

	syncadded.clear();
	syncdeleted[GET].clear();
	syncdeleted[PUT].clear();
	syncoverwritten.clear();
	syncidhandles.clear();

	for (node_map::iterator it = nodes.begin(); it != nodes.end(); it++) delete it->second;
	nodes.clear();

	for (newshare_list::iterator it = newshares.begin(); it != newshares.end(); it++) delete *it;
	newshares.clear();

	nodenotify.clear();
	usernotify.clear();
	users.clear();
	uhindex.clear();
	umindex.clear();

	*scsn = 0;

	if (pendingsc) pendingsc->disconnect();

	init();
	//for (int i = sizeof rootnodes/sizeof *rootnodes; i--; ) rootnodes[i] = UNDEF;
}

// purge and rebuild node/user tree
void CommandFetchNodes::procresult()
{
	if (client->json.isnumeric()) return client->app->fetchnodes_result((error)client->json.getint());

	for (;;)
	{
		switch (client->json.getnameid())
		{
			case 'f':	// nodes
				if (!client->readnodes(&client->json,0)) return client->app->fetchnodes_result(API_EINTERNAL);
				break;

			case MAKENAMEID2('o','k'):	// outgoing sharekeys
				client->readok(&client->json);
				break;

			case 's':	// outgoing shares
				client->readoutshares(&client->json);
				break;

			case 'u':	// users/contacts
				if (!client->readusers(&client->json)) return client->app->fetchnodes_result(API_EINTERNAL);
				break;

			case MAKENAMEID2('c','r'):	// crypto key request
				client->proccr(&client->json);
				break;

			case MAKENAMEID2('s','r'):	// sharekey distribution request
				client->procsr(&client->json);
				break;

			case MAKENAMEID2('s','n'):	// share node
				if (!client->setscsn(&client->json)) return client->app->fetchnodes_result(API_EINTERNAL);
				break;

			case EOO:
				if (!*client->scsn) return client->app->fetchnodes_result(API_EINTERNAL);

				client->mergenewshares(0);
				client->applykeys();
				client->app->fetchnodes_result(API_OK);
				client->initsc();

				// NULL vector: "notify all nodes"
				client->app->nodes_updated(NULL,client->nodes.size());
				return;

			default:
				if (!client->json.storeobject()) return client->app->fetchnodes_result(API_EINTERNAL);
		}
	}
}

TransferSlot::TransferSlot(Transfer* ctransfer)
{
	starttime = 0;
	progressreported = 0;
	progresscompleted = 0;
	lastdata = 0;

	fileattrsmutable = 0;

	reqs = NULL;
	pendingcmd = NULL;

	transfer = ctransfer;
	transfer->slot = this;

	connections = transfer->client->connections[transfer->type];

	reqs = new HttpReqXfer*[connections]();

	file = transfer->client->fsaccess->newfileaccess();

	slots_it = transfer->client->tslots.end();
}

// delete slot and associated resources, but keep transfer intact
TransferSlot::~TransferSlot()
{
	transfer->slot = NULL;

	if (slots_it != transfer->client->tslots.end()) transfer->client->tslots.erase(slots_it);

	if (pendingcmd) pendingcmd->cancel();

	if (file)
	{
		delete file;

		if (transfer->type == GET && transfer->localfilename.size()) transfer->client->fsaccess->unlinklocal(&transfer->localfilename);
	}

	while (connections--) delete reqs[connections];
	delete[] reqs;
}

// abort all HTTP connections
void TransferSlot::disconnect()
{
	for (int i = connections; i--; ) if (reqs[i]) reqs[i]->disconnect();
}

void HttpReq::post(MegaClient* client, const char* data, unsigned len)
{
	httpio = client->httpio;
	bufpos = 0;

	httpio->post(this,data,len);
}

void HttpReq::disconnect()
{
	if (httpio) httpio->cancel(this);
}

// coalesce block macs into file mac
int64_t TransferSlot::macsmac(chunkmac_map* macs)
{
	byte mac[SymmCipher::BLOCKSIZE] = { 0 };

	for (chunkmac_map::iterator it = macs->begin(); it != macs->end(); it++)
	{
		SymmCipher::xorblock(it->second.mac,mac);
		transfer->key.ecb_encrypt(mac);
	}

	macs->clear();

	uint32_t* m = (uint32_t*)mac;

	m[0] ^= m[1];
	m[1] = m[2]^m[3];

	return *(int64_t*)mac;
}

// file transfer state machine
void TransferSlot::doio(MegaClient* client)
{
	if (!tempurl.size()) return;

	time_t backoff = 0;
	m_off_t p = 0;

	for (int i = connections; i--; )
	{
		if (reqs[i])
		{
			switch (reqs[i]->status)
			{
				case REQ_INFLIGHT:
					p += reqs[i]->transferred(client);
					break;

				case REQ_SUCCESS:
					lastdata = client->waiter->ds;

					progresscompleted += reqs[i]->size;

					if (transfer->type == PUT)
					{
						// completed put transfers are signalled through the return of the upload token
						if (reqs[i]->in.size())
						{
							if (reqs[i]->in.size() == NewNode::UPLOADTOKENLEN*4/3)
							{
								if (Base64::atob(reqs[i]->in.data(),ultoken,NewNode::UPLOADTOKENLEN+1) == NewNode::UPLOADTOKENLEN)
								{
									memcpy(transfer->filekey,transfer->key.key,sizeof transfer->key.key);
									((int64_t*)transfer->filekey)[2] = transfer->ctriv;
									((int64_t*)transfer->filekey)[3] = macsmac(&transfer->chunkmacs);
									SymmCipher::xorblock(transfer->filekey+SymmCipher::KEYLENGTH,transfer->filekey);

									return transfer->complete();
								}
							}

							// fail with returned error
							return transfer->failed((error)atoi(reqs[i]->in.c_str()));
						}
					}
					else
					{
						reqs[i]->finalize(file,&transfer->key,&transfer->chunkmacs,transfer->ctriv,0,-1);

						if (progresscompleted == transfer->size)
						{
							// verify meta MAC
							if (macsmac(&transfer->chunkmacs) == transfer->metamac) return transfer->complete();
							else return transfer->failed(API_EKEY);
						}
					}

					reqs[i]->status = REQ_READY;
					break;

				case REQ_FAILURE:
					reqs[i]->status = REQ_PREPARED;
					break;

					if (reqs[i]->httpstatus == 509)
					{
						client->app->transfer_limit(transfer);

						// fixed ten-minute retry intervals
						backoff = 6000;
					}
					else return transfer->failed(API_ETEMPUNAVAIL);

				default:;
			}
		}

		if (!reqs[i] || reqs[i]->status == REQ_READY)
		{
			m_off_t npos = ChunkedHash::chunkceil(transfer->pos);

			if (npos > transfer->size) npos = transfer->size;

			if (npos > transfer->pos || !transfer->size)
			{
				if (!reqs[i]) reqs[i] = (transfer->type == PUT) ? (HttpReqXfer*)new HttpReqUL() : (HttpReqXfer*)new HttpReqDL();

				reqs[i]->prepare(file,tempurl.c_str(),&transfer->key,&transfer->chunkmacs,transfer->ctriv,transfer->pos,npos);
				reqs[i]->status = REQ_PREPARED;
				transfer->pos = npos;
			}
			else if (reqs[i]) reqs[i]->status = REQ_DONE;
		}

		if (reqs[i] && reqs[i]->status == REQ_PREPARED) reqs[i]->post(client);
	}

	p += progresscompleted;

	if (p != progressreported)
	{
		progressreported = p;
		lastdata = client->waiter->ds;

		progress();
	}

	if (client->waiter->ds-lastdata >= XFERTIMEOUT) return transfer->failed(API_EFAILED);
	else
	{
		if (!backoff)
		{
			// no other backoff: check again at XFERMAXFAIL
			backoff = XFERTIMEOUT-(client->waiter->ds-lastdata);
		}

		transfer->bt.backoff(client->waiter->ds,backoff);
	}
}

// transfer completion: copy received file locally, set timestamp(s), verify fingerprint, notify app, notify files
void Transfer::complete()
{
	if (type == GET)
	{
		// disconnect temp file from slot...
		delete slot->file;
		slot->file = NULL;

		// set timestamp (subsequent moves & copies are assumed not to alter mtime)
		client->fsaccess->setmtimelocal(&localfilename,mtime);

		// verify integrity of file
		FileAccess* fa = client->fsaccess->newfileaccess();
		FileFingerprint fingerprint;
		Node* n;

		if (fa->fopen(&localfilename,true,false))
		{
			fingerprint.genfingerprint(fa);
			delete fa;

			if (isvalid && !(fingerprint == *(FileFingerprint*)this))
			{
				client->fsaccess->unlinklocal(&localfilename);
				return failed(API_EWRITE);
			}
		}

		// set FileFingerprint on source node(s) if missing
		for (file_list::iterator it = files.begin(); it != files.end(); it++)
		{
			if ((n = client->nodebyhandle((*it)->h)))
			{
				if (!n->isvalid)
				{
					*(FileFingerprint*)n = fingerprint;

					n->serializefingerprint(&n->attrs.map['c']);
					client->setattr(n);
				}
			}
		}

		// ...and place it in all target locations
		string* renamedto = NULL;

		// rename file to one of its final target locations
		for (file_list::iterator it = files.begin(); it != files.end(); it++)
		{
			if (client->fsaccess->renamelocal(&localfilename,&(*it)->localname))
			{
				renamedto = &(*it)->localname;
				break;
			}
		}

		// copy to the other remaining target locations
		for (file_list::iterator it = files.begin(); it != files.end(); it++)
		{
			if (renamedto != &(*it)->localname) client->fsaccess->copylocal(renamedto ? renamedto : &localfilename,&(*it)->localname);
		}

		if (!renamedto) client->fsaccess->unlinklocal(&localfilename);
	}
	else
	{
		// files must not change during a PUT transfer
		if (genfingerprint(slot->file)) return failed(API_EREAD);
	}

	client->app->transfer_complete(this);

	// notify all files and give them an opportunity to self-destruct
	for (file_list::iterator it = files.begin(); it != files.end(); )
	{
		Transfer* t = (*it)->transfer;

		(*it)->transfer = NULL;
		(*it++)->completed(t,NULL);
	}

	delete this;
}

// transfer progress notification to app and related files
void TransferSlot::progress()
{
	transfer->client->app->transfer_update(transfer);

	for (file_list::iterator it = transfer->files.begin(); it != transfer->files.end(); it++) (*it)->progress();
}

HttpReq::HttpReq(int b)
{
	binary = b;

	status = REQ_READY;
	buf = NULL;

	httpio = NULL;
	httpiohandle = NULL;
	out = &outbuf;
}

HttpReq::~HttpReq()
{
	if (httpio) httpio->cancel(this);

	delete[] buf;
}

void HttpReq::setreq(const char* u, contenttype t)
{
	posturl = u;
	type = t;
}

// add data to fixed or variable buffer
void HttpReq::put(void* data, unsigned len)
{
	if (buf)
	{
		if (bufpos+len > buflen) len = buflen-bufpos;

		memcpy(buf+bufpos,data,len);
		bufpos += len;
	}
	else in.append((char*)data,len);
}

// make space for receiving data; adjust len if out of space
byte* HttpReq::reserveput(unsigned* len)
{
	if (buf)
	{
		if (bufpos+*len > buflen) *len = buflen-bufpos;
		return buf+bufpos;
	}
	else
	{
		if (bufpos+*len > in.size()) in.resize(bufpos+*len);
		*len = in.size()-bufpos;
		return (byte*)in.data()+bufpos;
	}
}

// confirm the receipt of data
void HttpReq::completeput(unsigned len)
{
	bufpos += len;
}

// number of bytes transferred in this request
m_off_t HttpReq::transferred(MegaClient*)
{
	if (buf) return bufpos;
	else return in.size();
}

// prepare file chunk download
void HttpReqDL::prepare(FileAccess* fa, const char* tempurl, SymmCipher* key, chunkmac_map* macs, uint64_t ctriv, m_off_t pos, m_off_t npos)
{
	char urlbuf[256];

	snprintf(urlbuf,sizeof urlbuf,"%s/%" PRIu64 "-%" PRIu64,tempurl,pos,npos-1);
	setreq(urlbuf,REQ_BINARY);

	dlpos = pos;
	size = (unsigned)(npos-pos);

	if (!buf || buflen != size)
	{
		// (re)allocate buffer
		if (buf) delete[] buf;

		buf = new byte[(size+SymmCipher::BLOCKSIZE-1)&-SymmCipher::BLOCKSIZE];
		buflen = size;
	}
}

// decrypt, mac and write downloaded chunk
void HttpReqDL::finalize(FileAccess* fa, SymmCipher* key, chunkmac_map* macs, uint64_t ctriv, m_off_t startpos, m_off_t endpos)
{
	byte mac[SymmCipher::BLOCKSIZE] = { 0 };

	key->ctr_crypt(buf,bufpos,dlpos,ctriv,mac,0);

	unsigned skip;
	unsigned prune;

	if (endpos == -1) skip = prune = 0;
	else
	{
		if (startpos > dlpos) skip = (unsigned)(startpos-dlpos);
		else skip = 0;

		if (dlpos+bufpos > endpos) prune = (unsigned)(dlpos+bufpos-endpos);
		else prune = 0;
	}

	fa->fwrite(buf+skip,bufpos-skip-prune,dlpos+skip);

	memcpy((*macs)[dlpos].mac,mac,sizeof mac);
}

// prepare chunk for uploading: mac and encrypt
void HttpReqUL::prepare(FileAccess* fa, const char* tempurl, SymmCipher* key, chunkmac_map* macs, uint64_t ctriv, m_off_t pos, m_off_t npos)
{
	byte mac[SymmCipher::BLOCKSIZE] = { 0 };
	char buf[256];

	snprintf(buf,sizeof buf,"%s/%" PRIu64,tempurl,pos);
	setreq(buf,REQ_BINARY);

	size = (unsigned)(npos-pos);

	// FIXME: check return value and abort upload in case file read fails
	fa->fread(out,size,(-(int)size)&(SymmCipher::BLOCKSIZE-1),pos);

	key->ctr_crypt((byte*)out->data(),size,pos,ctriv,mac,1);

	memcpy((*macs)[pos].mac,mac,sizeof mac);

	// unpad for POSTing
	out->resize(size);
}

// number of bytes sent in this request
m_off_t HttpReqUL::transferred(MegaClient* client)
{
	if (httpiohandle) return client->httpio->postpos(httpiohandle);

	return 0;
}

// chunk's start position
m_off_t ChunkedHash::chunkfloor(m_off_t p)
{
	m_off_t cp, np;

	cp = 0;

	for (unsigned i = 1; i <= 8; i++)
	{
		np = cp+i*SEGSIZE;
		if (p >= cp && p < np) return cp;
		cp = np;
	}

	return ((p-cp)&-(8*SEGSIZE))+cp;
}

// chunk's end position (== start of next chunk)
m_off_t ChunkedHash::chunkceil(m_off_t p)
{
	m_off_t cp, np;

	cp = 0;

	for (unsigned i = 1; i <= 8; i++)
	{
		np = cp+i*SEGSIZE;
		if (p >= cp && p < np) return np;
		cp = np;
	}

	return ((p-cp)&-(8*SEGSIZE))+cp+8*SEGSIZE;
}

// cryptographic signature generation/verification
HashSignature::HashSignature(Hash* h)
{
	hash = h;
}

HashSignature::~HashSignature()
{
	delete hash;
}

void HashSignature::add(const byte* data, unsigned len)
{
	hash->add(data,len);
}

unsigned HashSignature::get(AsymmCipher* privk, byte* sigbuf, unsigned sigbuflen)
{
	string h;

	hash->get(&h);

	return privk->rawdecrypt((const byte*)h.data(),h.size(),sigbuf,sigbuflen);
}

int HashSignature::check(AsymmCipher* pubk, const byte* sig, unsigned len)
{
	string h, s;

	hash->get(&h);

	s.resize(h.size());

	if (pubk->rawencrypt(sig,len,(byte*)s.data(),s.size()) != h.size()) return 0;

	return s == h;
}

bool operator==(FileFingerprint& lhs, FileFingerprint& rhs)
{
	// size differs - cannot be equal
	if (lhs.size != rhs.size) return false;

	// mtime differs - cannot be equal
	if (lhs.mtime != rhs.mtime) return false;

	// FileFingerprints not fully available - give it the benefit of the doubt
	if (!lhs.isvalid || !rhs.isvalid) return true;

	return !memcmp(lhs.crc,rhs.crc,sizeof lhs.crc);
}

FileFingerprint::FileFingerprint()
{
	// mark as invalid
	size = -1;
	mtime = 0;
	isvalid = false;
}

FileFingerprint& FileFingerprint::operator=(FileFingerprint& rhs)
{
	isvalid = rhs.isvalid;
	size = rhs.size;
	mtime = rhs.mtime;
	memcpy(crc,rhs.crc,sizeof crc);

	return *this;
}

bool FileFingerprint::genfingerprint(FileAccess* fa)
{
	bool changed = false;

	if (mtime != fa->mtime)
	{
		mtime = fa->mtime;
		changed = true;
	}

	if (size != fa->size)
	{
		size = fa->size;
		changed = true;
	}

	if (size <= (m_off_t)sizeof crc)
	{
		// tiny file: just read, NUL pad
		fa->frawread(crc,size,0);
		memset(crc+size,0,sizeof crc-size);
	}
	else if (size <= (m_off_t)(sizeof crc*sizeof crc))
	{
		// small file: read byte pattern, no CRC
		for (unsigned i = 0; i < sizeof crc; i++) fa->frawread(crc+i,1,i*(size-1)/(sizeof crc-1));
	}
	else
	{
		byte newcrc[sizeof crc];

		// larger file: parallel sparse CRC block pattern
		byte block[4*sizeof crc];
		unsigned blocks = size/(sizeof crc*sizeof crc);

		if (blocks > 32) blocks = 32;

		for (unsigned i = 0; i < sizeof crc/4; i++)
		{
			HashCRC32 crc32;

			for (unsigned j = 0; j < blocks; j++)
			{
				if (!fa->frawread(block,sizeof block,(size-sizeof block)*(i*blocks+j)/(sizeof crc/4*blocks-1)))
				{
					size = -1;
					return true;
				}

				crc32.add(block,sizeof block);
			}

			crc32.get(newcrc+4*i);
		}

		if (memcmp(crc,newcrc,sizeof crc))
		{
			memcpy(crc,newcrc,sizeof crc);
			changed = true;
		}
	}

	if (!isvalid)
	{
		isvalid = true;
		changed = true;
	}

	return changed;
}

// convert this FileFingerprint to string
void FileFingerprint::serializefingerprint(string* d)
{
	byte buf[sizeof crc+1+sizeof mtime];
	int l;

	memcpy(buf,crc,sizeof crc);
	l = Serialize64::serialize(buf+sizeof crc,mtime);

	d->resize((sizeof crc+l)*4/3+4);
	d->resize(Base64::btoa(buf,sizeof crc+l,(char*)d->c_str()));
}

// decode and set base64-encoded fingerprint
int FileFingerprint::unserializefingerprint(string* d)
{
	byte buf[sizeof crc+sizeof mtime+1];
	unsigned l;
	int64_t t;

	if ((l = Base64::atob(d->c_str(),buf,sizeof buf)) < sizeof crc+1) return 0;
	if (Serialize64::unserialize(buf+sizeof crc,l-sizeof crc,&t) < 0) return 0;

	memcpy(crc,buf,sizeof crc);

	mtime = t;

	isvalid = true;

	return 1;
}

// a new sync reads the full local tree and issues all commands required to equalize both sides
Sync::Sync(MegaClient* cclient, string* crootpath, Node* remotenode)
{
	client = cclient;
	
	localbytes = 0;
	localnodes[FILENODE] = 0;
	localnodes[FOLDERNODE] = 0;

	state = SYNC_INITIALSCAN;
	localroot.init(this,crootpath,FOLDERNODE,NULL,crootpath);
	localroot.setnode(remotenode);
	
	queuescan(NULL,NULL,NULL,NULL,true);
	procscanstack();
	
	sync_it = client->syncs.insert(client->syncs.end(),this);
}

Sync::~Sync()
{
	// prevent mass deletion while rootlocal destructor runs
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

// initialize fresh LocalNode object - must be called exactly once
void LocalNode::init(Sync* csync, string* clocalname, nodetype ctype, LocalNode* cparent, string* clocalpath)
{
	sync = csync;
	parent = cparent;
	node = NULL;
	type = ctype;
	syncid = sync->client->nextsyncid();
	
	localname = *clocalname;	

	if (parent)
	{
		// we don't construct a UTF-8 name for the root path
		name = *clocalname;
		sync->client->fsaccess->local2name(&name);

		parent->children[&localname] = this;
	}

	// enable folder notification
	if (type == FOLDERNODE) sync->client->fsaccess->addnotify(this,clocalpath);

	sync->client->syncactivity = true;
}

void LocalNode::setnode(Node* cnode)
{
	node = cnode;
	node->localnode = this;

	sync->client->syncidhandles[syncid] = node->nodehandle;
}

LocalNode::~LocalNode()
{
	if (sync->state >= SYNC_INITIALSCAN)
	{
		// eliminate queued filesystem events for direct children
		for (deque<ScanItem>::iterator it = sync->scanstack.begin(); it != sync->scanstack.end(); it++) if ((*it).parent == this) (*it).deleted = true;

		// record deletion
		sync->client->syncdeleted[type].insert(syncid);
	}

	if (type == FOLDERNODE) sync->client->fsaccess->delnotify(this);

	if (parent) parent->children.erase(&localname);

	for (localnode_map::iterator it = children.begin(); it != children.end(); ) delete it++->second;

	if (node)
	{
		node->syncdeleted = true;
		node->localnode = NULL;
	}
}

void LocalNode::getlocalpath(MegaClient* client, string* path)
{
	LocalNode* l = this;

	path->erase();

	while (l)
	{
		path->insert(0,l->localname);
		if ((l = l->parent)) path->insert(0,client->fsaccess->localseparator);
	}
}

void LocalNode::prepare()
{
	getlocalpath(transfer->client,&transfer->localfilename);
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
	for (it = parent->children.begin(); it != parent->children.end(); it++) if (scanseqno != it->second->scanseqno) delete it->second;

	delete da;
}

LocalNode* Sync::queuefsrecord(string* localpath, string* localname, LocalNode* parent, bool fulltree)
{
	localnode_map::iterator it;
	LocalNode* l;
	
	// check if this record is to be ignored
	if (!client->fsaccess->localhidden(localpath,localname))
	{
		l = (it = parent->children.find(localname)) != parent->children.end() ? it->second : NULL;
		queuescan(localpath,localname,l,parent,fulltree);

		return l;
	}
	
	return NULL;
}

void Sync::queuescan(string* localpath, string* localname, LocalNode* localnode, LocalNode* parent, bool fulltree)
{
//	client->syncactivity = true;

	// FIXME: efficient copy-free push_back?
	scanstack.resize(scanstack.size()+1);

	ScanItem* si = &scanstack.back();

	// FIXME: don't create mass copies of localpath
	if (localpath) si->localpath = *localpath;
	if (localname) si->localname = *localname;
	si->localnode = localnode;
	si->parent = parent;
	si->fulltree = fulltree;
	si->deleted = false;
	
	client->syncactivity = true;
}

// add or refresh local filesystem item from scan stack, add items to scan stack
// must be called with a scanstack.siz() > 0
void Sync::procscanstack()
{
	client->syncactivity = true;

	ScanItem* si = &*scanstack.begin();

	// ignore deleted ScanItems
	if (si->deleted)
	{
		scanstack.pop_front();
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
		localnode_map::iterator it = si->parent->children.find(localname);

		if (it != si->parent->children.end()) l = it->second;
		else l = NULL;
	}
	else l = NULL;

	if (l)
	{
		localnodes[l->type]--;
		if (l->type == FILENODE) localbytes -= l->size;
	}

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
		else if (l->genfingerprint(fa)) changed = true;

		if (changed) client->syncadded.insert(l->syncid);
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

	if (l)
	{
		if (changed)
		{
			client->syncactivity = true;
			client->fsaccess->local2path(localpath,&tmpname);
		}

		localnodes[l->type]++;

		if (l->type == FILENODE)
		{
			if (changed) client->app->syncupdate_local_file_addition(this,tmpname.c_str());
			localbytes += l->size;			
		}
		else if (changed) client->app->syncupdate_local_folder_addition(this,tmpname.c_str());		
	}

	delete fa;

	scanstack.pop_front();
}

// syncids are usable to indicate putnodes()-local parent linkage
handle MegaClient::nextsyncid()
{
	byte* ptr = (byte*)&currsyncid;

	while (!++*ptr && ptr < (byte*)&currsyncid+NODEHANDLE) ptr++;

	return currsyncid;
}

SyncFileGet::SyncFileGet(Node* cn, string* clocalname)
{
	n = cn;
	h = n->nodehandle;
	*(FileFingerprint*)this = *n;
	localname = *clocalname;

	n->syncget = this;
}

SyncFileGet::~SyncFileGet()
{
	n->syncget = NULL;
}

// complete, then self-destruct
void SyncFileGet::completed(Transfer* t, LocalNode* n)
{
	File::completed(t,n);
	delete this;
}

// initial downward sync - further updates are driven by incoming server-client commands
// this is called after the local node tree is complete, but before the first call to syncup()
// recursively traverse tree of remote Nodes and match with LocalNode tree
// create missing local folders
// add locally missing files to the GET queue
void MegaClient::syncdown(LocalNode* l, string* localpath)
{
	remotenode_map nchildren;
	remotenode_map::iterator rit;

	// build array of sync-relevant (newest alias wins) remote children by name
	attr_map::iterator ait;
	Node** npp;

	// UTF-8 converted local name
	string localname;

	string tmpname;

	// build child hash - nameclash resolution: use newest version
	for (node_list::iterator it = l->node->children.begin(); it != l->node->children.end(); it++)
	{
		// node must be decrypted and name defined to be considered
		if (!(*it)->syncdeleted && !(*it)->attrstring.size() && (ait = (*it)->attrs.map.find('n')) != (*it)->attrs.map.end())
		{
			// map name to node (overwrite only if newer)
			npp = &nchildren[&ait->second];
			if (!*npp || (*it)->mtime > (*npp)->mtime) *npp = *it;
		}
	}

	// eliminate remote items that exist locally, recuse into existing folders
	for (localnode_map::iterator lit = l->children.begin(); lit != l->children.end(); lit++)
	{
		localname = *lit->first;
		fsaccess->local2name(&localname);
		rit = nchildren.find(&localname);

		// do we have a corresponding remote child?
		if (rit != nchildren.end())
		{
			// corresponding remote node exists
			// local: folder, remote: file - ignore
			// local: file, remote: folder - ignore
			// local: folder, remote: folder - recurse
			// local: file, remote: file - overwrite if newer
			if (lit->second->type != rit->second->type)
			{
				// folder/file clash: do nothing (rather than attempting to second-guess the user)
				nchildren.erase(rit);
			}
			else if (lit->second->type == FILENODE)
			{
				// file on both sides - do not overwrite if local version older or identical
				if (lit->second->mtime > rit->second->mtime)
				{
					// local version is older
					nchildren.erase(rit);
				}
				else if (*lit->second == *(FileFingerprint*)rit->second)
				{
					// both files are identical
					nchildren.erase(rit);
				}
			}
			else
			{
				// recurse into directories of equal name
				size_t t = localpath->size();

				localpath->append(fsaccess->localseparator);
				localpath->append(lit->second->localname);

				lit->second->setnode(rit->second);

				syncdown(lit->second,localpath);

				localpath->resize(t);

				nchildren.erase(rit);
			}
		}
	}

	// create missing local folders / FolderNodes, initiate downloads of missing local files
	for (rit = nchildren.begin(); rit != nchildren.end(); rit++)
	{
		if ((ait = rit->second->attrs.map.find('n')) != rit->second->attrs.map.end())
		{
			size_t t = localpath->size();

			localpath->append(fsaccess->localseparator);
			localname = ait->second;
			fsaccess->name2local(&localname);
			localpath->append(localname);

			if (rit->second->type == FILENODE)
			{
				// start fetching this node, unless fetch is already in progress
				// FIXME: to cover renames that occur during the download, reconstruct localname in complete()
				if (!rit->second->syncget)
				{
					fsaccess->local2path(localpath,&tmpname);
					app->syncupdate_get(l->sync,tmpname.c_str());

					rit->second->syncget = new SyncFileGet(rit->second,localpath);
					startxfer(GET,rit->second->syncget);
					syncactivity = true;
				}
			}
			else
			{
				// create local path, add to LocalNodes and recurse
				if (fsaccess->mkdirlocal(localpath))
				{
					fsaccess->local2path(localpath,&tmpname);
					app->syncupdate_local_mkdir(l->sync,tmpname.c_str());

					LocalNode* ll;

					// create local folder and start notifications
					ll = new LocalNode;
					ll->init(l->sync,&ait->second,FOLDERNODE,l,localpath);
					ll->setnode(rit->second);

					syncdown(ll,localpath);

					syncactivity = true;
				}
			}

			localpath->resize(t);
		}
	}
}

// recursively traverse tree of LocalNodes and match with remote Nodes
// mark nodes to be rubbished in deleted. with their nodehandle
// mark additional nodes to to rubbished (those overwritten) by accumulating their nodehandles in rubbish.
// nodes to be added are stored in synccreate. - with nodehandle set to parent if attached to an existing node
// l and n are assumed to be folders and existing on both sides or scheduled for creation
void MegaClient::syncup(LocalNode* l/*, Node* n*/)
{
	remotenode_map nchildren;
	remotenode_map::iterator rit;

	// build array of sync-relevant (newest alias wins) remote children by name
	attr_map::iterator ait;
	Node** npp;

	// UTF-8 converted local name
	string localname;

	string tmpname;

//	if (n)
	if (l->node)
	{
		// corresponding remote node present: build child hash - nameclash resolution: use newest version
		for (node_list::iterator it = l->node->children.begin(); it != l->node->children.end(); it++)
		{
			// node must be decrypted and name defined to be considered
			if (!(*it)->attrstring.size() && (ait = (*it)->attrs.map.find('n')) != (*it)->attrs.map.end())
			{
				// map name to node (overwrite only if newer)
				npp = &nchildren[&ait->second];
				if (!*npp || (*it)->mtime > (*npp)->mtime) *npp = *it;
			}
		}
	}

	// check for elements that need to be created, deleted or updated on the remote side
	for (localnode_map::iterator lit = l->children.begin(); lit != l->children.end(); lit++)
	{
		if (!syncdeleted[lit->second->type].count(lit->second->syncid))
		{
			localname = *lit->first;
			fsaccess->local2name(&localname);
			rit = nchildren.find(&localname);

			// local node must be new and not deleted to be considered
			if (syncadded.count(lit->second->syncid))
			{
				// do we have a corresponding remote child?
				if (rit != nchildren.end())
				{
					// corresponding remote node exists
					// local: folder, remote: file - ignore
					// local: file, remote: folder - ignore
					// local: folder, remote: folder - recurse
					// local: file, remote: file - overwrite if newer
					if (lit->second->type != rit->second->type)
					{
						// folder/file clash - do nothing rather than attempting to second-guess the user
						continue;
					}

					// file on both sides - do not overwrite if local version older or identical
					if (lit->second->type == FILENODE)
					{
						if (lit->second->size == rit->second->size)
						{
							// check if file is likely to be identical
							if (lit->second->mtime < rit->second->mtime)
							{
								// do not overwrite more recent remote file
								continue;
							}

							if (rit->second->isvalid ? (*lit->second == *(FileFingerprint*)rit->second) : (lit->second->mtime == rit->second->mtime))
							{
								// files have the same size and the same mtime (or the same fingerprint, if available): no action needed
								lit->second->setnode(rit->second);
								continue;
							}
						}

						// overwriting a remote file - queue deletion of existing file
						syncoverwritten[lit->second->syncid] = rit->second->nodehandle;
						syncactivity = true;
					}
					else
					{
						// recurse into directories of equal name
						lit->second->setnode(rit->second);

						syncup(lit->second/*,rit->second*/);
						continue;
					}
				}

				// create remote folder or send file
				synccreate.push_back(lit->second);
				syncactivity = true;
			}

			if (lit->second->type == FOLDERNODE) syncup(lit->second/*,rit == nchildren.end() ? NULL : rit->second*/);
		}
	}
}

// execute updates stored in syncdeleted[], syncoverwritten[] and synccreate[]
// skip if a sync-related putnodes() is currently in progress
void MegaClient::syncupdate()
{
	// only one outstanding node update at a time
	if ((!syncadded.size() && !syncdeleted[FILENODE].size() && !syncdeleted[FOLDERNODE].size())) return;

	// split synccreate[] in separate subtrees and send off to putnodes() for creation on the server
	unsigned i, start, end;
	SymmCipher tkey;
	string tattrstring;
	AttrMap tattrs;
	Node* n;
	NewNode* nn;
	NewNode* nnp;
	LocalNode* l;
	string tmpname;

	for (start = 0; start < synccreate.size(); start = end)
	{
		// determine length of distinct subtree beneath existing node
		for (end = start; end < synccreate.size(); end++) if (end > start && synccreate[end]->parent->node) break;

		// add nodes that can be created immediately: folders & existing files; start uploads of new files
		nn = nnp = new NewNode[end-start];

		for (i = start; i < end; i++)
		{
			n = NULL;
			l = synccreate[i];

			if (l->type == FOLDERNODE || (n = nodebyfingerprint(l)))
			{
				// create remote folder or copy file if it already exists
				nnp->source = NEW_NODE;
				nnp->type = l->type;
				nnp->syncid = l->syncid;
				nnp->localnode = l;
				nnp->nodehandle = n ? n->nodehandle : l->syncid;
				nnp->parenthandle = i > start ? l->parent->syncid : UNDEF;

				if (n)
				{
					// this is a file - copy, use original key & attributes
					nnp->clienttimestamp = l->mtime;
					nnp->nodekey = n->nodekey;
					tattrs.map = n->attrs.map;
					
					app->syncupdate_remote_copy(l->sync,l->name.c_str());
				}
				else
				{
					// this is a folder - create, use fresh key & attributes
					nnp->clienttimestamp = time(NULL);
					nnp->nodekey.resize(Node::FOLDERNODEKEYLENGTH);
					PrnGen::genblock((byte*)nnp->nodekey.data(),Node::FOLDERNODEKEYLENGTH);
					tattrs.map.clear();

					app->syncupdate_remote_mkdir(l->sync,l->name.c_str());
				}

				// set new name, encrypt and attach attributes
				tattrs.map['n'] = l->name;
				tattrs.getjson(&tattrstring);
				tkey.setkey((const byte*)nnp->nodekey.data(),nnp->type);
				makeattr(&tkey,&nnp->attrstring,tattrstring.c_str());

				nnp++;
			}
			else if (l->type == FILENODE)
			{
				// FIXME: move if it is in rubbish to reduce node creation load
				startxfer(PUT,l);
				app->syncupdate_put(l->sync,l->name.c_str());
			}
		}

		if (nnp == nn) delete[] nn;
		else
		{
			syncadding++;

			reqs[r].add(new CommandPutNodes(this,synccreate[start]->parent->node->nodehandle,NULL,nn,nnp-nn,0,PUTNODES_SYNC));
			syncactivity = true;
		}
	}

	synccreate.clear();
	syncadded.clear();

	syncidhandle_map::iterator sit;

	// deletions of synced files: move to rubbish
	for (handle_set::iterator it = syncdeleted[FILENODE].begin(); it != syncdeleted[FILENODE].end(); it++)
	{
		if ((sit = syncidhandles.find(*it)) != syncidhandles.end())
		{
			if ((n = nodebyhandle(sit->second)))
			{
				app->syncupdate_remote_unlink(n);

				movetosyncdebris(n);
			}

			syncidhandles.erase(sit);
			syncactivity = true;
		}
	}

	syncdeleted[FILENODE].clear();

	// deletions that were queued while the putnodes() was still in progress
	for (handle_set::iterator it = syncdeleted[FOLDERNODE].begin(); it != syncdeleted[FOLDERNODE].end(); it++)
	{
		if ((sit = syncidhandles.find(*it)) != syncidhandles.end())
		{
			if ((n = nodebyhandle(sit->second)))
			{
				app->syncupdate_remote_rmdir(n);

				unlink(n);
			}

			syncidhandles.erase(sit);
			syncactivity = true;
		}
	}

	syncdeleted[FOLDERNODE].clear();
}

void MegaClient::putnodes_sync_result(error e, NewNode* nn)
{
	delete[] nn;

	if (e == API_OK)
	{
		Node* n;
		syncidhandle_map::iterator sit;

		// overwrites: overwritten node gets moved to rubbish bin
		for (syncidhandle_map::iterator it = syncoverwritten.begin(); it != syncoverwritten.end(); it++) if (syncidhandles.count(it->first)) if ((n = nodebyhandle(it->second))) movetosyncdebris(n);

		syncoverwritten.clear();
	}

	syncadding--;
	syncactivity = true;
}

// request upload of the file identified by the supplied fingerprint
void MegaClient::syncupload(LocalNode* l)
{
	startxfer(PUT,l);
}

// inject file into transfer subsystem
// file's fingerprint must be set
bool MegaClient::startxfer(direction d, File* f)
{
	if (!f->transfer)
	{
		if (d == PUT)
		{
			if (!f->isvalid)
			{
				// missing FileFingerprint for local file - generate
				FileAccess* fa = fsaccess->newfileaccess();
				if (fa->fopen(&f->localname,d == PUT,d == GET)) f->genfingerprint(fa);
				delete fa;
			}

			// if we are unable to obtain a valid file FileFingerprint, don't proceed
			if (!f->isvalid) return false;
		}

		Transfer* t;
		transfer_map::iterator it = transfers[d].find(f);

		if (it != transfers[d].end()) t = it->second;
		else
		{
			t = new Transfer(this,d);
			*(FileFingerprint*)t = *(FileFingerprint*)f;
			t->size = f->size;
			t->transfers_it = transfers[d].insert(pair<FileFingerprint*,Transfer*>((FileFingerprint*)t,t)).first;
			app->transfer_added(t);
		}

		f->file_it = t->files.insert(t->files.begin(),f);
		f->transfer = t;
	}

	return true;
}

// remove file from transfer subsystem
void MegaClient::stopxfer(File* f)
{
	if (f->transfer)
	{
		f->transfer->files.erase(f->file_it);

		if (!f->transfer->files.size())
		{
			// last file for this transfer removed
			app->transfer_removed(f->transfer);

			delete f->transfer;
		}

		f->transfer = NULL;
	}
}

Node* MegaClient::nodebyfingerprint(FileFingerprint* fingerprint)
{
	fingerprint_set::iterator it;

	if ((it = fingerprints.find(fingerprint)) != fingerprints.end()) return (Node*)*it;

	return NULL;
}

CommandMoveSyncDebris::CommandMoveSyncDebris(MegaClient* client, handle ch, Node* t)
{
	h = ch;

	cmd("m");

	arg("n",(byte*)&h,MegaClient::NODEHANDLE);
	arg("t",(byte*)&t->nodehandle,MegaClient::NODEHANDLE);

	client->movedebrisinflight++;
}

void CommandMoveSyncDebris::procresult()
{
	bool ok;
	handlecount_map::iterator it;

	if (client->json.isnumeric()) ok = (error)client->json.getint() == API_OK;
	else
	{
		client->json.storeobject();
		ok = false;
	}

	if ((it = client->newsyncdebris.find(h)) != client->newsyncdebris.end())
	{
		if (it->second == 3) ok = true;	// give up
		else it->second++;
		
		if (ok) client->newsyncdebris.erase(it);
	}

	if (!--client->movedebrisinflight) client->movetosyncdebris(NULL);
}

// FIXME: create sync subfolder in //bin to hold the sync rubbish
void MegaClient::movetosyncdebris(Node* n)
{
	if (n) newsyncdebris[n->nodehandle] = 0;

	if ((!n || !syncdebrisadding) && newsyncdebris.size())
	{
		Node* p;

		if ((p = nodebyhandle(rootnodes[RUBBISHNODE-ROOTNODE])))
		{
			// check if we have today's sync debris subfolder in rubbish bin
			handle h;
			time_t t = time(NULL);
			struct tm* ptm = gmtime(&t);
			char buf[32];

			sprintf(buf,"%04d-%02d-%02d",ptm->tm_year+1900,ptm->tm_mon+1,ptm->tm_mday);

			if ((p = childnodebyname(p,SYNCDEBRISFOLDERNAME)))
			{
				h = p->nodehandle;

				if ((p = childnodebyname(p,buf)))
				{
					for (handlecount_map::iterator it = newsyncdebris.begin(); it != newsyncdebris.end(); it++) reqs[r].add(new CommandMoveSyncDebris(this,it->first,p));
					return;
				}
			}
			else h = UNDEF;

			// create missing component(s) of the sync debris folder of the day
			NewNode* nn;
			SymmCipher tkey;
			string tattrstring;
			AttrMap tattrs;
			int i = h == UNDEF ? 2 : 1;

			nn = new NewNode[i]+i;

			while (i--)
			{
				nn--;

				nn->source = NEW_NODE;
				nn->type = FOLDERNODE;
				nn->nodehandle = i;
				nn->parenthandle = i ? 0 : UNDEF;

				nn->clienttimestamp = t;
				nn->nodekey.resize(Node::FOLDERNODEKEYLENGTH);
				PrnGen::genblock((byte*)nn->nodekey.data(),Node::FOLDERNODEKEYLENGTH);

				// set new name, encrypt and attach attributes
				tattrs.map['n'] = (i || h != UNDEF) ? buf : SYNCDEBRISFOLDERNAME;
				tattrs.getjson(&tattrstring);
				tkey.setkey((const byte*)nn->nodekey.data(),FOLDERNODE);
				makeattr(&tkey,&nn->attrstring,tattrstring.c_str());
			}

			reqs[r].add(new CommandPutNodes(this,h == UNDEF ? rootnodes[RUBBISHNODE-ROOTNODE] : h,NULL,nn,h == UNDEF ? 2 : 1,0,PUTNODES_SYNCDEBRIS));
			syncdebrisadding = true;
		}
	}
}

void MegaClient::putnodes_syncdebris_result(error e, NewNode* nn)
{
	delete[] nn;

	syncdebrisadding = false;
	
	if (e == API_OK) movetosyncdebris(NULL);
}

bool FileFingerprintCmp::operator() (const FileFingerprint* a, const FileFingerprint* b) const
{
	if (a->size < b->size) return true;
	if (a->size > b->size) return false;
	if (a->mtime < b->mtime) return true;
	if (a->mtime > b->mtime) return false;
	return memcmp(a->crc,b->crc,sizeof a->crc) < 0;
}

void File::start()
{
}

void File::progress()
{
}

void File::completed(Transfer* t, LocalNode* l)
{
	if (t->type == PUT)
	{
		NewNode* newnode = new NewNode[1];

		// build new node
		newnode->source = NEW_UPLOAD;

		// upload handle required to retrieve/include pending file attributes
		newnode->uploadhandle = t->uploadhandle;

		// reference to uploaded file
		memcpy(newnode->uploadtoken,t->slot->ultoken,sizeof newnode->uploadtoken);

		// file's crypto key
		newnode->nodekey.assign((char*)t->filekey,Node::FILENODEKEYLENGTH);
		newnode->clienttimestamp = t->mtime;
		newnode->type = FILENODE;
		newnode->parenthandle = UNDEF;

		if ((newnode->localnode = l)) newnode->syncid = l->syncid;

		AttrMap attrs;

		// store filename
		attrs.map['n'] = name;

		// store fingerprint
		t->serializefingerprint(&attrs.map['c']);

		string tattrstring;

		attrs.getjson(&tattrstring);

		t->client->makeattr(&t->key,&newnode->attrstring,tattrstring.c_str());

		if (targetuser.size())
		{
			// drop file into targetuser's inbox
			t->client->putnodes(targetuser.c_str(),newnode,1);
		}
		else
		{
			handle th = h;

			// inaccessible target folder - use / instead
			if (!t->client->nodebyhandle(th)) th = t->client->rootnodes[0];

			if (l) t->client->syncadding++;
			t->client->reqs[t->client->r].add(new CommandPutNodes(t->client,th,NULL,newnode,1,0,l ? PUTNODES_SYNC : PUTNODES_APP));
		}
	}
}

void LocalNode::completed(Transfer* t, LocalNode*)
{
	if (parent)
	{
		if (!t->client->syncdeleted[type].count(syncid))
		{
			// if parent node exists, complete directly - otherwise, complete to rubbish bin for later retrieval
			syncidhandle_map::iterator it = t->client->syncidhandles.find(parent->syncid);

			if (it != t->client->syncidhandles.end()) h = it->second;	// existing parent: synchronous completioh
			else h = t->client->rootnodes[RUBBISHNODE-ROOTNODE];		// parent is still being created: complete into //bin (FIXME: complete into //bin/SyncDebris/yyyy-mm-dd instead)

			File::completed(t,this);
		}
	}
}

// do not retry crypto errors or administrative takedowns; retry other types of failuresup to 16 times
bool File::failed(error e)
{
	return e != API_EKEY && e != API_EBLOCKED && transfer->failcount < 16;
}

void File::displayname(string* dname)
{
	if (name.size()) *dname = name;
	else
	{
		Node* n;

		if ((n = transfer->client->nodebyhandle(h))) *dname = n->displayname();
		else *dname = "DELETED/UNAVAILABLE";
	}
}
