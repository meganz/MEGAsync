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

#include "mega.h"

#ifdef USE_PTHREAD
#include <pthread.h>

#define DECLARE_MUTEX(mutex) pthread_mutex_t *mutex
#define INIT_MUTEX(mutex)                                       \
    mutex = new pthread_mutex_t;                                \
    pthread_mutex_init(mutex, NULL);

#define INIT_RECURSIVE_MUTEX(mutex)                             \
{                                                               \
    mutex = new pthread_mutex_t;                                \
    pthread_mutexattr_t attr;                                   \
    pthread_mutexattr_init(&attr);                              \
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);  \
    pthread_mutex_init(mutex, &attr);                \
}


#define MUTEX_LOCK(mutex) pthread_mutex_lock(mutex);
#define MUTEX_UNLOCK(mutex) pthread_mutex_unlock(mutex);
#define MUTEX_DELETE(mutex) delete mutex;

#define DECLARE_THREAD(thread) pthread_t thread;
#define INIT_THREAD(thread, threadEntryPoint, pointer) 	pthread_create(&thread, NULL, threadEntryPoint, pointer);
#define JOIN_THREAD(thread) pthread_join(thread, NULL);
#define DELETE_THREAD(thread)

#elif USE_QT
#include <QThread>
#include <QMutex>
#include "mega/gfx/qt.h"

class MegaThread: public QThread
{
public:
    MegaThread(void *(*start_routine)(void*), void *pointer)
    {
        this->start_routine = start_routine;
        this->pointer = pointer;
    }

protected:
    void *(*start_routine)(void*);
    void *pointer;

    virtual void run()
    {
        start_routine(pointer);
    }
};

#define DECLARE_MUTEX(mutex) QMutex *mutex;
#define INIT_MUTEX(mutex) mutex = new QMutex();
#define INIT_RECURSIVE_MUTEX(mutex) mutex = new QMutex(QMutex::Recursive);
#define MUTEX_LOCK(mutex) mutex->lock();
#define MUTEX_UNLOCK(mutex) mutex->unlock();
#define MUTEX_DELETE(mutex) delete mutex;

#define DECLARE_THREAD(thread) MegaThread *thread;
#define INIT_THREAD(thread, threadEntryPoint, pointer) \
    thread = new MegaThread(threadEntryPoint, pointer);\
    thread->start();

#define JOIN_THREAD(thread) thread->wait();
#define DELETE_THREAD(thread) delete thread;

class MegaGfxProc : public mega::GfxProcQT {};

#endif

#ifdef USE_ANDROID
class GfxProcessor
{
public:
	virtual bool readBitmap(const char* path) { return false; }
	virtual int getWidth() { return 0; }
	virtual int getHeight() { return 0; }
	virtual char *resizeBitmap(int w, int h, int px, int py, int rw, int rh) { return NULL; }
	virtual void freeBitmap() {}
	virtual ~GfxProcessor() {};
};

#include "mega/gfx/android.h"
class MegaGfxProc : public mega::GfxProcAndroid {};
#endif

using namespace std;

#ifdef WIN32

#include "win32/megaapiwinhttpio.h"
#include "mega/win32/megafs.h"
#include "win32/megaapiwait.h"
#include "mega.h"
#include "MegaProxySettings.h"

class MegaHttpIO : public mega::MegaApiWinHttpIO {};
class MegaFileSystemAccess : public mega::WinFileSystemAccess {};
class MegaWaiter : public mega::MegaApiWinWaiter {};

#else

#ifdef __ANDROID__
#include "android/megaapiandroidhttpio.h"
#else
#include "linux/megaapiposixhttpio.h"
#endif

#include "mega/posix/meganet.h"
#include "mega/posix/megafs.h"
#include "linux/megaapiwait.h"

class MegaHttpIO : public mega::MegaApiCurlHttpIO {};
class MegaFileSystemAccess : public mega::PosixFileSystemAccess {};
class MegaWaiter : public mega::MegaApiLinuxWaiter {};

#endif

class MegaDbAccess : public mega::SqliteDbAccess
{
public:
    MegaDbAccess(string *basePath = NULL) : SqliteDbAccess(basePath){}
};

class MegaApi;
class MegaNode
{
    public:

