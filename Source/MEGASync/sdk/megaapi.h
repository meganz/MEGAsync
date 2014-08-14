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

#include <inttypes.h>
typedef int64_t m_off_t;

#include "mega.h"
#include "mega/thread/posixthread.h"
#include "mega/thread/qtthread.h"
#include "mega/gfx/qt.h"
#include "mega/gfx/external.h"
#include "mega/thread/cppthread.h"
#include "mega/proxy.h"

#ifndef _WIN32
#include <openssl/ssl.h>
#include <curl/curl.h>
#include <fcntl.h>
#endif

////////////////////////////// SETTINGS //////////////////////////////
////////// Support for threads and mutexes
//Choose one of these options.
//Otherwise, C++11 threads and mutexes will be used

//#define USE_PTHREAD
//#define USE_QT

////////// Support for thumbnails and previews.
//Chose one of these options.
//If you selected QT for threads and mutexes, it will be also used for thumbnails and previews
//If you select USE_EXTERNAL_GFX, you will have to create a subclass of MegaGfxProcessor and pass it to the constructor of MegaApi
//If you don't define any option, no thumbnails nor previews will be created

//#define USE_EXTERNAL_GFX
//#define USE_FREEIMAGE

//Define WINDOWS_PHONE if you want to build the MEGA SDK for Windows Phone

//#define WINDOWS_PHONE
/////////////////////////// END OF SETTINGS ///////////////////////////

namespace mega
{

#ifdef USE_PTHREAD
typedef PosixThread MegaThread;
typedef PosixMutex MegaMutex;
#elif USE_QT
typedef QtThread MegaThread;
typedef QtMutex MegaMutex;
#else
typedef CppThread MegaThread;
typedef CppMutex MegaMutex;
#endif

#ifdef USE_EXTERNAL_GFX
class MegaGfxProcessor : public GfxProcessor 
{
public:
	virtual bool readBitmap(const char* path) { return false; }
	virtual int getWidth() { return 0; }
	virtual int getHeight() { return 0; }
	virtual int getBitmapDataSize(int w, int h, int px, int py, int rw, int rh) { return 0; }
	virtual bool getBitmapData(char *bitmapData, size_t size) { return false; }
	virtual void freeBitmap() {}
	virtual ~MegaGfxProcessor() {};
};
class MegaGfxProc : public GfxProcExternal {};
#elif USE_QT
class MegaGfxProc : public GfxProcQT {};
#elif USE_FREEIMAGE
class MegaGfxProc : public GfxProcFreeImage {};
#else
typedef GfxProc MegaGfxProc;
#endif

#ifdef WIN32
    #ifndef WINDOWS_PHONE
    class MegaHttpIO : public WinHttpIO {};
    class MegaFileSystemAccess : public WinFileSystemAccess {};
    class MegaWaiter : public WinWaiter {};
    #else
    class MegaHttpIO : public CurlHttpIO {};
    class MegaFileSystemAccess : public WinFileSystemAccess {};
    class MegaWaiter : public WinPhoneWaiter {};
    #endif
#else
    #ifdef __APPLE__
    typedef CurlHttpIO MegaHttpIO;
    typedef PosixFileSystemAccess MegaFileSystemAccess;
    typedef PosixWaiter MegaWaiter;
    #else
    class MegaHttpIO : public CurlHttpIO {};
    class MegaFileSystemAccess : public PosixFileSystemAccess {};
    class MegaWaiter : public PosixWaiter {};
    #endif
#endif

typedef Proxy MegaProxy;

class MegaDbAccess : public SqliteDbAccess
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

        MegaNode(const char *name, int type, m_off_t size, m_time_t ctime, m_time_t mtime, handle nodehandle, string *nodekey, string *attrstring);
        MegaNode(MegaNode *node);
        ~MegaNode();

        MegaNode *copy();
        static MegaNode *fromNode(Node *node);

        int getType();
        const char* getName();
        const char *getBase64Handle();
        m_off_t getSize();
        m_time_t getCreationTime();
        m_time_t getModificationTime();
        handle getHandle();
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
        MegaNode(Node *node);
        int type;
        const char *name;
        m_off_t size;
        m_time_t ctime;
        m_time_t mtime;
        handle nodehandle;
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
        static MegaUser *fromUser(User *user);

	private:
        MegaUser(User *user);

		const char *email;
		int visibility;
		time_t ctime;
};

class MegaShare
{
	public:
		enum {
			ACCESS_UNKNOWN = -1,
			ACCESS_READ = 0,
			ACCESS_READWRITE,
			ACCESS_FULL,
			ACCESS_OWNER
		};

