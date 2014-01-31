/**
 * @file mega/db.h
 * @brief Database access interface
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

#ifndef MEGA_DB_H
#define MEGA_DB_H 1

#include "filesystem.h"

namespace mega {

// generic host transactional database access interface
class MEGA_API DbTable
{
	static const int IDSPACING = 16;

public:
	// for a full sequential get: rewind to first record
	virtual void rewind() = 0;

	// get next record in sequence
	virtual bool next(uint32_t*, string*) = 0;
	bool next(uint32_t*, string*, SymmCipher*);

	// get specific record by key
	virtual bool get(uint32_t, string*) = 0;

	// update or add specific record
	virtual bool put(uint32_t, char*, unsigned) = 0;
	bool put(uint32_t, string*);
	bool put(uint32_t, Cachable*, SymmCipher*);

	// delete specific record
	virtual bool del(uint32_t) = 0;

	// delete all records
	virtual void truncate() = 0;

	// begin transaction
	virtual void begin() = 0;

	// commit transaction
	virtual void commit() = 0;

	// abort transaction
	virtual void abort() = 0;

	// autoincrement
	uint32_t nextid;

	DbTable();
	virtual ~DbTable() { }
};

struct MEGA_API DbAccess
{
	virtual DbTable* open(FileSystemAccess*, string*) = 0;

	virtual ~DbAccess() { }
};

} // namespace

#endif
