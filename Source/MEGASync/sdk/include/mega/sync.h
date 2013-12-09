/**
 * @file mega/sync.h
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

#ifndef MEGA_SYNC_H
#define MEGA_SYNC_H 1

#include "megaclient.h"

namespace mega {

class Sync
{
public:
	enum { MAIN, RETRY };

	MegaClient* client;

	// root of local filesystem tree, holding the sync's root folder
	LocalNode localroot;

	// queued ScanItems - [0] is the main queue, [1] for locked item retries
	scanitem_deque scanq[2];

	// current state
	syncstate state;

	// change state, signal to application
	void changestate(syncstate);

	// process and remove one scanq item
	void procscanq(int);

	m_off_t localbytes;
	unsigned localnodes[2];

	// add or update LocalNode item, scan newly added folders
	void queuescan(int, string*, string*, LocalNode*, LocalNode*, bool);

	// examine filesystem item and queue it for scanning
	LocalNode* queuefsrecord(string*, string*, LocalNode*, bool);

	// scan items in specified path and add as children of the specified LocalNode
	void scan(string*, FileAccess*, LocalNode*, bool);

	// determine status of a given path
	pathstate_t pathstate(string*);

	// own position in session sync list
	sync_list::iterator sync_it;

	// notified nodes originating from this sync bear this tag
	int tag;

	Sync(MegaClient*, string*, Node*, int = 0);
	~Sync();
};

} // namespace

#endif