		enum {
			TYPE_UNKNOWN = -1,
			TYPE_FILE = 0,
			TYPE_FOLDER,
			TYPE_ROOT,
			TYPE_INCOMING,
			TYPE_RUBBISH,
			TYPE_MAIL
		};

        MegaNode(const char *name, int type, m_off_t size, time_t ctime, time_t mtime, mega::handle nodehandle, string *nodekey, string *attrstring);
        MegaNode(MegaNode *node);
        ~MegaNode();

        MegaNode *copy();
        static MegaNode *fromNode(mega::Node *node);

        int getType();
        const char* getName();
        const char *getBase64Handle();
        m_off_t getSize();
        time_t getCreationTime();
        time_t getModificationTime();
        mega::handle getHandle();
        string* getNodeKey();
        string* getAttrString();
        int getTag();
        bool isFile();
        bool isFolder();
        bool isRemoved();
        bool isSyncDeleted();
        string getLocalPath();
        bool hasThumbnail();
        bool hasPreview();

    private:
        MegaNode(mega::Node *node);

        int type;
        const char *name;
        m_off_t size;
        time_t ctime;
        time_t mtime;
        mega::handle nodehandle;
        string nodekey;
        string attrstring;
        string localPath;

        int tag;
        bool removed;
        bool syncdeleted;
        bool thumbnailAvailable;
        bool previewAvailable;
};

class MegaUser
{
	public:
		enum {
			VISIBILITY_UNKNOWN = -1,
			VISIBILITY_HIDDEN = 0,
			VISIBILITY_VISIBLE,
			VISIBILITY_ME
		};

		~MegaUser();
		const char* getEmail();
		int getVisibility();
		time_t getTimestamp();
        static MegaUser *fromUser(mega::User *user);

	private:
        MegaUser(mega::User *user);

		const char *email;
		int visibility;
		time_t ctime;
};

struct MegaFile : public mega::File
{
    // app-internal sequence number for queue management
    int seqno;
    static int nextseqno;

    bool failed(mega::error e);
    MegaFile();
};

struct MegaFileGet : public MegaFile
{
    void completed(mega::Transfer*, mega::LocalNode*);
	MegaFileGet(mega::MegaClient *client, mega::Node* n, string dstPath);
    MegaFileGet(mega::MegaClient *client, MegaNode* n, string dstPath);
	~MegaFileGet() {}
};

struct MegaFilePut : public MegaFile
{
    void completed(mega::Transfer* t, mega::LocalNode*);
    MegaFilePut(mega::MegaClient *client, string* clocalname, mega::handle ch, const char* ctargetuser);
    ~MegaFilePut() {}
};

class NodeList
{
	public:
		NodeList();
		NodeList(mega::Node** newlist, int size);
		virtual ~NodeList();

		MegaNode* get(int i);
		int size();

	protected:
		MegaNode** list;
		int s;
};

class UserList
{
	public:
		UserList();
		UserList(mega::User** newlist, int size);
		virtual ~UserList();

		MegaUser* get(int i);
		int size();

	protected:
		MegaUser** list;
		int s;
};

//typedef ArrayWrapper<AccountBalance> BalanceList;
//typedef ArrayWrapper<AccountSession> SessionList;
//typedef ArrayWrapper<AccountPurchase> PurchaseList;
//typedef ArrayWrapper<AccountTransaction> TransactionList;
//typedef ArrayWrapper<const char*> StringList;
//typedef ArrayWrapper<Share*> ShareList;

class MegaRequestListener;
class MegaTransferListener;
class MegaTransfer;
class MegaApi;

