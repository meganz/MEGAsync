#ifndef MODELSELECTORMODELITEM_H
#define MODELSELECTORMODELITEM_H

#include <QList>
#include <QIcon>

#include "megaapi.h"

#include <memory>

namespace UserAttributes{
class FullName;
class Avatar;
}

class NodeSelectorModelItem : public QObject
{
    Q_OBJECT

public:
    static const int ICON_SIZE;

    enum STATUS{
        SYNC,
        SYNC_PARENT,
        SYNC_CHILD,
        BACKUP,
        NONE,
    };

    explicit NodeSelectorModelItem(std::unique_ptr<mega::MegaNode> node, bool showFiles, NodeSelectorModelItem *parentItem = 0);
    ~NodeSelectorModelItem();

    std::shared_ptr<mega::MegaNode> getNode() const;

    void createChildItems(std::unique_ptr<mega::MegaNodeList> nodeList);
    bool areChildrenInitialized();

    bool canFetchMore();

    QPointer<NodeSelectorModelItem> getParent();
    QPointer<NodeSelectorModelItem> getChild(int i);
    int getNumChildren();
    int indexOf(NodeSelectorModelItem *item);
    QString getOwnerName();
    QString getOwnerEmail();
    void setOwner(std::unique_ptr<mega::MegaUser> user);
    QPixmap getOwnerIcon();
    QIcon getStatusIcons();
    int getStatus();
    bool isSyncable();
    bool isCloudDrive();
    QPointer<NodeSelectorModelItem> addNode(std::shared_ptr<mega::MegaNode> node);
    QPointer<NodeSelectorModelItem> findChildNode(std::shared_ptr<mega::MegaNode> node);
    bool isVault();
    void displayFiles(bool enable);
    void setChatFilesFolder();
    void setAsVaultNode();
    int row();
    void updateNode(std::shared_ptr<mega::MegaNode> node);

    bool requestingChildren() const;
    void setRequestingChildren(bool newRequestingChildren);

signals:
    void infoUpdated(int role);

protected:
    QString mOwnerEmail;
    int mStatus;
    bool mChildrenSet;
    bool mRequestingChildren;
    long long mChildrenCounter;
    bool mShowFiles;
    bool mChildrenAreInit;
    bool mIsVault;

    std::shared_ptr<mega::MegaNode> mNode;
    QList<QPointer<NodeSelectorModelItem>> mChildItems;
    std::unique_ptr<mega::MegaUser> mOwner;

private slots:
    void onFullNameAttributeReady();
    void onAvatarAttributeReady();

private:
    void calculateSyncStatus(const QStringList& folders);
    mega::MegaApi* mMegaApi;
    std::shared_ptr<const UserAttributes::FullName> mFullNameAttribute;
    std::shared_ptr<const UserAttributes::Avatar> mAvatarAttribute;
};

#endif // MODELSELECTORMODELITEM_H
