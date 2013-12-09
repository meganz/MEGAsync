/*

MEGA SDK sample application for the gcc/POSIX environment, using cURL for HTTP I/O,
GNU Readline for console I/O and FreeImage for thumbnail creation

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

#ifndef MEGAAPI_H
#define MEGAAPI_H

#ifndef _WIN32
#include <openssl/ssl.h>
#include <curl/curl.h>
#include <fcntl.h>
#endif

#include <inttypes.h>
typedef int64_t m_off_t;

#include "mega/crypto/cryptopp.h"
#include "mega/megaclient.h"

#define USE_SQLITE
#include "mega/db/sqlite.h"

#ifdef WIN32

#include "mega/win32/meganet.h"
#include "mega/win32/megafs.h"
#include "win32/megaapiwait.h"
#include "mega.h"

#include <pthread.h>

using namespace std;
using namespace mega;

class MegaHttpIO : public WinHttpIO {};
class MegaFileSystemAccess : public WinFileSystemAccess {};
class MegaWaiter : public MegaApiWinWaiter {};

#else

#include "posix/net.h"
#include "posix/fs.h"
#include "posix/wait.h"

class MegaHttpIO : public PosixHttpIO {};
class MegaFileAccess : public PosixFileAccess {};
class MegaWaiter : public PosixWaiter {};

#endif

class MegaDbAccess : public SqliteDbAccess
{
public:
    MegaDbAccess(string *basePath = NULL) : SqliteDbAccess(basePath){}
};

struct MegaFile : public File
{
    // app-internal sequence number for queue management
    int seqno;
    static int nextseqno;

    bool failed(error e)
    {
        return e != API_EKEY && e != API_EBLOCKED && transfer->failcount < 10;
    }

    MegaFile()
    {
        seqno = ++nextseqno;
    }
};

struct MegaFileGet : public MegaFile
{
    void completed(Transfer*, LocalNode*)
    {
        delete this;
    }

	MegaFileGet(MegaClient *client, Node* n, string dstPath);
	~MegaFileGet() {}
};

struct MegaFilePut : public MegaFile
{
    void completed(Transfer* t, LocalNode*)
    {
        File::completed(t,NULL);
        delete this;
    }

	MegaFilePut(MegaClient *client, string* clocalname, handle ch, const char* ctargetuser)
    {
        // this assumes that the local OS uses an ASCII path separator, which should be true for most
        string separator = client->fsaccess->localseparator;

        // full local path
		localname = *clocalname;

        // target parent node
        h = ch;

        // target user
        targetuser = ctargetuser;

        // erase path component
		name = *clocalname;
        client->fsaccess->local2name(&name);
        client->fsaccess->local2name(&separator);

        name.erase(0,name.find_last_of(*separator.c_str())+1);
    }

    ~MegaFilePut() {}
};


//Wrapping for arrays, to ease the use of SWIG
template <class T>
class ArrayWrapper
{
	public:
		ArrayWrapper()
		{
			copied = 0;
			list = NULL;
			s = 0;
		}
		
		ArrayWrapper(T* newlist, int size, bool copy=0)
		{
			copied = 0;
			list = NULL;
			s = 0;
			setArray(newlist, size, copy);
		}
		
		virtual ~ArrayWrapper()
		{
			if(copied && list) delete [] list;
		}
		
				
		T get(int i)
		{
			//if(list && i >= 0 && i <= s)
				return list[i];
			//return NULL;	
		}
		
		int size() { return s; }
		
	protected:
		void setArray(T* newlist, int size, bool copy=0)
		{
			if(copied && list) delete [] list;
			if(!size) return;
			if(copy)
			{
				list = new T[size];
				memcpy(list, newlist, size*sizeof(T));
			}
			else
			{
				list = newlist;
			}

			copied = copy;
			s = size;
		}

		T* list;
		int s;
		bool copied;
};

typedef ArrayWrapper<Node*> NodeList;
typedef ArrayWrapper<User*> UserList;
typedef ArrayWrapper<AccountBalance> BalanceList;
typedef ArrayWrapper<AccountSession> SessionList;
typedef ArrayWrapper<AccountPurchase> PurchaseList;
typedef ArrayWrapper<AccountTransaction> TransactionList;
typedef ArrayWrapper<const char*> StringList;
typedef ArrayWrapper<Share*> ShareList;

class MegaRequestListener;
class MegaTransferListener;
class MegaTransfer;
class MegaApi;

//Encapsulates the information about a request instead of return it as individual parameters in the callbacks.
//This allow extending the information adding more getters without breaking client's code.
class MegaRequest
{	
	public:
		enum {TYPE_LOGIN, TYPE_MKDIR, TYPE_MOVE, TYPE_COPY, TYPE_RENAME, TYPE_REMOVE, TYPE_SHARE, 
		    TYPE_FOLDER_ACCESS, TYPE_IMPORT_LINK, TYPE_IMPORT_NODE, TYPE_EXPORT, TYPE_FETCH_NODES, TYPE_ACCOUNT_DETAILS, TYPE_CHANGE_PW, TYPE_UPLOAD, TYPE_LOGOUT, TYPE_FAST_LOGIN, TYPE_GET_PUBLIC_NODE, TYPE_GET_ATTR_FILE,
            TYPE_SET_ATTR_FILE, TYPE_RETRY_PENDING_CONNECTIONS, TYPE_ADD_CONTACT, TYPE_CREATE_ACCOUNT, TYPE_FAST_CREATE_ACCOUNT, TYPE_CONFIRM_ACCOUNT, TYPE_FAST_CONFIRM_ACCOUNT, TYPE_QUERY_SIGNUP_LINK, TYPE_SYNC};

		MegaRequest(int type, MegaRequestListener *listener = NULL);
		MegaRequest(MegaTransfer *transfer);
		MegaRequest(MegaRequest &request);
		virtual ~MegaRequest();

		MegaRequest *copy();		
		int getType() const;
		const char *getRequestString() const;
		const char* toString() const;
		const char* __str__() const;

		handle getNodeHandle() const;
		const char* getLink() const;
		handle getParentHandle() const;
		const char* getUserHandle() const;
		const char* getName() const;
		const char* getEmail() const;
		const char* getPassword() const;
		const char* getNewPassword() const;
		const char* getPrivateKey() const;
		const char* getAccess() const;
		const char* getFile() const;
		int getNumRetry() const;
		int getNextRetryDelay() const;
		Node *getPublicNode();
		int getAttrType() const;
		void *getParameter() const;

		void setNodeHandle(handle nodeHandle);
		void setLink(const char* link);
		void setParentHandle(handle parentHandle);
		void setUserHandle(const char* userHandle);
		void setName(const char* name);
		void setEmail(const char* email);
    	void setPassword(const char* email);
    	void setNewPassword(const char* email);
		void setPrivateKey(const char* privateKey);
		void setAccess(const char* access);
		void setNumRetry(int ds);
		void setNextRetryDelay(int delay);
		void setPublicNode(Node* node);
		void setNumDetails(int numDetails);
		void setFile(const char* file);
		void setAttrType(int type);
		void setParameter(void *parameter);

		MegaRequestListener *getListener() const;
		MegaTransfer * getTransfer() const;
		AccountDetails * getAccountDetails() const;
		int getNumDetails() const;
		
	protected:
		int type;
		handle nodeHandle;
		const char* link;
		const char* name;
		handle parentHandle;
		const char* userHandle;
		const char* email;
		const char* password;
		const char* newPassword;
		const char* privateKey;
		const char* access;
		const char* file;
		int attrType;
		
		MegaRequestListener *listener;
		MegaTransfer *transfer;
		AccountDetails *accountDetails;
		int numDetails;
		Node* publicNode;
		void* parameter;
		int numRetry;
		int nextRetryDelay;
};

class MegaTransfer
{
	public:
        enum {TYPE_DOWNLOAD, TYPE_UPLOAD};
		
		MegaTransfer(int type, MegaTransferListener *listener = NULL);
        MegaTransfer(const MegaTransfer &transfer);
		~MegaTransfer();
		
        MegaTransfer *copy();

		int getSlot() const;
		int getType() const;
		const char * getTransferString() const;
		const char* toString() const;
		const char* __str__() const;

		long long getStartTime() const;
		long long getTransferredBytes() const;
		long long getTotalBytes() const;
		const char* getPath() const;
		const char* getParentPath() const;
		handle getNodeHandle() const;
		handle getParentHandle() const;
		int getNumConnections() const;
		long long getStartPos() const;
		long long getEndPos() const;
		int getMaxSpeed() const;
		const char* getFileName() const;
		MegaTransferListener* getListener() const;
		int getNumRetry() const;
		int getMaxRetries() const;
		long long getTime() const;
		const char* getBase64Key() const;
		int getTag() const;
        Transfer *getTransfer() const;
		long long getSpeed() const;
		long long getDeltaSize() const;
		long long getUpdateTime() const;
		void *getParameter() const;

		void setStartTime(long long startTime);
		void setTransferredBytes(long long transferredBytes);
		void setTotalBytes(long long totalBytes);
		void setPath(const char* path);
		void setParentPath(const char* path);
		void setNodeHandle(handle nodeHandle);
		void setParentHandle(handle parentHandle);
		void setNumConnections(int connections);
		void setStartPos(long long startPos);
		void setEndPos(long long endPos);
		void setMaxSpeed(int maxSpeed);
		void setNumRetry(int retry);
		void setMaxRetries(int retry);
		void setTime(long long time);
		void setFileName(const char* fileName);
		void setSlot(int id);
		void setBase64Key(const char* base64Key);
		void setTag(int tag);
        void setTransfer(Transfer *transfer);
		void setSpeed(long long speed);
		void setDeltaSize(long long deltaSize);
		void setUpdateTime(long long updateTime);
		void setParameter(void *parameter);

	protected:
		int slot;
		int type;
		int tag;
		long long startTime;
		long long updateTime;
		long long time;
		long long transferredBytes;
		long long totalBytes;
		long long speed;
		long long deltaSize;
		handle nodeHandle;
		handle parentHandle;
		const char* path;
		const char* parentPath;
		const char* fileName;
		const char* base64Key;
		void *parameter;

		int numConnections;
		long long startPos;
		long long endPos;
		int maxSpeed;
		int retry;
		int maxRetries;
		
        Transfer *transfer;

		MegaTransferListener *listener;
		
		//Might be useful for streaming
		char *lastBuffer;
		long lastBufferStartOffset;
		int lastBufferlength;
};

class MegaError
{
	public:
		// error codes
		enum {
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
		};
	
		MegaError(int errorCode);
		MegaError(const MegaError &megaError);
        virtual ~MegaError(){}
		MegaError* copy();
		int getErrorCode() const;
		const char* getErrorString() const;
		const char* toString() const;
		const char* __str__() const;

		bool isTemporal() const;
		long getNextAttempt() const;
		void setNextAttempt(long nextAttempt);

		static const char *getErrorString(int errorCode);
	protected:
        //< 0 = API error code, > 0 = http error, 0 = No error
		int errorCode;
		long nextAttempt;
};


class TreeProcessor
{
	public:
	virtual int processNode(Node* node);
	virtual ~TreeProcessor();
};

class SearchTreeProcessor : public TreeProcessor
{
    public:
	SearchTreeProcessor(const char *search);
	virtual int processNode(Node* node);
    virtual ~SearchTreeProcessor() {}
	vector<Node *> &getResults();
    
    protected:
	const char *search;
	vector<Node *> results;
};

//Separate listeners for requests, transfers and global events
//It could be useful for some applications

//Request callbacks
class MegaRequestListener
{
	public:
	//Request callbacks
	virtual void onRequestStart(MegaApi* api, MegaRequest *request);
	virtual void onRequestFinish(MegaApi* api, MegaRequest *request, MegaError* e);
	virtual void onRequestTemporaryError(MegaApi *api, MegaRequest *request, MegaError* e);
	virtual ~MegaRequestListener();
};

//Transfer callbacks
class MegaTransferListener
{
	public:
	//Transfer callbacks
    virtual void onTransferStart(MegaApi *api, MegaTransfer *transfer);
	virtual void onTransferFinish(MegaApi* api, MegaTransfer *transfer, MegaError* e);
	virtual void onTransferUpdate(MegaApi *api, MegaTransfer *transfer);
	virtual void onTransferTemporaryError(MegaApi *api, MegaTransfer *transfer, MegaError* e);
	virtual ~MegaTransferListener();
};

//Global callbacks
class MegaGlobalListener
{
	public:
	//Global callbacks
	virtual void onUsersUpdate(MegaApi* api, UserList *users);
	virtual void onNodesUpdate(MegaApi* api, NodeList *nodes);
	virtual void onReloadNeeded(MegaApi* api);
	virtual ~MegaGlobalListener();
};

//All callbacks (no multiple inheritance because it isn't available in other programming languages)
class MegaListener
{
	public:
	virtual void onRequestStart(MegaApi* api, MegaRequest *request);
	virtual void onRequestFinish(MegaApi* api, MegaRequest *request, MegaError* e);
	virtual void onRequestTemporaryError(MegaApi *api, MegaRequest *request, MegaError* e);
	virtual void onTransferStart(MegaApi *api, MegaTransfer *transfer);
	virtual void onTransferFinish(MegaApi* api, MegaTransfer *transfer, MegaError* e);
	virtual void onTransferUpdate(MegaApi *api, MegaTransfer *transfer);
	virtual void onTransferTemporaryError(MegaApi *api, MegaTransfer *transfer, MegaError* e);
	virtual void onUsersUpdate(MegaApi* api, UserList *users);
	virtual void onNodesUpdate(MegaApi* api, NodeList *nodes);
	virtual void onReloadNeeded(MegaApi* api);

	//virtual void onSyncStateChanged(Sync*, syncstate) {}
	//virtual void onSyncGet(Sync*, const char*) {}
	//virtual void onSyncPut(Sync*, const char*) {}
	//virtual void onSyncRemoteCopy(Sync*, const char*) {}

	virtual ~MegaListener();
};


//Thread safe request queue
class RequestQueue
{
    protected:
        std::deque<MegaRequest *> requests;
        pthread_mutex_t mutex;

    public:
	RequestQueue()
	{
	    pthread_mutex_init(&mutex, NULL);
	}
	
	void push(MegaRequest *request)
	{
	    pthread_mutex_lock(&mutex);
	    requests.push_back(request);
    	pthread_mutex_unlock(&mutex);
	}
	
	void push_front(MegaRequest *request)
	{
	    pthread_mutex_lock(&mutex);
	    requests.push_front(request);
	    pthread_mutex_unlock(&mutex);
	}
	
	MegaRequest * pop()
	{
	    pthread_mutex_lock(&mutex);
	    if(requests.empty())
	    { 
		pthread_mutex_unlock(&mutex);
		return NULL;
	    }
	    MegaRequest *request = requests.front();
	    requests.pop_front();
    	    pthread_mutex_unlock(&mutex);
	    return request;
	}
};

//Thread safe transfer queue
class TransferQueue
{
    protected:
        std::deque<MegaTransfer *> transfers;
        pthread_mutex_t mutex;

    public:
    TransferQueue()
	{
	    pthread_mutex_init(&mutex, NULL);
	}

	void push(MegaTransfer *transfer)
	{
	    pthread_mutex_lock(&mutex);
	    transfers.push_back(transfer);
    	pthread_mutex_unlock(&mutex);
	}

	void push_front(MegaTransfer *transfer)
	{
	    pthread_mutex_lock(&mutex);
	    transfers.push_front(transfer);
	    pthread_mutex_unlock(&mutex);
	}

	MegaTransfer * pop()
	{
	    pthread_mutex_lock(&mutex);
	    if(transfers.empty())
	    {
	    	pthread_mutex_unlock(&mutex);
	    	return NULL;
	    }
	    MegaTransfer *transfer = transfers.front();
	    transfers.pop_front();
    	pthread_mutex_unlock(&mutex);
	    return transfer;
	}
};

class MegaApi : public MegaApp
{
	
public:	
	enum {	ORDER_NONE, ORDER_DEFAULT_ASC, ORDER_DEFAULT_DESC,
		ORDER_SIZE_ASC, ORDER_SIZE_DESC,
		ORDER_CREATION_ASC, ORDER_CREATION_DESC,
		ORDER_MODIFICATION_ASC, ORDER_MODIFICATION_DESC,
		ORDER_ALPHABETICAL_ASC, ORDER_ALPHABETICAL_DESC};

	static bool nodeComparatorDefaultASC  (Node *i, Node *j);
	static bool nodeComparatorDefaultDESC (Node *i, Node *j);
	static bool nodeComparatorSizeASC  (Node *i, Node *j);
	static bool nodeComparatorSizeDESC (Node *i, Node *j);	
	static bool nodeComparatorCreationASC  (Node *i, Node *j);
	static bool nodeComparatorCreationDESC  (Node *i, Node *j);
	static bool nodeComparatorModificationASC  (Node *i, Node *j);
	static bool nodeComparatorModificationDESC  (Node *i, Node *j);	
	static bool nodeComparatorAlphabeticalASC  (Node *i, Node *j);
	static bool nodeComparatorAlphabeticalDESC  (Node *i, Node *j);	
	static bool userComparatorDefaultASC (User *i, User *j);

    MegaApi(MegaListener *listener = NULL, string *basePath = NULL);
	virtual ~MegaApi();

	//Multiple listener management.
	//Allowing multiple listeners could be useful for some applications.
	void addListener(MegaListener* listener);
	void addRequestListener(MegaRequestListener* listener);
	void addTransferListener(MegaTransferListener* listener);
	void addGlobalListener(MegaGlobalListener* listener);
	void removeListener(MegaListener* listener);
	void removeRequestListener(MegaRequestListener* listener);
	void removeTransferListener(MegaTransferListener* listener);
	void removeGlobalListener(MegaGlobalListener* listener);

	//Utils
	const char* getBase64PwKey(const char *password);
	const char* getStringHash(const char* base64pwkey, const char* inBuf);
	static handle base64ToHandle(const char* base64Handle);
	static const char* ebcEncryptKey(const char* encryptionKey, const char* plainKey);
	void retryPendingConnections();

	//API requests
	void login(const char* email, const char* password, MegaRequestListener *listener = NULL);
	void fastLogin(const char* email, const char *stringHash, const char *base64pwkey, MegaRequestListener *listener = NULL);
	void createAccount(const char* email, const char* password, const char* name, MegaRequestListener *listener = NULL);
	void fastCreateAccount(const char* email, const char *base64pwkey, const char* name, MegaRequestListener *listener = NULL);
	void querySignupLink(const char* link, MegaRequestListener *listener = NULL);
	void confirmAccount(const char* link, const char *password, MegaRequestListener *listener = NULL);
	void fastConfirmAccount(const char* link, const char *base64pwkey, MegaRequestListener *listener = NULL);

	bool isLoggedIn();
	const char* getMyEmail();
	
    void createFolder(const char* name, Node* parent, MegaRequestListener *listener = NULL);
	void moveNode(Node* node, Node* newParent, MegaRequestListener *listener = NULL);
	void copyNode(Node* node, Node* newParent, MegaRequestListener *listener = NULL);
	void renameNode(Node* node, const char* newName, MegaRequestListener *listener = NULL);
	void remove(Node* node, MegaRequestListener *listener = NULL);
	void share(Node* node, User* user, const char *level, MegaRequestListener *listener = NULL);
	void share(Node* node, const char* email, const char *level, MegaRequestListener *listener = NULL);
	void folderAccess(const char* megaFolderLink, MegaRequestListener *listener = NULL);
	void importFileLink(const char* megaFileLink, Node* parent, MegaRequestListener *listener = NULL);
	void importPublicNode(Node *publicNode, Node* parent, MegaRequestListener *listener = NULL);
	void getPublicNode(const char* megaFileLink, MegaRequestListener *listener = NULL);
	void exportNode(Node *node, MegaRequestListener *listener = NULL);
	void fetchNodes(MegaRequestListener *listener = NULL);
	void getAccountDetails(MegaRequestListener *listener = NULL);
	void getAccountDetails(int storage, int transfer, int pro, int transactions, int purchases, int sessions, MegaRequestListener *listener = NULL);
	void changePassword(const char *oldPassword, const char *newPassword, MegaRequestListener *listener = NULL);
	void logout(MegaRequestListener *listener = NULL);
	void getNodeAttribute(Node* node, int type, char *dstFilePath, MegaRequestListener *listener = NULL);
	void setNodeAttribute(Node* node, int type, char *srcFilePath, MegaRequestListener *listener = NULL);
	void addContact(const char* email, MegaRequestListener* listener=NULL);
	
	//Transfers (MegaTransfer returned in MegaError if MegaError.getErrorCode()==API_OK)
	void startUpload(const char* localPath, Node* parent=NULL, int connections=1, int maxSpeed = 0, const char* fileName = NULL, MegaTransferListener *listener = NULL);
	void startUpload(const char* localPath, Node* parent, MegaTransferListener *listener);
	void startUpload(const char* localPath, Node* parent, const char* fileName, MegaTransferListener *listener);

	void startDownload(handle nodehandle, const char* target, int connections, long startPos, long endPos, const char* base64key, MegaTransferListener *listener);
	void startDownload(Node* node, const char* localFolder, int connections=1, long startPos = 0, long endPos = 0, const char* base64key = NULL, MegaTransferListener *listener = NULL);
	void startDownload(Node* node, const char* localFolder, long startPos, long endPos, MegaTransferListener *listener);
	void startDownload(Node* node, const char* localFolder, MegaTransferListener *listener);
	void startPublicDownload(Node* node, const char* localFolder, MegaTransferListener *listener = NULL);
//	void startPublicDownload(handle nodehandle, const char * base64key, const char* localFolder, MegaTransferListener *listener = NULL);

//	void cancelTransfer(MegaTransfer *transfer);


    void syncFolder(const char *localFolder, Node *megaFolder);

	//Filesystem
	NodeList* getChildren(Node* parent, int order=1);
	Node* getChildNode(Node *parent, const char* name);
	Node* getParentNode(Node* node);
	const char* getNodePath(Node* node);
	Node* getNodeByPath(const char *path, Node* cwd = NULL);
	Node* getNodeByHandle(handle handler);
	UserList* getContacts();
	User* getContact(const char* email);
	NodeList *getInShares(User* user);
	ShareList *getOutShares(Node *node);
	const char *getAccess(Node* node);
	bool processTree(Node* node, TreeProcessor* processor, bool recursive = 1);
	NodeList* search(Node* node, const char* searchString, bool recursive = 1);
	
	MegaError checkAccess(Node* node, const char *level);
	MegaError checkMove(Node* node, Node* target);
	
	Node* getRootNode();
	Node* getInboxNode();
	Node* getRubbishNode();
	Node* getMailNode();
	sync_list *getActiveSyncs();
	StringList *getRootNodeNames();
	StringList *getRootNodePaths();

	//Debug
	void setDebug(bool debug);
	bool getDebug();
	
	//General porpuse
	static char* strdup(const char* buffer);
	static Node* copyNode(Node *node);

	//Do not use
	static void *threadEntryPoint(void *param);

protected:
	static const char* rootnodenames[];
	static const char* rootnodepaths[];
	static StringList *rootNodeNames;
	static StringList *rootNodePaths;
	
	void fireOnRequestStart(MegaApi* api, MegaRequest *request);	
	void fireOnRequestFinish(MegaApi* api, MegaRequest *request, MegaError e);
	void fireOnRequestTemporaryError(MegaApi *api, MegaRequest *request, MegaError e);
	void fireOnTransferStart(MegaApi* api, MegaTransfer *transfer);
	void fireOnTransferFinish(MegaApi* api, MegaTransfer *transfer, MegaError e);
	void fireOnTransferUpdate(MegaApi *api, MegaTransfer *transfer);
	void fireOnTransferTemporaryError(MegaApi *api, MegaTransfer *transfer, MegaError e);	
	void fireOnUsersUpdate(MegaApi* api, UserList *users);
	void fireOnNodesUpdate(MegaApi* api, NodeList *nodes);
	void fireOnReloadNeeded(MegaApi* api);

	pthread_t thread;
	MegaClient *client;
    MegaHttpIO *httpio;
    MegaWaiter *waiter;
    MegaFileSystemAccess *fsAccess;
    MegaDbAccess *dbAccess;

	RequestQueue requestQueue;
	TransferQueue transferQueue;
	map<int, MegaRequest *> requestMap;
    map<Transfer*, MegaTransfer *> transferMap;

	set<MegaRequestListener *> requestListeners;
	set<MegaTransferListener *> transferListeners;
	set<MegaGlobalListener *> globalListeners;
	set<MegaListener *> listeners;

	pthread_mutex_t listenerMutex;
	pthread_mutex_t transferListenerMutex;
	pthread_mutex_t requestListenerMutex;
	pthread_mutex_t globalListenerMutex;
	pthread_mutex_t fileSystemMutex;

	MegaRequest *loginRequest;
	MegaTransfer *currentTransfer;
	int updatingSID;
	long long updateSIDtime;
	int threadExit;
	
	handle cwd;
	void loop();
	
	int maxRetries;

    // a request-level error occurred
    virtual void request_error(error);

    // login result
	virtual void login_result(error);

    // ephemeral session creation/resumption result
    virtual void ephemeral_result(error);
    virtual void ephemeral_result(handle, const byte*);

    // account creation
    virtual void sendsignuplink_result(error);
    virtual void querysignuplink_result(error);
    virtual void querysignuplink_result(handle, const char*, const char*, const byte*, const byte*, const byte*, size_t);
    virtual void confirmsignuplink_result(error);
    virtual void setkeypair_result(error);

    // account credentials, properties and history
    virtual void account_details(AccountDetails*,  bool, bool, bool, bool, bool, bool);
	virtual void account_details(AccountDetails*, error);

	virtual void setattr_result(handle, error);    
	virtual void rename_result(handle, error);
	virtual void unlink_result(handle, error);
	virtual void nodes_updated(Node**, int);
	virtual void users_updated(User**, int);

    // password change result
    virtual void changepw_result(error);

    // user attribute update notification
    virtual void userattr_update(User*, int, const char*);


	virtual void fetchnodes_result(error);
	virtual void putnodes_result(error, targettype, NewNode*);

    // share update result
	virtual void share_result(error);
	virtual void share_result(int, error);

    // file attribute fetch result
	virtual void fa_complete(Node*, fatype, const char*, uint32_t);
	virtual int fa_failed(handle, fatype, int);

    // file attribute modification result
	virtual void putfa_result(handle, fatype, error);

    // purchase transactions
    virtual void enumeratequotaitems_result(handle, unsigned, unsigned, unsigned, unsigned, unsigned, const char*) { }
    virtual void enumeratequotaitems_result(error) { }
    virtual void additem_result(error) { }
    virtual void checkout_result(error) { }
    virtual void checkout_result(const char*) { }

	virtual void checkfile_result(handle h, error e);
	virtual void checkfile_result(handle h, error e, byte* filekey, m_off_t size, time_t ts, time_t tm, string* filename, string* fingerprint, string* fileattrstring);

	// user invites/attributes
    virtual void invite_result(error);
    virtual void putua_result(error);
    virtual void getua_result(error);
    virtual void getua_result(byte*, unsigned);

    // file node export result
	virtual void exportnode_result(error);
	virtual void exportnode_result(handle, handle);

    // exported link access result
	virtual void openfilelink_result(error);
	virtual void openfilelink_result(handle, const byte*, m_off_t, string*, const char*, time_t, time_t, int);

    // global transfer queue updates (separate signaling towards the queued objects)
    virtual void transfer_added(Transfer*);
    virtual void transfer_removed(Transfer*);
    virtual void transfer_prepare(Transfer*);
    virtual void transfer_failed(Transfer*, error error);
    virtual void transfer_update(Transfer*);
    virtual void transfer_limit(Transfer*);
    virtual void transfer_complete(Transfer*);

	// sync status updates and events
	virtual void syncupdate_state(Sync*, syncstate);
	virtual void syncupdate_stuck(string*);
	virtual void syncupdate_local_folder_addition(Sync*, const char*);
	virtual void syncupdate_local_folder_deletion(Sync*, const char*);
	virtual void syncupdate_local_file_addition(Sync*, const char*);
	virtual void syncupdate_local_file_deletion(Sync*, const char*);
	virtual void syncupdate_get(Sync*, const char*);
	virtual void syncupdate_put(Sync*, const char*);
	virtual void syncupdate_remote_file_addition(Node*);
	virtual void syncupdate_remote_file_deletion(Node*);
	virtual void syncupdate_remote_folder_addition(Node*);
	virtual void syncupdate_remote_folder_deletion(Node*);
	virtual void syncupdate_remote_copy(Sync*, const char*);
	virtual void syncupdate_remote_move(string*, string*);

	virtual bool sync_syncable(Node*);
	virtual bool sync_syncable(const char*, string*, string*);

    // suggest reload due to possible race condition with other clients
	virtual void reload(const char*);

    // wipe all users, nodes and shares
	virtual void clearing();

    // failed request retry notification
	virtual void notify_retry(dstime);

    // generic debug logging
	virtual void debug_log(const char*);
	//////////
	
	void sendPendingRequests();
	void sendPendingTransfers();
	char *stringToArray(string &buffer);
};


#endif //MEGAAPI_H

