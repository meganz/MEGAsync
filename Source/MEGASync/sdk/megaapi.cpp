#include "mega.h"
#include "megaapi.h"
#include "megaapi_impl.h"

using namespace mega;

MegaProxy::MegaProxy()
{
    proxyType = AUTO;
}

void MegaProxy::setProxyType(int proxyType)
{
    this->proxyType = proxyType;
}

void MegaProxy::setProxyURL(string *proxyURL)
{
    this->proxyURL = *proxyURL;
}

void MegaProxy::setCredentials(string *username, string *password)
{
    this->username = *username;
    this->password = *password;
}

int MegaProxy::getProxyType()
{
    return proxyType;
}

string MegaProxy::getProxyURL()
{
    return this->proxyURL;
}

bool MegaProxy::credentialsNeeded()
{
    return (username.size() != 0);
}

string MegaProxy::getUsername()
{
    return username;
}

string MegaProxy::getPassword()
{
    return password;
}

NodeList::NodeList()
{ list = NULL; s = 0; }

NodeList::NodeList(Node** newlist, int size)
{
	list = NULL; s = size;
	if(!size) return;

	list = new MegaNode*[size];
	for(int i=0; i<size; i++)
		list[i] = MegaNode::fromNode(newlist[i]);
}

NodeList::~NodeList()
{
	if(!list) return;

	for(int i=0; i<s; i++)
		delete list[i];
	delete [] list;
}

MegaNode *NodeList::get(int i)
{
	if(!list || (i < 0) || (i >= s))
		return NULL;

	return list[i];
}

int NodeList::size()
{ return s; }


UserList::UserList()
{ list = NULL; s = 0; }

UserList::UserList(User** newlist, int size)
{
	list = NULL; s = size;
	if(!size) return;

	list = new MegaUser*[size];
	for(int i=0; i<size; i++)
		list[i] = MegaUser::fromUser(newlist[i]);
}

UserList::~UserList()
{
	if(!list) return;

	for(int i=0; i<s; i++)
		delete list[i];
	delete [] list;
}

MegaUser *UserList::get(int i)
{
	if(!list || (i < 0) || (i >= s))
		return NULL;

	return list[i];
}

int UserList::size()
{ return s; }

ShareList::ShareList()
{ list = NULL; s = 0; }

ShareList::ShareList(Share** newlist, uint64_t *uint64_tlist, int size)
{
	list = NULL; s = size;
	if(!size) return;

	list = new MegaShare*[size];
	for(int i=0; i<size; i++)
        list[i] = MegaShare::fromShare(uint64_tlist[i], newlist[i]);
}

ShareList::~ShareList()
{
	if(!list) return;

	for(int i=0; i<s; i++)
		delete list[i];
	delete [] list;
}

MegaShare *ShareList::get(int i)
{
	if(!list || (i < 0) || (i >= s))
		return NULL;

	return list[i];
}

int ShareList::size()
{ return s; }


TransferList::TransferList()
{ list = NULL; s = 0; }

TransferList::TransferList(MegaTransfer** newlist, int size)
{
	list = NULL; s = size;
	if(!size) return;

    list = new MegaTransfer*[size];
	for(int i=0; i<size; i++)
		list[i] = newlist[i]->copy();
}

TransferList::~TransferList()
{
	if(!list) return;

	for(int i=0; i<s; i++)
		delete list[i];
	delete [] list;
}

MegaTransfer *TransferList::get(int i)
{
	if(!list || (i < 0) || (i >= s))
		return NULL;

	return list[i];
}

int TransferList::size()
{ return s; }


MegaNode::MegaNode(const char *name, int type, int64_t size, int64_t ctime, int64_t mtime, uint64_t nodehandle, string *nodekey, string *attrstring)
{
    this->name = MegaApi::strdup(name);
    this->type = type;
    this->size = size;
    this->ctime = ctime;
    this->mtime = mtime;
    this->nodehandle = nodehandle;
    this->attrstring.assign(attrstring->data(), attrstring->size());
    this->nodekey.assign(nodekey->data(),nodekey->size());
    this->removed = false;
    this->syncdeleted = false;
    this->thumbnailAvailable = false;
    this->previewAvailable = false;
    this->tag = 0;
}

MegaNode::MegaNode(MegaNode *node)
{
    this->name = MegaApi::strdup(node->getName());
    this->type = node->type;
    this->size = node->getSize();
    this->ctime = node->getCreationTime();
    this->mtime = node->getModificationTime();
    this->nodehandle = node->getHandle();
    string * attrstring = node->getAttrString();
    this->attrstring.assign(attrstring->data(), attrstring->size());
    string *nodekey = node->getNodeKey();
    this->nodekey.assign(nodekey->data(),nodekey->size());
    this->removed = node->isRemoved();
    this->syncdeleted = node->isSyncDeleted();
    this->thumbnailAvailable = node->hasThumbnail();
    this->previewAvailable = node->hasPreview();
    this->tag = node->getTag();
    this->localPath = node->getLocalPath();
}

MegaNode::MegaNode(Node *node)
{
    this->name = MegaApi::strdup(node->displayname());
    this->type = node->type;
    this->size = node->size;
    this->ctime = node->ctime;
    this->mtime = node->mtime;
    this->nodehandle = node->nodehandle;
    this->attrstring.assign(node->attrstring.data(), node->attrstring.size());
    this->nodekey.assign(node->nodekey.data(),node->nodekey.size());
    this->removed = node->removed;
	this->syncdeleted = (node->syncdeleted != SYNCDEL_NONE);
    this->thumbnailAvailable = (node->hasfileattribute(0) != 0);
    this->previewAvailable = (node->hasfileattribute(1) != 0);
    this->tag = node->tag;
    if(node->localnode)
    {
        node->localnode->getlocalpath(&localPath, true);
        localPath.append("", 1);
    }
}

MegaNode *MegaNode::copy()
{
	return new MegaNode(this);
}

MegaNode::~MegaNode()
{
    delete [] name;
}

MegaNode *MegaNode::fromNode(Node *node)
{
    if(!node) return NULL;
    return new MegaNode(node);
}

const char *MegaNode::getBase64Handle()
{
    char *base64Handle = new char[12];
    Base64::btoa((byte*)&(nodehandle),MegaClient::NODEHANDLE,base64Handle);
    return base64Handle;
}

