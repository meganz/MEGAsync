/**
 * @file mega/transferslot.h
 * @brief Class for active transfer
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

#ifndef MEGA_TRANSFERSLOT_H
#define MEGA_TRANSFERSLOT_H 1

#include "http.h"
#include "node.h"
#include "backofftimer.h"

namespace mega {
// active transfer
struct MEGA_API TransferSlot
{
    // link to related transfer (never NULL)
    struct Transfer* transfer;

    // associated source/destination file
    FileAccess* fa;

    // command in flight to obtain temporary URL
    Command* pendingcmd;

    // transfer attempts are considered failed after XFERTIMEOUT seconds
    // without data flow
    static const dstime XFERTIMEOUT = 600;

    m_off_t progressreported, progresscompleted;

    dstime starttime, lastdata;

    // number of consecutive errors
    unsigned errorcount;

    // upload result
    byte ultoken[NewNode::UPLOADTOKENLEN + 1];

    // file attribute string
    string fileattrstring;

    // file attributes mutable
    int fileattrsmutable;

    // storage server access URL
    string tempurl;

    // maximum number of parallel connections and connection aray
    int connections;
    HttpReqXfer** reqs;

    // handle I/O for this slot
    void doio(MegaClient*);

    // disconnect and reconnect all open connections for this transfer
    void disconnect();

    // indicate progress
    void progress();

    // compute the meta MAC based on the chunk MACs
    int64_t macsmac(chunkmac_map*);

    // tslots list position
    transferslot_list::iterator slots_it;

    // slot operation retry timer
    bool retrying;
    BackoffTimer retrybt;

    // transfer failure flag
    bool failure;
    
    TransferSlot(Transfer*);
    ~TransferSlot();
};
} // namespace

#endif
