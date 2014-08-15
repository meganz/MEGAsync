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

#ifndef MEGAAPI_IMPL_H
#define MEGAAPI_IMPL_H

#include <inttypes.h>

#include "mega.h"
#include "mega/thread/posixthread.h"
#include "mega/thread/qtthread.h"
#include "mega/gfx/qt.h"
#include "mega/gfx/external.h"
#include "mega/thread/cppthread.h"
#include "mega/proxy.h"

#include "megaapi.h"

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

class TreeProcessor
{
    public:
        virtual bool processNode(Node* node);
        virtual ~TreeProcessor();
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

class MegaApiImpl : public MegaApp
{
    public:
        #ifdef USE_EXTERNAL_GFX
            MegaApiImpl(MegaApi *api, const char *appKey, MegaGfxProcessor* processor, const char *basePath = NULL, const char *userAgent = NULL);
        #else
            MegaApiImpl(MegaApi *api, const char *appKey, const char *basePath = NULL, const char *userAgent = NULL);
        #endif

        virtual ~MegaApiImpl();

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
        void resumeSync(const char *localFolder, MegaNode *megaFolder);
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

protected:
        static void *threadEntryPoint(void *param);

        void fireOnRequestStart(MegaRequest *request);
        void fireOnRequestFinish(MegaRequest *request, MegaError e);
        void fireOnRequestUpdate(MegaRequest *request);
        void fireOnRequestTemporaryError(MegaRequest *request, MegaError e);
        void fireOnTransferStart(MegaTransfer *transfer);
        void fireOnTransferFinish(MegaTransfer *transfer, MegaError e);
        void fireOnTransferUpdate(MegaTransfer *transfer);
        bool fireOnTransferData(MegaTransfer *transfer);
        void fireOnTransferTemporaryError(MegaTransfer *transfer, MegaError e);
        void fireOnUsersUpdate(UserList *users);
        void fireOnNodesUpdate(NodeList *nodes);
        void fireOnReloadNeeded();
        void fireOnSyncStateChanged();
        void fireOnFileSyncStateChanged(const char *filePath, MegaSyncState newState);

        MegaApi *api;
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

#endif //MEGAAPI_IMPL_H
