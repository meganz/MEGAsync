/**
 * @file transferslot.cpp
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

#include "mega/transferslot.h"
#include "mega/node.h"
#include "mega/transfer.h"
#include "mega/megaclient.h"
#include "mega/command.h"
#include "mega/base64.h"
#include "mega/megaapp.h"
#include "mega/utils.h"

namespace mega {
TransferSlot::TransferSlot(Transfer* ctransfer)
{
    starttime = 0;
    progressreported = 0;
    progresscompleted = 0;
    lastdata = Waiter::ds;
    errorcount = 0;

    failure = false;
    retrying = false;
    
    fileattrsmutable = 0;

    reqs = NULL;
    pendingcmd = NULL;

    transfer = ctransfer;
    transfer->slot = this;

    connections = transfer->size > 131072 ? transfer->client->connections[transfer->type] : 1;

    reqs = new HttpReqXfer*[connections]();

    fa = transfer->client->fsaccess->newfileaccess();

    slots_it = transfer->client->tslots.end();
}

// delete slot and associated resources, but keep transfer intact (can be
// reused on a new slot)
TransferSlot::~TransferSlot()
{
    transfer->slot = NULL;

    if (slots_it != transfer->client->tslots.end())
    {
        // advance main loop iterator if deleting next in line
        if (transfer->client->slotit != transfer->client->tslots.end() && *transfer->client->slotit == this)
        {
            transfer->client->slotit++;
        }

        transfer->client->tslots.erase(slots_it);
    }

    if (pendingcmd)
    {
        pendingcmd->cancel();
    }

    if (fa)
    {
        delete fa;

        if ((transfer->type == GET) && transfer->localfilename.size())
        {
            transfer->client->fsaccess->unlinklocal(&transfer->localfilename);
        }
    }

    while (connections--)
    {
        delete reqs[connections];
    }

    delete[] reqs;
}

// abort all HTTP connections
void TransferSlot::disconnect()
{
    for (int i = connections; i--;)
    {
        if (reqs[i])
        {
            reqs[i]->disconnect();
        }
    }
}

// coalesce block macs into file mac
int64_t TransferSlot::macsmac(chunkmac_map* macs)
{
    byte mac[SymmCipher::BLOCKSIZE] = { 0 };

    for (chunkmac_map::iterator it = macs->begin(); it != macs->end(); it++)
    {
        SymmCipher::xorblock(it->second.mac, mac);
        transfer->key.ecb_encrypt(mac);
    }

    macs->clear();

    uint32_t* m = (uint32_t*)mac;

    m[0] ^= m[1];
    m[1] = m[2] ^ m[3];

    return MemAccess::get<int64_t>((const char*)mac);
}

// file transfer state machine
void TransferSlot::doio(MegaClient* client)
{
    if (!fa)
    {
        // this is a pending completion, retry every 200 ms by default
        retrybt.backoff(2);
        retrying = true;

        return transfer->complete();
    }

    retrying = false;

    if (!tempurl.size())
    {
        return;
    }

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
                    lastdata = Waiter::ds;

                    progresscompleted += reqs[i]->size;

                    if (transfer->type == PUT)
                    {
                        errorcount = 0;

                        // completed put transfers are signalled through the
                        // return of the upload token
                        if (reqs[i]->in.size())
                        {
                            if (reqs[i]->in.size() == NewNode::UPLOADTOKENLEN * 4 / 3)
                            {
                                if (Base64::atob(reqs[i]->in.data(), ultoken, NewNode::UPLOADTOKENLEN + 1)
                                    == NewNode::UPLOADTOKENLEN)
                                {
                                    memcpy(transfer->filekey, transfer->key.key, sizeof transfer->key.key);
                                    ((int64_t*)transfer->filekey)[2] = transfer->ctriv;
                                    ((int64_t*)transfer->filekey)[3] = macsmac(&transfer->chunkmacs);
                                    SymmCipher::xorblock(transfer->filekey + SymmCipher::KEYLENGTH, transfer->filekey);

                                    return transfer->complete();
                                }
                            }

                            // fail with returned error
                            return transfer->failed((error)atoi(reqs[i]->in.c_str()));
                        }
                    }
                    else
                    {
                        if (reqs[i]->size == reqs[i]->bufpos)
                        {
                            errorcount = 0;

                            reqs[i]->finalize(fa, &transfer->key, &transfer->chunkmacs, transfer->ctriv, 0, -1);

                            if (progresscompleted == transfer->size)
                            {
                                // verify meta MAC
                                if (!progresscompleted || (macsmac(&transfer->chunkmacs) == transfer->metamac))
                                {
                                    return transfer->complete();
                                }
                                else
                                {
                                    return transfer->failed(API_EKEY);
                                }
                            }
                        }
                        else
                        {
                            errorcount++;
                            reqs[i]->status = REQ_PREPARED;
                            break;
                        }
                    }

                    reqs[i]->status = REQ_READY;
                    break;

                case REQ_FAILURE:
                    if (reqs[i]->httpstatus == 509)
                    {
                        client->app->transfer_limit(transfer);

                        // fixed ten-minute retry intervals
                        backoff = 6000;
                    }
                    else
                    {
                        if (!failure)
                        {
                            failure = true;
                            client->setchunkfailed(&reqs[i]->posturl);
                        }

                        reqs[i]->status = REQ_PREPARED;
                    }

                default:
                    ;
            }
        }

        if (!failure)
        {
            if (!reqs[i] || (reqs[i]->status == REQ_READY))
            {
                m_off_t npos = ChunkedHash::chunkceil(transfer->pos);

                if (npos > transfer->size)
                {
                    npos = transfer->size;
                }

                if ((npos > transfer->pos) || !transfer->size)
                {
                    if (!reqs[i])
                    {
                        reqs[i] = transfer->type == PUT ? (HttpReqXfer*)new HttpReqUL() : (HttpReqXfer*)new HttpReqDL();
                    }

                    if (reqs[i]->prepare(fa, tempurl.c_str(), &transfer->key,
                                         &transfer->chunkmacs, transfer->ctriv,
                                         transfer->pos, npos))
                    {
                        reqs[i]->status = REQ_PREPARED;
                        transfer->pos = npos;
                    }
                    else
                    {
                        if (!fa->retry)
                        {
                            return transfer->failed(API_EREAD);
                        }

                        // retry the read shortly
                        backoff = 2;
                    }
                }
                else if (reqs[i])
                {
                    reqs[i]->status = REQ_DONE;
                }
            }

            if (reqs[i] && (reqs[i]->status == REQ_PREPARED))
            {
                reqs[i]->post(client);
            }
        }
    }

    p += progresscompleted;

    if (p != progressreported)
    {
        progressreported = p;
        lastdata = Waiter::ds;

        progress();
    }

    if (Waiter::ds - lastdata >= XFERTIMEOUT && !failure)
    {
        failure = true;

        for (int i = connections; i--; )
        {
            if (reqs[i])
            {
                client->setchunkfailed(&reqs[i]->posturl);
            }
        }
    }

    if (errorcount > 10)
    {
        return transfer->failed(API_EFAILED);
    }
    else
    {
        if (!backoff)
        {
            // no other backoff: check again at XFERMAXFAIL
            backoff = XFERTIMEOUT - (Waiter::ds - lastdata);
        }

        transfer->bt.backoff(backoff);
    }
}

// transfer progress notification to app and related files
void TransferSlot::progress()
{
    transfer->client->app->transfer_update(transfer);

    for (file_list::iterator it = transfer->files.begin(); it != transfer->files.end(); it++)
    {
        (*it)->progress();
    }
}
} // namespace