		~MegaShare();
		const char *getUser();
		handle getNodeHandle();
		int getAccess();
		m_time_t getTimestamp();
        static MegaShare *fromShare(handle nodehandle, Share *share);

	private:
		MegaShare(handle nodehandle, Share *share);
		handle nodehandle;
		const char *user;
		int access;
		m_time_t ts;

};

struct MegaFile : public File
{
    // app-internal sequence number for queue management
    int seqno;
    static int nextseqno;
    bool failed(error e);
    MegaFile();
};

struct MegaFileGet : public MegaFile
{
    void completed(Transfer*, LocalNode*);
	MegaFileGet(MegaClient *client, Node* n, string dstPath);
    MegaFileGet(MegaClient *client, MegaNode* n, string dstPath);
	~MegaFileGet() {}
};

struct MegaFilePut : public MegaFile
{
    void completed(Transfer* t, LocalNode*);
    MegaFilePut(MegaClient *client, string* clocalname, string *filename, handle ch, const char* ctargetuser);
    ~MegaFilePut() {}
};

class NodeList
{
	public:
		NodeList();
		NodeList(Node** newlist, int size);
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
		UserList(User** newlist, int size);
		virtual ~UserList();
		MegaUser* get(int i);
		int size();

	protected:
		MegaUser** list;
		int s;
};

class ShareList
{
	public:
		ShareList();
		ShareList(Share** newlist, handle *handlelist, int size);
		virtual ~ShareList();
		MegaShare* get(int i);
		int size();

	protected:
		MegaShare** list;
		int s;
};

class MegaRequestListener;
class MegaTransferListener;
class MegaTransfer;
class MegaApi;

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

		handle getNodeHandle() const;
		const char* getLink() const;
		handle getParentHandle() const;
        const char* getSessionKey() const;
		const char* getName() const;
		const char* getEmail() const;
		const char* getPassword() const;
		const char* getNewPassword() const;
		const char* getPrivateKey() const;
		int getAccess() const;
		const char* getFile() const;
		int getNumRetry() const;
		int getNextRetryDelay() const;
        MegaNode *getPublicNode();
        int getParamType() const;
        bool getFlag() const;
        long long getTransferredBytes() const;
        long long getTotalBytes() const;

		void setNodeHandle(handle nodeHandle);
		void setLink(const char* link);
		void setParentHandle(handle parentHandle);
        void setSessionKey(const char* sessionKey);
		void setName(const char* name);
		void setEmail(const char* email);
    	void setPassword(const char* email);
    	void setNewPassword(const char* email);
		void setPrivateKey(const char* privateKey);
		void setAccess(int access);
		void setNumRetry(int ds);
		void setNextRetryDelay(int delay);
        void setPublicNode(MegaNode* publicNode);
		void setNumDetails(int numDetails);
		void setFile(const char* file);
        void setParamType(int type);
        void setFlag(bool flag);
        void setTransfer(int transfer);
        void setListener(MegaRequestListener *listener);
        void setTotalBytes(long long totalBytes);
        void setTransferredBytes(long long transferredBytes);
		MegaRequestListener *getListener() const;
        int getTransfer() const;
		AccountDetails * getAccountDetails() const;
		int getNumDetails() const;

	protected:
		int type;
		handle nodeHandle;
		const char* link;
		const char* name;
		handle parentHandle;
        const char* sessionKey;
		const char* email;
		const char* password;
		const char* newPassword;
		const char* privateKey;
		int access;
		const char* file;
		int attrType;
        bool flag;
        long long totalBytes;
        long long transferredBytes;
		MegaRequestListener *listener;
        int transfer;
		AccountDetails *accountDetails;
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
		m_time_t getStartTime() const;
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
		m_time_t getTime() const;
		const char* getBase64Key() const;
		int getTag() const;
        Transfer *getTransfer() const;
		long long getSpeed() const;
		long long getDeltaSize() const;
		m_time_t getUpdateTime() const;
        MegaNode *getPublicNode() const;
        bool isSyncTransfer() const;
        bool isStreamingTransfer() const;
		char *getLastBytes() const;
		void setStartTime(m_time_t startTime);
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
		void setTime(m_time_t time);
		void setFileName(const char* fileName);
		void setSlot(int id);
		void setBase64Key(const char* base64Key);
		void setTag(int tag);
        void setTransfer(Transfer *transfer);
		void setSpeed(long long speed);
		void setDeltaSize(long long deltaSize);
		void setUpdateTime(m_time_t updateTime);
        void setPublicNode(MegaNode *publicNode);
        void setSyncTransfer(bool syncTransfer);
		void setLastBytes(char *lastBytes);

	protected:
		int slot;
		int type;
		int tag;
        bool syncTransfer;
		m_time_t startTime;
		m_time_t updateTime;
		m_time_t time;
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
		char *lastBytes;
        MegaNode *publicNode;
		int numConnections;
		long long startPos;
		long long endPos;
		int maxSpeed;
		int retry;
		int maxRetries;
        Transfer *transfer;
		MegaTransferListener *listener;
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

	protected:
        //< 0 = API error code, > 0 = http error, 0 = No error
		int errorCode;
		long nextAttempt;
};


