#include "NodeSelectorModelItem.h"

#include "Avatar.h"
#include "FullName.h"
#include "MegaApplication.h"
#include "MyBackupsHandle.h"
#include "Utilities.h"

const int NodeSelectorModelItem::ICON_SIZE = 17;

using namespace mega;

NodeSelectorModelItem::NodeSelectorModelItem(std::unique_ptr<MegaNode> node,
                                             bool showFiles,
                                             NodeSelectorModelItem* parentItem):
    QObject(parentItem),
    mOwnerEmail(QString()),
    mStatus(Status::NONE),
    mRequestingChildren(false),
    mShowFiles(showFiles),
    mNodeAccess(mega::MegaShare::ACCESS_OWNER),
    mMegaApi(MegaSyncApp->getMegaApi()),
    mNode(std::move(node)),
    mOwner(nullptr)
{
    // In case we don´t have a valid node (which is an error), just use a goofy node
    if (!mNode)
    {
        mNode = std::make_shared<mega::MegaNode>();
    }

    // Share access is uniform across an inshare subtree: nested items inherit the parent's
    // cached level (primed on the inshare root through the SDK on the worker), so consumers
    // (rename/delete/link-share/sync eligibility) see the real access with no SDK call.
    if (parentItem)
    {
        mNodeAccess = parentItem->mNodeAccess.load();
    }

    resetChildrenCounter();

    if (mNode->isFile() || mNode->isInShare())
    {
        mStatus = Status::NONE;
    }
}

NodeSelectorModelItem::~NodeSelectorModelItem()
{
    qDeleteAll(mChildItems);
    mChildItems.clear();
}

bool NodeSelectorModelItem::isValid() const
{
    return mNode && mNode->getHandle() != mega::INVALID_HANDLE;
}

std::shared_ptr<mega::MegaNode> NodeSelectorModelItem::getNode() const
{
    return mNode;
}

bool NodeSelectorModelItem::isSpecialNode() const
{
    return (isCloudDrive() || isMyBackupsFolder() || isRubbishBin() || isS4Container());
}

bool NodeSelectorModelItem::isTakenDown() const
{
    return mNode && mNode->isTakenDown();
}

bool NodeSelectorModelItem::canBeRenamed() const
{
    if (isTakenDown() || isCloudDrive() || isMyBackupsFolder() || isRubbishBin() ||
        isInRubbishBin() || isS4Container() || (mMegaApi->isInVault(mNode.get())) ||
        (getNodeAccess() < mega::MegaShare::ACCESS_FULL))
    {
        return false;
    }

    return true;
}

QList<QPointer<NodeSelectorModelItem>>
    NodeSelectorModelItem::createChildItems(std::unique_ptr<mega::MegaNodeList> nodeList)
{
    QList<QPointer<NodeSelectorModelItem>> items;

    if (!mNode->isFile())
    {
        for (int i = 0; i < nodeList->size(); i++)
        {
            auto node = std::unique_ptr<MegaNode>(nodeList->get(i)->copy());
            auto child = createModelItem(std::move(node), mShowFiles, this);
            if (child->isValid())
            {
                items.append(child);
            }
            else
            {
                child->deleteLater();
            }
        }
    }

    return items;
}

void NodeSelectorModelItem::initializeChildItems(
    const QList<QPointer<NodeSelectorModelItem>>& items)
{
    for (const auto& item: items)
    {
        if (item)
        {
            connect(item,
                    &NodeSelectorModelItem::destroyed,
                    this,
                    &NodeSelectorModelItem::onChildDestroyed);
        }
    }
    mChildItems.append(items);
    mChildrenCounter = static_cast<int>(mChildItems.size());
    mRequestingChildren = false;
    mChildrenAreInit = true;
}

bool NodeSelectorModelItem::areChildrenInitialized() const
{
    return mChildrenAreInit;
}

