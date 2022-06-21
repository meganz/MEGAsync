#ifndef MEGAITEM_H
#define MEGAITEM_H

#include "QTMegaRequestListener.h"

#include <QList>
#include <QIcon>

#include <memory>

namespace UserAttributes{
class FullNameAttributeRequest;
class AvatarAttributeRequest;
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
    QIcon getFolderIcon();
    int getStatus();
    bool isSyncable();
    bool isRoot();
    int insertPosition(const std::unique_ptr<mega::MegaNode> &node);
    void insertNode(std::unique_ptr<mega::MegaNode> node, int index);
    void removeNode(std::shared_ptr<mega::MegaNode> node);
    void displayFiles(bool enable);
    void setCameraFolder();
    void setChatFilesFolder();
    int row();

    ~MegaItem();

signals:
    void infoUpdated(int role);

protected:
    bool mShowFiles;
    QString mOwnerEmail;
    int mStatus;
    bool mCameraFolder;
    bool mChatFilesFolder;
    bool mChildrenSet;

    std::shared_ptr<mega::MegaNode> mNode;
    QList<MegaItem*> mChildItems;
    std::unique_ptr<mega::MegaUser> mOwner;

private slots:
    void onFullNameAttributeReady();
    void onAvatarAttributeReady();

private:
    void calculateSyncStatus(const QStringList& folders);
    std::unique_ptr<mega::QTMegaRequestListener> mDelegateListener;
    std::shared_ptr<const UserAttributes::FullNameAttributeRequest> mFullNameAttribute;
    std::shared_ptr<const UserAttributes::AvatarAttributeRequest> mAvatarAttribute;

};

#endif // MEGAITEM_H
