#ifndef MODELSELECTORMODELITEM_H
#define MODELSELECTORMODELITEM_H

#include "megaapi.h"
#include "NodeSelectorTabTypes.h"

#include <QIcon>
#include <QList>
#include <QObject>
#include <QPointer>

#include <atomic>
#include <memory>

namespace UserAttributes
{
class FullName;
class Avatar;
}

struct MessageInfo;

class NodeSelectorModelItem: public QObject
{
    Q_OBJECT

public:
    static const int ICON_SIZE;

    enum class Status
    {
        SYNC = 0,
        SYNC_PARENT,
        SYNC_CHILD,
        BACKUP,
        NONE,
    };

    explicit NodeSelectorModelItem(std::unique_ptr<mega::MegaNode> node,
                                   bool showFiles,
                                   NodeSelectorModelItem* parentItem = 0);
    ~NodeSelectorModelItem();

    bool isValid() const;

    std::shared_ptr<mega::MegaNode> getNode() const;
    bool isSpecialNode() const;
    bool isTakenDown() const;
    bool canBeRenamed() const;

    QList<QPointer<NodeSelectorModelItem>>
        createChildItems(std::unique_ptr<mega::MegaNodeList> nodeList);
    void initializeChildItems(const QList<QPointer<NodeSelectorModelItem>>& items);
    bool areChildrenInitialized() const;

    bool canFetchMore();

    QPointer<NodeSelectorModelItem> getParent() const;
    QPointer<NodeSelectorModelItem> getChild(int i);
    virtual int getNumChildren();
    int indexOf(NodeSelectorModelItem* item);
    QString getOwnerName() const;
    QString getOwnerEmail() const;
    void setOwner(std::unique_ptr<mega::MegaUser> user);
    QPixmap getOwnerIcon();
    QIcon getStatusIcons();
    Status getStatus() const;
    virtual bool isSyncable();
    virtual bool isMyBackupsFolder() const;
    virtual bool isDeviceFolder() const;
    bool isFile() const;
    virtual bool isBackupFolder() const;
    bool isInShare() const;
    bool isInVault() const;
    bool isS4Container() const;
    bool isCloudDrive() const;
    bool isRubbishBin() const;
    bool isInRubbishBin() const;
    QList<QPointer<NodeSelectorModelItem>>
        buildNodes(const QList<std::shared_ptr<mega::MegaNode>>& nodes);
    void appendNodes(const QList<QPointer<NodeSelectorModelItem>>& items);
    QPointer<NodeSelectorModelItem> addNode(std::shared_ptr<mega::MegaNode> node);
    QList<QPointer<NodeSelectorModelItem>> addNodes(QList<std::shared_ptr<mega::MegaNode>> nodes);
    QPointer<NodeSelectorModelItem> findChildNode(std::shared_ptr<mega::MegaNode> node);
    void displayFiles(bool enable);
    void setChatFilesFolder();
    int row();
    void updateNode(std::shared_ptr<mega::MegaNode> node);
    void calculateSyncStatus();

    bool requestingChildren() const;
    void setRequestingChildren(bool newRequestingChildren);

    void resetChildrenCounter();

    int getNodeAccess() const;

signals:
    void infoUpdated(int role);

protected:
    // Resolves the access level through the SDK and caches it. Only inshare root nodes
    // (isInShare() == true) need it, and only from the NodeRequester worker thread: resolving
    // it from the GUI thread would block on the SDK mutex while a children fetch is in flight.
    // Nested items inherit the cached level from their parent instead (see the base
    // constructor); hot consumers (sync flags/tooltip) read the cache with no SDK call.
    void primeNodeAccess();
    // Copies this item's cached access level to all descendants (share access is uniform
    // across an inshare subtree). Called after re-priming an inshare root.
    void propagateNodeAccessToChildren();

    QString mOwnerEmail;
    Status mStatus;
    bool mRequestingChildren;
    int mChildrenCounter;
    bool mShowFiles;
    bool mChildrenAreInit;
    // Atomic: read from the proxy's concurrent sort/filter job (pool thread) while the GUI
    // thread may re-prime it on a share permission change (see updateNode).
    std::atomic<int> mNodeAccess;