bool NodeSelectorModelItem::canFetchMore()
{
    if (!mChildrenAreInit)
    {
        return true;
    }
    else if (mChildrenCounter == 0)
    {
        return false;
    }

    return mChildItems.isEmpty();
}

bool NodeSelectorModelItem::requestingChildren() const
{
    return mRequestingChildren;
}

void NodeSelectorModelItem::setRequestingChildren(bool newRequestingChildren)
{
    mRequestingChildren = newRequestingChildren;
}

void NodeSelectorModelItem::resetChildrenCounter()
{
    mChildrenCounter = mShowFiles ? MegaSyncApp->getMegaApi()->getNumChildren(mNode.get()) :
                                    MegaSyncApp->getMegaApi()->getNumChildFolders(mNode.get());

    // If it has no children, the item does not need to be init
    mChildrenAreInit = mChildrenCounter > 0 ? false : true;
}

int NodeSelectorModelItem::getNodeAccess() const
{
    // Only inshare root nodes resolve their access level (primed on the NodeRequester worker);
    // no consumer needs it for any other node, so the rest keep the ACCESS_OWNER default.
    // Never resolve it here: this getter runs in paint paths on the GUI thread, and
    // megaApi->getAccess blocks on the SDK mutex while the worker is fetching children
    // (500ms+ freezes on huge folders).
    return mNodeAccess;
}

void NodeSelectorModelItem::primeNodeAccess()
{
    mNodeAccess = Utilities::getNodeAccess(mNode.get());
}

QPointer<NodeSelectorModelItem> NodeSelectorModelItem::getParent() const
{
    return dynamic_cast<NodeSelectorModelItem*>(parent());
}

QPointer<NodeSelectorModelItem> NodeSelectorModelItem::getChild(int i)
{
    if (mChildItems.size() <= i)
    {
        return nullptr;
    }

    return mChildItems.at(i);
}

int NodeSelectorModelItem::getNumChildren()
{
    if (mNode->isFile())
    {
        return 0;
    }
    else if (!areChildrenInitialized())
    {
        return mChildrenCounter;
    }

    return static_cast<int>(mChildItems.size());
}

int NodeSelectorModelItem::indexOf(NodeSelectorModelItem* item)
{
    return static_cast<int>(mChildItems.indexOf(item));
}

QString NodeSelectorModelItem::getOwnerName() const
{
    if (mFullNameAttribute && mFullNameAttribute->isAttributeReady())
    {
        return mFullNameAttribute->getFullName();
    }

    return mOwnerEmail;
}

QString NodeSelectorModelItem::getOwnerEmail() const
{
    return mOwnerEmail;
}

void NodeSelectorModelItem::setOwner(std::unique_ptr<mega::MegaUser> user)
{
    if (!user)
    {
        return;
    }

    mOwner = std::move(user);
    mOwnerEmail = QString::fromUtf8(mOwner->getEmail());
    mFullNameAttribute = UserAttributes::FullName::requestFullName(mOwner->getEmail());
    if (mFullNameAttribute)
    {
        connect(mFullNameAttribute.get(),
                &UserAttributes::FullName::fullNameReady,
                this,
                &NodeSelectorModelItem::onFullNameAttributeReady);
        if (mFullNameAttribute->isAttributeReady())
        {
            onFullNameAttributeReady();
        }
    }
    mAvatarAttribute = UserAttributes::Avatar::requestAvatar(mOwner->getEmail());
    if (mAvatarAttribute)
    {
        connect(mAvatarAttribute.get(),
                &UserAttributes::Avatar::attributeReady,
                this,
                &NodeSelectorModelItem::onAvatarAttributeReady);
        if (mAvatarAttribute->isAttributeReady())
        {
            onAvatarAttributeReady();
        }
    }
}

void NodeSelectorModelItem::onFullNameAttributeReady()
{
    emit infoUpdated(Qt::DisplayRole);
}

void NodeSelectorModelItem::onAvatarAttributeReady()
{
    emit infoUpdated(Qt::DecorationRole);
}

