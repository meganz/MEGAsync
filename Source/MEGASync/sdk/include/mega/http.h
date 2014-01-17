/**
 * @file mega/http.h
 * @brief Generic host HTTP I/O interfaces
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

#ifndef MEGA_HTTP_H
#define MEGA_HTTP_H 1

#include "types.h"
#include "waiter.h"

namespace mega {

// generic host HTTP I/O interface
struct HttpIO : public EventTrigger
{
	// post request to target URL
	virtual void post(struct HttpReq*, const char* = NULL, unsigned = 0) = 0;

	// cancel request
	virtual void cancel(HttpReq*) = 0;

	// real-time POST progress information
	virtual m_off_t postpos(void*) = 0;

	// execute I/O operations
	virtual bool doio(void) = 0;

	// track Internet connectivity issues
	dstime noinetds;
	bool inetback;
	void inetstatus(bool, dstime);
	bool inetisback();

	HttpIO();
	virtual ~HttpIO() { }
};

// outgoing HTTP request
struct HttpReq
{
	reqstatus_t status;

	int httpstatus;

	contenttype_t type;

	string posturl;

	string* out;
	string in;

	string outbuf;

	byte* buf;
	unsigned buflen, bufpos;

	// HttpIO implementation-specific identifier for this connection
	void* httpiohandle;

	// while this request is in flight, points to the application's HttpIO object - NULL otherwise
	HttpIO* httpio;

	// set url and content type for subsequent requests
	void setreq(const char*, contenttype_t);

	// post request to the network
	void post(MegaClient*, const char* = NULL, unsigned = 0);

	// store chunk of incoming data
	void put(void*, unsigned);

	// reserve space for incoming data
	byte* reserveput(unsigned* len);

	// confirm receipt of data in reserved space
	void completeput(unsigned len);

	// disconnect open HTTP connection
	void disconnect();

	// progress information
	virtual m_off_t transferred(MegaClient*);

	// timestamp of last data received
	dstime lastdata;

	// prevent raw data from being dumped in debug mode
	bool binary;

	HttpReq(int = 0);
	virtual ~HttpReq();
};

// file chunk I/O
struct HttpReqXfer : public HttpReq
{
	unsigned size;

	virtual bool prepare(FileAccess*, const char*, SymmCipher*, chunkmac_map*, uint64_t, m_off_t, m_off_t) = 0;
	virtual void finalize(FileAccess*, SymmCipher*, chunkmac_map*, uint64_t, m_off_t, m_off_t) { }

	HttpReqXfer() : HttpReq(1) { };
};

// file chunk upload
struct HttpReqUL : public HttpReqXfer
{
	bool prepare(FileAccess*, const char*, SymmCipher*, chunkmac_map*, uint64_t, m_off_t, m_off_t);

	m_off_t transferred(MegaClient*);

	~HttpReqUL() { };
};

// file chunk download
struct HttpReqDL : public HttpReqXfer
{
	m_off_t dlpos;

	bool prepare(FileAccess*, const char*, SymmCipher*, chunkmac_map*, uint64_t, m_off_t, m_off_t);
	void finalize(FileAccess*, SymmCipher*, chunkmac_map*, uint64_t, m_off_t, m_off_t);

	~HttpReqDL() { };
};

// file attribute get
struct HttpReqGetFA : public HttpReq
{
	~HttpReqGetFA() { };
};

} // namespace

#endif