class TreeProcessor
{
    public:
        virtual bool processNode(Node* node);
        virtual ~TreeProcessor();
};

class MegaTreeProcessor
{
    public:
        virtual bool processMegaNode(MegaNode* node);
        virtual ~MegaTreeProcessor();
};

class SearchTreeProcessor : public TreeProcessor
{
    public:
        SearchTreeProcessor(const char *search);
        virtual bool processNode(Node* node);
        virtual ~SearchTreeProcessor() {}
        vector<Node *> &getResults();

    protected:
        const char *search;
        vector<Node *> results;
};

class SizeProcessor : public TreeProcessor
{
    protected:
        long long totalBytes;

    public:
        SizeProcessor();
        virtual bool processNode(Node* node);
        long long getTotalBytes();
};

//Request callbacks
class MegaRequestListener
{
    public:
        //Request callbacks
        virtual void onRequestStart(MegaApi* api, MegaRequest *request);
        virtual void onRequestFinish(MegaApi* api, MegaRequest *request, MegaError* e);
        virtual void onRequestUpdate(MegaApi*, MegaRequest *);
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

        //For streaming downloads only
        virtual bool onTransferData(MegaApi *api, MegaTransfer *transfer, char *buffer, size_t size);
};

//Global callbacks
class MegaGlobalListener
{
    public:
        //Global callbacks
    #if defined(__ANDROID__) || defined(WINDOWS_PHONE)
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
    #if defined(__ANDROID__) || defined(WINDOWS_PHONE)
        virtual void onUsersUpdate(MegaApi* api);
        virtual void onNodesUpdate(MegaApi* api);
    #else
        virtual void onUsersUpdate(MegaApi* api, UserList *users);
        virtual void onNodesUpdate(MegaApi* api, NodeList *nodes);
    #endif
        virtual void onReloadNeeded(MegaApi* api);
        virtual void onSyncStateChanged(MegaApi *api);

        virtual ~MegaListener();
};

//Thread safe request queue
class RequestQueue
{
    protected:
        std::deque<MegaRequest *> requests;
        MegaMutex mutex;

    public:
        RequestQueue();
        void push(MegaRequest *request);
        void push_front(MegaRequest *request);
        MegaRequest * pop();
        void removeListener(MegaRequestListener *listener);
};


//Thread safe transfer queue
class TransferQueue
{
    protected:
        std::deque<MegaTransfer *> transfers;
        MegaMutex mutex;

    public:
        TransferQueue();
        void push(MegaTransfer *transfer);
        void push_front(MegaTransfer *transfer);
        MegaTransfer * pop();
};

class MegaApi : public MegaApp
{
    public:
        #ifdef USE_EXTERNAL_GFX
            MegaApi(const char *appKey, MegaGfxProcessor* processor, const char *basePath = NULL, const char *userAgent = NULL);
        #else
            MegaApi(const char *appKey, const char *basePath = NULL, const char *userAgent = NULL);
        #endif

        virtual ~MegaApi();

        //Multiple listener management.
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
        const char *dumpSession();
        void fastLogin(const char* email, const char *stringHash, const char *base64pwkey, MegaRequestListener *listener = NULL);
        void fastLogin(const char* session, MegaRequestListener *listener = NULL);
        void createAccount(const char* email, const char* password, const char* name, MegaRequestListener *listener = NULL);
        void fastCreateAccount(const char* email, const char *base64pwkey, const char* name, MegaRequestListener *listener = NULL);
        void querySignupLink(const char* link, MegaRequestListener *listener = NULL);
        void confirmAccount(const char* link, const char *password, MegaRequestListener *listener = NULL);
        void fastConfirmAccount(const char* link, const char *base64pwkey, MegaRequestListener *listener = NULL);
        void setProxySettings(MegaProxy *proxySettings);
        MegaProxy *getAutoProxySettings();
        int isLoggedIn();
        const char* getMyEmail();