void NodeSelectorModelItem::onChildDestroyed()
{
    mChildrenCounter--;
}

QPixmap NodeSelectorModelItem::getOwnerIcon()
{
    if (mAvatarAttribute)
    {
        return mAvatarAttribute->getPixmap(ICON_SIZE);
    }

    return QPixmap();
}

QIcon NodeSelectorModelItem::getStatusIcons()
{
    QIcon statusIcons; // first is selected state icon / second is normal state icon

    if (mNode && (mNode->isTakenDown() || !mNode->isNodeKeyDecrypted()))
    {
        statusIcons.addFile(QLatin1String("://images/node_selector/alert-circle-hover.png"),
                            QSize(),
                            QIcon::Selected); // selected style icon
        statusIcons.addFile(QLatin1String("://images/node_selector/alert-circle-default.png"),
                            QSize(),
                            QIcon::Normal); // normal style icon
    }
    else
    {
        switch (mStatus)
        {
            case Status::SYNC:
            {
                statusIcons.addFile(QLatin1String("://images/Item-sync-press.png"),
                                    QSize(),
                                    QIcon::Selected); // selected style icon
                statusIcons.addFile(QLatin1String("://images/Item-sync-rest.png"),
                                    QSize(),
                                    QIcon::Normal); // normal style icon
                break;
            }
            case Status::SYNC_PARENT:
            {
                statusIcons.addFile(QLatin1String("://images/Item-sync-press.png"),
                                    QSize(),
                                    QIcon::Selected); // selected style icon
                statusIcons.addFile(
                    QLatin1String("://images/node_selector/icon-small-sync-disabled.png"),
                    QSize(),
                    QIcon::Normal); // normal style icon
                break;
            }
            default:
            {
                break;
            }
        }
    }

    return statusIcons;
}

NodeSelectorModelItem::Status NodeSelectorModelItem::getStatus() const
{
    return mStatus;
}

bool NodeSelectorModelItem::isSyncable()
{
    return !isTakenDown() && !isInRubbishBin() && mStatus != Status::SYNC &&
           mStatus != Status::SYNC_PARENT && mStatus != Status::SYNC_CHILD &&
           mStatus != Status::BACKUP && getNodeAccess() >= mega::MegaShare::ACCESS_FULL;
}

QList<QPointer<NodeSelectorModelItem>>
    NodeSelectorModelItem::buildNodes(const QList<std::shared_ptr<MegaNode>>& nodes)
{
    QList<QPointer<NodeSelectorModelItem>> items;
    foreach(const auto& node, nodes)
    {
        auto child = createModelItem(std::unique_ptr<MegaNode>(node->copy()), mShowFiles, this);
        if (child->isValid())
        {
            items.append(child);
        }
        else
        {
            child->deleteLater();
        }
    }

    return items;
}

void NodeSelectorModelItem::appendNodes(const QList<QPointer<NodeSelectorModelItem>>& items)
{
    for (const auto& item: items)
    {
        if (item)
        {
            connect(item,
                    &NodeSelectorModelItem::destroyed,
                    this,
                    &NodeSelectorModelItem::onChildDestroyed);
        }
    }
    mChildItems.append(items);
    mChildrenCounter += items.size();
}

QList<QPointer<NodeSelectorModelItem>>
    NodeSelectorModelItem::addNodes(QList<std::shared_ptr<MegaNode>> nodes)
{
    auto items = buildNodes(nodes);
    initializeChildItems(items);
    return items;
}

QPointer<NodeSelectorModelItem> NodeSelectorModelItem::findChildNode(std::shared_ptr<MegaNode> node)
{
    NodeSelectorModelItem* returnNode(nullptr);

    if (node)
    {
        for (int i = 0; i < mChildItems.size(); i++)
        {
            if (mChildItems[i]->getNode()->getHandle() == node->getHandle())
            {
                returnNode = mChildItems.takeAt(i);
                break;
            }
        }
    }

    return returnNode;
}

