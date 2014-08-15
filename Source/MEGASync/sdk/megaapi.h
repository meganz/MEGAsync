#ifndef MEGAAPI_H
#define MEGAAPI_H

#include <string>
#include <vector>
#include <inttypes.h>

#ifdef WIN32
#define MEGA_DEBRIS_FOLDER "Rubbish"
#else
#define MEGA_DEBRIS_FOLDER ".debris"
#endif

namespace mega
{

typedef uint64_t MegaHandle;
const MegaHandle INVALID_HANDLE = ~(MegaHandle)0;

typedef enum
{
    STATE_NONE,
    STATE_SYNCED,
    STATE_PENDING,
    STATE_SYNCING,
    STATE_IGNORED
} MegaSyncState;


//API classes (forward declaration)
class MegaListener;
class MegaRequestListener;
class MegaTransferListener;
class MegaGlobalListener;
class MegaTreeProcessor;
class MegaAccountDetails;
class MegaTransfer;
class MegaNode;
class MegaUser;
class MegaShare;
class MegaError;
class MegaRequest;
class MegaTransfer;
class NodeList;
class UserList;
class ShareList;
class TransferList;
class MegaApiImpl;
class MegaApi;

struct AccountDetails;
struct Node;
struct User;
struct Transfer;
struct Share;

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

#endif

class MegaProxy
{
public:
    enum MegaProxyType {NONE = 0, AUTO = 1, CUSTOM = 2};

    MegaProxy();
    void setProxyType(int proxyType);
    void setProxyURL(std::string *proxyURL);
    void setCredentials(std::string *username, std::string *password);
    int getProxyType();
    std::string getProxyURL();
    bool credentialsNeeded();
    std::string getUsername();
    std::string getPassword();

protected:
    int proxyType;
    std::string proxyURL;
    std::string username;
    std::string password;
};

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

        MegaNode(const char *name, int type, int64_t size, int64_t ctime, int64_t mtime, MegaHandle nodeMegaHandle, std::string *nodekey, std::string *attrstring);
        MegaNode(MegaNode *node);
        ~MegaNode();

        MegaNode *copy();
        static MegaNode *fromNode(Node *node);

        int getType();
        const char* getName();
        const char *getBase64Handle();
        int64_t getSize();
        int64_t getCreationTime();
        int64_t getModificationTime();
        MegaHandle getHandle();
        std::string* getNodeKey();
        std::string* getAttrString();
        int getTag();
        bool isFile();
        bool isFolder();
        bool isRemoved();
        bool isSyncDeleted();
        std::string getLocalPath();
        bool hasThumbnail();
        bool hasPreview();

    private:
        MegaNode(Node *node);
        int type;
        const char *name;
        int64_t size;
        int64_t ctime;
        int64_t mtime;
        MegaHandle nodehandle;
        std::string nodekey;
        std::string attrstring;
        std::string localPath;

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
        MegaHandle getNodeHandle();
		int getAccess();
        int64_t getTimestamp();
        static MegaShare *fromShare(MegaHandle nodeMegaHandle, Share *share);

	private:
        MegaShare(MegaHandle nodehandle, Share *share);
        MegaHandle nodehandle;
		const char *user;
		int access;
        int64_t ts;

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
        ShareList(Share** newlist, MegaHandle *MegaHandlelist, int size);
		virtual ~ShareList();
		MegaShare* get(int i);
		int size();

	protected:
		MegaShare** list;
		int s;
};

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

        MegaHandle getNodeHandle() const;
		const char* getLink() const;
        MegaHandle getParentHandle() const;
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

        void setNodeHandle(MegaHandle nodeHandle);
		void setLink(const char* link);
        void setParentHandle(MegaHandle parentHandle);
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
        MegaAccountDetails *getMegaAccountDetails() const;
		int getNumDetails() const;

	protected:
		int type;
        MegaHandle nodeHandle;
		const char* link;
		const char* name;
        MegaHandle parentHandle;
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
        int64_t getStartTime() const;
		long long getTransferredBytes() const;
		long long getTotalBytes() const;
		const char* getPath() const;
		const char* getParentPath() const;
        MegaHandle getNodeHandle() const;
        MegaHandle getParentHandle() const;
		int getNumConnections() const;
		long long getStartPos() const;
		long long getEndPos() const;
		int getMaxSpeed() const;
		const char* getFileName() const;
		MegaTransferListener* getListener() const;
		int getNumRetry() const;
		int getMaxRetries() const;
        int64_t getTime() const;
		const char* getBase64Key() const;
		int getTag() const;
        Transfer *getTransfer() const;
		long long getSpeed() const;
		long long getDeltaSize() const;
        int64_t getUpdateTime() const;
        MegaNode *getPublicNode() const;
        bool isSyncTransfer() const;
        bool isStreamingTransfer() const;
		char *getLastBytes() const;
        void setStartTime(int64_t startTime);
		void setTransferredBytes(long long transferredBytes);
		void setTotalBytes(long long totalBytes);
		void setPath(const char* path);
		void setParentPath(const char* path);
        void setNodeHandle(MegaHandle nodeHandle);
        void setParentHandle(MegaHandle parentHandle);
		void setNumConnections(int connections);
		void setStartPos(long long startPos);
		void setEndPos(long long endPos);
		void setMaxSpeed(int maxSpeed);
		void setNumRetry(int retry);
		void setMaxRetries(int retry);
        void setTime(int64_t time);
		void setFileName(const char* fileName);
		void setSlot(int id);
		void setBase64Key(const char* base64Key);
		void setTag(int tag);
        void setTransfer(Transfer *transfer);
		void setSpeed(long long speed);
		void setDeltaSize(long long deltaSize);
        void setUpdateTime(int64_t updateTime);
        void setPublicNode(MegaNode *publicNode);
        void setSyncTransfer(bool syncTransfer);
		void setLastBytes(char *lastBytes);

	protected:
		int slot;
		int type;
		int tag;
        bool syncTransfer;
        int64_t startTime;
        int64_t updateTime;
        int64_t time;
		long long transferredBytes;
		long long totalBytes;
		long long speed;
		long long deltaSize;
        MegaHandle nodeHandle;
        MegaHandle parentHandle;
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