int MegaNode::getType() { return type; }
const char* MegaNode::getName() { return name; }
int64_t MegaNode::getSize() { return size; }
int64_t MegaNode::getCreationTime() { return ctime; }
int64_t MegaNode::getModificationTime() { return mtime; }
uint64_t MegaNode::getHandle() { return nodehandle; }
string *MegaNode::getNodeKey() { return &nodekey; }
string *MegaNode::getAttrString() { return &attrstring; }

int MegaNode::getTag()
{
    return tag;
}

bool MegaNode::isRemoved()
{
    return removed;
}

bool MegaNode::isFile()
{
	return type == TYPE_FILE;
}

bool MegaNode::isFolder()
{
	return (type != TYPE_FILE) && (type != TYPE_UNKNOWN);
}

bool MegaNode::isSyncDeleted()
{
    return syncdeleted;
}

string MegaNode::getLocalPath()
{
    return localPath;
}

bool MegaNode::hasThumbnail()
{
	return thumbnailAvailable;
}

bool MegaNode::hasPreview()
{
	return previewAvailable;
}

MegaUser::MegaUser(User *user)
{
    email = MegaApi::strdup(user->email.c_str());
	visibility = user->show;
	ctime = user->ctime;
}

MegaUser::~MegaUser()
{
	delete[] email;
}

const char* MegaUser::getEmail()
{
	return email;
}

int MegaUser::getVisibility()
{
	return visibility;
}

time_t MegaUser::getTimestamp()
{
	return ctime;
}

MegaUser *MegaUser::fromUser(User *user)
{
	return new MegaUser(user);
}

MegaShare::MegaShare(uint64_t handle, Share *share)
{
    this->nodehandle = handle;
    this->user = share->user ? MegaApi::strdup(share->user->email.c_str()) : NULL;
	this->access = share->access;
	this->ts = share->ts;
}

MegaShare::~MegaShare()
{
	delete[] user;
}

const char *MegaShare::getUser()
{
	return user;
}

uint64_t MegaShare::getNodeHandle()
{
    return nodehandle;
}

int MegaShare::getAccess()
{
	return access;
}

int64_t MegaShare::getTimestamp()
{
	return ts;
}

MegaShare *MegaShare::fromShare(uint64_t nodeuint64_t, Share *share)
{
    return new MegaShare(nodeuint64_t, share);
}

MegaRequest::MegaRequest(int type, MegaRequestListener *listener)
{
	this->type = type;
	this->transfer = 0;
	this->listener = listener;
	this->nodeHandle = UNDEF;
	this->link = NULL;
	this->parentHandle = UNDEF;
    this->sessionKey = NULL;
	this->name = NULL;
	this->email = NULL;
	this->password = NULL;
	this->newPassword = NULL;
	this->privateKey = NULL;
	this->access = MegaShare::ACCESS_UNKNOWN;
	this->numRetry = 0;
	this->nextRetryDelay = 0;
	this->publicNode = NULL;
	this->numDetails = 0;
	this->file = NULL;
	this->attrType = 0;
    this->flag = false;
    this->totalBytes = -1;
    this->transferredBytes = 0;

	if(type == MegaRequest::TYPE_ACCOUNT_DETAILS) this->accountDetails = new AccountDetails();
	else this->accountDetails = NULL;
}

MegaRequest::MegaRequest(MegaRequest &request)
{
    this->link = NULL;
    this->sessionKey = NULL;
    this->name = NULL;
    this->email = NULL;
    this->password = NULL;
    this->newPassword = NULL;
    this->privateKey = NULL;
    this->access = MegaShare::ACCESS_UNKNOWN;
    this->publicNode = NULL;
    this->file = NULL;
    this->publicNode = NULL;

    this->type = request.getType();
	this->setNodeHandle(request.getNodeHandle());
	this->setLink(request.getLink());
	this->setParentHandle(request.getParentHandle());
    this->setSessionKey(request.getSessionKey());
	this->setName(request.getName());
	this->setEmail(request.getEmail());
	this->setPassword(request.getPassword());
	this->setNewPassword(request.getNewPassword());
	this->setPrivateKey(request.getPrivateKey());
	this->setAccess(request.getAccess());
	this->setNumRetry(request.getNumRetry());
	this->setNextRetryDelay(request.getNextRetryDelay());
	this->numDetails = 0;
	this->setFile(request.getFile());
    this->setParamType(request.getParamType());
    this->setPublicNode(request.getPublicNode());
    this->setFlag(request.getFlag());
    this->setTransfer(request.getTransfer());
    this->setTotalBytes(request.getTotalBytes());
    this->setTransferredBytes(request.getTransferredBytes());
	this->listener = request.getListener();
	this->accountDetails = NULL;
	if(request.getAccountDetails())
    {
		this->accountDetails = new AccountDetails();
        AccountDetails *temp = request.getAccountDetails();
        this->accountDetails->pro_level = temp->pro_level;
        this->accountDetails->subscription_type = temp->subscription_type;
        this->accountDetails->pro_until = temp->pro_until;
        this->accountDetails->storage_used = temp->storage_used;
        this->accountDetails->storage_max = temp->storage_max;
        this->accountDetails->transfer_own_used = temp->transfer_own_used;
        this->accountDetails->transfer_srv_used = temp->transfer_srv_used;
        this->accountDetails->transfer_max = temp->transfer_max;
        this->accountDetails->transfer_own_reserved = temp->transfer_own_reserved;
        this->accountDetails->transfer_srv_reserved = temp->transfer_srv_reserved;
        this->accountDetails->srv_ratio = temp->srv_ratio;
        this->accountDetails->transfer_hist_starttime = temp->transfer_hist_starttime;
        this->accountDetails->transfer_hist_interval = temp->transfer_hist_interval;
        this->accountDetails->transfer_reserved = temp->transfer_reserved;
        this->accountDetails->transfer_limit = temp->transfer_limit;
        this->accountDetails->storage = temp->storage;
	}
}


MegaRequest::~MegaRequest()
{
	if(link) delete [] link;
	if(name) delete [] name;
	if(email) delete [] email;
	if(password) delete [] password;
	if(newPassword) delete [] newPassword;
	if(privateKey) delete [] privateKey;
	if(accountDetails) delete accountDetails;
    if(sessionKey) delete [] sessionKey;
	if(publicNode) delete publicNode;
	if(file) delete [] file;
}

MegaRequest *MegaRequest::copy()
{
	return new MegaRequest(*this);
}