void NodeSelectorModelItem::displayFiles(bool enable)
{
    mShowFiles = enable;
}

int NodeSelectorModelItem::row()
{
    if (NodeSelectorModelItem* parent = getParent())
    {
        return static_cast<int>(
            parent->mChildItems.indexOf(const_cast<NodeSelectorModelItem*>(this)));
    }
    return 0;
}

void NodeSelectorModelItem::updateNode(std::shared_ptr<mega::MegaNode> node)
{
    mNode = node;
    // Re-resolve the access level only when the update actually flags a share change:
    // this runs on the GUI thread (rootNodeUpdated / update coordinator), so the blocking
    // getAccess call must stay out of the common update storms (renames, attribute
    // changes). A genuine permission change is rare and single-node, so the bounded
    // SDK call is acceptable here.
    if (mNode->isInShare() && (mNode->getChanges() & mega::MegaNode::CHANGE_TYPE_INSHARE))
    {
        primeNodeAccess();
        // Descendants hold a cached copy inherited at construction: keep them in sync.
        propagateNodeAccessToChildren();
    }
}

void NodeSelectorModelItem::propagateNodeAccessToChildren()
{
    for (const auto& child: mChildItems)
    {
        if (child)
        {
            child->mNodeAccess = mNodeAccess.load();
            child->propagateNodeAccessToChildren();
        }
    }
}

void NodeSelectorModelItem::calculateSyncStatus()
{
    if (mNode->isFile())
    {
        return;
    }

    mStatus = Status::NONE;

    if (isBackupFolder())
    {
        mStatus = Status::BACKUP;
        return;
    }

    // if current item has a parent and the parent is already a sync or a sync_child, current
    // item is also a sync_child if not, continue checking. This avoid to block the mutex in the
    // megaapi call below.
    if (parent())
    {
        if (auto parent_item = qobject_cast<NodeSelectorModelItem*>(parent()))
        {
            switch (parent_item->getStatus())
            {
                case Status::SYNC:
                case Status::SYNC_CHILD:
                {
                    mStatus = Status::SYNC_CHILD;
                    return;
                }
                default:
                {
                    break;
                }
            }
        }
    }

    if (mStatus == Status::NONE)
    {
        std::unique_ptr<MegaError> err(
            MegaSyncApp->getMegaApi()->isNodeSyncableWithError(mNode.get()));
        switch (err->getSyncError())
        {
            case mega::MegaSync::Error::ACTIVE_SYNC_ABOVE_PATH:
            {
                mStatus = Status::SYNC_CHILD;
                break;
            }
            case mega::MegaSync::Error::ACTIVE_SYNC_BELOW_PATH:
            {
                mStatus = Status::SYNC_PARENT;
                break;
            }
            case mega::MegaSync::Error::ACTIVE_SYNC_SAME_PATH:
            {
                mStatus = Status::SYNC;
                break;
            }
        }
    }
}

bool NodeSelectorModelItem::isCloudDrive() const
{
    auto rootNode(MegaSyncApp->getRootNode());
    return rootNode && mNode->getHandle() == rootNode->getHandle();
}

bool NodeSelectorModelItem::isRubbishBin() const
{
    return mNode->getHandle() == MegaSyncApp->getRubbishNode()->getHandle();
}

bool NodeSelectorModelItem::isInRubbishBin() const
{
    return mNode && mMegaApi->isInRubbish(mNode.get());
}

bool NodeSelectorModelItem::isMyBackupsFolder() const
{
    return false;
}

bool NodeSelectorModelItem::isDeviceFolder() const
{
    return false;
}

bool NodeSelectorModelItem::isFile() const
{
    return getNode() && getNode()->isFile();
}

bool NodeSelectorModelItem::isBackupFolder() const
{
    return false;
}

bool NodeSelectorModelItem::isInShare() const
{
    return mNode->isInShare();
}