//Encapsulates the information about a request instead of return it as individual parameters in the callbacks.
//This allow extending the information adding more getters without breaking client's code.
class MegaRequest
{
	public:
        enum {  TYPE_LOGIN, TYPE_MKDIR, TYPE_MOVE, TYPE_COPY,
                TYPE_RENAME, TYPE_REMOVE, TYPE_SHARE,
                TYPE_FOLDER_ACCESS, TYPE_IMPORT_LINK, TYPE_IMPORT_NODE,
                TYPE_EXPORT, TYPE_FETCH_NODES, TYPE_ACCOUNT_DETAILS,
                TYPE_CHANGE_PW, TYPE_UPLOAD, TYPE_LOGOUT, TYPE_FAST_LOGIN,
                TYPE_GET_PUBLIC_NODE, TYPE_GET_ATTR_FILE,
                TYPE_SET_ATTR_FILE, TYPE_GET_ATTR_USER,
                TYPE_SET_ATTR_USER, TYPE_RETRY_PENDING_CONNECTIONS,
                TYPE_ADD_CONTACT, TYPE_REMOVE_CONTACT, TYPE_CREATE_ACCOUNT, TYPE_FAST_CREATE_ACCOUNT,
                TYPE_CONFIRM_ACCOUNT, TYPE_FAST_CONFIRM_ACCOUNT,
                TYPE_QUERY_SIGNUP_LINK, TYPE_ADD_SYNC, TYPE_REMOVE_SYNC,
                TYPE_REMOVE_SYNCS, TYPE_PAUSE_TRANSFERS,
                TYPE_CANCEL_TRANSFER, TYPE_CANCEL_TRANSFERS,
                TYPE_DELETE };

		MegaRequest(int type, MegaRequestListener *listener = NULL);
		MegaRequest(MegaRequest &request);
		virtual ~MegaRequest();

		MegaRequest *copy();
		int getType() const;
		const char *getRequestString() const;
		const char* toString() const;
		const char* __str__() const;

		mega::handle getNodeHandle() const;
		const char* getLink() const;
		mega::handle getParentHandle() const;
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
        MegaNode *getPublicNode();
        int getParamType() const;
        bool getFlag() const;
        long long getTransferredBytes() const;
        long long getTotalBytes() const;

		void setNodeHandle(mega::handle nodeHandle);
		void setLink(const char* link);
		void setParentHandle(mega::handle parentHandle);
		void setUserHandle(const char* userHandle);
		void setName(const char* name);
		void setEmail(const char* email);
    	void setPassword(const char* email);
    	void setNewPassword(const char* email);
		void setPrivateKey(const char* privateKey);
		void setAccess(const char* access);
		void setNumRetry(int ds);
		void setNextRetryDelay(int delay);
        void setPublicNode(MegaNode* publicNode);
		void setNumDetails(int numDetails);
		void setFile(const char* file);
        void setParamType(int type);
        void setFlag(bool flag);
        void setTransfer(mega::Transfer *transfer);
        void setListener(MegaRequestListener *listener);
        void setTotalBytes(long long totalBytes);
        void setTransferredBytes(long long transferredBytes);
		MegaRequestListener *getListener() const;
        mega::Transfer * getTransfer() const;
		mega::AccountDetails * getAccountDetails() const;
		int getNumDetails() const;

	protected:
		int type;
		mega::handle nodeHandle;
		const char* link;
		const char* name;
		mega::handle parentHandle;
		const char* userHandle;
		const char* email;
		const char* password;
		const char* newPassword;
		const char* privateKey;
		const char* access;
		const char* file;
		int attrType;
        bool flag;
        long long totalBytes;
        long long transferredBytes;

		MegaRequestListener *listener;
        mega::Transfer *transfer;
		mega::AccountDetails *accountDetails;
		int numDetails;
        MegaNode* publicNode;
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
		mega::handle getNodeHandle() const;
		mega::handle getParentHandle() const;
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
        mega::Transfer *getTransfer() const;
		long long getSpeed() const;
		long long getDeltaSize() const;
		long long getUpdateTime() const;
        MegaNode *getPublicNode() const;
        bool isSyncTransfer() const;

		void setStartTime(long long startTime);
		void setTransferredBytes(long long transferredBytes);
		void setTotalBytes(long long totalBytes);
		void setPath(const char* path);
		void setParentPath(const char* path);
		void setNodeHandle(mega::handle nodeHandle);
		void setParentHandle(mega::handle parentHandle);
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
        void setTransfer(mega::Transfer *transfer);
		void setSpeed(long long speed);
		void setDeltaSize(long long deltaSize);
		void setUpdateTime(long long updateTime);
        void setPublicNode(MegaNode *publicNode);
        void setSyncTransfer(bool syncTransfer);

	protected:
		int slot;
		int type;
		int tag;
        bool syncTransfer;
		long long startTime;
		long long updateTime;
		long long time;
		long long transferredBytes;
		long long totalBytes;
		long long speed;
		long long deltaSize;
		mega::handle nodeHandle;
		mega::handle parentHandle;
		const char* path;
		const char* parentPath;
		const char* fileName;
		const char* base64Key;
        MegaNode *publicNode;