int MegaRequest::getType() const { return type; }
uint64_t MegaRequest::getNodeHandle() const { return nodeHandle; }
const char* MegaRequest::getLink() const { return link; }
uint64_t MegaRequest::getParentHandle() const { return parentHandle; }
const char* MegaRequest::getSessionKey() const { return sessionKey; }
const char* MegaRequest::getName() const { return name; }
const char* MegaRequest::getEmail() const { return email; }
const char* MegaRequest::getPassword() const { return password; }
const char* MegaRequest::getNewPassword() const { return newPassword; }
const char* MegaRequest::getPrivateKey() const { return privateKey; }
int MegaRequest::getAccess() const { return access; }
const char* MegaRequest::getFile() const { return file; }
int MegaRequest::getParamType() const { return attrType; }
bool MegaRequest::getFlag() const { return flag;}
long long MegaRequest::getTransferredBytes() const { return transferredBytes; }
long long MegaRequest::getTotalBytes() const { return totalBytes; }
int MegaRequest::getNumRetry() const { return numRetry; }
int MegaRequest::getNextRetryDelay() const { return nextRetryDelay; }
AccountDetails *MegaRequest::getAccountDetails() const { return accountDetails; }
MegaAccountDetails *MegaRequest::getMegaAccountDetails() const
{ return MegaAccountDetails::fromAccountDetails(accountDetails); }
int MegaRequest::getNumDetails() const { return numDetails; }
void MegaRequest::setNumDetails(int numDetails) { this->numDetails = numDetails; }
MegaNode *MegaRequest::getPublicNode() { return publicNode;}

void MegaRequest::setNodeHandle(uint64_t nodeHandle) { this->nodeHandle = nodeHandle; }
void MegaRequest::setParentHandle(uint64_t parentHandle) { this->parentHandle = parentHandle; }
void MegaRequest::setSessionKey(const char* sessionKey)
{
    if(this->sessionKey) delete [] this->sessionKey;
    this->sessionKey = MegaApi::strdup(sessionKey);
}

void MegaRequest::setNumRetry(int numRetry) { this->numRetry = numRetry; }
void MegaRequest::setNextRetryDelay(int nextRetryDelay) { this->nextRetryDelay = nextRetryDelay; }

void MegaRequest::setLink(const char* link)
{
	if(this->link) delete [] this->link;
    this->link = MegaApi::strdup(link);
}
void MegaRequest::setName(const char* name)
{
	if(this->name) delete [] this->name;
    this->name = MegaApi::strdup(name);
}
void MegaRequest::setEmail(const char* email)
{
	if(this->email) delete [] this->email;
    this->email = MegaApi::strdup(email);
}
void MegaRequest::setPassword(const char* password)
{
	if(this->password) delete [] this->password;
    this->password = MegaApi::strdup(password);
}
void MegaRequest::setNewPassword(const char* newPassword)
{
	if(this->newPassword) delete [] this->newPassword;
    this->newPassword = MegaApi::strdup(newPassword);
}
void MegaRequest::setPrivateKey(const char* privateKey)
{
	if(this->privateKey) delete [] this->privateKey;
    this->privateKey = MegaApi::strdup(privateKey);
}
void MegaRequest::setAccess(int access)
{
	this->access = access;
}

void MegaRequest::setFile(const char* file)
{
    if(this->file)
        delete [] this->file;
    this->file = MegaApi::strdup(file);
}

void MegaRequest::setParamType(int type)
{
    this->attrType = type;
}

void MegaRequest::setFlag(bool flag)
{
    this->flag = flag;
}

void MegaRequest::setTransfer(int transfer)
{
    this->transfer = transfer;
}

void MegaRequest::setListener(MegaRequestListener *listener)
{
    this->listener = listener;
}

void MegaRequest::setTotalBytes(long long totalBytes)
{
    this->totalBytes = totalBytes;
}

void MegaRequest::setTransferredBytes(long long transferredBytes)
{
    this->transferredBytes = transferredBytes;
}

void MegaRequest::setPublicNode(MegaNode *publicNode)
{
    if(this->publicNode) delete this->publicNode;
    if(!publicNode) this->publicNode = NULL;
    else this->publicNode = new MegaNode(publicNode);
}

const char *MegaRequest::getRequestString() const
{
	switch(type)
	{
		case TYPE_LOGIN: return "login";
		case TYPE_MKDIR: return "mkdir";
		case TYPE_MOVE: return "move";
		case TYPE_COPY: return "copy";
		case TYPE_RENAME: return "rename";
		case TYPE_REMOVE: return "remove";
		case TYPE_SHARE: return "share";
		case TYPE_FOLDER_ACCESS: return "folderaccess";
		case TYPE_IMPORT_LINK: return "importlink";
		case TYPE_IMPORT_NODE: return "importnode";
		case TYPE_EXPORT: return "export";
		case TYPE_FETCH_NODES: return "fetchnodes";
		case TYPE_ACCOUNT_DETAILS: return "accountdetails";
		case TYPE_CHANGE_PW: return "changepw";
		case TYPE_UPLOAD: return "upload";
		case TYPE_LOGOUT: return "logout";
		case TYPE_FAST_LOGIN: return "fastlogin";
		case TYPE_GET_PUBLIC_NODE: return "getpublicnode";
		case TYPE_GET_ATTR_FILE: return "getattrfile";
        case TYPE_SET_ATTR_FILE: return "setattrfile";
        case TYPE_GET_ATTR_USER: return "getattruser";
        case TYPE_SET_ATTR_USER: return "setattruser";
        case TYPE_RETRY_PENDING_CONNECTIONS: return "retrypending";
        case TYPE_ADD_CONTACT: return "addcontact";
        case TYPE_REMOVE_CONTACT: return "removecontact";
        case TYPE_CREATE_ACCOUNT: return "createaccount";
        case TYPE_FAST_CREATE_ACCOUNT: return "fastcreateaccount";
        case TYPE_CONFIRM_ACCOUNT: return "confirmaccount";
        case TYPE_FAST_CONFIRM_ACCOUNT: return "fastconfirmaccount";
        case TYPE_QUERY_SIGNUP_LINK: return "querysignuplink";
        case TYPE_ADD_SYNC: return "addsync";
        case TYPE_REMOVE_SYNC: return "removesync";
        case TYPE_REMOVE_SYNCS: return "removesyncs";
        case TYPE_PAUSE_TRANSFERS: return "pausetransfers";
        case TYPE_CANCEL_TRANSFER: return "canceltransfer";
        case TYPE_CANCEL_TRANSFERS: return "canceltransfers";
        case TYPE_DELETE: return "delete";
	}
	return "unknown";
}

