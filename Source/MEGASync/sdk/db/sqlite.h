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

#ifdef USE_SQLITE
#ifndef DBACCESS_CLASS
#define DBACCESS_CLASS SqliteDbAccess

#include <sqlite3.h>

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

#endif
#endif
