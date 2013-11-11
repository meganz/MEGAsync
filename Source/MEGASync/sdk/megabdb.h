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

#ifndef MEGABDB_H
#define MEGABDB_H 1

#include <db_cxx.h>

class BdbAccess : public DbAccess
{
	DbEnv* env;

public:
	DbTable* open(string*);

	BdbAccess();
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
	int next(uint32_t*, string*);
	int get(uint32_t, string*);
	int put(uint32_t, char*, unsigned);
	int del(uint32_t);
	void truncate();
	void begin();
	void commit();
	void abort();

	uint32_t nextid;

	BdbTable(DbEnv*);
	~BdbTable();
};

#endif
