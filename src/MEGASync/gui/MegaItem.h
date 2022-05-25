#ifndef MEGAITEM_H
#define MEGAITEM_H

#include "QTMegaRequestListener.h"

#include <QList>
#include <QIcon>

#include <memory>

class MegaItem : public QObject, public mega::MegaRequestListener
{
    Q_OBJECT
    static const int ICON_SIZE;
public:

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
    bool isVault();
    int insertPosition(const std::unique_ptr<mega::MegaNode> &node);
    void insertNode(std::unique_ptr<mega::MegaNode> node, int index);
    void removeNode(std::shared_ptr<mega::MegaNode> node);
    void displayFiles(bool enable);
    void setCameraFolder();
    void setChatFilesFolder();
    int row();

    ~MegaItem();

public slots:
    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e) override;

signals:
    void infoUpdated(int role);

protected:
    bool mShowFiles;
    QString mOwnerFirstName;
    QString mOwnerLastName;
    QString mOwnerEmail;
    QPixmap mOwnerIcon;
    int mStatus;
    bool mCameraFolder;
    bool mChatFilesFolder;
    bool mChildrenSet;

    std::shared_ptr<mega::MegaNode> mNode;
    QList<MegaItem*> mChildItems;
    std::unique_ptr<mega::MegaUser> mOwner;

private:
    void calculateSyncStatus(const QStringList& folders);
    mega::MegaApi* mMegaApi;
    std::unique_ptr<mega::QTMegaRequestListener> mDelegateListener;

};

#endif // MEGAITEM_H
