/*

MEGA SDK 2013-10-03 - Client Access Engine

(c) 2013 by Mega Limited, Wellsford, New Zealand

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
//#include <android/log.h>
//#define LOG(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "MEGA_JNI", __VA_ARGS__))

// root URL for API access
const char* const MegaClient::APIURL = "https://g.api.mega.co.nz/";

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

unsigned char* Base64::atob(const char* a, byte* b, int blen)
{
	byte c[4];
	int i;
	int done = 0;

	c[3] = 0;

	do {
		for (i = 0; i < 4; i++) if ((c[i] = from64(*a++)) == 255) break;

		if (!blen-- || !i) return b;
		*b++ = (c[0] << 2) | ((c[1] & 0x30) >> 4);
		if (!blen-- || i < 3) return b;
		*b++ = (c[1] << 4) | ((c[2] & 0x3c) >> 2);
		if (!blen-- || i < 4) return b;
		*b++ = (c[2] << 6) | c[3];
	} while (!done);

	return b;
}

char* Base64::btoa(const byte* b, int blen, char* a)
{
	for (;;)
	{
		if (blen <= 0) break;
		*a++ = to64(*b >> 2);
		*a++ = to64((*b << 4) | (((blen > 1) ? b[1] : 0) >> 4));
		if (blen < 2) break;
		*a++ = to64(b[1] << 2 | (((blen > 2) ? b[2] : 0) >> 6));
		if (blen < 3) break;
		*a++ = to64(b[2]);

		blen -= 3;
		b += 3;
	}

	*a = 0;

	return a;
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
int PaddedCBC::decrypt(string* data, SymmCipher* key)
{
	if ((data->size() & (key->BLOCKSIZE-1))) return 0;

	// decrypt and unpad
	key->cbc_decrypt((byte*)data->data(),data->size());

	size_t p = data->find_last_of('E');

	if (p == string::npos) return 0;

	data->resize(p);

	return 1;
}

// add or update record from string
int DbTable::put(uint32_t index, string* data)
{
	return put(index,(char*)data->data(),data->size());
}

// add or update record with padding and encryption
int DbTable::put(uint32_t type, Cachable* record, SymmCipher* key)
{
	string data;

	if (!record->serialize(&data)) return -1;

	PaddedCBC::encrypt(&data,key);

	if (!record->dbid) record->dbid = (nextid += IDSPACING) | type;

	return put(record->dbid,&data);
}

// get next record, decrypt and unpad
int DbTable::next(uint32_t* type, string* data, SymmCipher* key)
{
	if (next(type,data))
	{
		if (!*type) return 1;

		if (*type > nextid) nextid = *type & -IDSPACING;

		return PaddedCBC::decrypt(data,key);
	}

	return 0;
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

Node::Node(MegaClient* client, node_vector* dp, handle h, handle ph, nodetype t, m_off_t s, handle u, const char* fa, time_t tm, time_t ts)
{
	nodehandle = h;
	parenthandle = ph;

	parent = NULL;

	type = t;

	size = s;
	owner = u;

	copystring(&fileattrstring,fa);

	mtime = tm;
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
	}
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

	n = new Node(client,dp,h,ph,t,s,u,fa,tm,ts);

	if (k) n->setkey(k);

	if (numshares)
	{
		// read inshare or outshares
		while (Share::unserialize(client,(numshares > 0) ? -1 : 0,h,skey,&ptr,end) && numshares > 0 && --numshares);
	}

	ptr = n->attrs.unserialize(ptr,end-ptr);

	if (ptr == end)	return n;
	else return NULL;
}

// serialize node - nodes with pending or RSA keys are unsupported
int Node::serialize(string* d)
{
	// do not update state if undecrypted nodes are present
	if (attrstring.size()) return 0;

	switch (type)
	{
		case FILENODE:
			if ((int)nodekey.size() != FILENODEKEYLENGTH) return 0;
			break;
		case FOLDERNODE:
			if ((int)nodekey.size() != FOLDERNODEKEYLENGTH) return 0;
			break;
		default:
			if (nodekey.size()) return 0;
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
	d->append((char*)&ctime,sizeof(ctime));
	d->append((char*)&mtime,sizeof(mtime));
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

	return 1;
};

Node::~Node()
{
	// delete outshares, including pointers from users for this node
	for (share_map::iterator it = outshares.begin(); it != outshares.end(); it++) delete it->second;

	// remove from parent's children
	if (parent) parent->children.erase(child_it);

	// delete child-parent associations (normally not used, as nodes are deleted bottom-up)
	for (node_list::iterator it = children.begin(); it != children.end(); it++) (*it)->parent = NULL;

	if(inshare) delete inshare;
	if(sharekey) delete sharekey;
}

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

		l = Base64::atob(attrstring,buf,l)-buf;

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

		delete[] buf;

		attrstring.clear();
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
int MegaClient::decryptkey(const char* sk, byte* tk, int tl, SymmCipher* sc, int type, handle node)
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

		if (sl > 4096) return 0;

		byte* buf = new byte[sl];

		sl = Base64::atob(sk,buf,sl)-buf;

		// decrypt and set session ID for subsequent API communication
		if (!asymkey.decrypt(buf,sl,tk,tl))
		{
			delete[] buf;
			app->debug_log("Corrupt or invalid RSA node key detected");
			return 0;
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
		if (Base64::atob(sk,tk,tl)-tk != tl)
		{
			app->debug_log("Corrupt or invalid symmetric node key");
			return 0;
		}

		sc->ecb_decrypt(tk,tl);
	}

	return 1;
}

// attempt to apply node key - clears keystring if successful
int Node::applykey(MegaClient* client)
{
	if (!keystring.length()) return 0;

	int l = -1, t = 0;
	handle h;
	const char* k = NULL;
	SymmCipher* sc = &client->key;
	handle me = client->loggedin() ? client->me : *client->rootnodes;

	while ((t = keystring.find_first_of(':',t)) != (int)string::npos)
	{
		// compound key: locate suitable subkey (always symmetric)
		h = 0;

		l = Base64::atob(keystring.c_str()+(keystring.find_last_of('/',t)+1),(byte*)&h,sizeof h)-(byte*)&h;
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
		else return 0;
	}

	byte key[FILENODEKEYLENGTH];

	if (client->decryptkey(k,key,(type == FILENODE) ? FILENODEKEYLENGTH+0 : FOLDERNODEKEYLENGTH+0,sc,0,nodehandle))
	{
		keystring.clear();
		setkey(key);
	}

	return 1;
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

int Share::unserialize(MegaClient* client, int direction, handle h, const byte* key, const char** ptr, const char* end)
{
	if (*ptr+sizeof(handle)+sizeof(time_t)+2 > end) return 0;

	client->newshares.push_back(new NewShare(h,direction,*(handle*)*ptr,(accesslevel)(*ptr)[sizeof(handle)+sizeof(time_t)],*(time_t*)(*ptr+sizeof(handle)),key));

	*ptr += sizeof(handle)+sizeof(time_t)+2;

	return 1;
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
void MegaClient::mergenewshares(int notify)
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
				int auth = 1;

				if (s->outgoing > 0)
				{
					if (!checkaccess(n,OWNER))
					{
						app->debug_log("Attempt to create dislocated outbound share foiled");
						auth = 0;
					}
					else
					{
						byte buf[SymmCipher::KEYLENGTH];

						handleauth(s->h,buf);

						if (memcmp(buf,s->auth,sizeof buf))
						{
							app->debug_log("Attempt to create forged outbound share foiled");
							auth = 0;
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
						if (checkaccess(n,OWNER))
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
							if (!checkaccess(n,OWNER))
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

int User::serialize(string* d)
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

	return 1;
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
int JSON::storeobject(string* s)
{
	int openobject[2] = { 0 };
	const char* ptr = pos;

 	while(*ptr == ' ') { ptr++; pos++; }
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
		else if (*ptr != ':' && *ptr != ',') return 0;

		ptr++;

		if (!openobject[0] && !openobject[1])
		{
			if (s)
			{
				if (*pos == '"') s->assign(pos+1,ptr-pos-2);
				else s->assign(pos,ptr-pos);
			}
			pos = ptr;
			return 1;
		}
	}
}

// store string
// hex literals, booleans, unescaping unsupported
int JSON::storestring(string* s)
{
	const char* ptr = pos;
	const char* bptr;

	if (*ptr == ',') ptr++;

	if (*ptr++ != '"') return 0;

	bptr = ptr;

	while (*ptr && *ptr != '"') ptr++;

	if (*ptr == '"')
	{
		s->assign(bptr,ptr-bptr);
		pos = ptr+1;
		return 1;
	}

	return 0;
}

int JSON::isnumeric()
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
int JSON::is(const char* value)
{
	if (*pos == ',') pos++;

	if (*pos != '"') return 0;

	int t = strlen(value);

	if (memcmp(pos+1,value,t)) return 0;
	if (pos[t+1] != '"') return 0;

	pos += t+2;

	return 1;
}

// base64-decode binary value to designated fixed-length buffer
int JSON::storebinary(byte* dst, int dstlen)
{
	int l = 0;

	if (*pos == ',') pos++;

	if (*pos == '"')
	{
		l = Base64::atob(pos+1,dst,dstlen)-dst;

		// skip string
		storeobject();
	}

	return l;
}

// base64-decode binary value to designated string
int JSON::storebinary(string* dst)
{
	if (*pos == ',') pos++;

	if (*pos == '"')
	{
		const char* ptr;

		if (!(ptr = strchr(pos+1,'"'))) return 0;

		dst->resize((ptr-pos-1)/4*3+3);
		dst->resize(Base64::atob(pos+1,(byte*)dst->data(),dst->size())-(byte*)dst->data());

		// skip string
		storeobject();
	}

	return 1;
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
int JSON::enterarray()
{
	if (*pos == ',') pos++;

	if (*pos == '[')
	{
		pos++;
		return 1;
	}

	return 0;
}

// leave array (must be at end of array)
int JSON::leavearray()
{
	if (*pos == ']')
	{
		pos++;
		return 1;
	}

	return 0;
}

// try to enter object
int JSON::enterobject()
{
	if (*pos == '}') pos++;
	if (*pos == ',') pos++;

	if (*pos == '{')
	{
		pos++;
		return 1;
	}

	return 0;
}

// leave object (skip remainder)
int JSON::leaveobject()
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
		return 1;
	}

	return 0;
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

int BackoffTimer::armed(dstime ds) const
{
	return !next || ds >= next;
}

int BackoffTimer::arm(dstime ds)
{
	if (next+delta > ds)
	{
		next = ds;
		delta = 1;
		return 1;
	}
	else return 0;
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

int BackoffTimer::isset() const
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
	auth.resize(t+len*2);

	Base64::btoa(sid,len,(char*)(auth.c_str()+t));
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
int MegaClient::setscsn(JSON* j)
{
	handle t;

	if (j->storebinary((byte*)&t,sizeof t) != sizeof t) return 0;

	Base64::btoa((byte*)&t,sizeof t,scsn);

	return 1;
}

int MegaClient::nextreqtag()
{
	return ++reqtag;
}

// set warn level
void MegaClient::warn(const char* msg)
{
	app->debug_log(msg);
	warned = 1;
}

// reset and return warnlevel
int MegaClient::warnlevel()
{
	return warned ? (warned = 0) | 1 : 0;
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
}

MegaClient::MegaClient(MegaApp* a, HttpIO* h, DbAccess* d, const char* k)
{
	sctable = NULL;

	init();

	app = a;
	httpio = h;
	dbaccess = d;

	a->client = this;

	*scsn = 0;
	scsn[sizeof scsn-1] = 0;

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

// nonblocking state machine
void MegaClient::exec()
{
	httpio->updatedstime();

	do {
		// redispatch failed pending transfers that have elapsed their exponential backoff
		retrytransfers();

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
						else
						{
							pendingfa[pair<handle,fatype>(fa->th,fa->type)] = fah;
							restag = fa->tag;
							app->putfa_result(fa->th,fa->type,API_OK);
						}

						delete fa;
						newfa.erase(curfa);
					}

					btpfa.reset();
					curfa = newfa.end();
					break;

				case REQ_FAILURE:
					// repeat request with exponential backoff
					curfa = newfa.end();
					btpfa.backoff(httpio->ds);

				default:;
			}
		}

		if (newfa.size() && curfa == newfa.end() && btpfa.armed(httpio->ds))
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
					it->second->bt.backoff(httpio->ds);

				default:;
			}

			if (it->second->req.status != REQ_INFLIGHT && it->second->bt.armed(httpio->ds))
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

						btcs.backoff(httpio->ds);
						app->notify_retry(btcs.retryin(httpio->ds));

					default:;
				}

				if (pendingcs) break;
			}

			if (btcs.armed(httpio->ds))
			{
				if (btcs.isset()) r ^= 1;

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
								jsonsc.begin(pendingsc->in.c_str());
								procsc();

								delete pendingsc;
								pendingsc = NULL;

								btsc.reset();
								break;
							}
							else if (atoi(pendingsc->in.c_str()) == API_ESID) app->request_error(API_ESID);
							// fall through
						case REQ_FAILURE:	// failure, repeat with capped exponential backoff
							delete pendingsc;
							pendingsc = NULL;

							btsc.backoff(httpio->ds);

						default:;
					}
				}

				if (pendingsc) break;
			}

			if (*scsn && btsc.armed(httpio->ds))
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

		// file transfers
		for (int i = sizeof ft/sizeof *ft; i--; ) if (ft[i].inuse) ft[i].doio(this);
	} while (httpio->doio() || (!pendingcs && reqs[r].cmdspending() && btcs.armed(httpio->ds)));
}

// determine what to wait for and for how long, and invoke the app's blocking facility if needed
// optional parameter: maximum number of deciseconds to wait
void MegaClient::wait()
{
	dstime nds;

	httpio->updatedstime();

	// determine maximum delay for retries of previously failed transfers
	nds = transferretrydelay();

	if (nds+1) nds += httpio->ds;

	// retry failed file transfer events
	for (int i = sizeof ft/sizeof *ft; nds && i--; ) if (ft[i].inuse && ft[i].file) ft[i].bt.update(httpio->ds,&nds);

	// retry failed client-server requests
	if (!pendingcs) btcs.update(httpio->ds,&nds);

	// retry failed server-client requests
	if (!pendingsc && *scsn) btsc.update(httpio->ds,&nds);

	// retry failed file attribute puts
	if (curfa == newfa.end()) btpfa.update(httpio->ds,&nds);

	// retry failed file attribute gets
	for (fafc_map::iterator it = fafcs.begin(); it != fafcs.end(); it++) if (it->second->req.status != REQ_INFLIGHT) it->second->bt.update(httpio->ds,&nds);

	if (nds)
	{
		// nds is either MAX_INT (== no pending events) or > httpio->ds
		if (nds+1) nds -= httpio->ds;

		httpio->waitio(nds);
	}
}

// reset all backoff timers
int MegaClient::abortbackoff()
{
	int r = 0;
	httpio->updatedstime();

	for (int i = sizeof ft/sizeof *ft; i--; ) if (ft[i].inuse && ft[i].file) if (ft[i].bt.arm(httpio->ds)) r = 1;

	if (btcs.arm(httpio->ds)) r = 1;

	if (!pendingsc && btsc.arm(httpio->ds)) r = 1;

	if (curfa == newfa.end() && btpfa.arm(httpio->ds)) r = 1;

	for (fafc_map::iterator it = fafcs.begin(); it != fafcs.end(); it++) if (it->second->req.status != REQ_INFLIGHT && it->second->bt.arm(httpio->ds)) r = 1;

	return r;
}

// this will dispatch the next queued transfer unless one is already in progress and force isn't set
int MegaClient::dispatch(transfer_list* queue, int force)
{
	transfer_list::iterator nextit = queue->end();

	httpio->updatedstime();

	for (transfer_list::iterator it = queue->begin(); it != queue->end(); it++)
	{
		if ((*it)->td >= 0)
		{
			if (!force) return 0;
		}
		else
		{
			if ((*it)->bt.armed(httpio->ds) && (nextit == queue->end() || (*it)->bt.retryin(httpio->ds) < (*nextit)->bt.retryin(httpio->ds))) nextit = it;
		}
	}

	if (nextit != queue->end())
	{
		(*nextit)->start();
		return 1;
	}

	return 0;
}

FilePut::FilePut(const char* fn, handle th, const char* tu, const char* nn)
{
	filename = fn;
	target = th;
	targetuser = tu;
	newname = nn;
}

FileGet::FileGet(handle nh)
{
	nodehandle = nh;
}

QueuedTransfer::QueuedTransfer()
{
	td = -1;
	failcount = 0;
}

void QueuedTransfer::defer(MegaClient* client, int indefinitely)
{
	td = -1;

	if (indefinitely) bt.freeze();
	else bt.backoff(client->httpio->ds);
}

// get queue record based on open td and remove from queue - caller must delete after use
QueuedTransfer* MegaClient::gettransfer(transfer_list* queue, int td)
{
	for (transfer_list::iterator it = queue->begin(); it != queue->end(); it++) if (td == (*it)->td)
	{
		QueuedTransfer* t = *it;
		queue->erase(it);
		return t;
	}

	return NULL;
}

// defer failed transfer with exponential backoff
void MegaClient::defer(transfer_list* queue, int td, int indefinitely)
{
	for (transfer_list::iterator it = queue->begin(); it != queue->end(); it++) if (td == (*it)->td)
	{
		(*it)->defer(this,indefinitely);
		break;
	}
}

// clear transfer queue
void MegaClient::freequeue(transfer_list* queue)
{
	for (transfer_list::iterator it = queue->begin(); it != queue->end(); it++) delete *it;

	queue->clear();
}

// if we have no transfers running: none armed or unsuccessful dispatch: shorten max retry interval to queue minimum
int MegaClient::retrydeferred(transfer_list* queue, dstime ds)
{
	int havearmed = 0;

	for (transfer_list::iterator it = queue->begin(); it != queue->end(); it++)
	{
		if ((*it)->td >= 0) return 0;

		if (!havearmed) havearmed = (*it)->bt.armed(ds);
	}

	if (havearmed) return !dispatch(queue);

	return 1;
}

// retry failed/deferred transfers
void MegaClient::retrytransfers()
{
	if (getq.size()) retrydeferred((transfer_list*)&getq,httpio->ds);
	if (putq.size()) retrydeferred((transfer_list*)&putq,httpio->ds);
}

// return delay until next queued transfer dispatch
dstime MegaClient::maxdeferredtransferretrydelay(transfer_list* queue)
{
	dstime dsmin = ~(dstime)0;

	for (transfer_list::iterator it = queue->begin(); it != queue->end(); it++)
	{
		if ((*it)->td >= 0) return ~(dstime)0;
		if ((*it)->bt.backoff() < dsmin) dsmin = (*it)->bt.backoff();
	}

	return dsmin;
}

dstime MegaClient::transferretrydelay()
{
	dstime dsget, dsput;

	dsget = maxdeferredtransferretrydelay((transfer_list*)&getq);
	dsput = maxdeferredtransferretrydelay((transfer_list*)&putq);

	return min(dsget,dsput);
}

// disconnect everything
void MegaClient::disconnect()
{
	for (int i = sizeof ft/sizeof *ft; i--; ) ft[i].disconnect();

	if (pendingcs) pendingcs->disconnect();
	if (pendingsc) pendingsc->disconnect();

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

	for (i = sizeof ft/sizeof *ft; i--; ) ft[i].close();

	purgenodesusersabortsc();

	freequeue((transfer_list*)&putq);
	freequeue((transfer_list*)&getq);

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
void MegaClient::initsc()
{
	if (sctable)
	{
		sctable->begin();

		sctable->truncate();

		int complete;

		// 1. write current scsn
		handle tscsn;
		Base64::atob(scsn,(byte*)&tscsn,sizeof tscsn);
		complete = sctable->put(CACHEDSCSN,(char*)&tscsn,sizeof tscsn);

		if (complete)
		{
			// 2. write all users
			for (user_map::iterator it = users.begin(); it != users.end(); it++) if ((complete = sctable->put(CACHEDUSER,&it->second,&key)) <= 0) break;
		}

		if (complete > 0)
		{
			// 3. write new or modified nodes, purge deleted nodes
			for (node_map::iterator it = nodes.begin(); it != nodes.end(); it++) if ((complete = sctable->put(CACHEDNODE,it->second,&key)) <= 0) break;
		}

		finalizesc(complete);
	}
}

// erase and and fill user's local state cache
void MegaClient::updatesc()
{
	if (sctable)
	{
		sctable->begin();

		int complete;

		// 1. update associated scsn
		handle tscsn;
		Base64::atob(scsn,(byte*)&tscsn,sizeof tscsn);
		complete = sctable->put(CACHEDSCSN,(char*)&tscsn,sizeof tscsn);

		if (complete)
		{
			// 2. write new or update modified users
			for (user_vector::iterator it = usernotify.begin(); it != usernotify.end(); it++) if ((complete = sctable->put(CACHEDUSER,*it,&key)) <= 0) break;
		}

		if (complete > 0)
		{
			// 3. write new or modified nodes, purge deleted nodes
			for (node_vector::iterator it = nodenotify.begin(); it != nodenotify.end(); it++)
			{
				if ((*it)->removed && (*it)->dbid)
				{
					if (!(complete = sctable->del((*it)->dbid))) break;
				}
				else if ((complete = sctable->put(CACHEDNODE,*it,&key)) <= 0) break;
			}
		}

		finalizesc(complete);
	}
}

void MegaClient::finalizesc(int complete)
{
	if (complete > 0) sctable->commit();
	else
	{
		sctable->abort();

		if (!complete)
		{
			app->debug_log("Cache update DB write error - closing");

			delete sctable;
			sctable = NULL;
		}
		else app->debug_log("Cache update deferred");
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

	if (Base64::atob(strchr(n->fileattrstring.c_str()+p,'*')+1,(byte*)&fah,sizeof(fah))-(byte*)&fah != sizeof(fah)) return API_ENOENT;

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
		Base64::btoa((byte*)&it->second,sizeof(it->second),strchr(buf+3,0));
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

// generate upload handle for this upload
// (after 65536 uploads, a node handle clash is possible, but far too unlikely to be of concern)
handle MegaClient::uploadhandle(int td)
{
	if (!ft[td].uploadhandle)
	{
		byte* ptr = (byte*)(&nextuh+1);

		while (!++(*--ptr));

		ft[td].uploadhandle = nextuh;
	}

	return ft[td].uploadhandle;
}

// attach file attribute to a file
// to avoid unnecessary roundtrips to the attribute servers, also store locally
void MegaClient::putfa(SymmCipher* filekey, handle th, fatype t, const byte* data, unsigned len)
{
	// build encrypted file attribute data block
	byte* cdata;
	unsigned clen = (len+SymmCipher::BLOCKSIZE-1)&-SymmCipher::BLOCKSIZE;

	cdata = new byte[clen];

	memcpy(cdata,data,len);
	memset(cdata+len,0,clen-len);

	filekey->cbc_encrypt(cdata,clen);

	newfa.push_back(new HttpReqCommandPutFA(this,th,t,cdata,clen));

	// no other file attribute storage request currently in progress? POST this one.
	if (curfa == newfa.end())
	{
		curfa = newfa.begin();
		reqs[r].add(*curfa);
	}
}

// start upload
int MegaClient::topen(const char* localpath, int ms, int c)
{
	int td;

	if ((td = alloctd(1)) < 0) return td;

	// generate random encryption key/CTR IV for this file
	byte keyctriv[SymmCipher::KEYLENGTH+sizeof(int64_t)];
	PrnGen::genblock(keyctriv,sizeof keyctriv);

	ft[td].key.setkey(keyctriv);
	ft[td].ctriv = *(uint64_t*)(keyctriv+SymmCipher::KEYLENGTH);

	FileAccess* file = app->newfile();

	if (file->fopen(localpath,1,0))
	{
		ft[td].init(file->size,NULL,c);
		ft[td].upload = 1;
		reqs[r].add(ft[td].pendingcmd = new CommandPutFile(td,file,ms,c));
	}
	else
	{
		delete file;
		tclose(td);

		return API_ENOENT;
	}

	return td;
}

// request upload target URL
CommandPutFile::CommandPutFile(int t, FileAccess* f, int ms, int c)
{
	cmd("u");
	arg("s",f->size);
	arg("ms",ms);

	td = t;
	file = f;
	connections = c;
}

// set up file transfer with returned target URL
void CommandPutFile::procresult()
{
	if (client->json.isnumeric())
	{
		client->defer((transfer_list*)&client->putq,td);
		return client->app->topen_result(td,(error)client->json.getint());
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
				// if the app has already tclose()d this transfer, ignore
				if (this != client->ft[td].pendingcmd) return;

				if (p)
				{
					Node::copystring(&client->ft[td].tempurl,p);
					client->ft[td].file = file;
					client->ft[td].starttime = client->ft[td].lastdata = client->httpio->ds;
				}
				else
				{
					client->defer((transfer_list*)&client->putq,td);
					client->app->topen_result(td,API_EINTERNAL);
				}
				return;

			default:
				if (!client->json.storeobject())
				{
					client->defer((transfer_list*)&client->putq,td);
					return client->app->topen_result(td,API_EINTERNAL);
				}
		}
	}
}

// allocate file transfer descriptor
int MegaClient::alloctd(int upload)
{
	for (int i = 0; i < (int)sizeof ft/(int)sizeof *ft; i++)
	{
		if (!ft[i].inuse)
		{
			ft[i].inuse = 1;
			ft[i].upload = upload;
			ft[i].starttime = 0;
			ft[i].progressreported = 0;
			ft[i].progresscompleted = 0;
			ft[i].tag = reqtag;

			return i;
		}
	}

	return API_ETOOMANY;
}

// start download (k == NULL => private node)
int MegaClient::topen(handle h, const byte* k, m_off_t start, m_off_t len, int c)
{
	Node* n;
	int td;
	int priv;

	if ((priv = !k))
	{
		if (!(n = nodebyhandle(h))) return API_ENOENT;
		if (n->type != FILENODE) return API_EACCESS;
		k = (const byte*)n->nodekey.data();
	}

	if ((td = alloctd(0)) < 0) return td;

	ft[td].key.setkey(k,FILENODE);
	ft[td].ctriv = *(int64_t*)(k+SymmCipher::KEYLENGTH);
	ft[td].metamac = *(int64_t*)(k+SymmCipher::KEYLENGTH+sizeof(int64_t));

	ft[td].startpos = start;
	ft[td].endpos = (len >= 0) ? start+len : -1;
	ft[td].startblock = ChunkedHash::chunkfloor(start);

	// the size field must be valid right away for MegaClient::moretransfers()
	if (priv) ft[td].size = n->size;

	reqs[r].add(ft[td].pendingcmd = new CommandGetFile(td,h,priv,c));

	return td;
}

// close allocated transfer
void MegaClient::tclose(int td)
{
	ft[td].close();
}

// returns 1 if more transfers of the requested type can be dispatched (back-to-back overlap pipelining)
// FIXME: support overlapped partial reads
int MegaClient::moretransfers(int upload)
{
	m_off_t c = 0, r = 0;
	dstime t = 0;
	int f = 0;

	// determine average speed and total amount of data remaining
	for (int td = sizeof ft/sizeof *ft; td--; )
	{
		if (ft[td].inuse)
		{
			if (!ft[td].upload == !upload)
			{
				if (ft[td].starttime) t += httpio->ds-ft[td].starttime;
				c += ft[td].progressreported;
				r += ft[td].size-ft[td].progressreported;
			}
		}
		else f++;
	}

	// less than two channels available - do not dispatch any new transfers
	if (f < 2) return 0;

	// always blindly dispatch transfers up to MINPIPELINE
	if (r < MINPIPELINE) return 1;

	// dispatch more if less than two seconds of transfers left (at least 5 seconds must have elapsed for precise speed indication)
	if (t > 50)
	{
		int bpds = (int)(c/t);
		if (bpds > 100 && r/bpds < 20) return 1;
	}

	return 0;
}

void MegaClient::dispatchmore(int upload)
{
	// keep pipeline full by dispatching additional queued transfers, if appropriate and available
	while (moretransfers(upload) && dispatch(upload ? (transfer_list*)&putq : (transfer_list*)&getq,1));
}

// open local file for writing
void MegaClient::dlopen(int td, const char* tmpfilename)
{
	ft[td].file = app->newfile();

	if (!ft[td].file->fopen(tmpfilename,0,1))
	{
		app->transfer_failed(td,ft[td].filename,API_EWRITE);
		defer((transfer_list*)&putq,td);
		tclose(td);
	}
	else if (!ft[td].size) return app->transfer_complete(td,NULL,ft[td].filename.c_str());
}

// request temporary source URL
CommandGetFile::CommandGetFile(int t, handle h, int p, int c)
{
	cmd("g");
	arg(p ? "n" : "p",(byte*)&h,MegaClient::NODEHANDLE);
	arg("g",1);

	td = t;
	connections = c;
}

// decrypt returned attributes, open local file for writing and start transfer
void CommandGetFile::procresult()
{
	const char* g = NULL;
	const char* at = NULL;
	const char* fa = NULL;
	error e = API_EINTERNAL;
	m_off_t s = -1;
	int d = 0;
	int pfa = 0;
	byte* buf;

	if (client->json.isnumeric())
	{
		client->defer((transfer_list*)&client->getq,td);
		return client->app->topen_result(td,(error)client->json.getint());
	}

	for (;;)
	{
		switch (client->json.getnameid())
		{
			case 'g':
				g = client->json.getvalue();
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
				fa = client->json.getvalue();
				break;

			case MAKENAMEID3('p','f','a'):
				pfa = (int)client->json.getint();
				break;

			case 'e':
				e = (error)client->json.getint();
				break;

			case EOO:
				// if the app has already tclose()d this transfer, ignore
				if (this != client->ft[td].pendingcmd) return;

				if (d)
				{
					client->defer((transfer_list*)&client->getq,td,1);
					return client->app->topen_result(td,API_EBLOCKED);
				}
				else
				{
					if (g && s >= 0)
					{
						// decrypt at and set filename
						const char* eos = strchr(at,'"');
						string tmpfilename;

						if ((buf = Node::decryptattr(&client->ft[td].key,at,eos ? eos-at : strlen(at))))
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
											client->defer((transfer_list*)&client->getq,td);
											return client->app->topen_result(td,API_EINTERNAL);
										}
										break;

									case EOO:
										delete[] buf;

										client->ft[td].init(s,tmpfilename.c_str(),connections);
										Node::copystring(&client->ft[td].tempurl,g);
										client->ft[td].pos = ChunkedHash::chunkfloor(client->ft[td].startpos);

										client->ft[td].starttime = client->ft[td].lastdata = client->httpio->ds;

										return client->app->topen_result(td,&client->ft[td].filename,fa,pfa);

									default:
										if (!json.storeobject())
										{
											client->defer((transfer_list*)&client->getq,td);
											client->app->topen_result(td,API_EINTERNAL);
											return;
										}
								}
							}
						}
					}
				}

				client->defer((transfer_list*)&client->getq,td);
				return client->app->topen_result(td,e);

			default:
				if (!client->json.storeobject())
				{
					client->defer((transfer_list*)&client->getq,td);
					return client->app->topen_result(td,API_EINTERNAL);
				}
		}
	}
}

// server-client node update processing
void MegaClient::sc_updatenode()
{
	handle h = UNDEF;
	handle u = 0;
	const char* a = NULL;
	const char* k = NULL;
	time_t ts = 0;

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

			case MAKENAMEID2('a','t'):
				a = jsonsc.getvalue();
				break;

			case 'k':
				k = jsonsc.getvalue();
				break;

			case MAKENAMEID2('c','r'):
				proccr(&jsonsc);
				break;

			case MAKENAMEID2('t','s'):
				ts = jsonsc.getint();
				break;

			case EOO:
				if (!ISUNDEF(h))
				{
					Node* n;

					if ((n = nodebyhandle(h)))
					{
						if (u) n->owner = u;
						if (a) Node::copystring(&n->attrstring,a);
						if (k) Node::copystring(&n->keystring,k);
						if (ts+1) n->mtime = ts;

						n->applykey(this);

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
int MegaClient::sc_shares()
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
				have_ha = Base64::atob(jsonsc.getvalue(),ha,sizeof ha)-ha == sizeof ha;
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
				if (!loggedin() < FULLACCOUNT) return 0;

				// need a share node
				if (ISUNDEF(h)) return 0;

				// ignore unrelated share packets (should never be triggered)
				if (!ISUNDEF(oh) && !(outbound = oh == me) && uh != me) return 0;

				// am I the owner of the share? use ok, otherwise k.
				if (ok && oh == me) k = ok;

				if (k)
				{
					if (!decryptkey(k,sharekey,sizeof sharekey,&key,1,h)) return 0;

					if (ISUNDEF(oh) && ISUNDEF(uh))
					{
						// share key update on inbound share
						newshares.push_back(new NewShare(h,0,UNDEF,ACCESS_UNKNOWN,0,sharekey));
						return 1;
					}

					if (!ISUNDEF(oh) && !ISUNDEF(uh))
					{
						// new share - can be inbound or outbound
						newshares.push_back(new NewShare(h,outbound,outbound ? uh : oh,r,ts,sharekey,have_ha ? ha : NULL));
						return 0;
					}
				}
				else
				{
					if (!ISUNDEF(oh) && !ISUNDEF(uh))
					{
						// share revocation
						newshares.push_back(new NewShare(h,outbound,outbound ? uh : oh,ACCESS_UNKNOWN,0,NULL));
						return 1;
					}
				}
				return 0;

			default:
				if (!jsonsc.storeobject()) return 0;
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
							while (jsonsc.storestring(&ua))
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

// purge removed nodes after notification
void MegaClient::notifypurge(void)
{
	int i, t;

	updatesc();

	if ((t = nodenotify.size()))
	{
		applykeys();

		app->nodes_updated(&nodenotify[0],t);

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

// make attribute string, including magic prefix
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
CommandPutNodes::CommandPutNodes(MegaClient* client, handle th, const char* userhandle, NewNode* newnodes, int numnodes, int ctag)
{
	byte key[Node::FILENODEKEYLENGTH];
	int i;

	nn = newnodes;
	type = userhandle ? USER_HANDLE : NODE_HANDLE;

	cmd("p");
	notself(client);

	if (userhandle) arg("t",userhandle);
	else arg("t",(byte*)&th,MegaClient::NODEHANDLE);

	beginarray("n");

	ulhandles = new handle[numnodes];

	for (i = 0; i < numnodes; i++)
	{
		ulhandles[i] = nn[i].uploadhandle;

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
		arg("a",(byte*)nn[i].attrstring.c_str(),nn[i].attrstring.size());

		if (nn[i].nodekey.size() <= sizeof key)
		{
			client->key.ecb_encrypt((byte*)nn[i].nodekey.data(),key,nn[i].nodekey.size());
			arg("k",key,nn[i].nodekey.size());
		}
		else arg("k",(const byte*)nn[i].nodekey.data(),nn[i].nodekey.size());

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

CommandPutNodes::~CommandPutNodes()
{
	delete[] ulhandles;
}

// add new nodes and handle->node handle mapping
void CommandPutNodes::procresult()
{
	if (client->json.isnumeric()) return client->app->putnodes_result((error)client->json.getint(),type,nn);

	error e = API_EINTERNAL;

	for (;;)
	{
		switch (client->json.getnameid())
		{
			case 'f':
				if (client->readnodes(&client->json,1,ulhandles)) e = API_OK;
				break;

			default:
				if (client->json.storeobject()) continue;
				e = API_EINTERNAL;
				// fall through
			case EOO:
				client->applykeys();
                client->app->putnodes_result(e,type,nn);
				client->notifypurge();
				return;
		}
	}
}

// drop nodes into a user's inbox (has to have RSA keypair)
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
	//LOG("Check access %s START\n", n->displayname());
	// folder link access is always read-only
	if (!loggedin())
	{
		//LOG("Check access %s NOT LOGGED IN\n", n->displayname());
		return a == RDONLY;
	}

	// trace back to root node (always full access) or share node
	while (n)
	{
		//LOG("Check access Parent %s\n", n->displayname());
		if (n->inshare)
		{
			//LOG("Check access %s INSHARE\n", n->displayname());
			return n->inshare->access >= a;
		}
		if (!n->parent)
		{
			//LOG("Check access %s NO PARENT\n", n->displayname());
			return n->type > FOLDERNODE;
		}

		n = n->parent;
	}

	//LOG("Check access %s END\n", n->displayname());
	return 0;
}

// returns API_OK if a move operation is permitted, API_EACCESS or API_ECIRCULAR otherwise
error MegaClient::checkmove(Node* fn, Node* tn)
{
	//LOG("Check move %s START\n", fn->displayname());

	// condition 1: cannot move top-level node, must have full access to fn's parent
	if (!fn->parent || !checkaccess(fn->parent,FULL))
	{
		//LOG("Check move %s CONDITION_1\n", fn->displayname());
		return API_EACCESS;
	}

	// condition 2: target must be folder
	if (tn->type == FILENODE)
	{
		//LOG("Check move %s CONDITION_2\n", fn->displayname());
		return API_EACCESS;
	}

	// condition 3: must have write access to target
	if (!checkaccess(tn,RDWR))
	{
		//LOG("Check move %s CONDITION_3\n", fn->displayname());
		return API_EACCESS;
	}

	// condition 4: tn must not be below fn (would create circular linkage)
	for (;;)
	{
		if (tn == fn)
		{
			//LOG("Check move %s CONDITION_4\n", fn->displayname());
			return API_ECIRCULAR;
		}
		if (tn->inshare || !tn->parent) break;
		tn = tn->parent;
	}

	// condition 5: fn and tn must be in the same tree (same ultimate parent node or shared by the same user)
	for (;;)
	{
		if (fn->inshare || !fn->parent)
		{
			//LOG("Check move %s CONDITION_5\n", fn->displayname());
			break;
		}
		fn = fn->parent;
	}

	// moves within the same tree or between the user's own trees are permitted
	if (fn == tn || (!fn->inshare && !tn->inshare))
	{
		//LOG("Check move %s CONDITION_6\n", fn->displayname());
		return API_OK;
	}

	// moves between inbound shares from the same user are permitted
	if (fn->inshare && tn->inshare && fn->inshare->user == tn->inshare->user)
	{
		//LOG("Check move %s CONDITION_7\n", fn->displayname());
		return API_OK;
	}

	//LOG("Check move %s END\n", fn->displayname());
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
int Node::setparent(Node* p)
{
	//LOG("SETPARENT START %s\n", this->displayname());
	//if(!p)
	//	LOG("SETPARENT NULL\n");

	if (p == parent) return 0;
	//LOG("SETPARENT DISTINTO %s\n", this->displayname());

	if (parent) parent->children.erase(child_it);

	//LOG("SETPARENT ASIGNO %s\n", p->displayname());

	parent = p;
	parenthandle = parent->nodehandle;

	//LOG("SETPARENT ASIGNO AL PADRE %s\n", p->displayname());

	child_it = parent->children.insert(parent->children.end(),this);
	//LOG("SETPARENT FIN %s\n", this->displayname());

	return 1;
}

CommandMoveNode::CommandMoveNode(MegaClient* client, Node* n, Node* t)
{
	cmd("m");
	notself(client);

	arg("n",(byte*)&n->nodehandle,MegaClient::NODEHANDLE);
	arg("t",(byte*)&t->nodehandle,MegaClient::NODEHANDLE);

	TreeProcShareKeys tpsk;
	client->proctree(n,&tpsk);
	tpsk.get(this);

	h = n->nodehandle;
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

// remove node
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
int MegaClient::readnodes(JSON* j, int notify, handle* ulhandles)
{
	if (!j->enterarray()) return 0;

	node_vector dp;
	Node* n;

	while (j->enterobject())
	{
		handle h = UNDEF, ph = UNDEF;
		handle u = 0, su = UNDEF;
		nodetype t = TYPE_UNKNOWN;
		const char* a = NULL;
		const char* k = NULL;
		const char* fa = NULL;
		const char *sk = NULL;
		accesslevel r = ACCESS_UNKNOWN;
		m_off_t s = ~(m_off_t)0;
		time_t tm = 0, ts = 0;
		nameid name;

		while ((name = j->getnameid()) != EOO)
		{
			switch (name)
			{
				case 'h':	// new node: handle
					h = j->gethandle();

					if (ulhandles)
					{
						// map upload handle to node handle for pending file attributes
						if (*ulhandles) uhnh.insert(pair<handle,handle>(*ulhandles,h));
						ulhandles++;
					}
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

				case MAKENAMEID2('t','m'):	// last modified
					tm = j->getint();
					break;

				case MAKENAMEID2('t','s'):	// created
					ts = j->getint();
					break;

				case MAKENAMEID2('f','a'):	// file attributes
					fa = j->getvalue();
					break;

					// inbound share attributes
				case 'r':	// share access level
					r = (accesslevel)j->getint();
					break;

				case MAKENAMEID2('s','k'):	// share key
					sk = j->getvalue();
					break;

				case MAKENAMEID2('s','u'):	// sharing user
					su = j->gethandle(USERHANDLE);
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
					if (r == ACCESS_UNKNOWN) warn("Missing access level");
					if (!sk) warn("Missing share key for inbound share");

					if (warnlevel()) su = UNDEF;
					else decryptkey(sk,buf,sizeof buf,&key,1,h);
				}

				string fas;

				Node::copystring(&fas,fa);

				n = new Node(this,&dp,h,ph,t,s,u,fas.c_str(),tm,ts);

				Node::copystring(&n->attrstring,a);
				Node::copystring(&n->keystring,k);

				if (!ISUNDEF(su)) newshares.push_back(new NewShare(h,0,su,r,ts,buf));
			}

			if (notify) notifynode(n);
		}
	}

	// any child nodes arrived before their parents?
	for (int i = dp.size(); i--; ) if ((n = nodebyhandle(dp[i]->parenthandle))) dp[i]->setparent(n);

	return j->leavearray();
}

// decrypt and set encrypted sharekey
void MegaClient::setkey(SymmCipher* c, const char* k)
{
	byte newkey[SymmCipher::KEYLENGTH];

	if (Base64::atob(k,newkey,sizeof newkey)-newkey == sizeof newkey)
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
				have_ha = Base64::atob(j->getvalue(),ha,sizeof ha)-ha == sizeof ha;
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
	for (node_map::iterator it = nodes.begin(); it != nodes.end(); it++) if (it->second->applykey(this)) t++;

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
int MegaClient::readusers(JSON* j)
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
					if (!j->storeobject()) return 0;
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
	sprintf(buf,"%ld",(long)n);
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

	len = Base64::btoa(data,len,buf)-buf;

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

	if (Base64::atob(f,(byte*)&h,NODEHANDLE)-(byte*)&h != NODEHANDLE) return API_EARGS;
	if (Base64::atob(k,folderkey,sizeof folderkey)-folderkey != sizeof folderkey) return API_EARGS;

	setrootnode(h);
	key.setkey(folderkey);

	return API_OK;
}

void MegaClient::login(const char* email, const byte* pwkey, int nocache)
{
	logout();

	string t;
	string lcemail(email);

	key.setkey((byte*)pwkey);
	uint64_t emailhash = stringhash64(&lcemail,&key);

	if (!nocache && dbaccess && (sctable = dbaccess->open(&lcemail)) && sctable->get(CACHEDSCSN,&t))
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

		if (Base64::atob(uid,(byte*)&uh,sizeof uh)-(byte*)&uh == sizeof uh) return finduser(uh,add);
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

			ptr = Base64::btoa(key,n->nodekey.size(),strchr(buf,0));
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
int CommandSetShare::procuserresult(MegaClient* client)
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
					return 1;

				default:
					if (!client->json.storeobject()) return 0;
			}
		}
	}

	return 0;
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

// add new contact (by e-mail address)
error MegaClient::invite(const char* email, visibility show)
{
	if (!strchr(email,'@')) return API_EARGS;

	reqs[r].add(new CommandUserRequest(email,show));

	return API_OK;
}

CommandUserRequest::CommandUserRequest(const char* m, visibility show)
{
	cmd("ur");
	arg("u",m);
	arg("l",(int)show);
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

	reqs[r].add(new CommandPutUA(name.c_str(),priv ? (const byte*)data.data() : av, priv ? data.size() : avl));
}

CommandPutUA::CommandPutUA(const char *an, const byte* av, unsigned avl)
{
	cmd("up");
	arg(an,av,avl);
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

		reqs[r].add(new CommandGetUA(u->uid.c_str(),name.c_str(),p));
	}
}

CommandGetUA::CommandGetUA(const char* uid, const char* an, int p)
{
	priv = p;

	cmd("uga");
	arg("u",uid);
	arg("ua",an);
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

		l = Base64::atob(ptr,data,l)-data;

		if (priv)
		{
			d.assign((char*)data,l);
			delete data;

			if (!PaddedCBC::decrypt(&d,&client->key)) return client->app->getua_result(API_EINTERNAL);

			return client->app->getua_result((byte*)d.data(),d.size());
		}

		client->app->getua_result(data,l);

		delete data;
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
int Node::isbelow(Node* p) const
{
	const Node* n = this;

	for (;;)
	{
		if (!n) return 0;
		if (n == p) return 1;
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

void MegaClient::getaccountdetails(AccountDetails* ad, int storage, int transfer, int pro, int transactions, int purchases, int sessions)
{
	reqs[r].add(new CommandGetUserQuota(this,ad,storage,transfer,pro));
	if (transactions) reqs[r].add(new CommandGetUserTransactions(this,ad));
	if (purchases) reqs[r].add(new CommandGetUserPurchases(this,ad));
	if (sessions) reqs[r].add(new CommandGetUserSessions(this,ad));
}

CommandGetUserQuota::CommandGetUserQuota(MegaClient* client, AccountDetails* ad, int storage, int transfer, int pro)
{
	tag = client->reqtag;

	details = ad;

	cmd("uq");
	if (storage) arg("strg","1",0);
	if (transfer) arg("xfer","1",0);
	if (pro) arg("pro","1",0);
}

void CommandGetUserQuota::procresult()
{
	short td;
	int got_storage = 0;
	int got_transfer = 0;
	int got_pro = 0;

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
				got_transfer = 1;
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
				got_storage = 1;
				break;

			case MAKENAMEID6('c','a','x','f','e','r'):	// own transfer quota used
				details->transfer_own_used += client->json.getint();
				break;

			case MAKENAMEID6('c','s','x','f','e','r'):		// third-party transfer quota used
				details->transfer_srv_used += client->json.getint();
				break;

			case MAKENAMEID5('m','x','f','e','r'):		// total transfer quota
				details->transfer_max = client->json.getint();
				got_transfer = 1;
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
				client->app->account_details(details,got_storage,got_transfer,got_pro,0,0,0);
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

	client->app->account_details(details,0,0,0,0,1,0);
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

	client->app->account_details(details,0,0,0,1,0,0);
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
		client->json.storestring(&details->sessions[t].useragent);

		client->json.storestring(&details->sessions[t].ip);

		const char* country = client->json.getvalue();
		memcpy(details->sessions[t].country,country ? country : "\0\0",2);
		details->sessions[t].country[2] = 0;

		details->sessions[t].current = (int)client->json.getint();

		client->json.leavearray();
	}

	client->app->account_details(details,0,0,0,0,0,1);
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

	if (Base64::atob(ptr,(byte*)&ph,NODEHANDLE)-(byte*)&ph == NODEHANDLE)
	{
		ptr += 8;

		if (*ptr++ == '!')
		{
			if (Base64::atob(ptr,key,sizeof key)-key == sizeof key)
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
	time_t tm = 0;
	time_t ts = 0;
	const char* a = NULL;
	const char* fa = NULL;

	for (;;)
	{
		switch (client->json.getnameid())
		{
			case 's':
				s = client->json.getint();
				break;

			case MAKENAMEID2('a','t'):
				a = client->json.getvalue();
				break;

			case MAKENAMEID2('f','a'):
				fa = client->json.getvalue();
				break;

			case MAKENAMEID2('t','m'):
				tm = client->json.getint();
				break;

			case MAKENAMEID2('t','s'):
				ts = client->json.getint();
				break;

			case EOO:
				// we want at least the attributes
				if (a && s >= 0)
				{
					Node* n = new Node(NULL,NULL,ph,UNDEF,FILENODE,s,0,fa,tm,ts);

					Node::copystring(&n->attrstring,a);

					n->setkey(key);

					return client->app->openfilelink_result(n);
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

	Command *c = new CommandCreateEphemeralSession(keybuf,pwbuf,sscbuf);
	c->tag = reqtag;
	reqs[r].add(c);

}

CommandCreateEphemeralSession::CommandCreateEphemeralSession(const byte* key, const byte* cpw, const byte* ssc)
{
	memcpy(pw,cpw,sizeof pw);

	cmd("up");
	arg("k",key,SymmCipher::KEYLENGTH);
	arg("ts",ssc,2*SymmCipher::KEYLENGTH);
}

void CommandCreateEphemeralSession::procresult()
{
	if (client->json.isnumeric()) client->app->ephemeral_result((error)client->json.getint());
	else client->resumeephemeral(client->json.gethandle(MegaClient::USERHANDLE),pw);
}

void MegaClient::resumeephemeral(handle uh, const byte* pw)
{
	Command *c = new CommandResumeEphemeralSession(uh,pw);
	c->tag = reqtag;
	reqs[r].add(c);
}

CommandResumeEphemeralSession::CommandResumeEphemeralSession(handle cuh, const byte* cpw)
{
	memcpy(pw,cpw,sizeof pw);
	uh = cuh;

	cmd("us");
	arg("user",(byte*)&uh,MegaClient::USERHANDLE);
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

	Command *co = new CommandSendSignupLink(email,name,c);
	co->tag = reqtag;
	reqs[r].add(co);
}

CommandSendSignupLink::CommandSendSignupLink(const char* email, const char* name, byte* c)
{
	cmd("uc");
	arg("c",c,2*SymmCipher::KEYLENGTH);
	arg("n",(byte*)name,strlen(name));
	arg("m",(byte*)email,strlen(email));
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
	Command *c = new CommandQuerySignupLink(code,len);
	c->tag = reqtag;
	reqs[r].add(c);
}

CommandQuerySignupLink::CommandQuerySignupLink(const byte* code, unsigned len)
{
	confirmcode.assign((char*)code,len);

	cmd("ud");
	arg("c",code,len);
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
		if (!ISUNDEF(uh) && Base64::atob(pwcheck,pwcheckbuf,sizeof pwcheckbuf)-pwcheckbuf == sizeof pwcheckbuf && Base64::atob(kc,kcbuf,sizeof kcbuf)-kcbuf == sizeof kcbuf)
		{
			client->json.leavearray();

			return client->app->querysignuplink_result(uh,name.c_str(),email.c_str(),pwcheckbuf,kcbuf,(const byte*)confirmcode.data(),confirmcode.size());
		}
	}

	client->app->querysignuplink_result(API_EINTERNAL);
}

void MegaClient::confirmsignuplink(const byte* code, unsigned len, uint64_t emailhash)
{
	Command *c = new CommandConfirmSignupLink(code,len,emailhash);
	c->tag = reqtag;
	reqs[r].add(c);
}

CommandConfirmSignupLink::CommandConfirmSignupLink(const byte* code, unsigned len, uint64_t emailhash)
{
	cmd("up");
	arg("c",code,len);
	arg("uh",(byte*)&emailhash,sizeof emailhash);
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
	Integer pubk[AsymmCipher::PUBKEY];

	string privks, pubks;

	asymkey.genkeypair(asymkey.key,pubk,2048);

	AsymmCipher::serializeintarray(pubk,AsymmCipher::PUBKEY,&pubks);
	AsymmCipher::serializeintarray(asymkey.key,AsymmCipher::PRIVKEY,&privks);

	// add random padding and ECB-encrypt with master key
	unsigned t = privks.size();

	privks.resize((t+SymmCipher::BLOCKSIZE-1)&-SymmCipher::BLOCKSIZE);
	PrnGen::genblock((byte*)(privks.data()+t),privks.size()-t);

	key.ecb_encrypt((byte*)privks.data(),(byte*)privks.data(),(unsigned)privks.size());

	reqs[r].add(new CommandSetKeyPair((const byte*)privks.data(),privks.size(),(const byte*)pubks.data(),pubks.size()));
}

CommandSetKeyPair::CommandSetKeyPair(const byte* privk, unsigned privklen, const byte* pubk, unsigned pubklen)
{
	cmd("up");
	arg("privk",privk,privklen);
	arg("pubk",pubk,pubklen);
}

void CommandSetKeyPair::procresult()
{
	if (client->json.isnumeric()) return client->app->setkeypair_result((error)client->json.getint());

	client->json.storeobject();

	client->app->setkeypair_result(API_OK);
}

int MegaClient::fetchsc(DbTable* sctable)
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
					return 0;
				}
				break;

			case CACHEDUSER:
				if ((u = User::unserialize(this,&data))) u->dbid = id;
				else
				{
					app->debug_log("Failed - user record read error");
					return 0;
				}
		}
	}

	// any child nodes arrived before their parents?
	for (int i = dp.size(); i--; ) if ((n = nodebyhandle(dp[i]->parenthandle))) dp[i]->setparent(n);

	mergenewshares(0);

	return 1;
}

void MegaClient::fetchnodes()
{
	// only initial load from local cache
	if (!nodes.size() && sctable && !ISUNDEF(cachedscsn) && fetchsc(sctable))
	{
		app->debug_log("Session loaded from local cache");

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

	for (node_map::iterator it = nodes.begin(); it != nodes.end(); it++) delete it->second;
	nodes.clear();

	for (newshare_list::iterator it = newshares.begin(); it != newshares.end(); it++) delete *it;
	newshares.clear();

	users.clear();
	uhindex.clear();
	umindex.clear();

	*scsn = 0;

	if (pendingsc) pendingsc->disconnect();

	for (int i = sizeof rootnodes/sizeof *rootnodes; i--; ) rootnodes[i] = UNDEF;
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

void MegaClient::escapefilename(string* filename, const char* badchars)
{
	char buf[4];

	if (!badchars) badchars = "\\/:?\"<>|";

	// replace all occurrences of a badchar with %xx
	for (int i = filename->size(); i--; )
	{
		if (strchr(badchars,(*filename)[i]))
		{
			sprintf(buf,"%%%02x",(unsigned char)(*filename)[i]);
			filename->replace(i,1,buf);
		}
	}
}

int MegaClient::islchex(char c)
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z');
}

int MegaClient::hexval(char c)
{
	return c > '9' ? c-'a'+10 : c-'0';
}

// replace occurrences of %xx (x being a lowercase hex digit) with the encoded character
void MegaClient::unescapefilename(string* filename)
{
	char c;

	for (int i = filename->size()-2; i-- > 0; )
	{
		if ((*filename)[i] == '%' && islchex((*filename)[i+1]) && islchex((*filename)[i+2]))
		{
			c = (hexval((*filename)[i+1])<<4)+hexval((*filename)[i+2]);
			filename->replace(i,3,&c,1);
		}
	}
}

FileTransfer::FileTransfer()
{
	file = NULL;
	reqs = NULL;
	pendingcmd = NULL;

	inuse = 0;
	connections = 4;
}

void FileTransfer::init(m_off_t s, const char* fn, int c)
{
	size = s;

	if (fn) filename = fn;
	else filename.erase();

	uploadhandle = 0;

	pos = 0;

	connections = c;

	lastdata = 0;

	reqs = new HttpReqXfer*[connections]();

	bt.reset();
}

// abort all HTTP connections
void FileTransfer::disconnect()
{
	if (reqs) for (int i = connections; i--; ) if (reqs[i]) reqs[i]->disconnect();
}

// close file transfer
void FileTransfer::close()
{
	for (chunkmac_map::iterator it = chunkmacs.begin(); it != chunkmacs.end(); ) chunkmacs.erase(it++);

	pendingcmd = NULL;

	delete file;
	file = NULL;

	if (reqs)
	{
		for (int i = connections; i--; ) delete reqs[i];
		delete[] reqs;
		reqs = NULL;
	}

	inuse = 0;
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
int64_t FileTransfer::macsmac(chunkmac_map* macs)
{
	byte mac[SymmCipher::BLOCKSIZE] = { 0 };

	for (chunkmac_map::iterator it = macs->begin(); it != macs->end(); )
	{
		SymmCipher::xorblock(it->second.mac,mac);
		key.ecb_encrypt(mac);
		macs->erase(it++);
	}

	uint32_t* m = (uint32_t*)mac;

	m[0] ^= m[1];
	m[1] = m[2]^m[3];

	return *(int64_t*)mac;
}

// file transfer state machine
void FileTransfer::doio(MegaClient* client)
{
	if (file)
	{
		int backoff = 0;

		m_off_t progress = 0;

		for (int i = connections; i--; )
		{
			if (reqs[i])
			{
				switch (reqs[i]->status)
				{
					case REQ_INFLIGHT:
						progress += reqs[i]->transferred(client);
						break;

					case REQ_SUCCESS:
						lastdata = client->httpio->ds;

						progresscompleted += reqs[i]->size;

						client->dispatchmore(upload);

						if (upload)
						{
							if (reqs[i]->in.size())
							{
								if (reqs[i]->in.size() == UPLOADTOKENLEN*4/3)
								{
									byte ultoken[UPLOADTOKENLEN+1];
									byte filekey[Node::FILENODEKEYLENGTH];

									if (Base64::atob(reqs[i]->in.data(),ultoken,UPLOADTOKENLEN+1)-ultoken == UPLOADTOKENLEN)
									{
										memcpy(filekey,key.key,sizeof key.key);
										((int64_t*)filekey)[2] = ctriv;
										((int64_t*)filekey)[3] = macsmac(&chunkmacs);
										SymmCipher::xorblock(filekey+SymmCipher::KEYLENGTH,filekey);

										client->app->transfer_update(this-client->ft,size,size,starttime);
										return client->app->transfer_complete(this-client->ft,uploadhandle,ultoken,filekey,&key);
									}
								}

								client->app->transfer_failed(this-client->ft,(error)atoi(reqs[i]->in.c_str()));
								client->defer((transfer_list*)&client->putq,this-client->ft);	// defer decryption failures indefinitely
								return client->tclose(this-client->ft);
							}
						}
						else
						{
							reqs[i]->finalize(file,&key,&chunkmacs,ctriv,startpos,endpos);

							if (endpos != -1)
							{
								// partial read: complete?
								if (progresscompleted >= endpos-startblock) return client->app->transfer_complete(this-client->ft,&chunkmacs,filename.c_str());
							}
							else if (progresscompleted == size)
							{
								if (macsmac(&chunkmacs) == metamac)
								{
									client->app->transfer_update(this-client->ft,size,size,starttime);
									return client->app->transfer_complete(this-client->ft,&chunkmacs,filename.c_str());
								}
								else
								{
									client->app->transfer_failed(this-client->ft,filename,API_EKEY);
									client->defer((transfer_list*)&client->getq,this-client->ft,1);	// defer decryption failures indefinitely
									return client->tclose(this-client->ft);
								}
							}
						}

						reqs[i]->status = REQ_READY;
						break;

					case REQ_FAILURE:
						if (reqs[i]->httpstatus == 509)
						{
							// report over quota
							client->app->transfer_limit(this-client->ft);

							// fixed ten-minute retry intervals
							backoff = 6000;
						}
						else if (client->app->transfer_error(this-client->ft,reqs[i]->httpstatus,client->httpio->ds-lastdata)) return client->tclose(this-client->ft);

						reqs[i]->status = REQ_PREPARED;

					default:;
				}
			}

			if (!reqs[i] || reqs[i]->status == REQ_READY)
			{
				m_off_t npos = ChunkedHash::chunkceil(pos);

				if (npos > size) npos = size;

				if (npos > pos || !size)
				{
					if (!reqs[i]) reqs[i] = upload ? (HttpReqXfer*)new HttpReqUL() : (HttpReqXfer*)new HttpReqDL();

					reqs[i]->prepare(file,tempurl.c_str(),&key,&chunkmacs,ctriv,pos,npos);
					reqs[i]->status = REQ_PREPARED;
					pos = npos;
				}
				else if (reqs[i]) reqs[i]->status = REQ_DONE;
			}

			if (reqs[i] && reqs[i]->status == REQ_PREPARED) reqs[i]->post(client);
		}

		progress += progresscompleted;

		if (progress != progressreported)
		{
			client->app->transfer_update(this-client->ft,progress,size,starttime);
			progressreported = progress;

			lastdata = client->httpio->ds;

			client->dispatchmore(upload);
		}

		if (client->httpio->ds-lastdata >= XFERTIMEOUT)
		{
			if (upload)
			{
				client->app->transfer_failed(this-client->ft,API_EFAILED);
				client->defer((transfer_list*)&client->putq,this-client->ft);
			}
			else
			{
				client->app->transfer_failed(this-client->ft,filename,API_EFAILED);
				client->defer((transfer_list*)&client->getq,this-client->ft);
			}

			client->tclose(this-client->ft);
		}
		else
		{
			if (!backoff)
			{
				// no other backoff: check again at XFERMAXFAIL
				backoff = XFERTIMEOUT-(client->httpio->ds-lastdata);
			}

			bt.backoff(client->httpio->ds,backoff);
		}
	}
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
void HttpReqDL::prepare(FileAccess* file, const char* tempurl, SymmCipher* key, chunkmac_map* macs, uint64_t ctriv, m_off_t pos, m_off_t npos)
{
	char urlbuf[256];

	snprintf(urlbuf,sizeof urlbuf,"%s/%lu-%lu",tempurl,(long)pos,(long)npos-1);
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
void HttpReqDL::finalize(FileAccess* file, SymmCipher* key, chunkmac_map* macs, uint64_t ctriv, m_off_t startpos, m_off_t endpos)
{
	byte mac[SymmCipher::BLOCKSIZE] = { 0 };

	key->ctr_crypt(buf,bufpos,dlpos,ctriv,mac,0);

	memcpy((*macs)[dlpos].mac,mac,sizeof mac);

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

	file->fwrite(buf+skip,bufpos-skip-prune,dlpos+skip);
}

// prepare chunk for uploading: mac and encrypt
void HttpReqUL::prepare(FileAccess* file, const char* tempurl, SymmCipher* key, chunkmac_map* macs, uint64_t ctriv, m_off_t pos, m_off_t npos)
{
	byte mac[SymmCipher::BLOCKSIZE] = { 0 };
	char buf[256];

	snprintf(buf,sizeof buf,"%s/%lu",tempurl,(long)pos);
	setreq(buf,REQ_BINARY);

	size = (unsigned)(npos-pos);

	// FIXME: check return value and abort upload in case file read fails
	file->fread(out,size,(-(int)size)&(SymmCipher::BLOCKSIZE-1),pos);

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
