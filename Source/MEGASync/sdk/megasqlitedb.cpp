/*

MEGA SDK 2013-10-03 - sample application, Berkeley DB access

(c) 2013 by Mega Limited, Wellsford, New Zealand

Applications using the MEGA API must present a valid application key
and comply with the the rules set forth in the Terms of Service.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#include "megasqlitedb.h"

SqliteDbAccess::SqliteDbAccess(const char *path)
{
	dbpath = string(path);
	db = NULL;
}

SqliteDbAccess::~SqliteDbAccess()
{
	if(db) sqlite3_close(db);
}

DbTable* SqliteDbAccess::open(string* name)
{
	if (db)
	{
		sqlite3_close(db);
		db = NULL;
	}

	string dbdir = dbpath + *name + ".db";
	int rc;
	rc = sqlite3_open(dbdir.c_str(), &db);
	if(rc) return NULL;

	const char *sql = "CREATE TABLE IF NOT EXISTS statecache ("  \
	         "ID INTEGER PRIMARY KEY ASC NOT NULL, " \
	         "CONTENT        BLOB    NOT NULL);";
	rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
	if( rc ) return NULL;
	return new SqliteDbTable(db);
}

SqliteDbTable::SqliteDbTable(sqlite3 *db)
{
	this->db = db;
	pStmt = NULL;
}

SqliteDbTable::~SqliteDbTable()
{
	if(pStmt) sqlite3_finalize(pStmt);
	abort();
}

// set cursor to first record
void SqliteDbTable::rewind()
{
	if(pStmt)
	{
		sqlite3_reset(pStmt);
		return;
	}

	const char *sql = "SELECT ID, CONTENT FROM statecache;";
	sqlite3_prepare(db, sql, -1, &pStmt, NULL);
}

// retrieve next record through cursor
int SqliteDbTable::next(uint32_t* index, string* data)
{
	if (!pStmt) return 0;

	int rc = sqlite3_step(pStmt);
	if(rc !=  SQLITE_ROW)
	{
		sqlite3_finalize(pStmt);
		pStmt = NULL;
		return 0;
	}

	*index = sqlite3_column_int(pStmt, 0);
	data->assign((char*)sqlite3_column_blob(pStmt, 1), sqlite3_column_bytes(pStmt, 1));
	return 1;
}

// retrieve record by index
int SqliteDbTable::get(uint32_t index, string* data)
{
	sqlite3_stmt *stmt;
	const char *sql = "SELECT CONTENT FROM statecache WHERE ID = ?;";
	int rc = sqlite3_prepare(db, sql, -1, &stmt, NULL);
	if( rc ) return 0;

	rc = sqlite3_bind_int(stmt, 1, index);
	if( rc ) return 0;

	rc = sqlite3_step(stmt);
	if(rc !=  SQLITE_ROW) return 0;

	data->assign((char*)sqlite3_column_blob(stmt, 0), sqlite3_column_bytes(stmt, 0));

	sqlite3_finalize(stmt);
	return 1;
}

// add/update record by index
int SqliteDbTable::put(uint32_t index, char* data, unsigned len)
{
	sqlite3_stmt *stmt;
	const char *sql = "INSERT OR REPLACE INTO statecache (ID, CONTENT) VALUES (?, ?);";
	int rc = sqlite3_prepare(db, sql, -1, &stmt, NULL);
	if( rc ) return 0;

	rc = sqlite3_bind_int(stmt, 1, index);
	if( rc ) return 0;

	rc = sqlite3_bind_blob(stmt, 2, data, len, SQLITE_STATIC);
	if( rc ) return 0;

	rc = sqlite3_step(stmt);
	if(rc !=  SQLITE_DONE) return 0;

	sqlite3_finalize(stmt);
	return 1;
}

// delete record by index
int SqliteDbTable::del(uint32_t index)
{
	string sql = "DELETE FROM statecache WHERE ID = ";
	sql += index + ";";
	int rc = sqlite3_exec(db, sql.c_str(), 0, 0, NULL);
	if( rc ) return 0;
	return 1;
}

// truncate table
void SqliteDbTable::truncate()
{
	sqlite3_exec(db, "DELETE FROM statecache;", 0, 0, NULL);
}

// begin transaction
void SqliteDbTable::begin()
{
	sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, NULL);
}

// commit transaction
void SqliteDbTable::commit()
{
	sqlite3_exec(db, "COMMIT TRANSACTION;", 0, 0, NULL);
}

// abort transaction
void SqliteDbTable::abort()
{
	sqlite3_exec(db, "ROLLBACK TRANSACTION;", 0, 0, NULL);
}