		int numConnections;
		long long startPos;
		long long endPos;
		int maxSpeed;
		int retry;
		int maxRetries;

        mega::Transfer *transfer;

		MegaTransferListener *listener;

		//Might be useful for streaming
		char *lastBuffer;
		long lastBufferStartOffset;
		int lastBufferlength;
};

class TransferList
{
	public:
		TransferList();
		TransferList(MegaTransfer** newlist, int size);
		virtual ~TransferList();

		MegaTransfer* get(int i);
		int size();

	protected:
		MegaTransfer** list;
		int s;
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

	#ifdef USE_QT
        QString QgetErrorString() const;
        static QString QgetErrorString(int errorCode);
	#endif

	protected:
        //< 0 = API error code, > 0 = http error, 0 = No error
		int errorCode;
		long nextAttempt;
};


class TreeProcessor
{
	public:
	virtual int processNode(mega::Node* node);
	virtual ~TreeProcessor();
};

class SearchTreeProcessor : public TreeProcessor
{
    public:
	SearchTreeProcessor(const char *search);
	virtual int processNode(mega::Node* node);
    virtual ~SearchTreeProcessor() {}
    vector<mega::Node *> &getResults();

    protected:
	const char *search;
    vector<mega::Node *> results;
};

class SizeProcessor : public TreeProcessor
{
    long long totalBytes;
public:
    SizeProcessor();
    virtual int processNode(mega::Node* node);
    long long getTotalBytes();
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
    virtual void onRequestUpdate(MegaApi* api, MegaRequest *request);
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
#ifdef __ANDROID__
	virtual void onUsersUpdate(MegaApi* api);
	virtual void onNodesUpdate(MegaApi* api);
#else
    virtual void onUsersUpdate(MegaApi* api, UserList *users);
    virtual void onNodesUpdate(MegaApi* api, NodeList *nodes);
#endif
	virtual void onReloadNeeded(MegaApi* api);
	virtual ~MegaGlobalListener();
};

//All callbacks (no multiple inheritance because it isn't available in other programming languages)
class MegaListener
{
	public:
	virtual void onRequestStart(MegaApi* api, MegaRequest *request);
	virtual void onRequestFinish(MegaApi* api, MegaRequest *request, MegaError* e);
    virtual void onRequestUpdate(MegaApi* api, MegaRequest *request);
	virtual void onRequestTemporaryError(MegaApi *api, MegaRequest *request, MegaError* e);
	virtual void onTransferStart(MegaApi *api, MegaTransfer *transfer);
	virtual void onTransferFinish(MegaApi* api, MegaTransfer *transfer, MegaError* e);
	virtual void onTransferUpdate(MegaApi *api, MegaTransfer *transfer);
	virtual void onTransferTemporaryError(MegaApi *api, MegaTransfer *transfer, MegaError* e);
#ifdef __ANDROID__
	virtual void onUsersUpdate(MegaApi* api);
	virtual void onNodesUpdate(MegaApi* api);
#else
    virtual void onUsersUpdate(MegaApi* api, UserList *users);
    virtual void onNodesUpdate(MegaApi* api, NodeList *nodes);
#endif
	virtual void onReloadNeeded(MegaApi* api);
    virtual void onSyncStateChanged(MegaApi *api);

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
        DECLARE_MUTEX(mutex);

    public:
	RequestQueue()
	{
        INIT_MUTEX(mutex);
	}

	void push(MegaRequest *request)
	{
        MUTEX_LOCK(mutex);
	    requests.push_back(request);
        MUTEX_UNLOCK(mutex);
	}

	void push_front(MegaRequest *request)
	{
        MUTEX_LOCK(mutex);
	    requests.push_front(request);
        MUTEX_UNLOCK(mutex);
	}

	MegaRequest * pop()
	{
        MUTEX_LOCK(mutex);
	    if(requests.empty())
	    {
            MUTEX_UNLOCK(mutex);
            return NULL;
	    }
	    MegaRequest *request = requests.front();
	    requests.pop_front();
        MUTEX_UNLOCK(mutex);
	    return request;
	}

