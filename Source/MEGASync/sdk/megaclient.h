/*

MEGA SDK 2013-11-16 - Client Access Engine

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

#ifndef MEGACLIENT_H
#define MEGACLIENT_H 1

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <memory.h>
#include <time.h>

#ifdef _WIN32

#include <windows.h>
#define atoll _atoi64
#define snprintf _snprintf
#define _CRT_SECURE_NO_WARNINGS

#else

#include <unistd.h>
#include <arpa/inet.h>

#ifndef __MACH__
#include <endian.h>
#endif

#endif

// FIXME: #define PRI*64 if missing
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

typedef int64_t m_off_t;

#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>
#include <map>
#include <set>
#include <iterator>
#include <queue>
#include <list>

using namespace std;

#include "crypto/cryptopp.h"

// monotonously increasing time in deciseconds
typedef uint32_t dstime;

// wait for events
struct Waiter
{
	// current time
	dstime ds;

	// wait ceiling
	dstime maxds;

	// current time in deciseconds
	virtual dstime getdstime() = 0;

	// beging waiting cycle
	virtual void init(dstime) = 0;

	// add wakeup events
	void wakeupby(struct EventTrigger*);

	// wait for all added wakeup criteria (plus the host app's own), up to the specified number of deciseconds
	virtual int wait() = 0;

	static const int NEEDEXEC = 1;
	static const int HAVESTDIN = 2;
};

// interface enabling class to add its wakeup criteria to the waiter
struct EventTrigger
{
	virtual void addevents(Waiter*) = 0;
};

// generic host HTTP I/O interface
struct HttpIO : public EventTrigger
{
	// post request to target URL
	virtual void post(struct HttpReq*, const char* = NULL, unsigned = 0) = 0;

	// cancel request
	virtual void cancel(HttpReq*) = 0;

	// real-time POST progress information
	virtual m_off_t postpos(void*) = 0;

	// execute I/O operations
	virtual bool doio(void) = 0;

	virtual ~HttpIO() { }
};

// persistent resource cache storage
struct Cachable
{
	virtual bool serialize(string*) = 0;

	int32_t dbid;

	bool notified;

	Cachable();
	virtual ~Cachable() { }
};

typedef vector<struct Node*> node_vector;

// node types:
// FILE - regular file nodes
// FOLDER - regular folder nodes
// ROOT - the cloud drive root node
// INCOMING - inbox
// RUBBISH - rubbish bin
// MAIL - mail message
typedef enum { TYPE_UNKNOWN = -1, FILENODE = 0, FOLDERNODE, ROOTNODE, INCOMINGNODE, RUBBISHNODE, MAILNODE } nodetype;

typedef enum { SYNC_CANCELED = -1, SYNC_INITIALSCAN = 0, SYNC_ACTIVE, SYNC_FAILED } syncstate;

// generic host file access interface
struct FileAccess
{
	// file size
	m_off_t size;

	// mtime of a file opened for reading
	time_t mtime;

	// type of opened path
	nodetype type;

	// open for reading, writing or reading and writing
	virtual bool fopen(string*, bool, bool) = 0;

	// absolute position read, with NUL padding
	virtual bool fread(string*, unsigned, unsigned, m_off_t) = 0;

	// absolute position read to byte buffer
	virtual bool frawread(byte*, unsigned, m_off_t) = 0;

	// absolute position write
	virtual bool fwrite(const byte*, unsigned, m_off_t) = 0;

	virtual ~FileAccess() { }
};

// generic host directory enumeration
struct DirAccess
{
	// open for scanning
	virtual bool dopen(string*, FileAccess*, bool) = 0;

	// get next record
	virtual bool dnext(string*, nodetype* = NULL) = 0;

	virtual ~DirAccess() { }
};

typedef vector<struct LocalNode*> localnode_vector;

typedef list<class Sync*> sync_list;

// generic host filesystem access interface
struct FileSystemAccess : public EventTrigger
{
	// local path separator, e.g. "/"
	string localseparator;

	// instantiate FileAccess object
	virtual FileAccess* newfileaccess() = 0;

	// return DirAccess object or NULL if unsuccessful
	virtual DirAccess* newdiraccess() = 0;

	// check if character is lowercase hex ASCII
	bool islchex(char);

	// convert MEGA path (UTF-8) to local format
	virtual void path2local(string*, string*) = 0;

	// convert local path to MEGA format (UTF-8)
	virtual void local2path(string*, string*) = 0;

	// convert MEGA-formatted filename (UTF-8) to local filesystem name; escape forbidden characters using urlencode
	virtual void name2local(string*, const char* = NULL) = 0;

	// reverse local2name()
	virtual void local2name(string*) = 0;

	// generate local temporary file name
	virtual void tmpnamelocal(string*, string* = NULL) = 0;

	// check if local file/name is to be treated as hidden
	virtual bool localhidden(string*, string*) = 0;

	// rename file, overwrite target
	virtual bool renamelocal(string*, string*) = 0;

	// copy file, overwrite target
	virtual bool copylocal(string*, string*) = 0;

	// move file or folder tree to OS-managed local rubbish bin
	virtual bool rubbishlocal(string*) = 0;

	// delete file
	virtual bool unlinklocal(string*) = 0;

	// delete empty directory
	virtual bool rmdirlocal(string*) = 0;

	// create directory
	virtual bool mkdirlocal(string*) = 0;

	// set mtime
	virtual bool setmtimelocal(string*, time_t) = 0;

	// change working directory
	virtual bool chdirlocal(string*) = 0;

	// add notification (has to be called for all directories in tree for full crossplatform support)
	virtual void addnotify(LocalNode*, string*) = 0;

	// delete notification
	virtual void delnotify(LocalNode*) = 0;

	// return next notified local name and corresponding parent node
	virtual bool notifynext(sync_list*, string*, LocalNode**) = 0;

	// true if notifications were unreliable and/or a full rescan is required
	virtual bool notifyfailed() = 0;

	virtual ~FileSystemAccess() { }
};

// generic host transactional database access interface
class DbTable
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

struct DbAccess
{
	virtual DbTable* open(FileSystemAccess*, string*) = 0;

	virtual ~DbAccess() { }
};

struct Console
{
	virtual void readpwchar(char*, int, int* pw_buf_pos, char**) = 0;
	virtual void setecho(bool) = 0;

	virtual ~Console() { }
};

// error codes
typedef enum {
	API_OK = 0,
	API_EINTERNAL = -1,				// internal error
	API_EARGS = -2,					// bad arguments
	API_EAGAIN = -3,				// request failed, retry with exponential backoff
	API_ERATELIMIT = -4,			// too many requests, slow down
	API_EFAILED = -5,				// request failed permanently
	API_ETOOMANY = -6,				// too many requests for this resource
	API_ERANGE = -7,				// resource access out of rage
	API_EEXPIRED = -8,				// resource expired
	API_ENOENT = -9,				// resource does not exist
	API_ECIRCULAR = -10,			// circular linkage
	API_EACCESS = -11,				// access denied
	API_EEXIST = -12,				// resource already exists
	API_EINCOMPLETE = -13,			// request incomplete
	API_EKEY = -14,					// cryptographic error
	API_ESID = -15,					// bad session ID
	API_EBLOCKED = -16,				// resource administratively blocked
	API_EOVERQUOTA = -17,			// quote exceeded
	API_ETEMPUNAVAIL = -18,			// resource temporarily not available
	API_ETOOMANYCONNECTIONS = -19,	// too many connections on this resource
	API_EWRITE = -20,				// file could not be written to (or failed post-write integrity check)
	API_EREAD = -21,				// file could not be read from (or changed unexpectedly during reading)
	API_EAPPKEY = -22				// invalid or missing application key
} error;

// returned by loggedin()
typedef enum { NOTLOGGEDIN, EPHEMERALACCOUNT, CONFIRMEDACCOUNT, FULLACCOUNT } sessiontype;

// node/user handles are 8-11 base64 characters, case sensitive, and thus fit in a 64-bit int
typedef uint64_t handle;

// (can use unordered_set if available)
typedef set<handle> handle_set;

// enumerates a node's children
// FIXME: switch to forward_list once C++11 becomes more widely available
typedef list<Node*> node_list;

// numeric representation of string (up to 8 chars)
typedef uint64_t nameid;

// maps attribute names to attribute values
typedef map<nameid,string> attr_map;

struct AttrMap
{
	attr_map map;

	// compute rough storage size
	unsigned storagesize(int);

	// convert nameid to string
	int nameid2string(nameid, char*);

	// export as JSON string
	void getjson(string*);

	// export as raw binary serialize
	void serialize(string*);

	// import raw binary serialize
	const char* unserialize(const char*, unsigned);
};

// contact visibility:
// HIDDEN - not shown
// VISIBLE - shown
typedef enum { VISIBILITY_UNKNOWN = -1, HIDDEN = 0, VISIBLE, ME } visibility;

// user/contact
struct User : public Cachable
{
	// user handle
	handle userhandle;

	// string identifier for API requests (either e-mail address or ASCII user handle)
	string uid;

	// e-mail address
	string email;

	// persistent attributes (n = name, a = avatar)
	AttrMap attrs;

	// visibility status
	visibility show;

	// shares by this user
	handle_set sharing;

	// contact establishment timestamp
	time_t ctime;

	// user's public key
	AsymmCipher pubk;
	int pubkrequested;

	// actions to take after arrival of the public key
	deque<class PubKeyAction*> pkrs;

	void set(visibility, time_t);

	bool serialize(string*);
	static User* unserialize(class MegaClient*, string*);

	User(const char* = NULL);
};

typedef enum { REQ_BINARY, REQ_JSON } contenttype;

typedef enum { UPLOAD, DOWNLOAD } transfertype;

// new node source types
typedef enum { NEW_NODE, NEW_PUBLIC, NEW_UPLOAD } newnodesource;

// access levels:
// RDONLY - cannot add, rename or delete
// RDWR - cannot rename or delete
// FULL - all operations that do not require ownership permitted
// OWNER - node is in caller's ROOT, INCOMING or RUBBISH trees
typedef enum { ACCESS_UNKNOWN = -1, RDONLY = 0, RDWR, FULL, OWNER } accesslevel;

// HttpReq states
typedef enum { REQ_READY, REQ_PREPARED, REQ_INFLIGHT, REQ_SUCCESS, REQ_FAILURE, REQ_DONE } reqstatus;

typedef enum { SHARE, SHAREOWNERKEY, OUTSHARE } sharereadmode;

typedef enum { USER_HANDLE, NODE_HANDLE } targettype;

// undefined node handle
const handle UNDEF = ~(handle)0;

#define ISUNDEF(h) (!((h)+1))

// maps node handles to Node pointers
typedef map<handle,Node*> node_map;

// maps node handles to Share pointers
typedef map<handle,struct Share*> share_map;

// maps node handles NewShare pointers
typedef list<struct NewShare*> newshare_list;

// generic handle vector
typedef vector<handle> handle_vector;

// pairs of node handles
typedef set<pair<handle,handle> > handlepair_set;

// node and user vectors
typedef vector<struct NodeCore*> nodecore_vector;
typedef vector<struct User*> user_vector;

// actual user data (indexed by userid)
typedef map<int,User> user_map;

// maps user handles to userids
typedef map<handle,int> uh_map;

// maps lowercase user e-mail addresses to userids
typedef map<string,int> um_map;

// file attribute data
typedef map<unsigned,string> fadata_map;

// syncid to node handle mapping
typedef map<handle,handle> syncidhandle_map;

// NewNodes index to syncid mapping
typedef map<int,handle> newnodesyncid_map;

// for dynamic node addition requests, used by the sync subsystem
typedef vector<struct NewNode*> newnode_vector;

// file chunk MAC
struct ChunkMAC
{
	byte mac[SymmCipher::BLOCKSIZE];
};

// file chunk macs
typedef map<m_off_t,ChunkMAC> chunkmac_map;

// convert
#define MAKENAMEID2(a,b) (nameid)(((a)<<8)+(b))
#define MAKENAMEID3(a,b,c) (nameid)(((a)<<16)+((b)<<8)+(c))
#define MAKENAMEID4(a,b,c,d) (nameid)(((a)<<24)+((b)<<16)+((c)<<8)+(d))
#define MAKENAMEID5(a,b,c,d,e) (nameid)((((uint64_t)a)<<32)+((b)<<24)+((c)<<16)+((d)<<8)+(e))
#define MAKENAMEID6(a,b,c,d,e,f) (nameid)((((uint64_t)a)<<40)+(((uint64_t)b)<<32)+((c)<<24)+((d)<<16)+((e)<<8)+(f))
#define MAKENAMEID7(a,b,c,d,e,f,g) (nameid)((((uint64_t)a)<<48)+(((uint64_t)b)<<40)+(((uint64_t)c)<<32)+((d)<<24)+((e)<<16)+((f)<<8)+(g))
#define MAKENAMEID8(a,b,c,d,e,f,g,h) (nameid)((((uint64_t)a)<<56)+(((uint64_t)b)<<48)+(((uint64_t)c)<<40)+(((uint64_t)d)<<32)+((e)<<24)+((f)<<16)+((g)<<8)+(h))

#define EOO 0

// request command component
class Command
{
	static const int MAXDEPTH = 8;

	char levels[MAXDEPTH];

	error result;

protected:
	bool canceled;

	string json;

public:
	MegaClient* client;

	int tag;

	char level;
	char persistent;

	void cmd(const char*);
	void notself(MegaClient*);
	void cancel(void);

	void arg(const char*, const char*, int = 1);
	void arg(const char*, const byte*, int);
	void arg(const char*, m_off_t);
	void addcomma();
	void appendraw(const char*);
	void appendraw(const char*, int);
	void beginarray();
	void beginarray(const char*);
	void endarray();
	void beginobject();
	void endobject();
	void element(int);
	void element(handle, int = sizeof(handle));
	void element(const byte*, int);

	void openobject();
	void closeobject();
	int elements();

	virtual void procresult();

	const char* getstring();

	Command();
	virtual ~Command() { };
};

// linear non-strict JSON scanner
struct JSON
{
	const char* pos;	// make private

	bool isnumeric();

	void begin(const char*);

	m_off_t getint();
	double getfloat();
	const char* getvalue();

	nameid getnameid();
	nameid getnameid(const char*);

	bool is(const char*);

	int storebinary(byte*, int);
	bool storebinary(string*);

	handle gethandle(int = 6);

	bool enterarray();
	bool leavearray();

	bool enterobject();
	bool leaveobject();

	bool storestring(string*);
	bool storeobject(string* = NULL);

	static void unescape(string*);
};

struct ChunkedHash
{
	static const int SEGSIZE = 131072;

	static m_off_t chunkfloor(m_off_t);
	static m_off_t chunkceil(m_off_t);
};

// modified base64 encoding/decoding (unpadded, -_ instead of +/)
class Base64
{
	static byte to64(byte);
	static byte from64(byte);

public:
	static int btoa(const byte*, int, char*);
	static int atob(const char*, byte*, int);
};

// 64-bit int serialization/unserialization
struct Serialize64
{
	static int serialize(byte*, int64_t);
	static int unserialize(byte*, int, int64_t*);
};

// padded CBC encryption
struct PaddedCBC
{
	static void encrypt(string*, SymmCipher*);
	static bool decrypt(string*, SymmCipher*);
};

// API request
class Request
{
	vector<Command*> cmds;

public:
	void add(Command*);

	int cmdspending();

	int get(string*);

	void procresult(MegaClient*);

	void clear();
};

// share credentials
struct Share
{
	accesslevel access;
	User* user;
	time_t ts;

	void removeshare(handle);
	void update(accesslevel, time_t);

	void serialize(string*);
	static bool unserialize(MegaClient*, int, handle, const byte*, const char**, const char*);

	Share(User*, accesslevel, time_t);
};

// new share credentials (will be merged into node as soon as it appears)
struct NewShare
{
	handle h;
	int outgoing;
	handle peer;
	accesslevel access;
	time_t ts;

	bool have_key, have_auth;

	byte key[SymmCipher::BLOCKSIZE];
	byte auth[SymmCipher::BLOCKSIZE];

	NewShare(handle, int, handle, accesslevel, time_t, const byte*, const byte* = NULL);
};

// outgoing HTTP request
struct HttpReq
{
	reqstatus status;

	int httpstatus;

	contenttype type;

	string posturl;

	string* out;
	string in;

	string outbuf;

	byte* buf;
	unsigned buflen, bufpos;

	// HttpIO implementation-specific identifier for this connection
	void* httpiohandle;

	// while this request is in flight, points to the application's HttpIO object - NULL otherwise
	HttpIO* httpio;

	// set url and content type for subsequent requests
	void setreq(const char*, contenttype);

	// post request to the network
	void post(MegaClient*, const char* = NULL, unsigned = 0);

	// store chunk of incoming data
	void put(void*, unsigned);

	// reserve space for incoming data
	byte* reserveput(unsigned* len);

	// confirm receipt of data in reserved space
	void completeput(unsigned len);

	// disconnect open HTTP connection
	void disconnect();

	// progress information
	virtual m_off_t transferred(MegaClient*);

	// timestamp of last data received
	dstime lastdata;

	// prevent raw data serialize for debug output
	int binary;

	HttpReq(int = 0);
	virtual ~HttpReq();
};

// file chunk I/O
struct HttpReqXfer : public HttpReq
{
	unsigned size;

	virtual void prepare(FileAccess*, const char*, SymmCipher*, chunkmac_map*, uint64_t, m_off_t, m_off_t) = 0;
	virtual void finalize(FileAccess*, SymmCipher*, chunkmac_map*, uint64_t, m_off_t, m_off_t) { }

	HttpReqXfer() : HttpReq(1) { };
};

// file chunk upload
struct HttpReqUL : public HttpReqXfer
{
	void prepare(FileAccess*, const char*, SymmCipher*, chunkmac_map*, uint64_t, m_off_t, m_off_t);

	m_off_t transferred(MegaClient*);

	~HttpReqUL() { };
};

// file chunk download
struct HttpReqDL : public HttpReqXfer
{
	m_off_t dlpos;

	void prepare(FileAccess*, const char*, SymmCipher*, chunkmac_map*, uint64_t, m_off_t, m_off_t);
	void finalize(FileAccess*, SymmCipher*, chunkmac_map*, uint64_t, m_off_t, m_off_t);

	~HttpReqDL() { };
};

// file attribute type
typedef uint16_t fatype;

#define THUMBNAIL120X120 0

// maps handle-index pairs to file attribute handle
typedef map<pair<handle,fatype>,pair<handle,int> > fa_map;

// list of new file attributes to write
// file attribute put
struct HttpReqCommandPutFA : public HttpReq, public Command
{
	handle th;
	fatype type;
	byte* data;
	unsigned len;

	void procresult();

	HttpReqCommandPutFA(MegaClient*, handle, fatype, byte*, unsigned len);
	~HttpReqCommandPutFA();
};

// FIXME: use forward_list instad (C++11)
typedef list<HttpReqCommandPutFA*> putfa_list;

// file attribute get
struct HttpReqGetFA : public HttpReq
{
	~HttpReqGetFA() { };
};

class CommandGetFA : public Command
{
	int part;
	handle fahref;

public:
	void procresult();

	CommandGetFA(int, handle);
};

// generic timer facility with exponential backoff
class BackoffTimer
{
	dstime next;
	dstime delta;

public:
	// reset timer
	void reset();

	// trigger exponential backoff
	void backoff(dstime);

	// set absolute backoff
	void backoff(dstime, dstime);

	// check if timer has elapsed
	bool armed(dstime) const;

	// arm timer
	bool arm(dstime);

	// time left for event to become armed
	dstime retryin(dstime);

	// current backoff delta
	dstime backoff();

	// put on hold indefinitely
	void freeze();

	// time of next trigger or 0 if no trigger since last backoff
	dstime nextset() const;

	// update time to wait
	void update(dstime, dstime*);

	BackoffTimer();
};

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

// file attribute fetch map
typedef map<handle,FileAttributeFetch*> faf_map;

// file attribute fetch channel map
typedef map<int,FileAttributeFetchChannel*> fafc_map;

// transfer type
typedef enum { GET, PUT } direction;

// sparse file fingerprint, including size & mtime
struct FileFingerprint
{
	m_off_t size;
	time_t mtime;
	byte crc[32];

	// if true, represents actual file data
	// if false, constructed from node ctime/key
	bool isvalid;

	bool genfingerprint(FileAccess*);
	void serializefingerprint(string*);
	int unserializefingerprint(string*);

	FileFingerprint& operator=(FileFingerprint&);

	FileFingerprint();
};

// orders transfers by file fingerprints, ordered by size / mtime / sparse CRC
struct FileFingerprintCmp
{
    bool operator() (const FileFingerprint* a, const FileFingerprint* b) const;
};

// maps FileFingerprints to node
typedef multiset<FileFingerprint*, FileFingerprintCmp> fingerprint_set;

bool operator==(FileFingerprint&, FileFingerprint&);

struct NodeCore
{
	static const int FILENODEKEYLENGTH = 32;
	static const int FOLDERNODEKEYLENGTH = 16;

	// node's own handle
	handle nodehandle;

	// parent node handle (in a Node context, temporary placeholder until parent is set)
	handle parenthandle;

	// node type
	nodetype type;

	// full folder/file key, symmetrically or asymmetrically encrypted
	string nodekey;

	// new node's client-controlled timestamp (should be last modification)
	time_t clienttimestamp;

	// node attributes
	string attrstring;
};

// new node for putnodes()
struct NewNode : public NodeCore
{
	static const int UPLOADTOKENLEN = 27;

	newnodesource source;

	handle uploadhandle;
	byte uploadtoken[UPLOADTOKENLEN];

	handle syncid;
	LocalNode* localnode;

	NewNode() { syncid = UNDEF; }
};

// filesystem node
struct Node : public NodeCore, Cachable, FileFingerprint
{
	MegaClient* client;

	// node crypto keys
	string keystring;

	// change parent node association
	bool setparent(Node*);

	// copy JSON-delimited string
	static void copystring(string*, const char*);

	// try to resolve node key string
	bool applykey();

	// decrypt attribute string and set fileattrs
	void setattr();

	// display name (UTF-8)
	const char* displayname();

	// node-specific key
	SymmCipher key;

	// node attributes
	AttrMap attrs;

	// owner
	handle owner;

	// actual time this node was created
	time_t ctime;

	// FILENODE nodes only: size, fingerprint, nonce, meta MAC, attribute string
	int64_t ctriv;
	int64_t metamac;

	string fileattrstring;

	// check presence of file attribute
	int hasfileattribute(fatype) const;

	// decrypt node attribute string
	static byte* decryptattr(SymmCipher*, const char*, int);

	// inbound share
	Share* inshare;

	// outbound shares by user
	share_map outshares;

	// incoming/outgoing share key
	SymmCipher* sharekey;

	// app-private pointer
	void* appdata;

	bool removed;

	void setkey(const byte* = NULL);

	void setfingerprint();

	void faspec(string*);

	// parent
	Node* parent;

	// children
	node_list children;

	// own position in parent's children
	node_list::iterator child_it;

	// own position in fingerprint set (only valid for file nodes)
	fingerprint_set::iterator fingerprint_it;

	// related synced item or NULL
	LocalNode* localnode;

	// active sync get
	struct SyncFileGet* syncget;

	// source tag
	int tag;
	
	// check if node is below this node
	bool isbelow(Node*) const;

	bool serialize(string*);
	static Node* unserialize(MegaClient*, string*, node_vector*);

	Node(MegaClient*, vector<Node*>*, handle, handle, nodetype, m_off_t, handle, const char*, time_t);
	~Node();
};

typedef list<struct TransferSlot*> transferslot_list;

// active transfer
struct TransferSlot
{
	// link to related transfer (never NULL)
	struct Transfer* transfer;

	// associated source/destination file
	FileAccess* file;

	// command in flight to obtain temporary URL
	Command* pendingcmd;

	// transfer attempts are considered failed after XFERTIMEOUT seconds without data flow
	static const dstime XFERTIMEOUT = 600;

	m_off_t progressreported, progresscompleted;

	dstime starttime, lastdata;

	// upload result
	byte ultoken[NewNode::UPLOADTOKENLEN+1];

	// file attribute string
	string fileattrstring;

	// file attributes mutable
	int fileattrsmutable;

	// storage server access URL
	string tempurl;

	// maximum number of parallel connections and connection aray
	int connections;
	HttpReqXfer** reqs;

	// handle I/O for this slot
	void doio(MegaClient*);

	// disconnect and reconnect all open connections for this transfer
	void disconnect();

	// indicate progress
	void progress();

	// compute the meta MAC based on the chunk MACs
	int64_t macsmac(chunkmac_map*);

	// tslots list position
	transferslot_list::iterator slots_it;

	TransferSlot(Transfer*);
	~TransferSlot();
};

// list of files
typedef list<struct File*> file_list;

// map a FileFingerprint to the transfer for that FileFingerprint
typedef map<FileFingerprint*,Transfer*,FileFingerprintCmp> transfer_map;

// pending/active up/download ordered by file fingerprint (size - mtime - sparse CRC)
struct Transfer : public FileFingerprint
{
	// PUT or GET
	direction type;

	// transfer slot this transfer is active in (can be NULL if still queued)
	TransferSlot* slot;

	// files belonging to this transfer - transfer terminates upon its last file is removed
	file_list files;

	// failures/backoff
	unsigned failcount;
	BackoffTimer bt;

	// representative local filename for this transfer
	string localfilename;

	m_off_t pos, size;

	byte filekey[Node::FILENODEKEYLENGTH];

	// CTR mode IV
	int64_t ctriv;

	// meta MAC
	int64_t metamac;

	// file crypto key
	SymmCipher key;

	chunkmac_map chunkmacs;

	// upload handle for file attribute attachment (only set if file attribute queued)
	handle uploadhandle;

	// signal failure
	void failed(error);

	// signal completion
	void complete();

	// position in transfers[type]
	transfer_map::iterator transfers_it;

	// backlink to base
	MegaClient* client;

	Transfer(MegaClient*, direction);
	virtual ~Transfer();
};

// file to be transferred
struct File : public FileFingerprint
{
	// set localfilename in attached transfer
	virtual void prepare();

	// file transfer dispatched, expect updates/completion/failure
	virtual void start();

	// progress update
	virtual void progress();

	// transfer completion
	virtual void completed(Transfer*, LocalNode*);

	// transfer failed
	virtual bool failed(error);

	// generic filename for this transfer
	void displayname(string*);

	// normalized name (UTF-8 with unescaped special chars)
	string name;

	// source/target node handle
	handle h;

	// for remote file drops: uid or e-mail address of recipient
	string targetuser;

	// transfer linkage
	Transfer* transfer;
	file_list::iterator file_it;

	// local filename (must be set upon injection for uploads, can be set in start() for downloads)
	string localfilename;

	File();
	virtual ~File();
};

// action to be performed upon arrival of a user's public key
class PubKeyAction
{
public:
	int tag;

	virtual void proc(MegaClient*, User*) = 0;

	virtual ~PubKeyAction() { }
};

class PubKeyActionCreateShare : public PubKeyAction
{
	handle h;	// node to create share on
	accesslevel a;	// desired access level

public:
	void proc(MegaClient*, User*);

	PubKeyActionCreateShare(handle, accesslevel, int);
};

class PubKeyActionSendShareKey : public PubKeyAction
{
	handle sh;	// share node the key was requested on

public:
	void proc(MegaClient*, User*);

	PubKeyActionSendShareKey(handle);
};

class PubKeyActionPutNodes : public PubKeyAction
{
	NewNode* nn;	// nodes to add
	int nc;			// number of nodes to add

public:
	void proc(MegaClient*, User*);

	PubKeyActionPutNodes(NewNode*, int, int);
};

// log into full account (ephemeral sessions are curently unsupported)
class CommandLogin : public Command
{
public:
	void procresult();

	CommandLogin(MegaClient*, const char*, uint64_t);
};

class CommandSetMasterKey : public Command
{
public:
	void procresult();

	CommandSetMasterKey(MegaClient*, const byte*, const byte*, uint64_t);
};

class CommandCreateEphemeralSession : public Command
{
	byte pw[SymmCipher::KEYLENGTH];

public:
	void procresult();

	CommandCreateEphemeralSession(MegaClient*, const byte*, const byte*, const byte*);
};

class CommandResumeEphemeralSession : public Command
{
	byte pw[SymmCipher::KEYLENGTH];
	handle uh;

public:
	void procresult();

	CommandResumeEphemeralSession(MegaClient*, handle, const byte*);
};

class CommandSendSignupLink : public Command
{
public:
	void procresult();

	CommandSendSignupLink(MegaClient*, const char*, const char*, byte*);
};

class CommandQuerySignupLink : public Command
{
	string confirmcode;

public:
	void procresult();

	CommandQuerySignupLink(MegaClient*, const byte*, unsigned);
};

class CommandConfirmSignupLink : public Command
{
public:
	void procresult();

	CommandConfirmSignupLink(MegaClient*, const byte*, unsigned, uint64_t);
};

class CommandSetKeyPair : public Command
{
public:
	void procresult();

	CommandSetKeyPair(MegaClient*, const byte*, unsigned, const byte*, unsigned);
};

// invite contact/set visibility
class CommandUserRequest : public Command
{
public:
	void procresult();

	CommandUserRequest(MegaClient*, const char*, visibility);
};

// set user attributes
class CommandPutUA : public Command
{
public:
	CommandPutUA(MegaClient*, const char*, const byte*, unsigned);

	void procresult();
};

class CommandGetUA : public Command
{
	int priv;

public:
	CommandGetUA(MegaClient*, const char*, const char*, int);

	void procresult();
};

// reload nodes/shares/contacts
class CommandFetchNodes : public Command
{
public:
	void procresult();

	CommandFetchNodes(MegaClient*);
};

// update own node keys
class CommandNodeKeyUpdate : public Command
{
public:
	CommandNodeKeyUpdate(MegaClient*, handle_vector*);
};

class CommandShareKeyUpdate : public Command
{
public:
	CommandShareKeyUpdate(MegaClient*, handle, const char*, const byte*, int);
	CommandShareKeyUpdate(MegaClient*, handle_vector*);
};

class CommandKeyCR : public Command
{
public:
	CommandKeyCR(MegaClient*, node_vector*, node_vector*, const char*);
};

class CommandMoveNode : public Command
{
	handle h;

public:
	void procresult();

	CommandMoveNode(MegaClient*, Node*, Node*);
};

class CommandMoveSyncDebris : public Command
{
	handle h;

public:
	void procresult();

	CommandMoveSyncDebris(MegaClient*, handle, Node*);
};

class CommandSingleKeyCR : public Command
{
public:
	CommandSingleKeyCR(handle, handle, const byte*, unsigned);
};

class CommandDelNode : public Command
{
	handle h;

public:
	void procresult();

	CommandDelNode(MegaClient*, handle);
};

class CommandPubKeyRequest : public Command
{
	User* u;

public:
	void procresult();

	CommandPubKeyRequest(MegaClient*, User*);
};

class CommandGetFile : public Command
{
	TransferSlot* ts;

public:
	void procresult();

	CommandGetFile(TransferSlot* ts, handle, int);
};

class CommandPutFile : public Command
{
	TransferSlot* ts;

public:
	void procresult();

	CommandPutFile(TransferSlot* ts, int);
};

class CommandAttachFA : public Command
{
	handle h;
	fatype type;

public:
	void procresult();

	CommandAttachFA(handle, fatype, handle, int);
};

typedef enum { PUTNODES_APP, PUTNODES_SYNC, PUTNODES_SYNCDEBRIS } putsource;

class CommandPutNodes : public Command
{
	NewNode* nn;
	targettype type;
	putsource source;

public:
	void procresult();

	CommandPutNodes(MegaClient*, handle, const char*, NewNode*, int, int, putsource = PUTNODES_APP);
};

class CommandSetAttr : public Command
{
	handle h;

public:
	void procresult();

	CommandSetAttr(MegaClient*, Node*);
};

class CommandSetShare : public Command
{
	handle sh;
	User* user;
	accesslevel access;

	bool procuserresult(MegaClient*);

public:
	void procresult();

	CommandSetShare(MegaClient*, Node*, User*, accesslevel, int);
};

// account details/history
struct AccountBalance
{
	double amount;
	char currency[4];
};

struct AccountSession
{
	time_t timestamp, mru;
	string useragent;
	string ip;
	char country[3];
	int current;
};

struct AccountPurchase
{
	time_t timestamp;
	char handle[12];
	char currency[4];
	double amount;
	int method;
};

struct AccountTransaction
{
	time_t timestamp;
	char handle[12];
	char currency[4];
	double delta;
};

struct AccountDetails
{
	// subscription information (summarized)
	int pro_level;
	char subscription_type;

	time_t pro_until;

	// quota related to the session account
	m_off_t storage_used, storage_max;
	m_off_t transfer_own_used, transfer_srv_used, transfer_max;
	m_off_t transfer_own_reserved, transfer_srv_reserved;
	double srv_ratio;

	// transfer history pertaining to requesting IP address
	time_t transfer_hist_starttime;		// transfer history start timestamp
	time_t transfer_hist_interval;		// timespan that a single transfer window record covers
	vector<m_off_t> transfer_hist;		// transfer window - oldest to newest, bytes consumed per twrtime interval

	m_off_t transfer_reserved;			// byte quota reserved for the completion of active transfers

	m_off_t transfer_limit;				// current byte quota for the requesting IP address (dynamic, overage will be drawn from account quota)

	vector<AccountBalance> balances;
	vector<AccountSession> sessions;
	vector<AccountPurchase> purchases;
	vector<AccountTransaction> transactions;
};

class CommandGetUserQuota : public Command
{
	AccountDetails* details;

public:
	void procresult();

	CommandGetUserQuota(MegaClient*, AccountDetails*, bool, bool, bool);
};

class CommandGetUserTransactions : public Command
{
	AccountDetails* details;

public:
	void procresult();

	CommandGetUserTransactions(MegaClient*, AccountDetails*);
};

class CommandGetUserPurchases : public Command
{
	AccountDetails* details;

public:
	void procresult();

	CommandGetUserPurchases(MegaClient*, AccountDetails*);
};

class CommandGetUserSessions : public Command
{
	AccountDetails* details;

public:
	void procresult();

	CommandGetUserSessions(MegaClient*, AccountDetails*);
};

class CommandSetPH : public Command
{
	handle h;

public:
	void procresult();

	CommandSetPH(MegaClient*, Node*, int);
};

class CommandGetPH : public Command
{
	handle ph;
	byte key[Node::FILENODEKEYLENGTH];

public:
	void procresult();

	CommandGetPH(MegaClient*, handle, const byte*);
};

class CommandPurchaseAddItem : public Command
{
public:
	void procresult();

	CommandPurchaseAddItem(MegaClient*, int, handle, unsigned, char*, unsigned, char*, char*);
};

class CommandPurchaseCheckout : public Command
{
public:
	void procresult();

	CommandPurchaseCheckout(MegaClient*, int);
};

// cr element share/node map key generator
class ShareNodeKeys
{
	node_vector shares;
	vector<string> items;

	string keys;

	int addshare(Node*);

public:
	void add(Node*, Node*, int);
	void add(NodeCore*, Node*, int, const byte* = NULL, int = 0);

	void get(Command*);
};

// node tree processor
class TreeProc
{
public:
	virtual void proc(MegaClient*, Node*) = 0;

	virtual ~TreeProc() { }
};

class TreeProcDel : public TreeProc
{
public:
	void proc(MegaClient*, Node*);
};

class TreeProcListOutShares : public TreeProc
{
public:
	void proc(MegaClient*, Node*);
};

class TreeProcCopy : public TreeProc
{
public:
	NewNode* nn;
	unsigned nc;

	void allocnodes(void);

	void proc(MegaClient*, Node*);
	TreeProcCopy();
	~TreeProcCopy();
};

class TreeProcDU : public TreeProc
{
public:
	m_off_t numbytes;
	int numfiles;
	int numfolders;

	void proc(MegaClient*, Node*);
	TreeProcDU();
};

class TreeProcShareKeys : public TreeProc
{
	ShareNodeKeys snk;
	Node* sn;

public:
	void proc(MegaClient*, Node*);
	void get(Command*);

	TreeProcShareKeys(Node* = NULL);
};

typedef set<pair<int,handle> > fareq_set;

struct StringCmp
{
    bool operator() (const string* a, const string* b) const
	{
        return *a < *b;
    }
};

typedef map<const string*,LocalNode*,StringCmp> localnode_map;
typedef map<const string*,Node*,StringCmp> remotenode_map;

struct LocalNode : public File
{
	class Sync* sync;

	// parent linkage
	LocalNode* parent;

	// children by name
	localnode_map children;

	// related cloud node, if any
	Node* node;

	// original local fs name
	string localname;

	// FILENODE or FOLDERNODE
	nodetype type;

	// correspondin remote node
	handle nodehandle;

	// global sync reference
	handle syncid;

	// for folders: generic OS filesystem notification handle/pointer
	void* notifyhandle;

	// build full local path to this node
	void getlocalpath(MegaClient*, string*);

	void prepare();
	void completed(Transfer*, LocalNode*);

	LocalNode(Sync*, string*, string*, nodetype, LocalNode*, handle);
	~LocalNode();
};

// FIXME: use forward_list instead
typedef list<NewNode*> newnode_list;
typedef list<handle> handle_list;

typedef map<handle,NewNode*> handlenewnode_map;

typedef map<handle,char> handlecount_map;

class MegaClient
{
public:
	// own identity
	handle me;

	// root nodes (files, incoming, rubbish, mail)
	handle rootnodes[4];

	// all nodes
	node_map nodes;

	// all users
	user_map users;

	// process API requests and HTTP I/O
	void exec();

	// wait for I/O or other events
	int wait();

	// abort exponential backoff
	bool abortbackoff();

	// ID tag of the next request
	int nextreqtag();

	// corresponding ID tag of the currently executing callback
	int restag;

	// ephemeral session support
	void createephemeral();
	void resumeephemeral(handle, const byte*);

	// full account confirmation/creation support
	void sendsignuplink(const char*, const char*, const byte*);
	void querysignuplink(const byte*, unsigned);
	void confirmsignuplink(const byte*, unsigned, uint64_t);
	void setkeypair();

	// user login: e-mail, pwkey
	void login(const char*, const byte*, bool = false);

	// check if logged in
	sessiontype loggedin();

	// set folder link: node, key
	error folderaccess(const char*, const char*);

	// open exported file link
	error openfilelink(const char*);

	// change login password
	error changepw(const byte*, const byte*);

	// load all trees: nodes, shares, contacts
	void fetchnodes();

	// retrieve user details
	void getaccountdetails(AccountDetails*, bool, bool, bool, bool, bool, bool);

	// update node attributes
	error setattr(Node*, const char** = NULL);

	// prefix and encrypt attribute json
	void makeattr(SymmCipher*, string*, const char*, int = -1);

	// check node access level
	int checkaccess(Node*, accesslevel);

	// check if a move operation would succeed
	error checkmove(Node*, Node*);

	// delete node
	error unlink(Node*);

	// move node to new parent folder
	error rename(Node*, Node*);

	// start/stop file transfer
	bool startxfer(direction d, File* f);
	void stopxfer(File* f);

	// active syncs
	sync_list syncs;

	// start folder synchronization
	bool addsync(string* localname, Node* n);

	// number of parallel connections per transfer (PUT/GET)
	unsigned char connections[2];

	// generate & return next upload handle
	handle uploadhandle(int);

	// add nodes to specified parent node (complete upload, copy files, make folders)
	void putnodes(handle, NewNode*, int);

	// send files/folders to user
	void putnodes(const char*, NewNode*, int);

	// attach file attribute
	void putfa(Transfer*, fatype, const byte*, unsigned);

	// queue file attribute retrieval
	error getfa(Node*, fatype, int = 0);

	// attach/update/delete user attribute
	void putua(const char*, const byte* = NULL, unsigned = 0, int = 0);

	// queue user attribute retrieval
	void getua(User*, const char* = NULL, int = 0);

	// add new contact (by e-mail address)
	error invite(const char*, visibility = VISIBLE);

	// add/remove/update outgoing share
	void setshare(Node*, const char*, accesslevel);

	// export node link or remove existing exported link for this node
	error exportnode(Node*, int);

	// close all open HTTP connections
	void disconnect();

	// abort session and free all state information
	void logout();

	// maximum outbound throughput (per target server)
	int putmbpscap;

	// shopping basket
	handle_vector purchase_basket;

	// enumerate Pro account purchase options
	void purchase_enumeratequotaitems();

	// clear shopping basket
	void purchase_begin();

	// add item to basket
	void purchase_additem(int, handle, unsigned, char*, unsigned, char*, char*);

	// submit purchased products for payment
	void purchase_checkout(int);

private:
	// API request queue double buffering:
	// reqs[r] is open for adding commands
	// reqs[r^1] is being processed on the API server
	HttpReq* pendingcs;
	BackoffTimer btcs;

	// server-client command trigger connection
	HttpReq* pendingsc;
	BackoffTimer btsc;

	// root URL for API requestrs
	static const char* const APIURL;

	// sync debris folder in rubbish
	static const char* const SYNCDEBRISFOLDERNAME;
	
	// notify URL for new server-client commands
	string scnotifyurl;

	// unique request ID
	char reqid[10];

	// auth URI component for API requests
	string auth;

	// API response JSON object
	JSON response;

	// response record processing issue
	bool warned;

	// next local user record identifier to use
	int userid;

	// pending file attribute writes
	putfa_list newfa;

	// current attribute being sent
	putfa_list::iterator curfa;
	BackoffTimer btpfa;

	// pending file attribute reads, grouped by queued file attribute retrievals
	fareq_set fareqs;

	// next internal upload handle
	handle nextuh;

	// maximum number of concurrent transfers
	static const unsigned MAXTRANSFERS = 8;

	// determine if more transfers fit in the pipeline
	bool moretransfers(direction);

	// time at which next deferred transfer retry kicks in
	dstime nexttransferretry(direction d, dstime dsmin);

	// fetch state serialize from local cache
	bool fetchsc(DbTable*);

	// server-client command processing
	void sc_updatenode();
	void sc_deltree();
	void sc_newnodes();
	void sc_contacts();
	void sc_keys();
	void sc_fileattr();
	void sc_userattr();
	bool sc_shares();

	void init();

	// add node to vector and return index
	unsigned addnode(node_vector*, Node*);

	// crypto request response
	void cr_response(node_vector*, node_vector*, JSON*);

	// read node tree from JSON object
	void readtree(JSON*);

	// used by wait() to handle event timing
	void checkevent(dstime, dstime*, dstime*);

	// converts UTF-8 to 32-bit word array
	static char* str_to_a32(const char*, int*);

public:
	// application callbacks
	struct MegaApp* app;

	// event waiter
	Waiter* waiter;

	// HTTP access
	HttpIO* httpio;

	// directory change notification
	struct FileSystemAccess* fsaccess;

	// DB access
	DbAccess* dbaccess;

	// state cache table for logged in user
	DbTable* sctable;

	// scsn as read from sctable
	handle cachedscsn;

	// record type indicator for sctable
	enum { CACHEDSCSN, CACHEDNODE, CACHEDUSER } sctablerectype;

	// initialize/update state cache referenced sctable
	void initsc();
	void updatesc();
	void finalizesc(int);

	// MegaClient-Server response JSON
	JSON json;

	// Server-MegaClient request JSON
	JSON jsonsc;

	// no two interrelated client instances should ever have the same sessionid
	char sessionid[10];

	// application key
	char appkey[16];

	// incoming shares to be attached to a corresponding node
	newshare_list newshares;

	// current request tag
	int reqtag;

	// user maps: by handle and by case-normalized e-mail address
	uh_map uhindex;
	um_map umindex;

	// pending file attributes
	fa_map pendingfa;

	// file attribute fetch channels
	fafc_map fafcs;

	// file attribute fetches
	faf_map fafs;

	// generate attribute string based on the pending attributes for this upload
	void pendingattrstring(handle, string*);

	// merge newly received share into nodes
	void mergenewshares(int);

	// transfer queues (PUT/GET)
	transfer_map transfers[2];

	// transfer tslots
	transferslot_list tslots;
	
	// FileFingerprint to node mapping
	fingerprint_set fingerprints;

	// asymmetric to symmetric key rewriting
	handle_vector nodekeyrewrite;
	handle_vector sharekeyrewrite;

	static const char* const EXPORTEDLINK;

	// minimum number of bytes in transit for upload/download pipelining
	static const int MINPIPELINE = 65536;

	// server-client request sequence number
	char scsn[12];

	bool setscsn(JSON*);

	void purgenodes(node_vector* = NULL);
	void purgeusers(user_vector* = NULL);
	bool readusers(JSON*);

	user_vector usernotify;
	void notifyuser(User*);

	node_vector nodenotify;
	void notifynode(Node*);

	// write changed/added/deleted users to the DB cache and notify the application
	void notifypurge();

	// remove node subtree
	void deltree(handle);

	Node* nodebyhandle(handle);
	Node* nodebyfingerprint(FileFingerprint*);

	// generate & return upload handle
	handle getuploadhandle();

	// nodes being moved to //bin/SyncDebris with move failcount
	handlecount_map newsyncdebris;

	// we are adding the //bin/SyncDebris/yyyy-mm-dd subfolder(s)
	bool syncdebrisadding;
	
	// number of newsyncdebris nodes being moved at the moment
	int movedebrisinflight;

	// activity flag
	bool syncactivity;

	// added to a synced folder
	handle_set syncadded;

	// deleted from a synced folder (split by FILENODE/FOLDERNODE)
	handle_set syncdeleted[2];

	// overwritten in a sync'ed folder
	syncidhandle_map syncoverwritten;

	// local nodes that need to be added remotely
	localnode_vector synccreate;

	// number of sync-initiated putnodes() in progress
	int syncadding;

	// sync id dispatch
	handle nextsyncid();
	handle currsyncid;

	// SyncDebris folder addition result
	void putnodes_syncdebris_result(error,NewNode*);
	
	// sync id to handle mapping
	syncidhandle_map syncidhandles;

	// if no sync putnodes operation is in progress, apply the updates stored in syncadded/syncdeleted/syncoverwritten to the remote tree
	void syncupdate();

	// create missing folders, copy/start uploading missing files
	void syncup(LocalNode* = NULL, Node* = NULL);

	// trigger syncing of local file
	void syncupload(LocalNode*);

	// sync putnodes() completion
	void putnodes_sync_result(error, NewNode*);

	// start downloading/copy missing files, create missing directories
	void syncdown(Node*, LocalNode*, string*);

	// move node to //bin/SyncDebris/yyyy-mm-dd/
	void movetosyncdebris(Node*);

	// determine if all transfer slots are full
	bool slotavail();

	// dispatch as many queued transfers as possible
	void dispatchmore(direction);

	// transfer queue dispatch/retry handling
	bool dispatch(direction);

	void defer(direction, int td, int = 0);
	void freeq(direction);

	dstime transferretrydelay();

	// active request buffer
	int r;

	// client-server request double-buffering
	Request reqs[2];

	// upload handle -> node handle map (filled by upload completion)
	handlepair_set uhnh;

	// file attribute fetch failed
	void faf_failed(int);

	// process object arrays by the API server
	int readnodes(JSON*, int, putsource = PUTNODES_APP, NewNode* = NULL, int = 0);

	void readok(JSON*);
	void readokelement(JSON*);
	void readoutshares(JSON*);
	void readoutshareelement(JSON*);

	void readcr();
	void readsr();

	void procsnk(JSON*);
	void procsuk(JSON*);

	void setkey(SymmCipher*, const char*);
	bool decryptkey(const char*, byte*, int, SymmCipher*, int, handle);

	void handleauth(handle, byte*);

	void procsc();

	// API warnings
	void warn(const char*);
	bool warnlevel();

	Node* childnodebyname(Node*, const char*);

	// purge account state and abort server-client connection
	void purgenodesusersabortsc();

	static const int USERHANDLE = 8;
	static const int NODEHANDLE = 6;

	// session ID length (binary)
	static const unsigned SIDLEN = 2*SymmCipher::KEYLENGTH+USERHANDLE*4/3+1;

	void proccr(JSON*);
	void procsr(JSON*);

	// account access: master key
	// folder link access: folder key
	SymmCipher key;

	// account access (full account): RSA key
	AsymmCipher asymkey;

	// apply keys
	int applykeys();

	// symmetric password challenge
	int checktsid(byte* sidbuf, unsigned len);

	// locate user by e-mail address or by handle
	User* finduser(const char*, int = 0);
	User* finduser(handle, int = 0);
	void mapuser(handle, const char*);

	// queue public key request for user
	void queuepubkeyreq(User*, PubKeyAction*);

	// simple string hash
	static void stringhash(const char*, byte*, SymmCipher*);
	static uint64_t stringhash64(string*, SymmCipher*);

	// set authentication context, either a session ID or a exported folder node handle
	void setsid(const byte*, unsigned);
	void setrootnode(handle);

	// process node subtree
	void proctree(Node*, TreeProc*);

	// hash password
	error pw_key(const char*, byte*);

	// convert hex digit to number
	static int hexval(char);

	MegaClient(MegaApp*, Waiter*, HttpIO*, FileSystemAccess*, DbAccess*, const char*);
	~MegaClient();
};

// callback interface
struct MegaApp
{
	MegaClient* client;

	// a request-level error occurred (other than API_EAGAIN, which will lead to a retry)
	virtual void request_error(error) { }

	// login result
	virtual void login_result(error) { }

	// ephemeral session creation/resumption result
	virtual void ephemeral_result(error) { }
	virtual void ephemeral_result(handle, const byte*) { }

	// account creation
	virtual void sendsignuplink_result(error) { }
	virtual void querysignuplink_result(error) { }
	virtual void querysignuplink_result(handle, const char*, const char*, const byte*, const byte*, const byte*, size_t) { }
	virtual void confirmsignuplink_result(error) { }
	virtual void setkeypair_result(error) { }

	// account credentials, properties and history
	virtual void account_details(AccountDetails*, bool, bool, bool, bool, bool, bool) { }
	virtual void account_details(AccountDetails*, error) { }

	// node attribute update failed (not invoked unless error != API_OK)
	virtual void setattr_result(handle, error) { }

	// move node failed (not invoked unless error != API_OK)
	virtual void rename_result(handle, error) { }

	// node deletion failed (not invoked unless error != API_OK)
	virtual void unlink_result(handle, error) { }

	// nodes have been updated
	virtual void nodes_updated(Node**, int) { }

	// users have been added or updated
	virtual void users_updated(User**, int) { }

	// password change result
	virtual void changepw_result(error) { }

	// user attribute update notification
	virtual void userattr_update(User*, int, const char*) { }

	// node fetch has failed
	virtual void fetchnodes_result(error) { }

	// node addition has failed
	virtual void putnodes_result(error, targettype, NewNode*) { }

	// share update result
	virtual void share_result(error) { }
	virtual void share_result(int, error) { }

	// file attribute fetch result
	virtual void fa_complete(Node*, fatype, const char*, uint32_t) { }
	virtual int fa_failed(handle, fatype, int) { return 0; }

	// file attribute modification result
	virtual void putfa_result(handle, fatype, error) { }

	// purchase transactions
	virtual void enumeratequotaitems_result(handle, unsigned, unsigned, unsigned, unsigned, unsigned, const char*) { }
	virtual void enumeratequotaitems_result(error) { }
	virtual void additem_result(error) { }
	virtual void checkout_result(error) { }
	virtual void checkout_result(const char*) { }

	// user invites/attributes
	virtual void invite_result(error) { }
	virtual void putua_result(error) { }
	virtual void getua_result(error) { }
	virtual void getua_result(byte*, unsigned) { }

	// file node export result
	virtual void exportnode_result(error) { }
	virtual void exportnode_result(handle, handle) { }

	// exported link access result
	virtual void openfilelink_result(error) { }
	virtual void openfilelink_result(Node*) { }

	// global transfer queue updates (separate signaling towards the queued objects)
	virtual void transfer_added(Transfer*) { }
	virtual void transfer_removed(Transfer*) { }
	virtual void transfer_prepare(Transfer*) { }
	virtual void transfer_failed(Transfer*, error) { }
	virtual void transfer_update(Transfer*) { }
	virtual void transfer_limit(Transfer*) { }
	virtual void transfer_complete(Transfer*) { }

	// sync updates
	virtual void syncupdate_state(Sync*, syncstate) { }
	virtual void syncupdate_local_folder_addition(Sync*, const char*) { }
	virtual void syncupdate_local_folder_deletion(Sync*, const char*) { }
	virtual void syncupdate_local_file_addition(Sync*, const char*) { }
	virtual void syncupdate_local_file_deletion(Sync*, const char*) { }
	virtual void syncupdate_get(Sync*, const char*) { }
	virtual void syncupdate_put(Sync*, const char*) { }
	virtual void syncupdate_local_mkdir(Sync*, const char*) { }
	virtual void syncupdate_local_unlink(Node*) { }
	virtual void syncupdate_local_rmdir(Node*) { }
	virtual void syncupdate_remote_unlink(Node*) { }
	virtual void syncupdate_remote_rmdir(Node*) { }
	virtual void syncupdate_remote_mkdir(Sync*, const char*) { }
	virtual void syncupdate_remote_copy(Sync*, const char*) { }

	// suggest reload due to possible race condition with other clients
	virtual void reload(const char*) { }

	// wipe all users, nodes and shares
	virtual void clearing() { }

	// failed request retry notification
	virtual void notify_retry(dstime) { }

	// generic debug logging
	virtual void debug_log(const char*) { }

	virtual ~MegaApp() { }
};

class HashSignature
{
	Hash* hash;

public:
	// add data
	void add(const byte*, unsigned);

	// generate signature
	unsigned get(AsymmCipher*, byte*, unsigned);

	// verify signature
	int check(AsymmCipher*, const byte*, unsigned);

	HashSignature(Hash*);
	~HashSignature();
};

struct ScanItem
{
	string localpath;
	string localname;
	LocalNode* parent;
	bool fulltree;
	bool deleted;
};

class Sync
{
	// reference to pending/running uploads
	handle syncid;

	SymmCipher tkey;
	string tattrstring;
	AttrMap tattrs;
	
public:
	MegaClient* client;

	// remote root
	handle rooth;

	// full filesystem path this sync starts at
	string rootpath;

	// in-memory representation of the local tree
	LocalNode* rootlocal;

	// queued ScanItems
	deque<ScanItem> scanstack;

	// current state
	syncstate state;

	// change state, signal to application
	void changestate(syncstate);

	// process one scanstack item
	LocalNode* procscanstack();

	m_off_t localbytes;
	unsigned localnodes[2];

	// add or update LocalNode item, scan newly added folders
	void addscan(string*, string*, LocalNode*, bool);

	// scan items in specified path and add as children of the specified LocalNode
	void scan(string*, FileAccess*, LocalNode*, bool);

	sync_list::iterator sync_it;
	
	Sync(MegaClient*, string*, Node*);
	~Sync();
};


struct SyncFileGet : public File
{
	Node* n;

	// self-destruct after completion
	void completed(Transfer*, LocalNode*);

	SyncFileGet(Node*, string*);
	~SyncFileGet();
};

#endif