MegaRequestListener *MegaRequest::getListener() const { return listener; }
int MegaRequest::getTransfer() const { return transfer; }

const char *MegaRequest::toString() const { return getRequestString(); }
const char *MegaRequest::__str__() const { return getRequestString(); }

MegaTransfer::MegaTransfer(int type, MegaTransferListener *listener)
{
	this->type = type;
	this->slot = -1;
	this->tag = -1;
	this->path = NULL;
	this->nodeHandle = UNDEF;
	this->parentHandle = UNDEF;
	this->startPos = 0;
	this->endPos = 0;
	this->numConnections = 1;
	this->maxSpeed = 1;
	this->parentPath = NULL;
	this->listener = listener;
	this->retry = 0;
	this->maxRetries = 3;
	this->time = 0;
	this->startTime = 0;
	this->transferredBytes = 0;
	this->totalBytes = 0;
	this->fileName = NULL;
	this->base64Key = NULL;
	this->transfer = NULL;
	this->speed = 0;
	this->deltaSize = 0;
	this->updateTime = 0;
    this->publicNode = NULL;
    this->lastBytes = NULL;
    this->syncTransfer = false;
}

MegaTransfer::MegaTransfer(const MegaTransfer &transfer)
{
    path = NULL;
    parentPath = NULL;
    fileName = NULL;
    base64Key = NULL;
    publicNode = NULL;
	lastBytes = NULL;

    this->listener = transfer.getListener();
    this->transfer = transfer.getTransfer();
	this->type = transfer.getType();
	this->setSlot(transfer.getSlot());
	this->setTag(transfer.getTag());
	this->setPath(transfer.getPath());
	this->setNodeHandle(transfer.getNodeHandle());
	this->setParentHandle(transfer.getParentHandle());
	this->setStartPos(transfer.getStartPos());
	this->setEndPos(transfer.getEndPos());
	this->setNumConnections(transfer.getNumConnections());
	this->setMaxSpeed(transfer.getMaxSpeed());
	this->setParentPath(transfer.getParentPath());
	this->setNumRetry(transfer.getNumRetry());
	this->setMaxRetries(transfer.getMaxRetries());
	this->setTime(transfer.getTime());
	this->setStartTime(transfer.getStartTime());
	this->setTransferredBytes(transfer.getTransferredBytes());
	this->setTotalBytes(transfer.getTotalBytes());
	this->setFileName(transfer.getFileName());
	this->setBase64Key(transfer.getBase64Key());
	this->setSpeed(transfer.getSpeed());
	this->setDeltaSize(transfer.getDeltaSize());
	this->setUpdateTime(transfer.getUpdateTime());
    this->setPublicNode(transfer.getPublicNode());
    this->setTransfer(transfer.getTransfer());
    this->setSyncTransfer(transfer.isSyncTransfer());
}

MegaTransfer* MegaTransfer::copy()
{
	return new MegaTransfer(*this);
}

int MegaTransfer::getSlot() const { return slot; }
int MegaTransfer::getTag() const { return tag; }
Transfer* MegaTransfer::getTransfer() const { return transfer; }
long long MegaTransfer::getSpeed() const { return speed; }
long long MegaTransfer::getDeltaSize() const { return deltaSize; }
int64_t MegaTransfer::getUpdateTime() const { return updateTime; }
MegaNode *MegaTransfer::getPublicNode() const { return publicNode; }
bool MegaTransfer::isSyncTransfer() const { return syncTransfer; }
bool MegaTransfer::isStreamingTransfer() const { return transfer == NULL; }
int MegaTransfer::getType() const { return type; }
int64_t MegaTransfer::getStartTime() const { return startTime; }
long long MegaTransfer::getTransferredBytes() const {return transferredBytes; }
long long MegaTransfer::getTotalBytes() const { return totalBytes; }
const char* MegaTransfer::getPath() const { return path; }
const char* MegaTransfer::getParentPath() const { return parentPath; }
uint64_t MegaTransfer::getNodeHandle() const { return nodeHandle; }
uint64_t MegaTransfer::getParentHandle() const { return parentHandle; }
int MegaTransfer::getNumConnections() const { return numConnections; }
long long MegaTransfer::getStartPos() const { return startPos; }
long long MegaTransfer::getEndPos() const { return endPos; }
int MegaTransfer::getMaxSpeed() const { return maxSpeed; }
int MegaTransfer::getNumRetry() const { return retry; }
int MegaTransfer::getMaxRetries() const { return maxRetries; }
int64_t MegaTransfer::getTime() const { return time; }
const char* MegaTransfer::getFileName() const { return fileName; }
const char* MegaTransfer::getBase64Key() const { return base64Key; }
char * MegaTransfer::getLastBytes() const { return lastBytes; }

void MegaTransfer::setSlot(int slot) { this->slot = slot; }
void MegaTransfer::setTag(int tag) { this->tag = tag; }
void MegaTransfer::setTransfer(Transfer *transfer) { this->transfer = transfer; }
void MegaTransfer::setSpeed(long long speed) { this->speed = speed; }
void MegaTransfer::setDeltaSize(long long deltaSize){ this->deltaSize = deltaSize; }
void MegaTransfer::setUpdateTime(int64_t updateTime) { this->updateTime = updateTime; }

void MegaTransfer::setPublicNode(MegaNode *publicNode)
{
    if(this->publicNode) delete this->publicNode;
    if(!publicNode) this->publicNode = NULL;
    else this->publicNode = new MegaNode(publicNode);
}

void MegaTransfer::setSyncTransfer(bool syncTransfer) { this->syncTransfer = syncTransfer; }
void MegaTransfer::setStartTime(int64_t startTime) { this->startTime = startTime; }
void MegaTransfer::setTransferredBytes(long long transferredBytes) { this->transferredBytes = transferredBytes; }
void MegaTransfer::setTotalBytes(long long totalBytes) { this->totalBytes = totalBytes; }
void MegaTransfer::setLastBytes(char *lastBytes) { this->lastBytes = lastBytes; }

void MegaTransfer::setPath(const char* path)
{
	if(this->path) delete [] this->path;
    this->path = MegaApi::strdup(path);
	if(!this->path) return;

	for(int i = strlen(path)-1; i>=0; i--)
	{
		if((path[i]=='\\') || (path[i]=='/'))
		{
			setFileName(&(path[i+1]));
            char *parentPath = MegaApi::strdup(path);
            parentPath[i+1] = '\0';
            setParentPath(parentPath);
            delete parentPath;
			return;
		}
	}
	setFileName(path);
}
void MegaTransfer::setParentPath(const char* path)
{
	if(this->parentPath) delete [] this->parentPath;
    this->parentPath =  MegaApi::strdup(path);
}

