/**
 * @file posix/net.cpp
 * @brief POSIX network access layer (using cURL)
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

#include "mega.h"

namespace mega {
CurlHttpIO::CurlHttpIO()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);

    curlm = curl_multi_init();

    curlsh = curl_share_init();
    curl_share_setopt(curlsh, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
    curl_share_setopt(curlsh, CURLSHOPT_SHARE, CURL_LOCK_DATA_SSL_SESSION);

    contenttypejson = curl_slist_append(NULL, "Content-Type: application/json");
    contenttypejson = curl_slist_append(contenttypejson, "Expect:");

    contenttypebinary = curl_slist_append(NULL, "Content-Type: application/octet-stream");
    contenttypebinary = curl_slist_append(contenttypebinary, "Expect:");
}

CurlHttpIO::~CurlHttpIO()
{
    curl_global_cleanup();
}

void CurlHttpIO::setuseragent(string* u)
{
    useragent = u;
}

// wake up from cURL I/O
void CurlHttpIO::addevents(Waiter* w, int flags)
{
    int t;
    PosixWaiter* pw = (PosixWaiter*)w;

    curl_multi_fdset(curlm, &pw->rfds, &pw->wfds, &pw->efds, &t);

    pw->bumpmaxfd(t);
}

// POST request to URL
void CurlHttpIO::post(HttpReq* req, const char* data, unsigned len)
{
    if (debug)
    {
        cout << "POST target URL: " << req->posturl << endl;

        if (req->binary)
        {
            cout << "[sending " << req->out->size() << " bytes of raw data]" << endl;
        }
        else
        {
            cout << "Sending: " << *req->out << endl;
        }
    }

    CURL* curl;

    req->in.clear();

    if ((curl = curl_easy_init()))
    {
        curl_easy_setopt(curl, CURLOPT_URL,           req->posturl.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS,    data ? data : req->out->data());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data ? len : req->out->size());
        curl_easy_setopt(curl, CURLOPT_USERAGENT,     useragent->c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER,    req->type == REQ_JSON ? contenttypejson : contenttypebinary);
        curl_easy_setopt(curl, CURLOPT_SHARE,         curlsh);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA,     (void*)req);
        curl_easy_setopt(curl, CURLOPT_PRIVATE,       (void*)req);

        curl_multi_add_handle(curlm, curl);

        req->status = REQ_INFLIGHT;

        req->httpiohandle = (void*)curl;
    }
    else
    {
        req->status = REQ_FAILURE;
    }
}

// cancel pending HTTP request
void CurlHttpIO::cancel(HttpReq* req)
{
    if (req->httpiohandle)
    {
        curl_multi_remove_handle(curlm, (CURL*)req->httpiohandle);
        curl_easy_cleanup((CURL*)req->httpiohandle);

        req->httpstatus = 0;
        req->status = REQ_FAILURE;
        req->httpiohandle = NULL;
    }
}

// real-time progress information on POST data
m_off_t CurlHttpIO::postpos(void* handle)
{
    double bytes;

    curl_easy_getinfo(handle, CURLINFO_SIZE_UPLOAD, &bytes);

    return (m_off_t)bytes;
}

// process events
bool CurlHttpIO::doio()
{
    bool done = false;

    CURLMsg *msg;
    int dummy;

    curl_multi_perform(curlm, &dummy);

    while ((msg = curl_multi_info_read(curlm, &dummy)))
    {
        HttpReq* req;

        if ((curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, (char**)&req) == CURLE_OK) && req)
        {
            req->httpio = NULL;

            if (msg->msg == CURLMSG_DONE)
            {
                curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &req->httpstatus);

                if (debug)
                {
                    cout << "CURLMSG_DONE with HTTP status: " << req->httpstatus << endl;

                    if (req->binary)
                    {
                        cout << "[received " << req->in.size() << " bytes of raw data]" << endl;
                    }
                    else
                    {
                        cout << "Received: " << req->in.c_str() << endl;
                    }
                }

                req->status = req->httpstatus == 200 ? REQ_SUCCESS : REQ_FAILURE;
                
                inetstatus(req->status);
                
                done = true;
            }
            else
            {
                req->status = REQ_FAILURE;
            }
        }

        curl_multi_remove_handle(curlm, msg->easy_handle);
        curl_easy_cleanup(msg->easy_handle);
    }

    return done;
}

// callback for incoming HTTP payload
size_t CurlHttpIO::write_data(void *ptr, size_t size, size_t nmemb, void *target)
{
    size *= nmemb;

    ((HttpReq*)target)->put(ptr, size);

    return size;
}
} // namespace
