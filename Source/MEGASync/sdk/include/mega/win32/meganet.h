/**
 * @file mega/win32/meganet.h
 * @brief Win32 network access layer (using WinHTTP)
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
#define HTTPIO_CLASS WinHttpIO

#include "mega/megaclient.h"

#include "megawaiter.h"

namespace mega {

extern bool debug;

class WinHttpIO : public HttpIO
{
	CRITICAL_SECTION csHTTP;
	HANDLE hWakeupEvent;

	protected:
	WinWaiter* waiter;
	HINTERNET hSession;
	bool completion;

public:
	static const unsigned HTTP_POST_CHUNK_SIZE = 16384;

	static VOID CALLBACK asynccallback(HINTERNET, DWORD_PTR, DWORD, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength);

	void updatedstime();

	void post(HttpReq*, const char* = 0, unsigned = 0);
	void cancel(HttpReq*);

	m_off_t postpos(void*);

	bool doio(void);

	void addevents(Waiter*, int);

	void entercs();
	void leavecs();

	void httpevent();

	WinHttpIO();
	~WinHttpIO();
};

struct WinHttpContext
{
	HINTERNET hRequest;
	HINTERNET hConnect;

	HttpReq* req;                   // backlink to underlying HttpReq
	WinHttpIO* httpio;              // backlink to application-wide WinHttpIO object

	unsigned postpos;
	unsigned postlen;
	const char* postdata;
};

} // namespace

#endif