void MegaTransfer::setFileName(const char* fileName)
{
	if(this->fileName) delete [] this->fileName;
    this->fileName =  MegaApi::strdup(fileName);
}

void MegaTransfer::setBase64Key(const char* base64Key)
{
	if(this->base64Key) delete [] this->base64Key;
    this->base64Key =  MegaApi::strdup(base64Key);
}

void MegaTransfer::setNodeHandle(uint64_t nodeHandle) { this->nodeHandle = nodeHandle; }
void MegaTransfer::setParentHandle(uint64_t parentHandle) { this->parentHandle = parentHandle; }
void MegaTransfer::setNumConnections(int connections) { this->numConnections = connections; }
void MegaTransfer::setStartPos(long long startPos) { this->startPos = startPos; }
void MegaTransfer::setEndPos(long long endPos) { this->endPos = endPos; }
void MegaTransfer::setMaxSpeed(int maxSpeed) {this->maxSpeed = maxSpeed; }
void MegaTransfer::setNumRetry(int retry) {this->retry = retry; }
void MegaTransfer::setMaxRetries(int maxRetries) {this->maxRetries = maxRetries; }
void MegaTransfer::setTime(int64_t time) { this->time = time; }

const char * MegaTransfer::getTransferString() const
{
	switch(type)
	{
	case TYPE_UPLOAD:
		return "upload";
	case TYPE_DOWNLOAD:
		return "download";
	}

	return "unknown";
}

MegaTransferListener* MegaTransfer::getListener() const { return listener; }

MegaTransfer::~MegaTransfer()
{
	if(path) delete[] path;
	if(parentPath) delete[] parentPath;
	if(fileName) delete [] fileName;
    if(base64Key) delete [] base64Key;
    if(publicNode) delete publicNode;
}

const char * MegaTransfer::toString() const { return getTransferString(); }
const char * MegaTransfer::__str__() const { return getTransferString(); }

MegaError::MegaError(int errorCode)
{
	this->errorCode = errorCode;
	this->nextAttempt = 0;
}

MegaError::MegaError(const MegaError &megaError)
{
	errorCode = megaError.getErrorCode();
	nextAttempt = megaError.getNextAttempt();
}

MegaError* MegaError::copy()
{
	return new MegaError(*this);
}

int MegaError::getErrorCode() const { return errorCode; }
const char* MegaError::getErrorString() const
{
    return MegaError::getErrorString(errorCode);
}

const char* MegaError::getErrorString(int errorCode)
{
	if(errorCode <= 0)
	{
		switch(errorCode)
		{
		case API_OK:
            return "No error";
		case API_EINTERNAL:
            return "Internal error";
		case API_EARGS:
            return "Invalid argument";
		case API_EAGAIN:
            return "Request failed, retrying";
		case API_ERATELIMIT:
            return "Rate limit exceeded";
		case API_EFAILED:
            return "Failed permanently";
		case API_ETOOMANY:
            return "Too many concurrent connections or transfers";
		case API_ERANGE:
            return "Out of range";
		case API_EEXPIRED:
            return "Expired";
		case API_ENOENT:
            return "Not found";
		case API_ECIRCULAR:
            return "Circular linkage detected";
		case API_EACCESS:
            return "Access denied";
		case API_EEXIST:
            return "Already exists";
		case API_EINCOMPLETE:
            return "Incomplete";
		case API_EKEY:
            return "Invalid key/Decryption error";
		case API_ESID:
            return "Bad session ID";
		case API_EBLOCKED:
            return "Blocked";
		case API_EOVERQUOTA:
            return "Over quota";
		case API_ETEMPUNAVAIL:
            return "Temporarily not available";
		case API_ETOOMANYCONNECTIONS:
            return "Connection overflow";
		case API_EWRITE:
            return "Write error";
		case API_EREAD:
            return "Read error";
		case API_EAPPKEY:
            return "Invalid application key";
		default:
            return "Unknown error";
		}
	}
    return "HTTP Error";
}

const char* MegaError::toString() const { return getErrorString(); }
const char* MegaError::__str__() const { return getErrorString(); }


bool MegaError::isTemporal() const { return (nextAttempt==0); }
long MegaError::getNextAttempt() const { return nextAttempt; }
void MegaError::setNextAttempt(long nextAttempt) { this->nextAttempt = nextAttempt; }

//Request callbacks
void MegaRequestListener::onRequestStart(MegaApi *, MegaRequest *)
{ }
void MegaRequestListener::onRequestFinish(MegaApi *, MegaRequest *, MegaError *)
{ }
void MegaRequestListener::onRequestUpdate(MegaApi *, MegaRequest *)
{ }
void MegaRequestListener::onRequestTemporaryError(MegaApi *, MegaRequest *, MegaError *)
{ }
MegaRequestListener::~MegaRequestListener() {}

//Transfer callbacks
void MegaTransferListener::onTransferStart(MegaApi *, MegaTransfer *)
{ }
void MegaTransferListener::onTransferFinish(MegaApi*, MegaTransfer *, MegaError*)
{ }
void MegaTransferListener::onTransferUpdate(MegaApi *, MegaTransfer *)
{ }
bool MegaTransferListener::onTransferData(MegaApi *, MegaTransfer *, char *, size_t)
{ return true; }
void MegaTransferListener::onTransferTemporaryError(MegaApi *, MegaTransfer *, MegaError*)
{ }
MegaTransferListener::~MegaTransferListener()
{ }

//Global callbacks
#if defined(__ANDROID__) || defined(WINDOWS_PHONE)
void MegaGlobalListener::onUsersUpdate(MegaApi*)
{ }
void MegaGlobalListener::onNodesUpdate(MegaApi*)
{ }
#else
void MegaGlobalListener::onUsersUpdate(MegaApi *, UserList *)
{ }
void MegaGlobalListener::onNodesUpdate(MegaApi *, NodeList *)
{ }
#endif

void MegaGlobalListener::onReloadNeeded(MegaApi *)
{ }
MegaGlobalListener::~MegaGlobalListener()
{ }

