/**
 * @file mega/transfer.h
 * @brief pending/active up/download ordered by file fingerprint
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

#ifndef MEGA_TRANSFER_H
#define MEGA_TRANSFER_H 1

#include "node.h"
#include "backofftimer.h"

namespace mega {

// pending/active up/download ordered by file fingerprint (size - mtime - sparse CRC)
struct Transfer : public FileFingerprint
{
	// PUT or GET
	direction_t type;

	// transfer slot this transfer is active in (can be NULL if still queued)
	TransferSlot* slot;

	// files belonging to this transfer - transfer terminates upon its last file is removed
	file_list files;

	// failures/backoff
	unsigned failcount;
	BackoffTimer bt;

	// representative local filename for this transfer
	string localfilename;

	m_off_t pos;

	byte filekey[FILENODEKEYLENGTH];

	// CTR mode IV
	int64_t ctriv;

	// meta MAC
	int64_t metamac;

	// file crypto key
	SymmCipher key;

	chunkmac_map chunkmacs;

	// upload handle for file attribute attachment (only set if file attribute queued)
	handle uploadhandle;

	// signal failure
	void failed(error);

	// signal completion
	void complete();

	// position in transfers[type]
	transfer_map::iterator transfers_it;

	// backlink to base
	MegaClient* client;
	int tag;
	Transfer(MegaClient*, direction_t);
	virtual ~Transfer();
};

} // namespace

#endif
