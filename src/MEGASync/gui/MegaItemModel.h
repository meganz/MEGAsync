#ifndef MEGAITEMMODEL_H
#define MEGAITEMMODEL_H

#include "MegaItem.h"
#include <megaapi.h>
#include "Utilities.h"

#include <QAbstractItemModel>
#include <QList>
#include <QIcon>
#include <QEventLoop>

#include <memory>

namespace UserAttributes{
class CameraUploadFolder;
}



enum class MegaItemModelRoles
{
    DATE_ROLE = Qt::UserRole,
    IS_FILE_ROLE,
    STATUS_ROLE,
    last
};

enum class NodeRowDelegateRoles
{
    ENABLED_ROLE = toInt(MegaItemModelRoles::last),  //ALWAYS use last enum value from previous enum class for new enums
    INDENT_ROLE,
    INIT_ROLE,
    last
};

class MegaItemModel;

class NodeRequester : public QObject
{
    Q_OBJECT


public:
    NodeRequester(MegaItemModel* model);
    void setShowFiles(bool newShowFiles);
    void lockMutex(bool state) const;
    const std::atomic<bool>& isWorking() const;
    int rootIndexSize() const;
    int rootIndexOf(MegaItem *item);
    MegaItem* getRootItem(int index) const;

public slots:
    void requestNodeAndCreateChildren(MegaItem* item, const QModelIndex& parentIndex, bool showFiles, std::shared_ptr<mega::MegaCancelToken> calcelToken);

    void createCloudDriveRootItem();
    void createIncomingSharesRootItems(std::shared_ptr<mega::MegaNodeList> nodeList);
    void onAddNodeRequested(std::shared_ptr<mega::MegaNode> newNode, MegaItem* parentItem);
    void removeItem(MegaItem *item);
    void removeRootItem(MegaItem* item);
    void abort();

signals:
     void nodesReady(MegaItem* parent);
     void megaCloudDriveRootItemCreated(MegaItem* item);
     void megaIncomingSharesRootItemsCreated(QList<MegaItem*> item);
     void nodeAdded(MegaItem* item);

private:
     void finishWorker();

     bool mShowFiles = true;
     std::atomic<bool> mAborted{false};
     MegaItemModel* mModel;
     QList<MegaItem*> mRootItems;
     mutable QMutex mMutex;
};

class MegaItemModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    static const int ROW_HEIGHT;

    enum COLUMN{
      NODE = 0,
      STATUS,
      USER,
      DATE,
      last
    };

    explicit MegaItemModel(QObject *parent = 0);

    int columnCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex & index) const override;
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                                    int role = Qt::DisplayRole) const override;
    bool canFetchMore(const QModelIndex &parent) const override;

    void setDisableFolders(bool option);
    void setSyncSetupMode(bool value);
    void addNode(std::shared_ptr<mega::MegaNode> node, const QModelIndex &parent);
    void removeNode(const QModelIndex &item);
    void showFiles(bool show);
    int countTotalRows(const QModelIndex &index = QModelIndex());

    std::shared_ptr<mega::MegaNode> getNode(const QModelIndex &index) const;
    QVariant getIcon(const QModelIndex &index, MegaItem* item) const;
    QVariant getText(const QModelIndex &index, MegaItem* item) const;
    void setFetchStep(int step);
    virtual ~MegaItemModel();
    void endInsertingRows(){endInsertRows();}
    void beginInsertingRows(const QModelIndex& index, int rowCount){beginInsertRows(index, 0 , rowCount-1);}

    void loadTreeFromNode(const std::shared_ptr<mega::MegaNode> node);
    QModelIndex getIndexFromNode(const std::shared_ptr<mega::MegaNode> node, const QModelIndex& parent);

    virtual void firstLoad() = 0;
    void rootItemsLoaded();

    QPair<QModelIndexList, bool> needsToBeExpandedAndSelected();
    void clearIndexesToMap();
    void abort();

signals:
    void levelsAdded(const QModelIndexList& parent);
    void requestChildNodes(MegaItem* parent, const QModelIndex& parentIndex,
                           int nodeType, std::shared_ptr<mega::MegaCancelToken> cancelToken) const;
    void firstLoadFinished(const QModelIndex& parent);
    void requestAddNode(std::shared_ptr<mega::MegaNode> newNode, MegaItem* parent);
    void removeItem(MegaItem* items);
    void removeRootItem(MegaItem* items);
    void deleteWorker();

    void blockUi(bool state) const;

protected:
    QModelIndex findItemByNodeHandle(const mega::MegaHandle &handle, const QModelIndex& parent);
    void fetchItemChildren(const QModelIndex& parent);
    void addRootItems();
    virtual void loadLevelFinished();
    void continueWithNextItemToLoad(const QModelIndex &parentIndex);

    int mRequiredRights;
    bool mDisplayFiles;
    bool mSyncSetupMode;
    bool mShowFiles;
    mutable QModelIndexList mIndexesToMap;
    bool mNeedsToBeSelected;
    NodeRequester* mNodeRequesterWorker;
    QList<std::shared_ptr<mega::MegaNode>> mNodesToLoad;

private slots:
    void onChildNodesReady(MegaItem *parent);
    void onNodeAdded(MegaItem* childItem);

private:
    int insertPosition(const std::unique_ptr<mega::MegaNode>& node);
    virtual void createRootNodes() = 0;
    virtual int rootItemsCount() const = 0;
    void createChildItems(std::shared_ptr<mega::MegaNodeList> childNodes, const QModelIndex& index, MegaItem* parent);
    QIcon getFolderIcon(MegaItem* item) const;
    bool fetchMoreRecursively(const QModelIndex& parentIndex);
    std::shared_ptr<mega::MegaCancelToken> mCancelToken;

    std::shared_ptr<const UserAttributes::CameraUploadFolder> mCameraFolderAttribute;
    std::shared_ptr<const UserAttributes::MyChatFilesFolder> mMyChatFilesFolderAttribute;

    QThread* mNodeRequesterThread;
};

class MegaItemModelCloudDrive : public MegaItemModel , public mega::MegaRequestListener
{
    Q_OBJECT

public:
    explicit MegaItemModelCloudDrive(QObject *parent = 0);
    virtual ~MegaItemModelCloudDrive();

    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e) override;
    void createRootNodes() override;
    int rootItemsCount() const override;

    void fetchMore(const QModelIndex &parent) override;
    void firstLoad() override;

signals:
    void requestCloudDriveRootCreation();

private slots:
    void onRootItemCreated(MegaItem*item);

private:
    std::unique_ptr<mega::QTMegaRequestListener> mDelegateListener;
    bool mLoadingRoot;
};

class MegaItemModelIncomingShares : public MegaItemModel
{
    Q_OBJECT

public:
    explicit MegaItemModelIncomingShares(QObject *parent = 0);
    virtual ~MegaItemModelIncomingShares();

    void createRootNodes() override;
    int rootItemsCount() const override;

    void fetchMore(const QModelIndex &parent) override;
    void firstLoad() override;

public slots:
    void onItemInfoUpdated(int role);

signals:
    void requestIncomingSharesRootCreation(std::shared_ptr<mega::MegaNodeList> nodes, MegaItemModelIncomingShares* model);

private slots:
    void onRootItemsCreated(QList<MegaItem *> item);

private:
    std::shared_ptr<mega::MegaNodeList> mSharedNodeList;
};

#endif // MEGAITEMMODEL_H