//All callbacks
void MegaListener::onRequestStart(MegaApi *, MegaRequest *)
{ }
void MegaListener::onRequestFinish(MegaApi *, MegaRequest *, MegaError *)
{ }
void MegaListener::onRequestUpdate(MegaApi * , MegaRequest *)
{ }
void MegaListener::onRequestTemporaryError(MegaApi *, MegaRequest *, MegaError *)
{ }
void MegaListener::onTransferStart(MegaApi *, MegaTransfer *)
{ }
void MegaListener::onTransferFinish(MegaApi *, MegaTransfer *, MegaError *)
{ }
void MegaListener::onTransferUpdate(MegaApi *, MegaTransfer *)
{ }
void MegaListener::onTransferTemporaryError(MegaApi *, MegaTransfer *, MegaError *)
{ }

#if defined(__ANDROID__) || defined(WINDOWS_PHONE)
void MegaListener::onUsersUpdate(MegaApi*)
{ }
void MegaListener::onNodesUpdate(MegaApi*)
{ }
#else
void MegaListener::onUsersUpdate(MegaApi *, UserList *)
{ }
void MegaListener::onNodesUpdate(MegaApi *, NodeList *)
{ }
#endif

void MegaListener::onReloadNeeded(MegaApi *)
{ }

void MegaListener::onSyncFileStateChanged(MegaApi *, const char *, MegaSyncState)
{ }

void MegaListener::onSyncStateChanged(MegaApi *)
{ }

MegaListener::~MegaListener() {}

bool MegaTreeProcessor::processMegaNode(MegaNode*)
{ return false; /* Stops the processing */ }
MegaTreeProcessor::~MegaTreeProcessor()
{ }

#ifdef USE_EXTERNAL_GFX
MegaApi::MegaApi(const char *appKey, MegaGfxProcessor* processor, const char *basePath, const char *userAgent)
{
    pImpl = new MegaApiImpl(this, appKey, processor, basePath, userAgent);
}
#else
MegaApi::MegaApi(const char *appKey, const char *basePath, const char *userAgent)
{
    pImpl = new MegaApiImpl(this, appKey, basePath, userAgent);
}
#endif

MegaApi::~MegaApi()
{
    delete pImpl;
}

int MegaApi::isLoggedIn()
{
    return pImpl->isLoggedIn();
}

const char* MegaApi::getMyEmail()
{
    return pImpl->getMyEmail();
}

const char* MegaApi::getBase64PwKey(const char *password)
{
    return pImpl->getBase64PwKey(password);
}

const char* MegaApi::getStringHash(const char* base64pwkey, const char* inBuf)
{
    return pImpl->getStringHash(base64pwkey, inBuf);
}

const char* MegaApi::ebcEncryptKey(const char* encryptionKey, const char* plainKey)
{
    return MegaApiImpl::ebcEncryptKey(encryptionKey, plainKey);
}

uint64_t MegaApi::base64ToHandle(const char* base64Handle)
{
    return MegaApiImpl::base64ToHandle(base64Handle);
}

void MegaApi::retryPendingConnections()
{
    pImpl->retryPendingConnections();
}

void MegaApi::fastLogin(const char* email, const char *stringHash, const char *base64pwkey, MegaRequestListener *listener)
{
    pImpl->fastLogin(email, stringHash, base64pwkey,listener);
}

void MegaApi::fastLogin(const char *session, MegaRequestListener *listener)
{
    pImpl->fastLogin(session, listener);
}

void MegaApi::login(const char *login, const char *password, MegaRequestListener *listener)
{
    pImpl->login(login, password, listener);
}

const char *MegaApi::dumpSession()
{
    return pImpl->dumpSession();
}

void MegaApi::createAccount(const char* email, const char* password, const char* name, MegaRequestListener *listener)
{
    pImpl->createAccount(email, password, name, listener);
}

void MegaApi::fastCreateAccount(const char* email, const char *base64pwkey, const char* name, MegaRequestListener *listener)
{
    pImpl->fastCreateAccount(email, base64pwkey, name, listener);
}

void MegaApi::querySignupLink(const char* link, MegaRequestListener *listener)
{
    pImpl->querySignupLink(link, listener);
}

void MegaApi::confirmAccount(const char* link, const char *password, MegaRequestListener *listener)
{
    pImpl->confirmAccount(link, password, listener);
}

void MegaApi::fastConfirmAccount(const char* link, const char *base64pwkey, MegaRequestListener *listener)
{
    pImpl->fastConfirmAccount(link, base64pwkey, listener);
}

void MegaApi::setProxySettings(MegaProxy *proxySettings)
{
    pImpl->setProxySettings(proxySettings);
}

MegaProxy *MegaApi::getAutoProxySettings()
{
    return pImpl->getAutoProxySettings();
}

void MegaApi::createFolder(const char *name, MegaNode *parent, MegaRequestListener *listener)
{
    pImpl->createFolder(name, parent, listener);
}

void MegaApi::moveNode(MegaNode *node, MegaNode *newParent, MegaRequestListener *listener)
{
    pImpl->moveNode(node, newParent, listener);
}

void MegaApi::copyNode(MegaNode *node, MegaNode* target, MegaRequestListener *listener)
{
    pImpl->copyNode(node, target, listener);
}

void MegaApi::renameNode(MegaNode *node, const char *newName, MegaRequestListener *listener)
{
    pImpl->renameNode(node, newName, listener);
}

void MegaApi::remove(MegaNode *node, MegaRequestListener *listener)
{
    pImpl->remove(node, listener);
}

void MegaApi::sendFileToUser(MegaNode *node, MegaUser *user, MegaRequestListener *listener)
{
    pImpl->sendFileToUser(node, user, listener);
}

void MegaApi::sendFileToUser(MegaNode *node, const char* email, MegaRequestListener *listener)
{
    pImpl->sendFileToUser(node, email, listener);
}

void MegaApi::share(MegaNode* node, MegaUser *user, int access, MegaRequestListener *listener)
{
    pImpl->share(node, user, access, listener);
}

void MegaApi::share(MegaNode *node, const char* email, int access, MegaRequestListener *listener)
{
    pImpl->share(node, email, access, listener);
}

void MegaApi::folderAccess(const char* megaFolderLink, MegaRequestListener *listener)
{
    pImpl->folderAccess(megaFolderLink, listener);
}

void MegaApi::importFileLink(const char* megaFileLink, MegaNode *parent, MegaRequestListener *listener)
{
    pImpl->importFileLink(megaFileLink, parent, listener);
}

void MegaApi::importPublicNode(MegaNode *publicNode, MegaNode* parent, MegaRequestListener *listener)
{
    pImpl->importPublicNode(publicNode, parent, listener);
}