class MegaTreeProcessor
{
    public:
        virtual bool processMegaNode(MegaNode* node);
        virtual ~MegaTreeProcessor();
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
        virtual void onSyncFileStateChanged(MegaApi *api, const char *filePath, MegaSyncState newState);
        virtual void onSyncStateChanged(MegaApi *api);

        virtual ~MegaListener();
};

class MegaApi
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
        static MegaHandle base64ToHandle(const char* base64Handle);
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
        void startStreaming(MegaNode* node, int64_t startPos, int64_t size, MegaTransferListener *listener);
        void startPublicDownload(MegaNode* node, const char* localPath, MegaTransferListener *listener = NULL);
        void cancelTransfer(MegaTransfer *transfer, MegaRequestListener *listener=NULL);
        void cancelTransfers(int direction, MegaRequestListener *listener=NULL);
        void pauseTransfers(bool pause, MegaRequestListener* listener=NULL);
        void setUploadLimit(int bpslimit);
        TransferList *getTransfers();

        //Sync
        MegaSyncState syncPathState(std::string *path);
        MegaNode *getSyncedNode(std::string *path);
        void syncFolder(const char *localFolder, MegaNode *megaFolder);
        void resumeSync(const char *localFolder, MegaNode *megaFolder);
        void removeSync(MegaHandle nodeMegaHandle, MegaRequestListener *listener=NULL);
        int getNumActiveSyncs();
        void stopSyncs(MegaRequestListener *listener=NULL);
        int getNumPendingUploads();
        int getNumPendingDownloads();
        int getTotalUploads();
        int getTotalDownloads();
        void resetTotalDownloads();
        void resetTotalUploads();
        std::string getLocalPath(MegaNode *node);
        void updateStatics();
        void update();
        bool isIndexing();
        bool isWaiting();
        bool isSynced(MegaNode *n);
        void setExcludedNames(std::vector<std::string> *excludedNames);
        bool moveToLocalDebris(const char *path);
        bool isSyncable(const char *name);

        //Filesystem
        enum {	ORDER_NONE, ORDER_DEFAULT_ASC, ORDER_DEFAULT_DESC,
            ORDER_SIZE_ASC, ORDER_SIZE_DESC,
            ORDER_CREATION_ASC, ORDER_CREATION_DESC,
            ORDER_MODIFICATION_ASC, ORDER_MODIFICATION_DESC,
            ORDER_ALPHABETICAL_ASC, ORDER_ALPHABETICAL_DESC};

        NodeList* getChildren(MegaNode *parent, int order=1);
        MegaNode *getChildNode(MegaNode *parent, const char* name);
        MegaNode *getParentNode(MegaNode *node);
        const char* getNodePath(MegaNode *node);
        MegaNode *getNodeByPath(const char *path, MegaNode *n = NULL);
        MegaNode *getNodeByHandle(MegaHandle MegaHandler);
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

    #ifdef WIN32
        static void utf16ToUtf8(const wchar_t* utf16data, int utf16size, std::string* path);
        static void utf8ToUtf16(const char* utf8data, std::string* utf16string);
    #endif
        static char* strdup(const char* buffer);

private:
        MegaApiImpl *pImpl;
};

class HashSignature;
class AsymmCipher;
class MegaHashSignature
{
public:
    MegaHashSignature(const char *base64Key);
    ~MegaHashSignature();
    void init();
    void add(const unsigned char *data, unsigned size);
    bool check(const char *base64Signature);

private:
    HashSignature *hashSignature;
    AsymmCipher* asymmCypher;
};

class MegaAccountDetails
{
public:
    int getProLevel();
    long long getStorageMax();
    long long getStorageUsed();
    long long getTransferMax();
    long long getTransferOwnUsed();

    long long getStorageUsed(MegaHandle handle);
    long long getNumFiles(MegaHandle handle);
    long long getNumFolders(MegaHandle handle);

    static MegaAccountDetails *fromAccountDetails(AccountDetails *details);
private:
    MegaAccountDetails(AccountDetails *details);
    AccountDetails *details;
};

}

#endif //MEGAAPI_H
