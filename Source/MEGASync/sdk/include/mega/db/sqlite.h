/**
 * @file sqlite.h
 * @brief SQLite DB access layer
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

#ifdef USE_SQLITE
#ifndef DBACCESS_CLASS
#define DBACCESS_CLASS SqliteDbAccess

#include <sqlite3.h>

namespace mega {

class SqliteDbAccess : public DbAccess
{
    string dbpath;
    sqlite3* db;

public:
    DbTable* open(FileSystemAccess*, string*);

    SqliteDbAccess(string* = NULL);
	~SqliteDbAccess();
};

class SqliteDbTable : public DbTable
{
	sqlite3* db;
	sqlite3_stmt* pStmt;

public:
	void rewind();
	bool next(uint32_t*, string*);
	bool get(uint32_t, string*);
	bool put(uint32_t, char*, unsigned);
	bool del(uint32_t);
	void truncate();
	void begin();
	void commit();
	void abort();

	uint32_t nextid;

	SqliteDbTable(sqlite3*);
	~SqliteDbTable();
};

} // namespace

#endif
#endif