void MegaApi::getPublicNode(const char* megaFileLink, MegaRequestListener *listener)
{
    pImpl->getPublicNode(megaFileLink, listener);
}

void MegaApi::getThumbnail(MegaNode* node, const char *dstFilePath, MegaRequestListener *listener)
{
    pImpl->getThumbnail(node, dstFilePath, listener);
}

void MegaApi::setThumbnail(MegaNode* node, const char *srcFilePath, MegaRequestListener *listener)
{
    pImpl->setThumbnail(node, srcFilePath, listener);
}

void MegaApi::getPreview(MegaNode* node, const char *dstFilePath, MegaRequestListener *listener)
{
    pImpl->getPreview(node, dstFilePath, listener);
}

void MegaApi::setPreview(MegaNode* node, const char *srcFilePath, MegaRequestListener *listener)
{
    pImpl->setPreview(node, srcFilePath, listener);
}

void MegaApi::getUserAvatar(MegaUser* user, const char *dstFilePath, MegaRequestListener *listener)
{
    pImpl->getUserAvatar(user, dstFilePath, listener);
}

void MegaApi::exportNode(MegaNode *node, MegaRequestListener *listener)
{
    pImpl->exportNode(node, listener);
}

void MegaApi::disableExport(MegaNode *node, MegaRequestListener *listener)
{
    pImpl->disableExport(node, listener);
}

void MegaApi::fetchNodes(MegaRequestListener *listener)
{
    pImpl->fetchNodes(listener);
}

void MegaApi::getAccountDetails(MegaRequestListener *listener)
{
    pImpl->getAccountDetails(listener);
}

void MegaApi::changePassword(const char *oldPassword, const char *newPassword, MegaRequestListener *listener)
{
    pImpl->changePassword(oldPassword, newPassword, listener);
}

void MegaApi::logout(MegaRequestListener *listener)
{
    pImpl->logout(listener);
}

void MegaApi::addContact(const char* email, MegaRequestListener* listener)
{
    pImpl->addContact(email, listener);
}

void MegaApi::removeContact(const char* email, MegaRequestListener* listener)
{
    pImpl->removeContact(email, listener);
}

void MegaApi::pauseTransfers(bool pause, MegaRequestListener* listener)
{
    pImpl->pauseTransfers(pause, listener);
}

//-1 -> AUTO, 0 -> NONE, >0 -> b/s
void MegaApi::setUploadLimit(int bpslimit)
{
    pImpl->setUploadLimit(bpslimit);
}

TransferList *MegaApi::getTransfers()
{
    return pImpl->getTransfers();
}

void MegaApi::startUpload(const char* localPath, MegaNode* parent, MegaTransferListener *listener)
{
    pImpl->startUpload(localPath, parent, listener);
}

void MegaApi::startUpload(const char* localPath, MegaNode* parent, const char* fileName, MegaTransferListener *listener)
{
    pImpl->startUpload(localPath, parent, fileName, listener);
}

void MegaApi::startDownload(MegaNode *node, const char* localFolder, MegaTransferListener *listener)
{
    pImpl->startDownload(node, localFolder, listener);
}

void MegaApi::startPublicDownload(MegaNode* node, const char* localPath, MegaTransferListener *listener)
{
    pImpl->startPublicDownload(node, localPath, listener);
}

void MegaApi::cancelTransfer(MegaTransfer *t, MegaRequestListener *listener)
{
    pImpl->cancelTransfer(t, listener);
}

void MegaApi::cancelTransfers(int direction, MegaRequestListener *listener)
{
    pImpl->cancelTransfers(direction, listener);
}

void MegaApi::startStreaming(MegaNode* node, int64_t startPos, int64_t size, MegaTransferListener *listener)
{
    pImpl->startStreaming(node, startPos, size, listener);
}

//Move local files inside synced folders to the "Rubbish" folder.
bool MegaApi::moveToLocalDebris(const char *path)
{
    return pImpl->moveToLocalDebris(path);
}

MegaSyncState MegaApi::syncPathState(string* path)
{
    return (MegaSyncState)pImpl->syncPathState(path);
}

MegaNode *MegaApi::getSyncedNode(string *path)
{
    return pImpl->getSyncedNode(path);
}

void MegaApi::syncFolder(const char *localFolder, MegaNode *megaFolder)
{
   pImpl->syncFolder(localFolder, megaFolder);
}

void MegaApi::resumeSync(const char *localFolder, MegaNode *megaFolder)
{
    pImpl->resumeSync(localFolder, megaFolder);
}

void MegaApi::removeSync(uint64_t nodeuint64_t, MegaRequestListener* listener)
{
    pImpl->removeSync(nodeuint64_t, listener);
}

int MegaApi::getNumActiveSyncs()
{
    return pImpl->getNumActiveSyncs();
}

void MegaApi::stopSyncs(MegaRequestListener *listener)
{
   pImpl->stopSyncs(listener);
}

int MegaApi::getNumPendingUploads()
{
    return pImpl->getNumPendingUploads();
}

int MegaApi::getNumPendingDownloads()
{
    return pImpl->getNumPendingDownloads();
}

int MegaApi::getTotalUploads()
{
    return pImpl->getTotalUploads();
}

int MegaApi::getTotalDownloads()
{
    return pImpl->getTotalDownloads();
}

void MegaApi::resetTotalDownloads()
{
    pImpl->resetTotalDownloads();
}

void MegaApi::resetTotalUploads()
{
    pImpl->resetTotalUploads();
}

string MegaApi::getLocalPath(MegaNode *n)
{
    return pImpl->getLocalPath(n);
}

MegaNode *MegaApi::getRootNode()
{
    return pImpl->getRootNode();
}

MegaNode* MegaApi::getInboxNode()
{
    return pImpl->getInboxNode();
}

MegaNode* MegaApi::getRubbishNode()
{
    return pImpl->getRubbishNode();
}

UserList* MegaApi::getContacts()
{
    return pImpl->getContacts();
}

MegaUser* MegaApi::getContact(const char* email)
{
    return pImpl->getContact(email);
}

NodeList* MegaApi::getInShares(MegaUser *megaUser)
{
    return pImpl->getInShares(megaUser);
}

NodeList* MegaApi::getInShares()
{
    return pImpl->getInShares();
}

ShareList* MegaApi::getOutShares(MegaNode *megaNode)
{
    return pImpl->getOutShares(megaNode);
}

int MegaApi::getAccess(MegaNode* megaNode)
{
    return pImpl->getAccess(megaNode);
}