bool NodeSelectorModelItem::isInVault() const
{
    return MegaSyncApp->getMegaApi()->isInVault(mNode.get());
}

bool NodeSelectorModelItem::isS4Container() const
{
    // Do not cache the container handle: the SDK docs state it can change at any
    // time (e.g. S4 being enabled/disabled from another client)
    return mNode && mMegaApi->isS4Enabled() && mNode->getHandle() == mMegaApi->getS4Container();
}

NodeSelectorModelItemSearch::NodeSelectorModelItemSearch(std::unique_ptr<mega::MegaNode> node,
                                                         TabTypes type,
                                                         NodeSelectorModelItem* parentItem):
    NodeSelectorModelItem(std::move(node), false, parentItem),
    mType(type)
{
    // Owner and access are only shown for inshare roots; nested nodes keep the
    // columns empty, so don't resolve the owner (avoids avatar/fullname requests).
    if ((mType & TabType::INCOMING_SHARE) && mNode->isInShare())
    {
        auto user = std::unique_ptr<mega::MegaUser>(
            MegaSyncApp->getMegaApi()->getUserFromInShare(mNode.get(), true));
        setOwner(std::move(user));
        primeNodeAccess();
    }

    calculateSyncStatus();
}

NodeSelectorModelItemSearch::~NodeSelectorModelItemSearch() {}

void NodeSelectorModelItemSearch::setType(TabTypes type)
{
    if (mType != type)
    {
        mType = type;
        // No access re-resolution here: this runs on the GUI thread (rootNodeUpdated,
        // CHANGE_TYPE_PARENT branch), and a type change comes from a parent move — an
        // inshare root can never become one through a move, so there is nothing to prime.
        // New inshare roots are created (and primed) on the NodeRequester worker.
        emit tabTypeChanged(type);
    }
}

int NodeSelectorModelItemSearch::getNumChildren()
{
    return static_cast<int>(mChildItems.size());
}

bool NodeSelectorModelItemSearch::isMyBackupsFolder() const
{
    if (!mType.testFlag(TabType::BACKUP) || !mNode)
    {
        return false;
    }

    // Identify by the actual node, not by tree position: the "My Backups" root is excluded
    // from search paths, so the topmost backup item is a device folder, which used to be
    // wrongly classified as the My Backups folder when relying on parent() == nullptr.
    auto backupsHandle =
        UserAttributes::MyBackupsHandle::requestMyBackupsHandle()->getMyBackupsHandle();
    return mNode->getHandle() == backupsHandle;
}

bool NodeSelectorModelItemSearch::isDeviceFolder() const
{
    if (!mType.testFlag(TabType::BACKUP) || !mNode)
    {
        return false;
    }

    // A device folder is the only backup node carrying a device id.
    return !QString::fromUtf8(mNode->getDeviceId()).isEmpty();
}

bool NodeSelectorModelItemSearch::isBackupFolder() const
{
    if (!mType.testFlag(TabType::BACKUP))
    {
        return false;
    }

    auto parentItem = getParent();
    return parentItem && parentItem->isDeviceFolder();
}

NodeSelectorModelItem*
    NodeSelectorModelItemSearch::createModelItem(std::unique_ptr<mega::MegaNode> node,
                                                 bool showFiles,
                                                 NodeSelectorModelItem* parentItem)
{
    Q_UNUSED(showFiles)
    return new NodeSelectorModelItemSearch(std::move(node), mType, parentItem);
}

NodeSelectorModelItemIncomingShare::NodeSelectorModelItemIncomingShare(
    std::unique_ptr<mega::MegaNode> node,
    bool showFiles,
    NodeSelectorModelItem* parentItem):
    NodeSelectorModelItem(std::move(node), showFiles, parentItem)
{
    // Only the inshare root resolves its access level through the SDK (this constructor runs
    // on the NodeRequester worker); nested nodes inherit the cached level from their parent
    // (see the base constructor).
    if (mNode->isInShare())
    {
        primeNodeAccess();
    }

    if (!parentItem)
    {
        auto user = std::unique_ptr<mega::MegaUser>(
            MegaSyncApp->getMegaApi()->getUserFromInShare(mNode.get()));
        setOwner(std::move(user));
    }
    calculateSyncStatus();
}

