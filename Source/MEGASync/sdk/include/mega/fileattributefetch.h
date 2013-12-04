/**
 * @file mega/fileattributefetch.h
 * @brief Classes for file attributes fetching
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

#ifndef MEGA_FILEATTRIBUTEFETCH_H
#define MEGA_FILEATTRIBUTEFETCH_H 1

#include "backofftimer.h"
#include "types.h"
#include "http.h"

namespace mega {

// file attribute fetching for a specific source cluster
struct FileAttributeFetchChannel
{
	handle fahref;
	BackoffTimer bt;
	HttpReq req;

	// post request to target URL
	void dispatch(MegaClient*, int, const char*);

	// parse fetch result and remove completed attributes from pending
	void parse(MegaClient*, int, string*);

	FileAttributeFetchChannel();
};

// pending individual attribute fetch
struct FileAttributeFetch
{
	handle nodehandle;
	fatype type;
	int fac;		// attribute cluster ID
	unsigned char dispatched;
	unsigned char retries;
	int tag;

	FileAttributeFetch(handle, fatype, int, int);
};

} // namespace

#endif
