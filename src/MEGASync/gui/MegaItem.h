#ifndef MEGAITEM_H
#define MEGAITEM_H

#include "QTMegaRequestListener.h"

#include <QList>
#include <QIcon>

#include <memory>

namespace UserAttributes{
class FullName;
class Avatar;
class CameraUploadFolder;
class MyChatFilesFolder;
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

    explicit MegaItem(std::unique_ptr<mega::MegaNode> node, MegaItem *parentItem = 0, bool showFiles = false);

    std::shared_ptr<mega::MegaNode> getNode();
    void setChildren(std::shared_ptr<mega::MegaNodeList> children);

    bool areChildrenSet();
    MegaItem *getParent();
    MegaItem *getChild(int i);
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
    bool isVault();
    void addNode(std::unique_ptr<mega::MegaNode> node);
    void removeNode(std::shared_ptr<mega::MegaNode> node);
    void displayFiles(bool enable);
    void setChatFilesFolder();
    void setAsVaultNode();
    int row();

    ~MegaItem();

signals:
    void infoUpdated(int role);

protected:
    bool mShowFiles;
    QString mOwnerEmail;
    int mStatus;
    bool mChildrenSet;
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
    std::unique_ptr<mega::QTMegaRequestListener> mDelegateListener;
    std::shared_ptr<const UserAttributes::FullName> mFullNameAttribute;
    std::shared_ptr<const UserAttributes::Avatar> mAvatarAttribute;
};

#endif // MEGAITEM_H