    void removeListener(MegaRequestListener *listener)
    {
        MUTEX_LOCK(mutex);

        std::deque<MegaRequest *>::iterator it = requests.begin();
        while(it != requests.end())
        {
            MegaRequest *request = (*it);
            if(request->getListener()==listener)
                request->setListener(NULL);
            it++;
        }

        MUTEX_UNLOCK(mutex);
    }
};


//Thread safe transfer queue
class TransferQueue
{
    protected:
        std::deque<MegaTransfer *> transfers;
        DECLARE_MUTEX(mutex);

    public:
    TransferQueue()
	{
        INIT_MUTEX(mutex);
    }

	void push(MegaTransfer *transfer)
	{
        MUTEX_LOCK(mutex);
	    transfers.push_back(transfer);
        MUTEX_UNLOCK(mutex);
	}

	void push_front(MegaTransfer *transfer)
	{
        MUTEX_LOCK(mutex);
	    transfers.push_front(transfer);
        MUTEX_UNLOCK(mutex);
	}

	MegaTransfer * pop()
	{
        MUTEX_LOCK(mutex);
	    if(transfers.empty())
	    {
            MUTEX_UNLOCK(mutex);
	    	return NULL;
	    }
	    MegaTransfer *transfer = transfers.front();
	    transfers.pop_front();
        MUTEX_UNLOCK(mutex);
	    return transfer;
	}
};

class MegaApi : public mega::MegaApp
{

public:
	enum {	ORDER_NONE, ORDER_DEFAULT_ASC, ORDER_DEFAULT_DESC,
		ORDER_SIZE_ASC, ORDER_SIZE_DESC,
		ORDER_CREATION_ASC, ORDER_CREATION_DESC,
		ORDER_MODIFICATION_ASC, ORDER_MODIFICATION_DESC,
		ORDER_ALPHABETICAL_ASC, ORDER_ALPHABETICAL_DESC};

    static bool nodeComparatorDefaultASC  (mega::Node *i, mega::Node *j);
    static bool nodeComparatorDefaultDESC (mega::Node *i, mega::Node *j);
    static bool nodeComparatorSizeASC  (mega::Node *i, mega::Node *j);
    static bool nodeComparatorSizeDESC (mega::Node *i, mega::Node *j);
    static bool nodeComparatorCreationASC  (mega::Node *i, mega::Node *j);
    static bool nodeComparatorCreationDESC  (mega::Node *i, mega::Node *j);
    static bool nodeComparatorModificationASC  (mega::Node *i, mega::Node *j);
    static bool nodeComparatorModificationDESC  (mega::Node *i, mega::Node *j);
    static bool nodeComparatorAlphabeticalASC  (mega::Node *i, mega::Node *j);
    static bool nodeComparatorAlphabeticalDESC  (mega::Node *i, mega::Node *j);
	static bool userComparatorDefaultASC (mega::User *i, mega::User *j);

	#ifdef __ANDROID__
		MegaApi(const char *basePath = NULL, GfxProcessor* processor = NULL);
	#else
		MegaApi(const char *basePath = NULL);
	#endif

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
	static mega::handle base64ToHandle(const char* base64Handle);
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
    void setProxySettings(MegaProxySettings *proxySettings);
    MegaProxySettings *getAutoProxySettings();
    int isLoggedIn();
	const char* getMyEmail();

    void createFolder(const char* name, MegaNode *parent, MegaRequestListener *listener = NULL);
    void moveNode(MegaNode* node, MegaNode* newParent, MegaRequestListener *listener = NULL);
    void copyNode(MegaNode* node, MegaNode *newParent, MegaRequestListener *listener = NULL);
    void renameNode(MegaNode* node, const char* newName, MegaRequestListener *listener = NULL);
    void remove(MegaNode* node, MegaRequestListener *listener = NULL);
    void share(MegaNode *node, mega::User* user, const char *level, MegaRequestListener *listener = NULL);
    void share(MegaNode* node, const char* email, const char *level, MegaRequestListener *listener = NULL);
	void folderAccess(const char* megaFolderLink, MegaRequestListener *listener = NULL);
	void importFileLink(const char* megaFileLink, MegaNode* parent, MegaRequestListener *listener = NULL);
    void importPublicNode(MegaNode *publicNode, MegaNode *parent, MegaRequestListener *listener = NULL);
	void getPublicNode(const char* megaFileLink, MegaRequestListener *listener = NULL);
	void getThumbnail(MegaNode* node, char *dstFilePath, MegaRequestListener *listener = NULL);
	void setThumbnail(MegaNode* node, char *srcFilePath, MegaRequestListener *listener = NULL);
	void getPreview(MegaNode* node, char *dstFilePath, MegaRequestListener *listener = NULL);
	void setPreview(MegaNode* node, char *srcFilePath, MegaRequestListener *listener = NULL);
	void getUserAvatar(MegaUser* user, char *dstFilePath, MegaRequestListener *listener = NULL);
	//void setUserAvatar(MegaUser* user, char *srcFilePath, MegaRequestListener *listener = NULL);
	void exportNode(MegaNode *node, MegaRequestListener *listener = NULL);
	void fetchNodes(MegaRequestListener *listener = NULL);
	void getAccountDetails(MegaRequestListener *listener = NULL);
	void changePassword(const char *oldPassword, const char *newPassword, MegaRequestListener *listener = NULL);
	void addContact(const char* email, MegaRequestListener* listener=NULL);
	void removeContact(const char* email, MegaRequestListener* listener=NULL);
	void logout(MegaRequestListener *listener = NULL);

