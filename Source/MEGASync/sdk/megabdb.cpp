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

#include <sys/stat.h>

#include "mega.h"
#include "megaclient.h"
#include "megabdb.h"

BdbAccess::BdbAccess()
{
	env = NULL;
}

BdbAccess::~BdbAccess()
{
	if (env) env->close(0);
}

DbTable* BdbAccess::open(string* name)
{
	if (env) env->close(0);

	string dbdir = "megaclient_statecache_";
	
	dbdir.append(*name);
	MegaClient::escapefilename(&dbdir);
	mkdir(dbdir.c_str(),0700);

	env = new DbEnv(0);
	env->open(dbdir.c_str(),DB_CREATE|DB_REGISTER|DB_INIT_TXN|DB_INIT_MPOOL|DB_INIT_LOCK|DB_RECOVER,0);

	return new BdbTable(env);
}

// tablename can be modified - do not reuse
BdbTable::BdbTable(DbEnv* env)
{
	db = new Db(env,0);
    db->open(NULL,"statecache",NULL,DB_BTREE,DB_CREATE|DB_DIRTY_READ|DB_AUTO_COMMIT,0);
	dbenv = env;
	dbtxn = NULL;
	dbcursor = NULL;

	nextid = 0;
}

BdbTable::~BdbTable()
{
	if (dbcursor) dbcursor->close();

	abort();

	if (db)
	{
		db->close(0);
		delete db;
    }
}

// set cursor to first record
void BdbTable::rewind()
{
	if (dbcursor) dbcursor->close();

	db->cursor(dbtxn,&dbcursor,DB_DIRTY_READ);
}

// retrieve next record through cursor
int BdbTable::next(uint32_t* index, string* data)
{
	if (!dbcursor) return 0;

	Dbt key, value;

	if (dbcursor->get(&key,&value,DB_NEXT|DB_DIRTY_READ)) return 0;

	if (sizeof(*index) != key.get_size()) return 0;

	*index = *(uint32_t*)key.get_data();

	data->assign((char*)value.get_data(),value.get_size());

	return 1;
}

// retrieve record by index
int BdbTable::get(uint32_t index, string* data)
{
	Dbt key((char*)&index,sizeof index), value;

	if (db->get(dbtxn,&key,&value,DB_READ_UNCOMMITTED)) return 0;

	data->assign((char*)value.get_data(),value.get_size());

	return 1;
}

// add/update record by index
int BdbTable::put(uint32_t index, char* data, unsigned len)
{
	if (dbcursor)
	{
		dbcursor->close();
		dbcursor = NULL;
	}

	Dbt key((char*)&index,sizeof index), value(data,len);

	if (db->put(dbtxn,&key,&value,0)) return 0;

	return 1;
}

// delete record by index
int BdbTable::del(uint32_t index)
{
	Dbt key((char*)&index,sizeof index);

	db->del(dbtxn,&key,0);

	return 1;
}

// truncate table
void BdbTable::truncate()
{
	u_int32_t count;

	if (dbcursor)
	{
		dbcursor->close();
		dbcursor = NULL;
	}

	db->truncate(dbtxn,&count,0);
}

// begin transaction
void BdbTable::begin()
{
	dbenv->txn_begin(NULL,&dbtxn,DB_TXN_SYNC|DB_READ_UNCOMMITTED);
}

// commit transaction
void BdbTable::commit()
{
	if (dbtxn)
	{
		dbtxn->commit(DB_TXN_SYNC);
		dbtxn = NULL;
	}
}

// abort transaction
void BdbTable::abort()
{
	if (dbtxn)
	{
		dbtxn->abort();
		dbtxn = NULL;
	}
}
