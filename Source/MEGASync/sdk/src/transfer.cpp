/**
 * @file transfer.cpp
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

#include "mega/transfer.h"
#include "mega/megaclient.h"
#include "mega/transferslot.h"
#include "mega/megaapp.h"

namespace mega {
Transfer::Transfer(MegaClient* cclient, direction_t ctype)
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
    for (file_list::iterator it = files.begin(); it != files.end(); it++)
    {
        ( *it )->transfer = NULL;
    }

    client->transfers[type].erase(transfers_it);
    delete slot;
}

// transfer attempt failed, notify all related files, collect request on
// whether to abort the transfer, kill transfer if unanimous
void Transfer::failed(error e)
{
    bool defer = false;

    bt.backoff();

    client->app->transfer_failed(this, e);

    for (file_list::iterator it = files.begin(); it != files.end(); it++)
    {
        if (( *it )->failed(e) && !defer)
        {
            defer = true;
        }
    }

    if (defer)
    {
        failcount++;
        delete slot;
    }
    else
    {
        delete this;
    }
}

// transfer completion: copy received file locally, set timestamp(s), verify
// fingerprint, notify app, notify files
void Transfer::complete()
{
    if (slot->fa)
    {
        client->app->transfer_complete(this);
    }

    if (type == GET)
    {
        // disconnect temp file from slot...
        delete slot->fa;
        slot->fa = NULL;

        // FIXME: multiple overwrite race conditions below (make copies
        // from open file instead of closing/reopening!)

        // set timestamp (subsequent moves & copies are assumed not to alter
        // mtime)
        client->fsaccess->setmtimelocal(&localfilename, mtime);

        // verify integrity of file
        FileAccess* fa = client->fsaccess->newfileaccess();
        FileFingerprint fingerprint;
        Node* n;

        if (fa->fopen(&localfilename, true, false))
        {
            fingerprint.genfingerprint(fa);
            delete fa;

            if (isvalid && !( fingerprint == *(FileFingerprint*)this ))
            {
                client->fsaccess->unlinklocal(&localfilename);
                return failed(API_EWRITE);
            }
        }

        // set FileFingerprint on source node(s) if missing
        for (file_list::iterator it = files.begin(); it != files.end(); it++)
        {
            if (( *it )->hprivate && ( n = client->nodebyhandle(( *it )->h)))
            {
                if (!n->isvalid)
                {
                    *(FileFingerprint*)n = fingerprint;

                    n->serializefingerprint(&n->attrs.map['c']);
                    client->setattr(n);
                }
            }
        }

        // ...and place it in all target locations. first, update the files'
        // local target filenames,
        // in case they have changed during the upload
        for (file_list::iterator it = files.begin(); it != files.end(); it++)
        {
            ( *it )->updatelocalname();
        }

        string tmplocalname;
        bool transient_error, success;

        // place file in all target locations - use up to one renames, copy
        // operations for the rest
        // remove and complete successfully completed files
        for (file_list::iterator it = files.begin(); it != files.end(); )
        {
            transient_error = false;
            success = false;

            if (!tmplocalname.size())
            {
                if (client->fsaccess->renamelocal(&localfilename, &( *it )->localname))
                {
                    tmplocalname = ( *it )->localname;
                    success = true;
                }
                else if (client->fsaccess->transient_error)
                {
                    transient_error = true;
                }
            }

            if (!success)
            {
                if (client->fsaccess->copylocal(tmplocalname.size() ? &tmplocalname : &localfilename,
                                               &( *it )->localname))
                {
                    success = true;
                }
                else if (client->fsaccess->transient_error)
                {
                    transient_error = true;
                }
            }

            if (success || !transient_error)
            {
                if (success)
                {
                    // prevent deletion of associated Transfer object in completed()
                    ( *it )->transfer = NULL;
                    ( *it )->completed(this, NULL);
                }

                if (success || !( *it )->failed(API_EAGAIN))
                {
                    files.erase(it++);
                }
                else
                {
                    it++;
                }
            }
            else
            {
                it++;
            }
        }

        if (!tmplocalname.size() && !files.size())
        {
            client->fsaccess->unlinklocal(&localfilename);
        }
    }
    else
    {
        // files must not change during a PUT transfer
        if (genfingerprint(slot->fa, true))
        {
            return failed(API_EREAD);
        }

        // notify all files and give them an opportunity to self-destruct
        for (file_list::iterator it = files.begin(); it != files.end(); )
        {
            ( *it )->transfer = NULL;     // prevent deletion of associated
                                          // Transfer object in completed()
            ( *it )->completed(this, NULL);
            files.erase(it++);
        }
    }

    if (!files.size())
    {
        delete this;
    }
    else
    {
        // some files are still pending completion, close fa and set retry
        // timer
        delete slot->fa;
        slot->fa = NULL;

        slot->retrying = true;
        slot->retrybt.backoff(11);
    }
}
} // namespace