	//Transfers
    void startUpload(const char* localPath, MegaNode *parent, MegaTransferListener *listener=NULL);
    void startUpload(const char* localPath, MegaNode* parent, const char* fileName, MegaTransferListener *listener = NULL);
    void startDownload(MegaNode* node, const char* localFolder, MegaTransferListener *listener = NULL);
    void startPublicDownload(MegaNode* node, const char* localFolder, MegaTransferListener *listener = NULL);
    //	void startPublicDownload(handle nodehandle, const char * base64key, const char* localFolder, MegaTransferListener *listener = NULL);
    bool isRegularTransfer(MegaTransfer *transfer);
    void cancelTransfer(MegaTransfer *transfer, MegaRequestListener *listener=NULL);
    void cancelTransfers(int direction, MegaRequestListener *listener=NULL);
    void pauseTransfers(bool pause, MegaRequestListener* listener=NULL);
    void setUploadLimit(int bpslimit);
    TransferList *getTransfers();

    //Sync
    mega::treestate_t syncPathState(string *path);
    MegaNode *getSyncedNode(string *path);
    void syncFolder(const char *localFolder, MegaNode *megaFolder);
    void removeSync(mega::handle nodehandle, MegaRequestListener *listener=NULL);
    int getNumActiveSyncs();
    void stopSyncs(MegaRequestListener *listener=NULL);
    int getNumPendingUploads();
    int getNumPendingDownloads();
    int getTotalUploads();
    int getTotalDownloads();
    void resetTotalDownloads();
    void resetTotalUploads();
    string getLocalPath(MegaNode *node);
    void updateStatics();
    void update();
    bool isIndexing();
    bool isWaiting();
    bool isSynced(MegaNode *n);
    void setExcludedNames(vector<string> *excludedNames);
    bool moveToLocalDebris(const char *path);
    bool is_syncable(const char* name);

	//Filesystem
    NodeList* getChildren(MegaNode *parent, int order=1);
    MegaNode *getParentNode(MegaNode *node);
    const char* getNodePath(MegaNode *node);
    MegaNode *getNodeByPath(const char *path, MegaNode *n = NULL);
    MegaNode *getNodeByHandle(mega::handle handler);
    UserList* getContacts();
    MegaUser* getContact(const char* email);
	NodeList *getInShares(MegaUser* user);
	NodeList *getInShares();
    //ShareList *getOutShares(Node *node);
	const char *getAccess(MegaNode* node);
    long long getSize(MegaNode *node);

	MegaError checkAccess(MegaNode* node, const char *level);
	MegaError checkMove(MegaNode* node, MegaNode* target);

    MegaNode *getRootNode();
    MegaNode* getInboxNode();
    MegaNode *getRubbishNode();
    MegaNode* getMailNode();
	//StringList *getRootNodeNames();
	//StringList *getRootNodePaths();

	//Debug
	void setDebug(bool debug);
	bool getDebug();

	//General porpuse
	static char* strdup(const char* buffer);

	//Do not use
	static void *threadEntryPoint(void *param);

protected:
	//static const char* rootnodenames[];
	//static const char* rootnodepaths[];
	//static StringList *rootNodeNames;
	//static StringList *rootNodePaths;

