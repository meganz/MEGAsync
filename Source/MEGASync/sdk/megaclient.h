/*

MEGA SDK 2013-10-03 - Client Access Engine

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

#include "mega.h"
#include "megacrypto.h"

// monotonously increasing time in deciseconds
typedef uint32_t dstime;

// generic host HTTP I/O interface
struct HttpIO
{
	// current time
	dstime ds;

	// update time
	virtual void updatedstime() = 0;

	// post request to target URL
	virtual void post(struct HttpReq*, const char* = NULL, unsigned = 0) = 0;

	// cancel request
	virtual void cancel(HttpReq*) = 0;

	// real-time progress information
	virtual m_off_t postpos(void*) = 0;

	// execute I/O operations
	// (all callbacks emerge from this function)
	virtual int doio(void) = 0;

	// wait for socket and application events (UI, timers...)
	virtual void waitio(uint32_t) = 0;

	virtual ~HttpIO() { }
};

// persistent resource cache storage
struct Cachable
{
	virtual int serialize(string*) = 0;

	int32_t dbid;

	bool notified;

	Cachable();
	virtual ~Cachable() { }
};

typedef vector<struct Node*> node_vector;

// generic host transactional database access interface
class DbTable
{
	static const int IDSPACING = 16;

public:
	// for a full sequential get: rewind to first record
	virtual void rewind() = 0;

	// get next record in sequence
	virtual int next(uint32_t*, string*) = 0;
	int next(uint32_t*, string*, SymmCipher*);

	// get specific record by key
	virtual int get(uint32_t, string*) = 0;

	// update or add specific record
	virtual int put(uint32_t, char*, unsigned) = 0;
	int put(uint32_t, string*);
	int put(uint32_t, Cachable*, SymmCipher*);

	// delete specific record
	virtual int del(uint32_t) = 0;

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

class DbAccess
{
public:
	virtual DbTable* open(string*) = 0;

	DbAccess() { }
	virtual ~DbAccess() { }
};

// generic host file access interface
struct FileAccess
{
	// file size
	m_off_t size;

	// mtime of a file opened for reading
	time_t mtime;

	// open for reading, writing or reading and writing
	virtual int fopen(const char*, int, int) = 0;

	// absolute position read, with NUL padding
	virtual int fread(string*, unsigned, unsigned, m_off_t) = 0;

	// absolute position write
	virtual int fwrite(const byte*, unsigned, m_off_t) = 0;

	// rename file, overwrite target
	static int rename(const char*, const char*);

	// delete file
	static int unlink(const char*);

	virtual ~FileAccess() { }
};

// error codes
typedef enum {
	API_OK = 0,
	API_EINTERNAL = -1,		// internal error
	API_EARGS = -2,			// bad arguments
	API_EAGAIN = -3,		// request failed, retry with exponential backoff
	API_ERATELIMIT = -4,	// too many requests, slow down
	API_EFAILED = -5,		// request failed permanently
	API_ETOOMANY = -6,		// too many requests for this resource
	API_ERANGE = -7,		// resource access out of rage
	API_EEXPIRED = -8,		// resource expired
	API_ENOENT = -9,		// resource does not exist
	API_ECIRCULAR = -10,	// circular linkage
	API_EACCESS = -11,		// access denied
	API_EEXIST = -12,		// resource already exists
	API_EINCOMPLETE = -13,	// request incomplete
	API_EKEY = -14,			// cryptographic error
	API_ESID = -15,			// bad session ID
	API_EBLOCKED = -16,		// resource administratively blocked
	API_EOVERQUOTA = -17,	// quote exceeded
	API_ETEMPUNAVAIL = -18,	// resource temporarily not available
	API_ETOOMANYCONNECTIONS = -19, // too many connections on this resource
	API_EWRITE = -20,		// file could not be written to
	API_EREAD = -21,		// file could not be read from
	API_EAPPKEY = -22		// invalid or missing application key
} error;

// returned by loggedin()
typedef enum { NOTLOGGEDIN=0, EPHEMERALACCOUNT, CONFIRMEDACCOUNT, FULLACCOUNT } sessiontype;

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

	int serialize(string*);
	static User* unserialize(class MegaClient*, string*);

	User(const char* = NULL);
};

typedef enum { REQ_BINARY, REQ_JSON } contenttype;

typedef enum { UPLOAD, DOWNLOAD } transfertype;

// node types:
// FILE - regular file nodes
// FOLDER - regular folder nodes
// ROOT - the cloud drive root node
// INCOMING - inbox
// RUBBISH - rubbish bin
// MAIL - mail message
typedef enum { TYPE_UNKNOWN = -1, FILENODE = 0, FOLDERNODE, ROOTNODE, INCOMINGNODE, RUBBISHNODE, MAILNODE } nodetype;

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
	string json;

public:
	MegaClient* client;

	int tag;

	char level;
	char persistent;

	void cmd(const char*);
	void notself(MegaClient*);

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

	int isnumeric();

	void begin(const char*);

	m_off_t getint();
	double getfloat();
	const char* getvalue();

	nameid getnameid();
	nameid getnameid(const char*);

	int is(const char*);

	int storebinary(byte*, int);
	int storebinary(string*);
	
	handle gethandle(int = 6);

	int enterarray();
	int leavearray();

	int enterobject();
	int leaveobject();

	int storestring(string*);
	int storeobject(string* = NULL);

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
	static char* btoa(const byte*, int, char*);
	static byte* atob(const char*, byte*, int);
};

// padded CBC encryption
struct PaddedCBC
{
	static void encrypt(string*, SymmCipher*);
	static int decrypt(string*, SymmCipher*);
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
	static int unserialize(MegaClient*, int, handle, const byte*, const char**, const char*);

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

	// while this request is in flight, points to the applications HttpIO object - NULL otherwise
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
typedef map<pair<handle,fatype>,handle> fa_map;

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
	int armed(dstime) const;

	// arm timer
	int arm(dstime);

	// time left for event to become armed
	dstime retryin(dstime);

	// current backoff delta
	dstime backoff();

	// put on hold indefinitely
	void freeze();

	// has a backoff occurred?
	int isset() const;

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
	int fac;		// cluster ID
	unsigned char dispatched;
	unsigned char retries;
	int tag;

	FileAttributeFetch(handle, fatype, int, int);
};

// file attribute fetch map
typedef map<handle,FileAttributeFetch*> faf_map;

// file attribute fetch channel map
typedef map<int,FileAttributeFetchChannel*> fafc_map;

struct FileTransfer
{
	static const int UPLOADTOKENLEN = 27;

	int inuse;

	int tag;

	FileAccess* file;

	Command* pendingcmd;

	m_off_t progressreported, progresscompleted;
	m_off_t pos, size;
	m_off_t startpos, endpos;
	m_off_t startblock;

	dstime starttime, lastdata;

	BackoffTimer bt;

	static const dstime XFERTIMEOUT = 600;

	int64_t ctriv;
	int64_t metamac;

	SymmCipher key;

	string tempurl;

	string filename;

	// upload flag (if 0, it's a download)
	int upload;

	// upload handle for file attribute attachment (only set if file attribute queued)
	handle uploadhandle;

	int connections;
	HttpReqXfer** reqs;

	chunkmac_map chunkmacs;

	void init(m_off_t, const char*, int = 0);

	void doio(MegaClient*);

	void disconnect();

	void close();

	int64_t macsmac(chunkmac_map*);

	FileTransfer();
};

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

	// creation and modification times
	time_t ctime;
	time_t mtime;

	// node attributes
	string attrstring;
};

// new node for putnodes()
struct NewNode : public NodeCore
{
	newnodesource source;

	handle uploadhandle;
	byte uploadtoken[FileTransfer::UPLOADTOKENLEN];
};

// filesystem node
struct Node : public NodeCore, Cachable
{
	// node crypto keys
	string keystring;

	// change parent node association
	int setparent(Node*);

	// copy JSON-delimited string
	static void copystring(string*, const char*);

	// try to resolve node key string
	int applykey(MegaClient*);

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

	// FILENODE nodes only: size, nonce, meta MAC, attribute string
	m_off_t size;

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

	void faspec(string*);

	// parent
	Node* parent;

	// children
	node_list children;

	// own position in parent's children
	node_list::iterator child_it;

	// check if node is below this node
	int isbelow(Node*) const;

	int serialize(string*);
	static Node* unserialize(MegaClient*, string*, node_vector*);

	Node(MegaClient*, vector<Node*>*, handle, handle, nodetype, m_off_t, handle, const char*, time_t, time_t);
	~Node();
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

	CommandCreateEphemeralSession(const byte*, const byte*, const byte*);
};

class CommandResumeEphemeralSession : public Command
{
	byte pw[SymmCipher::KEYLENGTH];
	handle uh;
	
public:
	void procresult();

	CommandResumeEphemeralSession(handle, const byte*);
};

class CommandSendSignupLink : public Command
{
public:
	void procresult();

	CommandSendSignupLink(const char*, const char*, byte*);
};

class CommandQuerySignupLink : public Command
{
	string confirmcode;

public:
	void procresult();

	CommandQuerySignupLink(const byte*, unsigned);
};

class CommandConfirmSignupLink : public Command
{
	string confirmcode;
	int query;

public:
	void procresult();

	CommandConfirmSignupLink(const byte*, unsigned, uint64_t);
};

class CommandSetKeyPair : public Command
{
public:
	void procresult();

	CommandSetKeyPair(const byte*, unsigned, const byte*, unsigned);
};

// invite contact/set visibility
class CommandUserRequest : public Command
{
public:
	void procresult();

	CommandUserRequest(const char*, visibility);	
};

// set user attributes
class CommandPutUA : public Command
{
public:
	CommandPutUA(const char*, const byte*, unsigned);

	void procresult();
};

class CommandGetUA : public Command
{
	int priv;

public:
	CommandGetUA(const char*, const char*, int);
	
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
	CommandKeyCR(MegaClient* client, node_vector*, node_vector*, const char*);
};

class CommandMoveNode : public Command
{
	handle h;

public:
	void procresult();

	CommandMoveNode(MegaClient*, Node*, Node*);
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
	int td;
	int connections;

public:
	void procresult();

	CommandGetFile(int, handle, int, int);
};

class CommandPutFile : public Command
{
	int td;
	FileAccess* file;
	int connections;

public:
	void procresult();

	CommandPutFile(int, FileAccess*, int, int);
};

class CommandAttachFA : public Command
{
	handle h;
	fatype type;

public:
	void procresult();

	CommandAttachFA(handle, fatype, handle, int);
};

class CommandPutNodes : public Command
{
	handle* ulhandles;
	NewNode* nn;
	targettype type;

public:
	void procresult();

	CommandPutNodes(MegaClient*, handle, const char*, NewNode*, int, int);
	~CommandPutNodes();
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

	int procuserresult(MegaClient*);

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

	CommandGetUserQuota(MegaClient*, AccountDetails*, int, int, int);
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

// transfer queuing
struct QueuedTransfer
{
	int failcount;
	BackoffTimer bt;

	int td;

	virtual void start() = 0;
	virtual void defer(MegaClient*, int = 0);

	QueuedTransfer();
	virtual ~QueuedTransfer() { }
};

struct FilePut : public QueuedTransfer
{
	string filename;
	string newname;

	handle target;
	string targetuser;

	virtual void start() = 0;

	FilePut(const char*, handle, const char*, const char*);
	virtual ~FilePut() { }
};

struct FileGet : public QueuedTransfer
{
	handle nodehandle;

	virtual void start() = 0;

	FileGet(handle);
	virtual ~FileGet() { }
};

// FIXME: use forward_list instead
typedef list<QueuedTransfer*> transfer_list;
typedef list<FilePut*> put_list;
typedef list<FileGet*> get_list;

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
	void wait();

	// abort exponential backoff
	int abortbackoff();

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
	void login(const char*, const byte*, int = 0);

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

	// add/update user attributes
	// FIXME: implement
	// error setuserattr(const char*);

	// retrieve user details
	void getaccountdetails(AccountDetails*, int, int, int, int, int, int);

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

	// start upload
	int topen(const char*, int = 0, int = 3);

	// start (partial) download
	int topen(handle, const byte* = NULL, m_off_t = 0, m_off_t = -1, int = 3);

	// close/cancel/abort transfer
	void tclose(int);

	// upload queue
	put_list putq;

	// download queue
	get_list getq;

	// return queued transfer by td or NULL
	QueuedTransfer* gettransfer(transfer_list* queue, int td);

	// obtain upload handle
	handle uploadhandle(int);

	// open target file for download
	void dlopen(int, const char*);

	// add nodes to specified parent node (complete upload, copy files, make folders)
	void putnodes(handle, NewNode*, int);

	// add nodes to (send files/folders to user)
	void putnodes(const char*, NewNode*, int);

	// attach file attribute
	void putfa(SymmCipher*, handle, fatype, const byte*, unsigned);

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

	// escape filename based on a set of locally forbidden characters
	static void escapefilename(string*, const char* = NULL);

	// unescape filename escaped by escapefilename()
	static void unescapefilename(string*);

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

	// notify URL for new server-client commands
	string scnotifyurl;

	// unique request ID
	char reqid[10];

public:
	// auth URI component for API requests
	string auth;

private:
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

	// next upload handle
	handle nextuh;

	// fetch state serialize from local cache
	int fetchsc(DbTable*);

	// server-client command processing
	void sc_updatenode();
	void sc_deltree();
	void sc_newnodes();
	void sc_contacts();
	void sc_keys();
	void sc_fileattr();
	void sc_userattr();
	int sc_shares();

	void init();

	// add node to vector and return index
	unsigned addnode(node_vector*, Node*);

	// crypto request response
	void cr_response(node_vector*, node_vector*, JSON*);

	// read node tree from JSON object
	void readtree(JSON*);

	// determine if more transfers fit in the pipeline
	int moretransfers(int);

	// used by wait() to handle event timing
	void checkevent(dstime, dstime*, dstime*);

	// determine if the character is a lowercase hex digit
	static int islchex(char);

	// converts UTF-8 to 32-bit word array
	static char* str_to_a32(const char*, int*);

public:
	// application callbacks
	struct MegaApp* app;

	// HTTP access
	HttpIO* httpio;

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

	// up to ten concurrent file transfers
	FileTransfer ft[10];

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

	// allocate transfer descriptor in ft[]
	int alloctd(int);

	// asymmetric to symmetric key rewriting
	handle_vector nodekeyrewrite;
	handle_vector sharekeyrewrite;

	static const char* const EXPORTEDLINK;

	// minimum number of bytes in transit for upload/download pipelining
	static const int MINPIPELINE = 65536;

	// server-client request sequence number
	char scsn[12];

	int setscsn(JSON*);

	void purgenodes(node_vector* = NULL);
	void purgeusers(user_vector* = NULL);
	int readusers(JSON*);

	user_vector usernotify;
	void notifyuser(User*);

	node_vector nodenotify;
	void notifynode(Node*);

	// write changed/added/deleted users to the DB cache and notify the application
	void notifypurge();

	Node* nodebyhandle(handle);

	void deltree(handle);

	// dispatch as many queued transfers as possible
	void dispatchmore(int);

	// transfer queue dispatch/retry handling
	int dispatch(transfer_list*, int = 0);
	void defer(transfer_list*, int td, int = 0);
	void freequeue(transfer_list*);
	int retrydeferred(transfer_list*, dstime);
	void retrytransfers();
	dstime maxdeferredtransferretrydelay(transfer_list*);
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
	int readnodes(JSON*, int, handle* = NULL);

	void readok(JSON*);
	void readokelement(JSON* j);
	void readoutshares(JSON* j);
	void readoutshareelement(JSON* j);

	void readcr();
	void readsr();

	void procsnk(JSON*);
	void procsuk(JSON*);
	
	void setkey(SymmCipher*, const char*);
	int decryptkey(const char*, byte*, int, SymmCipher*, int, handle);

	void handleauth(handle, byte*);

	void procsc();

	// API warnings
	void warn(const char*);
	int warnlevel();

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

	MegaClient(MegaApp*, HttpIO*, DbAccess*, const char*);
	~MegaClient();
};

// callback interface
struct MegaApp
{
	MegaClient* client;

	// returns a new instance of the application's own FileAccess class
	virtual FileAccess* newfile() = 0;

	// a request-level error occurred (other than API_EAGAIN, which will lead to a retry)
	virtual void request_error(error) = 0;

	// login result
	virtual void login_result(error) = 0;

	// ephemeral session creation/resumption result
	virtual void ephemeral_result(error) = 0;
	virtual void ephemeral_result(handle, const byte*) = 0;

	// account creation
	virtual void sendsignuplink_result(error) = 0;
	virtual void querysignuplink_result(error) = 0;
	virtual void querysignuplink_result(handle, const char*, const char*, const byte*, const byte*, const byte*, size_t) = 0;	
	virtual void confirmsignuplink_result(error) = 0;
	virtual void setkeypair_result(error) = 0;

	// account credentials, properties and history
	virtual void account_details(AccountDetails*, int, int, int, int, int, int) = 0;
	virtual void account_details(AccountDetails*, error) = 0;

	// node attribute update failed (not invoked unless error != API_OK)
	virtual void setattr_result(handle, error) = 0;

	// move node failed (not invoked unless error != API_OK)
	virtual void rename_result(handle, error) = 0;

	// node deletion failed (not invoked unless error != API_OK)
	virtual void unlink_result(handle, error) = 0;

	// nodes have been updated
	virtual void nodes_updated(Node**, int) = 0;

	// users have been added or updated
	virtual void users_updated(User**, int) = 0;

	// node fetch has failed
	virtual void fetchnodes_result(error) = 0;

	// node addition has failed
	virtual void putnodes_result(error, targettype, NewNode*) = 0;

	// share update result
	virtual void share_result(error) = 0;
	virtual void share_result(int, error) = 0;

	// file attribute fetch result
	virtual void fa_complete(Node*, fatype, const char*, uint32_t) = 0;
	virtual int fa_failed(handle, fatype, int) = 0;

	// file attribute modification result
	virtual void putfa_result(handle, fatype, error) = 0;

	// user invites/attributes
	virtual void invite_result(error) = 0;
	virtual void putua_result(error) = 0;
	virtual void getua_result(error) = 0;
	virtual void getua_result(byte*, unsigned) = 0;

	// file node export result
	virtual void exportnode_result(error) = 0;
	virtual void exportnode_result(handle, handle) = 0;

	// exported link access result
	virtual void openfilelink_result(error) = 0;
	virtual void openfilelink_result(Node*) = 0;

	// topen() result
	virtual void topen_result(int, error) = 0;
	virtual void topen_result(int, string*, const char*, int) = 0;

	// transfer progress information
	virtual void transfer_update(int, m_off_t, m_off_t, dstime) = 0;

	// intermitten transfer error
	virtual int transfer_error(int, int, int) = 0;

	// transfer error
	virtual void transfer_failed(int, error) = 0;
	virtual void transfer_failed(int, string&, error) = 0;

	// transfer limit reached
	virtual void transfer_limit(int) = 0;

	// transfer completed
	virtual void transfer_complete(int, chunkmac_map*, const char*) = 0;
	virtual void transfer_complete(int, handle, const byte*, const byte*, SymmCipher*) = 0;

	virtual void changepw_result(error) = 0;

	// user attribute update notification
	virtual void userattr_update(User*, int, const char*) = 0;
	
	// suggest reload due to possible race condition with other clients
	virtual void reload(const char*) = 0;

	// wipe all users, nodes and shares
	virtual void clearing() = 0;

	// failed request retry notification
	virtual void notify_retry(dstime) = 0;

	// generic debug logging
	virtual void debug_log(const char*) = 0;

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
#endif
