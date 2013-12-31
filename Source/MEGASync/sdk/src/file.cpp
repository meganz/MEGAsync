/**
 * @file file.cpp
 * @brief Classes for transferring files
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

#include "mega/file.h"
#include "mega/transfer.h"
#include "mega/transferslot.h"
#include "mega/megaclient.h"
#include "mega/sync.h"
#include "mega/command.h"

namespace mega {

File::File()
{
	transfer = NULL;
	hprivate = true;
	syncxfer = false;
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
		newnode->nodekey.assign((char*)t->filekey,FILENODEKEYLENGTH);
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
			t->client->reqs[t->client->r].add(new CommandPutNodes(t->client,th,NULL,newnode,1,l ? l->sync->tag : t->tag,l ? PUTNODES_SYNC : PUTNODES_APP));
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

SyncFileGet::SyncFileGet(Sync* csync, Node* cn, string* clocalname)
{
	sync = csync;

	n = cn;
	h = n->nodehandle;
	*(FileFingerprint*)this = *n;
	localname = *clocalname;

	syncxfer = true;
	n->syncget = this;
}

SyncFileGet::~SyncFileGet()
{
	n->syncget = NULL;
}

// update localname (parent's localnode 
void SyncFileGet::updatelocalname()
{
	attr_map::iterator ait;

	if ((ait = n->attrs.map.find('n')) != n->attrs.map.end())
	{
		if (n->parent && n->parent->localnode)
		{
			string tmpname = ait->second;
			
			sync->client->fsaccess->name2local(&tmpname);
			n->parent->localnode->getlocalpath(&localname);

			localname.append(sync->client->fsaccess->localseparator);
			localname.append(tmpname);
		}
	}
}

// add corresponding LocalNode (by path), then self-destruct
void SyncFileGet::completed(Transfer* t, LocalNode* n)
{
	sync->checkpath(NULL,&localname);
	delete this;
}

} // namespace