	void fireOnRequestStart(MegaApi* api, MegaRequest *request);
	void fireOnRequestFinish(MegaApi* api, MegaRequest *request, MegaError e);
    void fireOnRequestUpdate(MegaApi* api, MegaRequest *request);
	void fireOnRequestTemporaryError(MegaApi *api, MegaRequest *request, MegaError e);
	void fireOnTransferStart(MegaApi* api, MegaTransfer *transfer);
	void fireOnTransferFinish(MegaApi* api, MegaTransfer *transfer, MegaError e);
	void fireOnTransferUpdate(MegaApi *api, MegaTransfer *transfer);
	void fireOnTransferTemporaryError(MegaApi *api, MegaTransfer *transfer, MegaError e);
    void fireOnUsersUpdate(MegaApi* api, UserList *users);
    void fireOnNodesUpdate(MegaApi* api, NodeList *nodes);
	void fireOnReloadNeeded(MegaApi* api);
    void fireOnSyncStateChanged(MegaApi* api);

    DECLARE_THREAD(thread);
	mega::MegaClient *client;
    MegaHttpIO *httpio;
    MegaWaiter *waiter;
    MegaFileSystemAccess *fsAccess;
    MegaDbAccess *dbAccess;
    MegaGfxProc *gfxAccess;

	RequestQueue requestQueue;
	TransferQueue transferQueue;
	map<int, MegaRequest *> requestMap;
    map<mega::Transfer*, MegaTransfer *> transferMap;
    int pendingUploads;
    int pendingDownloads;
    int totalUploads;
    int totalDownloads;
	set<MegaRequestListener *> requestListeners;
	set<MegaTransferListener *> transferListeners;
	set<MegaGlobalListener *> globalListeners;
	set<MegaListener *> listeners;
    bool waiting;
    bool waitingRequest;
    vector<string> excludedNames;

    DECLARE_MUTEX(sdkMutex);

	MegaRequest *loginRequest;
	MegaTransfer *currentTransfer;
	int updatingSID;
	long long updateSIDtime;
	int threadExit;
    dstime pausetime;
	void loop();

	int maxRetries;

    // a request-level error occurred
    virtual void request_error(mega::error);

    virtual void request_response_progress(m_off_t, m_off_t);

    // login result
	virtual void login_result(mega::error);

    // ephemeral session creation/resumption result
    virtual void ephemeral_result(mega::error);
    virtual void ephemeral_result(mega::handle, const byte*);

    // account creation
    virtual void sendsignuplink_result(mega::error);
    virtual void querysignuplink_result(mega::error);
    virtual void querysignuplink_result(mega::handle, const char*, const char*, const byte*, const byte*, const byte*, size_t);
    virtual void confirmsignuplink_result(mega::error);
    virtual void setkeypair_result(mega::error);

    // account credentials, properties and history
    virtual void account_details(mega::AccountDetails*,  bool, bool, bool, bool, bool, bool);
	virtual void account_details(mega::AccountDetails*, mega::error);

	virtual void setattr_result(mega::handle, mega::error);
	virtual void rename_result(mega::handle, mega::error);
	virtual void unlink_result(mega::handle, mega::error);
	virtual void nodes_updated(mega::Node**, int);
	virtual void users_updated(mega::User**, int);

    // password change result
    virtual void changepw_result(mega::error);

    // user attribute update notification
    virtual void userattr_update(mega::User*, int, const char*);


	virtual void fetchnodes_result(mega::error);
    virtual void putnodes_result(mega::error, mega::targettype_t, mega::NewNode*);

    // share update result
	virtual void share_result(mega::error);
	virtual void share_result(int, mega::error);

    // file attribute fetch result
	virtual void fa_complete(mega::Node*, mega::fatype, const char*, uint32_t);
	virtual int fa_failed(mega::handle, mega::fatype, int);

    // file attribute modification result
	virtual void putfa_result(mega::handle, mega::fatype, mega::error);

    // purchase transactions
    virtual void enumeratequotaitems_result(mega::handle, unsigned, unsigned, unsigned, unsigned, unsigned, const char*) { }
    virtual void enumeratequotaitems_result(mega::error) { }
    virtual void additem_result(mega::error) { }
    virtual void checkout_result(mega::error) { }
    virtual void checkout_result(const char*) { }

