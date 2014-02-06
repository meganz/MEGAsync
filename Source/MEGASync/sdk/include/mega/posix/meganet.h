/**
 * @file mega/posix/meganet.h
 * @brief POSIX network access layer (using cURL)
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

#ifndef HTTPIO_CLASS
#define HTTPIO_CLASS CurlHttpIO

#include "mega.h"

namespace mega {

class CurlHttpIO: public HttpIO
{
    string* useragent;

protected:
    CURLM* curlm;
    CURLSH* curlsh;

    static size_t write_data(void *, size_t, size_t, void *);

    curl_slist* contenttypejson;
    curl_slist* contenttypebinary;

public:
    void post(HttpReq*, const char* = 0, unsigned = 0);
    void cancel(HttpReq*);

    m_off_t postpos(void*);

    bool doio(void);

    void addevents(Waiter*, int);

    void setuseragent(string*);

    CurlHttpIO();
    ~CurlHttpIO();
};

} // namespace

#endif
