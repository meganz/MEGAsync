/**
 * @file bdb.h
 * @brief Berkeley DB access layer
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

#ifdef USE_BDB
#ifndef DBACCESS_CLASS
#define DBACCESS_CLASS BdbAccess

#include <db_cxx.h>

namespace mega {
class MEGA_API BdbAccess : public DbAccess
{
    DbEnv* env;
    string dbpathprefix;

public:
    DbTable* open(FileSystemAccess*, string*);

    BdbAccess(string* = NULL);
    ~BdbAccess();
};

class MEGA_API BdbTable : public DbTable
{
    Db* db;
    DbTxn* dbtxn;
    DbEnv* dbenv;
    Dbc* dbcursor;

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

    BdbTable(DbEnv*);
    ~BdbTable();
};
} // namespace

#endif
#endif
