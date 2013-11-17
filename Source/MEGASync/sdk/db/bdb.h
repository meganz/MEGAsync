/*

MEGA SDK 2013-11-16 - sample application, Berkeley DB access

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

#ifdef USE_BDB
#ifndef DBACCESS_CLASS
#define DBACCESS_CLASS BdbAccess

#include <db_cxx.h>

class BdbAccess : public DbAccess
{
	DbEnv* env;
	string dbpathprefix;

public:
	DbTable* open(FileSystemAccess*, string*);

	BdbAccess(string* = NULL);
	~BdbAccess();
};

class BdbTable : public DbTable
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

#endif
#endif
