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

#ifdef ENABLE_LOCALCACHE

#include "mega.h"
#include "megaclient.h"
#include "megabdb.h"

// basepath is prepended to the name
BdbAccess::BdbAccess(string* cdbpathprefix)
{
	if (cdbpathprefix) dbpathprefix = *cdbpathprefix;
	else dbpathprefix = "megaclient_statecache_";	// FIXME: Unicode support for default prefix?
	
	env = NULL;
}

BdbAccess::~BdbAccess()
{
	if (env) env->close(0);
}

DbTable* BdbAccess::open(FileSystemAccess* fsaccess, string* name)
{
	if (env) env->close(0);

	string dbname = *name;
	fsaccess->name2local(&dbname);
	dbname.insert(0,dbpathprefix);
	fsaccess->mkdirlocal(&dbname);

	env = new DbEnv(DB_CXX_NO_EXCEPTIONS);

	env->set_lk_max_lockers(10000000);
	env->set_lk_max_locks(10000000);
	env->set_lk_max_objects(10000000);
	
	if (env->open(dbname.c_str(),DB_CREATE|DB_REGISTER|DB_INIT_TXN|DB_INIT_MPOOL|DB_INIT_LOCK|DB_RECOVER,0)) return NULL;

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
bool BdbTable::next(uint32_t* index, string* data)
{
	if (!dbcursor) return false;

	Dbt key, value;

	if (dbcursor->get(&key,&value,DB_NEXT|DB_DIRTY_READ)) return false;

	if (sizeof(*index) != key.get_size()) return false;

	*index = *(uint32_t*)key.get_data();

	data->assign((char*)value.get_data(),value.get_size());

	return true;
}

// retrieve record by index
bool BdbTable::get(uint32_t index, string* data)
{
	Dbt key((char*)&index,sizeof index), value;

	if (db->get(dbtxn,&key,&value,DB_READ_UNCOMMITTED)) return false;

	data->assign((char*)value.get_data(),value.get_size());

	return true;
}

// add/update record by index
bool BdbTable::put(uint32_t index, char* data, unsigned len)
{
	if (dbcursor)
	{
		dbcursor->close();
		dbcursor = NULL;
	}

	Dbt key((char*)&index,sizeof index), value(data,len);

	if (db->put(dbtxn,&key,&value,0)) return false;

	return true;
}

// delete record by index
bool BdbTable::del(uint32_t index)
{
	Dbt key((char*)&index,sizeof index);

	db->del(dbtxn,&key,0);

	return true;
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
#endif