        void createFolder(const char* name, MegaNode *parent, MegaRequestListener *listener = NULL);
        void moveNode(MegaNode* node, MegaNode* newParent, MegaRequestListener *listener = NULL);
        void copyNode(MegaNode* node, MegaNode *newParent, MegaRequestListener *listener = NULL);
        void renameNode(MegaNode* node, const char* newName, MegaRequestListener *listener = NULL);
        void remove(MegaNode* node, MegaRequestListener *listener = NULL);
        void sendFileToUser(MegaNode *node, MegaUser *user, MegaRequestListener *listener = NULL);
        void sendFileToUser(MegaNode *node, const char* email, MegaRequestListener *listener = NULL);
        void share(MegaNode *node, MegaUser* user, int level, MegaRequestListener *listener = NULL);
        void share(MegaNode* node, const char* email, int level, MegaRequestListener *listener = NULL);
        void folderAccess(const char* megaFolderLink, MegaRequestListener *listener = NULL);
        void importFileLink(const char* megaFileLink, MegaNode* parent, MegaRequestListener *listener = NULL);
        void importPublicNode(MegaNode *publicNode, MegaNode *parent, MegaRequestListener *listener = NULL);
        void getPublicNode(const char* megaFileLink, MegaRequestListener *listener = NULL);
        void getThumbnail(MegaNode* node, const char *dstFilePath, MegaRequestListener *listener = NULL);
        void setThumbnail(MegaNode* node, const char *srcFilePath, MegaRequestListener *listener = NULL);
        void getPreview(MegaNode* node, const char *dstFilePath, MegaRequestListener *listener = NULL);
        void setPreview(MegaNode* node, const char *srcFilePath, MegaRequestListener *listener = NULL);
        void getUserAvatar(MegaUser* user, const char *dstFilePath, MegaRequestListener *listener = NULL);
        void exportNode(MegaNode *node, MegaRequestListener *listener = NULL);
        void disableExport(MegaNode *node, MegaRequestListener *listener = NULL);
        void fetchNodes(MegaRequestListener *listener = NULL);
        void getAccountDetails(MegaRequestListener *listener = NULL);
        void changePassword(const char *oldPassword, const char *newPassword, MegaRequestListener *listener = NULL);
        void addContact(const char* email, MegaRequestListener* listener=NULL);
        void removeContact(const char* email, MegaRequestListener* listener=NULL);
        void logout(MegaRequestListener *listener = NULL);

        //Transfers
        void startUpload(const char* localPath, MegaNode *parent, MegaTransferListener *listener=NULL);
        void startUpload(const char* localPath, MegaNode* parent, const char* fileName, MegaTransferListener *listener = NULL);
        void startDownload(MegaNode* node, const char* localPath, MegaTransferListener *listener = NULL);
        void startStreaming(MegaNode* node, m_off_t startPos, m_off_t size, MegaTransferListener *listener);
        void startPublicDownload(MegaNode* node, const char* localPath, MegaTransferListener *listener = NULL);
        void cancelTransfer(MegaTransfer *transfer, MegaRequestListener *listener=NULL);
        void cancelTransfers(int direction, MegaRequestListener *listener=NULL);
        void pauseTransfers(bool pause, MegaRequestListener* listener=NULL);
        void setUploadLimit(int bpslimit);
        TransferList *getTransfers();

        //Sync
        treestate_t syncPathState(string *path);
        MegaNode *getSyncedNode(string *path);
        void syncFolder(const char *localFolder, MegaNode *megaFolder);
        void removeSync(handle nodehandle, MegaRequestListener *listener=NULL);
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
        MegaNode *getChildNode(MegaNode *parent, const char* name);
        MegaNode *getParentNode(MegaNode *node);
        const char* getNodePath(MegaNode *node);
        MegaNode *getNodeByPath(const char *path, MegaNode *n = NULL);
        MegaNode *getNodeByHandle(handle handler);
        UserList* getContacts();
        MegaUser* getContact(const char* email);
        NodeList *getInShares(MegaUser* user);
        NodeList *getInShares();
        ShareList *getOutShares(MegaNode *node);
        int getAccess(MegaNode* node);
        long long getSize(MegaNode *node);

        MegaError checkAccess(MegaNode* node, int level);
        MegaError checkMove(MegaNode* node, MegaNode* target);

        MegaNode *getRootNode();
        MegaNode* getInboxNode();
        MegaNode *getRubbishNode();
        NodeList* search(MegaNode* node, const char* searchString, bool recursive = 1);
        bool processMegaTree(MegaNode* node, MegaTreeProcessor* processor, bool recursive = 1);

        //General porpuse
        static char* strdup(const char* buffer);

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

