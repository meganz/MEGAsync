#ifndef MODELSELECTORMODELITEM_H
#define MODELSELECTORMODELITEM_H

#include "megaapi.h"

#include <QIcon>
#include <QList>

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
    bool canBeRenamed() const;

    void createChildItems(std::unique_ptr<mega::MegaNodeList> nodeList);
    bool areChildrenInitialized() const;

    bool canFetchMore();

    QPointer<NodeSelectorModelItem> getParent();
    QPointer<NodeSelectorModelItem> getChild(int i);
    virtual int getNumChildren();
    int indexOf(NodeSelectorModelItem* item);
    QString getOwnerName();
    QString getOwnerEmail();
    void setOwner(std::unique_ptr<mega::MegaUser> user);
    QPixmap getOwnerIcon();
    QIcon getStatusIcons();
    Status getStatus() const;
    virtual bool isSyncable();
    virtual bool isVault() const;
    virtual bool isVaultDevice() const;
    bool isInShare() const;
    bool isInVault() const;
    bool isCloudDrive() const;
    bool isRubbishBin() const;
    bool isInRubbishBin() const;
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
    void updateLoadingMessage(std::shared_ptr<MessageInfo> message);

protected:
    QString mOwnerEmail;
    Status mStatus;
    bool mRequestingChildren;
    int mChildrenCounter;
    bool mShowFiles;
    bool mChildrenAreInit;
    mutable int mNodeAccess;
    mutable qint64 mNodeAccessLastUpdate;

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
    bool isVault() const override;
    bool isVaultDevice() const override;

private:
    NodeSelectorModelItem* createModelItem(std::unique_ptr<mega::MegaNode> node,
                                           bool showFiles,
                                           NodeSelectorModelItem* parentItem = 0) override;
};

class NodeSelectorModelItemSearch: public NodeSelectorModelItem
{
    Q_OBJECT

public:
    enum class Type
    {
        NONE = 0x0,
        BACKUP = 0x01,
        INCOMING_SHARE = 0x02,
        CLOUD_DRIVE = 0x04,
        RUBBISH = 0x08,
    };
    Q_DECLARE_FLAGS(Types, Type);

    explicit NodeSelectorModelItemSearch(std::unique_ptr<mega::MegaNode> node,
                                         Types type,
                                         NodeSelectorModelItem* parentItem = 0);
    ~NodeSelectorModelItemSearch();

    Types getType()
    {
        return mType;
    }

    void setType(Types type);
    int getNumChildren() override;

signals:
    void typeChanged(Types type);

private:
    NodeSelectorModelItem* createModelItem(std::unique_ptr<mega::MegaNode> node,
                                           bool showFiles,
                                           NodeSelectorModelItem* parentItem = 0) override;
    Types mType;
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

Q_DECLARE_OPERATORS_FOR_FLAGS(NodeSelectorModelItemSearch::Types)
Q_DECLARE_METATYPE(NodeSelectorModelItemSearch::Types)

#endif // MODELSELECTORMODELITEM_H