bool MegaApi::processMegaTree(MegaNode* n, MegaTreeProcessor* processor, bool recursive)
{
    return pImpl->processMegaTree(n, processor, recursive);
}

NodeList* MegaApi::search(MegaNode* n, const char* searchString, bool recursive)
{
    return pImpl->search(n, searchString, recursive);
}

long long MegaApi::getSize(MegaNode *n)
{
    return pImpl->getSize(n);
}

void MegaApi::addListener(MegaListener* listener)
{
    pImpl->addListener(listener);
}

void MegaApi::addRequestListener(MegaRequestListener* listener)
{
    pImpl->addRequestListener(listener);
}

void MegaApi::addTransferListener(MegaTransferListener* listener)
{
    pImpl->addTransferListener(listener);
}

void MegaApi::addGlobalListener(MegaGlobalListener* listener)
{
    pImpl->addGlobalListener(listener);
}

void MegaApi::removeListener(MegaListener* listener)
{
    pImpl->removeListener(listener);
}

void MegaApi::removeRequestListener(MegaRequestListener* listener)
{
    pImpl->removeRequestListener(listener);
}

void MegaApi::removeTransferListener(MegaTransferListener* listener)
{
    pImpl->removeTransferListener(listener);
}

void MegaApi::removeGlobalListener(MegaGlobalListener* listener)
{
    pImpl->removeGlobalListener(listener);
}

MegaError MegaApi::checkAccess(MegaNode* megaNode, int level)
{
    return pImpl->checkAccess(megaNode, level);
}

MegaError MegaApi::checkMove(MegaNode* megaNode, MegaNode* targetNode)
{
    return pImpl->checkMove(megaNode, targetNode);
}

NodeList *MegaApi::getChildren(MegaNode* p, int order)
{
    return pImpl->getChildren(p, order);
}

MegaNode *MegaApi::getChildNode(MegaNode *parent, const char* name)
{
    return pImpl->getChildNode(parent, name);
}

MegaNode* MegaApi::getParentNode(MegaNode* n)
{
    return pImpl->getParentNode(n);
}

const char* MegaApi::getNodePath(MegaNode *node)
{
    return pImpl->getNodePath(node);
}

MegaNode* MegaApi::getNodeByPath(const char *path, MegaNode* node)
{
    return pImpl->getNodeByPath(path, node);
}

MegaNode* MegaApi::getNodeByHandle(uint64_t uint64_t)
{
    return pImpl->getNodeByHandle(uint64_t);
}

void MegaApi::updateStatics()
{
    pImpl->updateStatics();
}

void MegaApi::update()
{
   pImpl->update();
}

bool MegaApi::isIndexing()
{
    return pImpl->isIndexing();
}

bool MegaApi::isWaiting()
{
    return pImpl->isWaiting();
}

bool MegaApi::isSynced(MegaNode *n)
{
    return pImpl->isSynced(n);
}

bool MegaApi::isSyncable(const char *name)
{
   return pImpl->is_syncable(name);
}

void MegaApi::setExcludedNames(vector<string> *excludedNames)
{
    pImpl->setExcludedNames(excludedNames);
}


char* MegaApi::strdup(const char* buffer)
{
    if(!buffer)
        return NULL;
    int tam = strlen(buffer)+1;
    char *newbuffer = new char[tam];
    memcpy(newbuffer, buffer, tam);
    return newbuffer;
}

#ifdef WIN32
// convert Windows Unicode to UTF-8
void MegaApi::utf16ToUtf8(const wchar_t* utf16data, int utf16size, string* path)
{
    path->resize((utf16size + 1) * 4);

    path->resize(WideCharToMultiByte(CP_UTF8, 0, utf16data,
        utf16size,
        (char*)path->data(),
        path->size() + 1,
        NULL, NULL));
}

void MegaApi::utf8ToUtf16(const char* utf8data, string* utf16string)
{
    int size = strlen(utf8data) + 1;

    // make space for the worst case
    utf16string->resize(size * sizeof(wchar_t));

    // resize to actual result
    utf16string->resize(sizeof(wchar_t) * (MultiByteToWideChar(CP_UTF8, 0,
        utf8data,
        size,
        (wchar_t*)utf16string->data(),
        utf16string->size())));
}
#endif

MegaHashSignature::MegaHashSignature(const char *base64Key)
{
    hashSignature = new mega::HashSignature(new mega::Hash());
    asymmCypher = new AsymmCipher();

    string pubks;
    int len = strlen(base64Key)/4*3+3;
    pubks.resize(len);
    pubks.resize(mega::Base64::atob(base64Key, (byte *)pubks.data(), len));
    asymmCypher->setkey(mega::AsymmCipher::PUBKEY,(byte*)pubks.data(), pubks.size());
}

MegaHashSignature::~MegaHashSignature()
{
    delete hashSignature;
    delete asymmCypher;
}

void MegaHashSignature::init()
{
    hashSignature->get(asymmCypher, NULL, 0);
}

void MegaHashSignature::add(const unsigned char *data, unsigned size)
{
    hashSignature->add(data, size);
}

bool MegaHashSignature::check(const char *base64Signature)
{
    char signature[512];
    int l = mega::Base64::atob(base64Signature, (byte *)signature, sizeof(signature));
    if(l != sizeof(signature))
        return false;

    return hashSignature->check(asymmCypher, (const byte *)signature, sizeof(signature));
}


int MegaAccountDetails::getProLevel()
{
    return details->pro_level;
}

long long MegaAccountDetails::getStorageMax()
{
    return details->storage_max;
}

long long MegaAccountDetails::getStorageUsed()
{
    return details->storage_used;
}

long long MegaAccountDetails::getTransferMax()
{
    return details->transfer_max;
}

long long MegaAccountDetails::getTransferOwnUsed()
{
    return details->transfer_own_used;
}

long long MegaAccountDetails::getStorageUsed(MegaHandle handle)
{
    return details->storage[handle].bytes;
}

long long MegaAccountDetails::getNumFiles(MegaHandle handle)
{
    return details->storage[handle].files;
}

long long MegaAccountDetails::getNumFolders(MegaHandle handle)
{
    return details->storage[handle].folders;
}

MegaAccountDetails *MegaAccountDetails::fromAccountDetails(AccountDetails *details)
{
    return new MegaAccountDetails(details);
}

MegaAccountDetails::MegaAccountDetails(AccountDetails *details)
{
    this->details = new AccountDetails;
    (*(this->details)) = (*details);
}