	virtual void checkfile_result(mega::handle h, mega::error e);
    virtual void checkfile_result(mega::handle h, mega::error e, byte* filekey, m_off_t size, m_time_t ts, m_time_t tm, string* filename, string* fingerprint, string* fileattrstring);

	// user invites/attributes
    virtual void invite_result(mega::error);
    virtual void putua_result(mega::error);
    virtual void getua_result(mega::error);
    virtual void getua_result(byte*, unsigned);

    // file node export result
	virtual void exportnode_result(mega::error);
	virtual void exportnode_result(mega::handle, mega::handle);

    // exported link access result
	virtual void openfilelink_result(mega::error);
    virtual void openfilelink_result(mega::handle, const byte*, m_off_t, string*, const char*, m_time_t, m_time_t, int);

    // global transfer queue updates (separate signaling towards the queued objects)
    virtual void transfer_added(mega::Transfer*);
    virtual void transfer_removed(mega::Transfer*);
    virtual void transfer_prepare(mega::Transfer*);
    virtual void transfer_failed(mega::Transfer*, mega::error error);
    virtual void transfer_update(mega::Transfer*);
    virtual void transfer_limit(mega::Transfer*);
    virtual void transfer_complete(mega::Transfer*);

	// sync status updates and events
    virtual void syncupdate_state(mega::Sync*, mega::syncstate_t);
    virtual void syncupdate_scanning(bool scanning);
	virtual void syncupdate_stuck(string*);
	virtual void syncupdate_local_folder_addition(mega::Sync*, const char*);
	virtual void syncupdate_local_folder_deletion(mega::Sync*, const char*);
	virtual void syncupdate_local_file_addition(mega::Sync*, const char*);
	virtual void syncupdate_local_file_deletion(mega::Sync*, const char*);
	virtual void syncupdate_get(mega::Sync*, const char*);
	virtual void syncupdate_put(mega::Sync*, const char*);
	virtual void syncupdate_remote_file_addition(mega::Node*);
	virtual void syncupdate_remote_file_deletion(mega::Node*);
	virtual void syncupdate_remote_folder_addition(mega::Node*);
	virtual void syncupdate_remote_folder_deletion(mega::Node*);
	virtual void syncupdate_remote_copy(mega::Sync*, const char*);
	virtual void syncupdate_remote_move(string*, string*);
    virtual void syncupdate_treestate(mega::LocalNode*);
	virtual bool sync_syncable(mega::Node*);
    virtual bool sync_syncable(const char*name, string*, string*);
    virtual void syncupdate_local_lockretry(bool);

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

	//Pending
	mega::Node* getChildNode(mega::Node *parent, const char* name);
	bool processTree(mega::Node* node, TreeProcessor* processor, bool recursive = 1);
	NodeList* search(mega::Node* node, const char* searchString, bool recursive = 1);
	void getAccountDetails(int storage, int transfer, int pro, int transactions, int purchases, int sessions, MegaRequestListener *listener = NULL);
    void getNodeAttribute(MegaNode* node, int type, char *dstFilePath, MegaRequestListener *listener = NULL);
    void setNodeAttribute(MegaNode* node, int type, char *srcFilePath, MegaRequestListener *listener = NULL);
    void getUserAttribute(MegaUser* node, int type, char *dstFilePath, MegaRequestListener *listener = NULL);
    void setUserAttribute(MegaUser* node, int type, char *srcFilePath, MegaRequestListener *listener = NULL);
    void startUpload(const char* localPath, MegaNode* parent, int connections, int maxSpeed, const char* fileName, MegaTransferListener *listener);
    void startUpload(const char* localPath, MegaNode* parent, int maxSpeed, MegaTransferListener *listener = NULL);
	void startDownload(mega::handle nodehandle, const char* target, int connections, long startPos, long endPos, const char* base64key, MegaTransferListener *listener);
    void startDownload(MegaNode* node, const char* localFolder, int connections, long startPos, long endPos, const char* base64key, MegaTransferListener *listener);
    void startDownload(MegaNode* node, const char* localFolder, long startPos, long endPos, MegaTransferListener *listener);
    bool checkTransfer(mega::Transfer *transfer);
    void cancelTransfer(mega::Transfer *t, MegaRequestListener *listener=NULL);
    bool isRegularTransfer(mega::Transfer *transfer);
};


#endif //MEGAAPI_H

