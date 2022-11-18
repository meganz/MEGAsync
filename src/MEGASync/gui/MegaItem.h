#ifndef MEGAITEM_H
#define MEGAITEM_H

#include <QList>
#include <QIcon>

#include "megaapi.h"

#include <memory>

namespace UserAttributes{
class FullName;
class Avatar;
}

class MegaItem : public QObject
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

    explicit MegaItem(std::unique_ptr<mega::MegaNode> node, bool showFiles, MegaItem *parentItem = 0);
    ~MegaItem();

    std::shared_ptr<mega::MegaNode> getNode() const;

    void createChildItems(std::unique_ptr<mega::MegaNodeList> nodeList);
    bool childrenAreInit();

    bool canFetchMore();

    MegaItem *getParent();
    MegaItem *getChild(int i);
    int getNumItemChildren();
    int getNumChildren();
    int indexOf(MegaItem *item);
    QString getOwnerName();
    QString getOwnerEmail();
    void setOwner(std::unique_ptr<mega::MegaUser> user);
    QPixmap getOwnerIcon();
    QIcon getStatusIcons();
    int getStatus();
    bool isSyncable();
    bool isRoot();
    MegaItem* addNode(std::shared_ptr<mega::MegaNode> node);
    MegaItem* removeNode(std::shared_ptr<mega::MegaNode> node);
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
    QList<MegaItem*> mChildItems;
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

#endif // MEGAITEM_H
