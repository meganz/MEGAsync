/**
 * @file commands.cpp
 * @brief Implementation of various commands
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

#include "mega/types.h"
#include "mega/command.h"
#include "mega/megaapp.h"
#include "mega/fileattributefetch.h"
#include "mega/base64.h"
#include "mega/transferslot.h"
#include "mega/transfer.h"
#include "mega/utils.h"
#include "mega/user.h"

namespace mega {

HttpReqCommandPutFA::HttpReqCommandPutFA(MegaClient* client, handle cth, fatype ctype, byte* cdata, unsigned clen)
{
	cmd("ufa");
	arg("s",clen);

	persistent = true;	// object will be recycled either for retry or for posting to the file attribute server

	th = cth;
	type = ctype;
	data = cdata;
	len = clen;

	binary = true;

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

// request upload target URL
CommandPutFile::CommandPutFile(TransferSlot* ctslot, int ms)
{
	tslot = ctslot;

	cmd("u");
	arg("s",tslot->fa->size);
	arg("ms",ms);
}

void CommandPutFile::cancel()
{
	Command::cancel();
	tslot = NULL;
}

// set up file transfer with returned target URL
void CommandPutFile::procresult()
{
	if (tslot) tslot->pendingcmd = NULL;
	if (canceled) return;

	if (client->json.isnumeric()) return tslot->transfer->failed((error)client->json.getint());

	for (;;)
	{
		switch (client->json.getnameid())
		{
			case 'p':
				client->json.storeobject(&tslot->tempurl);
				break;

			case EOO:
				if (tslot->tempurl.size())
				{
					tslot->starttime = tslot->lastdata = client->waiter->ds;
					return tslot->progress();
				}
				else return tslot->transfer->failed(API_EINTERNAL);

			default:
				if (!client->json.storeobject()) return tslot->transfer->failed(API_EINTERNAL);
		}
	}
}

// request temporary source URL
// p == private node
CommandGetFile::CommandGetFile(TransferSlot* ctslot, byte* key, handle h, bool p)
{
	cmd("g");
	arg(p ? "n" : "p",(byte*)&h,MegaClient::NODEHANDLE);
	arg("g",1);

	tslot = ctslot;

	if (!tslot)
	{
		ph = h;
		memcpy(filekey,key,FILENODEKEYLENGTH);
	}
}

void CommandGetFile::cancel()
{
	Command::cancel();
	tslot = NULL;
}

// process file credentials
void CommandGetFile::procresult()
{
	if (tslot) tslot->pendingcmd = NULL;
	if (canceled) return;

	if (client->json.isnumeric())
	{
		if (tslot) return tslot->transfer->failed((error)client->json.getint());
		return client->app->checkfile_result(ph,(error)client->json.getint());
	}

	const char* at = NULL;
	error e = API_EINTERNAL;
	m_off_t s = -1;
	int d = 0;
	byte* buf;
	time_t ts = 0, tm = 0;

	// credentials relevant to a non-TransferSlot scenario (node query)
	string fileattrstring;
	string filenamestring;
	string filefingerprint;

	for (;;)
	{
		switch (client->json.getnameid())
		{
			case 'g':
				client->json.storeobject(tslot ? &tslot->tempurl : NULL);
				e = API_OK;
				break;

			case 's':
				s = client->json.getint();
				break;

			case 'd':
				d = 1;
				break;

			case MAKENAMEID2('t','s'):
				ts = client->json.getint();
				break;

			case MAKENAMEID3('t','m','d'):
				tm = ts+client->json.getint();
				break;
			
			case MAKENAMEID2('a','t'):
				at = client->json.getvalue();
				break;

			case MAKENAMEID2('f','a'):
				if (tslot) client->json.storeobject(&tslot->fileattrstring);
				else client->json.storeobject(&fileattrstring);
				break;

			case MAKENAMEID3('p','f','a'):
				if (tslot) tslot->fileattrsmutable = (int)client->json.getint();
				break;

			case 'e':
				e = (error)client->json.getint();
				break;

			case EOO:
				if (d || !at)
				{
					e = at ? API_EBLOCKED : API_EINTERNAL;

					if (tslot) return tslot->transfer->failed(e);
					return client->app->checkfile_result(ph,e);
				}
				else
				{
					// decrypt at and set filename
					SymmCipher key;
					const char* eos = strchr(at,'"');

					key.setkey(filekey,FILENODE);

					if ((buf = Node::decryptattr(tslot ? &tslot->transfer->key : &key,at,eos ? eos-at : strlen(at))))
					{
						JSON json;

						json.begin((char*)buf+5);

						for (;;)
						{
							switch (json.getnameid())
							{
								case 'c':
									if (!json.storeobject(&filefingerprint))
									{
										delete[] buf;
										if (tslot) return tslot->transfer->failed(API_EINTERNAL);
										return client->app->checkfile_result(ph,API_EINTERNAL);
									}
									break;

								case 'n':
									if (!json.storeobject(&filenamestring))
									{
										delete[] buf;
										if (tslot) return tslot->transfer->failed(API_EINTERNAL);
										return client->app->checkfile_result(ph,API_EINTERNAL);
									}
									break;

								case EOO:
									delete[] buf;

									if (tslot)
									{
										tslot->starttime = tslot->lastdata = client->waiter->ds;
										if (tslot->tempurl.size() && s >= 0) return tslot->progress();
										return tslot->transfer->failed(e);
									}
									else return client->app->checkfile_result(ph,e,filekey,s,ts,tm,&filenamestring,&filefingerprint,&fileattrstring);

								default:
									if (!json.storeobject())
									{
										delete[] buf;
										if (tslot) return tslot->transfer->failed(API_EINTERNAL);
										else return client->app->checkfile_result(ph,API_EINTERNAL);
									}
							}
						}
					}

					if (tslot) return tslot->transfer->failed(API_EKEY);
					else return client->app->checkfile_result(ph,API_EKEY);
				}

			default:
				if (!client->json.storeobject())
				{
					if (tslot) return tslot->transfer->failed(API_EINTERNAL);
					else return client->app->checkfile_result(ph,API_EINTERNAL);
				}
		}
	}
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

// (the result is not processed directly - we rely on the server-client response)
CommandPutNodes::CommandPutNodes(MegaClient* client, handle th, const char* userhandle, NewNode* newnodes, int numnodes, int ctag, putsource_t csource)
{
	byte key[FILENODEKEYLENGTH];
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

CommandMoveNode::CommandMoveNode(MegaClient* client, Node* n, Node* t, syncdel_t csyncdel)
{
	cmd("m");
	notself(client);

	h = n->nodehandle;

	if ((syncdel = csyncdel) != SYNCDEL_NONE) syncn = n;

	arg("n",(byte*)&h,MegaClient::NODEHANDLE);
	arg("t",(byte*)&t->nodehandle,MegaClient::NODEHANDLE);

	TreeProcShareKeys tpsk;
	client->proctree(n,&tpsk);
	tpsk.get(this);

	tag = client->reqtag;
}

void CommandMoveNode::procresult()
{
	if (client->json.isnumeric())
	{
		error e = (error)client->json.getint();

		if (syncdel != SYNCDEL_NONE)
		{
			if (e == API_OK)
			{
				Node* n;

				// update all todebris records in the subtree
				for (node_set::iterator it = client->todebris.begin(); it != client->todebris.end(); it++)
				{
					n = *it;

					do {
						if (n == syncn)
						{
							(*it)->syncdeleted = syncdel;
							break;
						}
					} while ((n = n->parent));
				}
			}
			else syncn->syncdeleted = SYNCDEL_NONE;
		}

		client->app->rename_result(h,e);
	}
	else
	{
		client->json.storeobject();
		client->app->rename_result(h,API_EINTERNAL);
	}
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
CommandSetShare::CommandSetShare(MegaClient* client, Node* n, User* u, accesslevel_t a, int newshare)
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

CommandPurchaseAddItem::CommandPurchaseAddItem(MegaClient* chan, int itemclass, handle item, unsigned price, char* curreny, unsigned tax, char* country, char* affiliate)
{
	cmd("uts");

	// FIXME: implement
}

void CommandPurchaseAddItem::procresult()
{
	// FIXME: implement
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

CommandUserRequest::CommandUserRequest(MegaClient* client, const char* m, visibility_t show)
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

// set node keys (e.g. to convert asymmetric keys to symmetric ones)
CommandNodeKeyUpdate::CommandNodeKeyUpdate(MegaClient* client, handle_vector* v)
{
	byte nodekey[FILENODEKEYLENGTH];

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

CommandGetPH::CommandGetPH(MegaClient* client, handle cph, const byte* ckey, int cop)
{
	cmd("g");
	arg("p",(byte*)&cph,MegaClient::NODEHANDLE);

	ph = cph;
	memcpy(key,ckey,sizeof key);
	tag = client->reqtag;
	op = cop;
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
					client->app->openfilelink_result(ph,key,s,&a,fa.c_str(),ts,tm,op);
				}
				else client->app->openfilelink_result(API_EINTERNAL);
				return;

			default:
				if (!client->json.storeobject()) client->app->openfilelink_result(API_EINTERNAL);
		}
	}
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

// fetch full node tree
CommandFetchNodes::CommandFetchNodes(MegaClient* client)
{
	cmd("f");
	arg("c","1",0);
	arg("r","1",0);

	tag = client->reqtag;
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
				client->syncsup = false;
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

} // namespace