    #ifdef WIN32
        static void utf16ToUtf8(const wchar_t* utf16data, int utf16size, std::string* path);
        static void utf8ToUtf16(const char* utf8data, std::string* utf16string);
    #endif

    protected:
        static void *threadEntryPoint(void *param);

        void fireOnRequestStart(MegaApi* api, MegaRequest *request);
        void fireOnRequestFinish(MegaApi* api, MegaRequest *request, MegaError e);
        void fireOnRequestUpdate(MegaApi* api, MegaRequest *request);
        void fireOnRequestTemporaryError(MegaApi *api, MegaRequest *request, MegaError e);
        void fireOnTransferStart(MegaApi* api, MegaTransfer *transfer);
        void fireOnTransferFinish(MegaApi* api, MegaTransfer *transfer, MegaError e);
        void fireOnTransferUpdate(MegaApi *api, MegaTransfer *transfer);
        bool fireOnTransferData(MegaApi *api, MegaTransfer *transfer);
        void fireOnTransferTemporaryError(MegaApi *api, MegaTransfer *transfer, MegaError e);
        void fireOnUsersUpdate(MegaApi* api, UserList *users);
        void fireOnNodesUpdate(MegaApi* api, NodeList *nodes);
        void fireOnReloadNeeded(MegaApi* api);
        void fireOnSyncStateChanged(MegaApi* api);

        MegaThread thread;
        MegaClient *client;
        MegaHttpIO *httpio;
        MegaWaiter *waiter;
        MegaFileSystemAccess *fsAccess;
        MegaDbAccess *dbAccess;
        MegaGfxProc *gfxAccess;

        RequestQueue requestQueue;
        TransferQueue transferQueue;
        map<int, MegaRequest *> requestMap;
        map<int, MegaTransfer *> transferMap;
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
        MegaMutex sdkMutex;
        MegaTransfer *currentTransfer;
        int threadExit;
        dstime pausetime;
        void loop();

        int maxRetries;

        // a request-level error occurred
        virtual void request_error(error);
        virtual void request_response_progress(m_off_t, m_off_t);

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
        virtual void putnodes_result(error, targettype_t, NewNode*);

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
        virtual void checkfile_result(handle h, error e, byte* filekey, m_off_t size, m_time_t ts, m_time_t tm, string* filename, string* fingerprint, string* fileattrstring);

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
        virtual void openfilelink_result(handle, const byte*, m_off_t, string*, const char*, m_time_t, m_time_t, int);

        // global transfer queue updates (separate signaling towards the queued objects)
        virtual void transfer_added(Transfer*);
        virtual void transfer_removed(Transfer*);
        virtual void transfer_prepare(Transfer*);
        virtual void transfer_failed(Transfer*, error error);
        virtual void transfer_update(Transfer*);
        virtual void transfer_limit(Transfer*);
        virtual void transfer_complete(Transfer*);

        virtual dstime pread_failure(error, int, void*);
        virtual bool pread_data(byte*, m_off_t, m_off_t, void*);

        // sync status updates and events
        virtual void syncupdate_state(Sync*, syncstate_t);
        virtual void syncupdate_scanning(bool scanning);
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
        virtual void syncupdate_treestate(LocalNode*);
        virtual bool sync_syncable(Node*);
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

        //Internal
        Node* getChildNodeInternal(Node *parent, const char* name);
        bool processTree(Node* node, TreeProcessor* processor, bool recursive = 1);
        NodeList* search(Node* node, const char* searchString, bool recursive = 1);
        void getAccountDetails(bool storage, bool transfer, bool pro, bool transactions, bool purchases, bool sessions, MegaRequestListener *listener = NULL);
        void getNodeAttribute(MegaNode* node, int type, const char *dstFilePath, MegaRequestListener *listener = NULL);
        void setNodeAttribute(MegaNode* node, int type, const char *srcFilePath, MegaRequestListener *listener = NULL);
        void getUserAttribute(MegaUser* node, int type, const char *dstFilePath, MegaRequestListener *listener = NULL);
        void setUserAttribute(MegaUser* node, int type, const char *srcFilePath, MegaRequestListener *listener = NULL);
        void startUpload(const char* localPath, MegaNode* parent, int connections, int maxSpeed, const char* fileName, MegaTransferListener *listener);
        void startUpload(const char* localPath, MegaNode* parent, int maxSpeed, MegaTransferListener *listener = NULL);
        void startDownload(handle nodehandle, const char* target, int connections, long startPos, long endPos, const char* base64key, MegaTransferListener *listener);
};

}

#endif //MEGAAPI_H