    mega::MegaApi* mMegaApi;
    std::shared_ptr<mega::MegaNode> mNode;
    QList<QPointer<NodeSelectorModelItem>> mChildItems;
    std::unique_ptr<mega::MegaUser> mOwner;

private slots:
    void onFullNameAttributeReady();
    void onAvatarAttributeReady();
    void onChildDestroyed();

private:
    virtual NodeSelectorModelItem* createModelItem(std::unique_ptr<mega::MegaNode> node,
                                                   bool showFiles,
                                                   NodeSelectorModelItem* parentItem = 0) = 0;
    std::shared_ptr<const UserAttributes::FullName> mFullNameAttribute;
    std::shared_ptr<const UserAttributes::Avatar> mAvatarAttribute;
};

Q_DECLARE_METATYPE(NodeSelectorModelItem::Status)

class NodeSelectorModelItemCloudDrive: public NodeSelectorModelItem
{
public:
    explicit NodeSelectorModelItemCloudDrive(std::unique_ptr<mega::MegaNode> node,
                                             bool showFiles,
                                             NodeSelectorModelItem* parentItem = 0);
    ~NodeSelectorModelItemCloudDrive();

private:
    NodeSelectorModelItem* createModelItem(std::unique_ptr<mega::MegaNode> node,
                                           bool showFiles,
                                           NodeSelectorModelItem* parentItem = 0) override;
};

class NodeSelectorModelItemIncomingShare: public NodeSelectorModelItem
{
public:
    explicit NodeSelectorModelItemIncomingShare(std::unique_ptr<mega::MegaNode> node,
                                                bool showFiles,
                                                NodeSelectorModelItem* parentItem = 0);
    ~NodeSelectorModelItemIncomingShare();

private:
    NodeSelectorModelItem* createModelItem(std::unique_ptr<mega::MegaNode> node,
                                           bool showFiles,
                                           NodeSelectorModelItem* parentItem = 0) override;
};

class NodeSelectorModelItemBackup: public NodeSelectorModelItem
{
public:
    explicit NodeSelectorModelItemBackup(std::unique_ptr<mega::MegaNode> node,
                                         bool showFiles,
                                         NodeSelectorModelItem* parentItem = 0);
    ~NodeSelectorModelItemBackup();
    bool isSyncable() override;
    bool isMyBackupsFolder() const override;
    bool isDeviceFolder() const override;
    bool isBackupFolder() const override;

private:
    NodeSelectorModelItem* createModelItem(std::unique_ptr<mega::MegaNode> node,
                                           bool showFiles,
                                           NodeSelectorModelItem* parentItem = 0) override;
};

class NodeSelectorModelItemSearch: public NodeSelectorModelItem
{
    Q_OBJECT

public:
    explicit NodeSelectorModelItemSearch(std::unique_ptr<mega::MegaNode> node,
                                         TabTypes type,
                                         NodeSelectorModelItem* parentItem = 0);
    ~NodeSelectorModelItemSearch();

    TabTypes getType()
    {
        return mType;
    }

    void setType(TabTypes type);
    int getNumChildren() override;
    bool isMyBackupsFolder() const override;
    bool isDeviceFolder() const override;
    bool isBackupFolder() const override;

signals:
    void tabTypeChanged(TabTypes type);

private:
    NodeSelectorModelItem* createModelItem(std::unique_ptr<mega::MegaNode> node,
                                           bool showFiles,
                                           NodeSelectorModelItem* parentItem = 0) override;
    TabTypes mType;
};

class NodeSelectorModelItemRubbish: public NodeSelectorModelItem
{
public:
    explicit NodeSelectorModelItemRubbish(std::unique_ptr<mega::MegaNode> node,
                                          bool showFiles,
                                          NodeSelectorModelItem* parentItem = 0);
    ~NodeSelectorModelItemRubbish();

private:
    NodeSelectorModelItem* createModelItem(std::unique_ptr<mega::MegaNode> node,
                                           bool showFiles,
                                           NodeSelectorModelItem* parentItem = 0) override;
};

#endif // MODELSELECTORMODELITEM_H