NodeSelectorModelItemIncomingShare::~NodeSelectorModelItemIncomingShare() {}

NodeSelectorModelItem*
    NodeSelectorModelItemIncomingShare::createModelItem(std::unique_ptr<mega::MegaNode> node,
                                                        bool showFiles,
                                                        NodeSelectorModelItem* parentItem)
{
    return new NodeSelectorModelItemIncomingShare(std::move(node), showFiles, parentItem);
}

NodeSelectorModelItemBackup::NodeSelectorModelItemBackup(std::unique_ptr<mega::MegaNode> node,
                                                         bool showFiles,
                                                         NodeSelectorModelItem* parentItem):
    NodeSelectorModelItem(std::move(node), showFiles, parentItem)
{
    mStatus = Status::BACKUP;
}

NodeSelectorModelItemBackup::~NodeSelectorModelItemBackup() {}

bool NodeSelectorModelItemBackup::isSyncable()
{
    return false;
}

bool NodeSelectorModelItemBackup::isMyBackupsFolder() const
{
    if (!mNode)
    {
        return false;
    }

    // Identify by the actual node, not by tree position, so the classification stays correct
    // regardless of where the item sits in the tree.
    auto backupsHandle =
        UserAttributes::MyBackupsHandle::requestMyBackupsHandle()->getMyBackupsHandle();
    return mNode->getHandle() == backupsHandle;
}

bool NodeSelectorModelItemBackup::isDeviceFolder() const
{
    if (!mNode)
    {
        return false;
    }

    // A device folder is the only backup node carrying a device id.
    return !QString::fromUtf8(mNode->getDeviceId()).isEmpty();
}

bool NodeSelectorModelItemBackup::isBackupFolder() const
{
    auto parentItem = getParent();
    return parentItem && parentItem->isDeviceFolder();
}

NodeSelectorModelItem*
    NodeSelectorModelItemBackup::createModelItem(std::unique_ptr<mega::MegaNode> node,
                                                 bool showFiles,
                                                 NodeSelectorModelItem* parentItem)
{
    return new NodeSelectorModelItemBackup(std::move(node), showFiles, parentItem);
}

NodeSelectorModelItemCloudDrive::NodeSelectorModelItemCloudDrive(
    std::unique_ptr<mega::MegaNode> node,
    bool showFiles,
    NodeSelectorModelItem* parentItem):
    NodeSelectorModelItem(std::move(node), showFiles, parentItem)
{
    calculateSyncStatus();
}

NodeSelectorModelItemCloudDrive::~NodeSelectorModelItemCloudDrive() {}

NodeSelectorModelItem*
    NodeSelectorModelItemCloudDrive::createModelItem(std::unique_ptr<mega::MegaNode> node,
                                                     bool showFiles,
                                                     NodeSelectorModelItem* parentItem)
{
    return new NodeSelectorModelItemCloudDrive(std::move(node), showFiles, parentItem);
}

////////////////
NodeSelectorModelItemRubbish::NodeSelectorModelItemRubbish(std::unique_ptr<mega::MegaNode> node,
                                                           bool showFiles,
                                                           NodeSelectorModelItem* parentItem):
    NodeSelectorModelItem(std::move(node), showFiles, parentItem)
{}

NodeSelectorModelItemRubbish::~NodeSelectorModelItemRubbish() {}

NodeSelectorModelItem*
    NodeSelectorModelItemRubbish::createModelItem(std::unique_ptr<mega::MegaNode> node,
                                                  bool showFiles,
                                                  NodeSelectorModelItem* parentItem)
{
    return new NodeSelectorModelItemRubbish(std::move(node), showFiles, parentItem);
}
