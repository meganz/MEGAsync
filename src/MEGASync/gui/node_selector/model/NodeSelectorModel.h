#ifndef NODESELECTORMODEL_H
#define NODESELECTORMODEL_H

#include "NodeSelectorModelItem.h"
#include "Utilities.h"
#include <megaapi.h>

#include <QAbstractItemModel>
#include <QList>
#include <QIcon>
#include <QPointer>

#include <memory>

namespace UserAttributes{
class CameraUploadFolder;
class MyChatFilesFolder;
}

enum class NodeSelectorModelRoles
{
    DATE_ROLE = Qt::UserRole,
    IS_FILE_ROLE,
    STATUS_ROLE,
    last
};

enum class NodeRowDelegateRoles
{
    ENABLED_ROLE = toInt(NodeSelectorModelRoles::last),  //ALWAYS use last enum value from previous enum class for new enums
    INDENT_ROLE,
    SMALL_ICON_ROLE,
    INIT_ROLE,
    last
};

enum class HeaderRoles
{
    ICON_ROLE = toInt(NodeRowDelegateRoles::last),  //ALWAYS use last enum value from previous enum class for new enums
    last
};

class NodeSelectorModel;

class NodeRequester : public QObject
{
    Q_OBJECT


public:
    NodeRequester(NodeSelectorModel* model);
    void setShowFiles(bool newShowFiles);
    void setSyncSetupMode(bool value);
    void lockMutex(bool state) const;
    const std::atomic<bool>& isWorking() const;
    int rootIndexSize() const;
    int rootIndexOf(NodeSelectorModelItem *item);
    NodeSelectorModelItem* getRootItem(int index) const;

    void cancelCurrentRequest();
    void restartSearch();

public slots:
    void requestNodeAndCreateChildren(NodeSelectorModelItem* item, const QModelIndex& parentIndex, bool showFiles);
    void search(const QString& text);
    void createCloudDriveRootItem();
    void createIncomingSharesRootItems(std::shared_ptr<mega::MegaNodeList> nodeList);
    void createBackupRootItems(mega::MegaHandle backupsHandle);

    void onAddNodeRequested(std::shared_ptr<mega::MegaNode> newNode, const QModelIndex& parentIndex, NodeSelectorModelItem* parentItem);
    void removeItem(NodeSelectorModelItem *item);
    void removeRootItem(NodeSelectorModelItem* item);
    void abort();

signals:
     void nodesReady(NodeSelectorModelItem* parent);
     void megaCloudDriveRootItemCreated(NodeSelectorModelItem* item);
     void megaIncomingSharesRootItemsCreated(QList<NodeSelectorModelItem*> item);
     void megaBackupRootItemsCreated(NodeSelectorModelItem* item);
     void searchItemsCreated(QList<NodeSelectorModelItem*> item);
     void nodeAdded(NodeSelectorModelItem* item);

private:
     void finishWorker();
     bool isAborted();

     std::atomic<bool> mShowFiles{true};
     std::atomic<bool> mAborted{false};
     std::atomic<bool> mSearchCanceled{false};
     std::atomic<bool> mSyncSetupMode{false};
     NodeSelectorModel* mModel;
     QList<NodeSelectorModelItem*> mRootItems;
     mutable QMutex mMutex;
     std::shared_ptr<mega::MegaCancelToken> mCancelToken;
};

class NodeSelectorModel : public QAbstractItemModel
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

    struct IndexesActionInfo
    {
        bool needsToBeSelected = false;
        QModelIndexList indexesToBeExpanded;
        bool needsToBeEntered = false;
    };

    explicit NodeSelectorModel(QObject *parent = 0);
    virtual ~NodeSelectorModel();

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
    void removeNode(const QModelIndex &index);
    void showFiles(bool show);

    QVariant getIcon(const QModelIndex &index, NodeSelectorModelItem* item) const;
    QVariant getText(const QModelIndex &index, NodeSelectorModelItem* item) const;
    void setFetchStep(int step);
    void endInsertingRows(){endInsertRows();}
    void beginInsertingRows(const QModelIndex& index, int rowCount){beginInsertRows(index, 0 , rowCount-1);}

    void loadTreeFromNode(const std::shared_ptr<mega::MegaNode> node);
    QModelIndex getIndexFromNode(const std::shared_ptr<mega::MegaNode> node, const QModelIndex& parent);

    virtual void firstLoad() = 0;
    void rootItemsLoaded();

    IndexesActionInfo needsToBeExpandedAndSelected();
    void clearIndexesNodeInfo();
    void abort();

    virtual bool canBeDeleted() const;

signals:
    void levelsAdded(const QModelIndexList& parent, bool force = false);
    void requestChildNodes(NodeSelectorModelItem* parent, const QModelIndex& parentIndex,
                           int nodeType);
    void firstLoadFinished(const QModelIndex& parent);
    void requestAddNode(std::shared_ptr<mega::MegaNode> newNode, const QModelIndex& parentIndex, NodeSelectorModelItem* parent);
    void removeItem(NodeSelectorModelItem* items);
    void removeRootItem(NodeSelectorModelItem* items);
    void deleteWorker();
    void blockUi(bool state);

protected:
    QModelIndex findItemByNodeHandle(const mega::MegaHandle &handle, const QModelIndex& parent);
    void fetchItemChildren(const QModelIndex& parent);
    void addRootItems();
    virtual void loadLevelFinished();
    bool continueWithNextItemToLoad(const QModelIndex &parentIndex);

    int mRequiredRights;
    bool mDisplayFiles;
    bool mSyncSetupMode;
    bool mShowFiles;
    mutable IndexesActionInfo mIndexesActionInfo;
    NodeRequester* mNodeRequesterWorker;
    QList<std::shared_ptr<mega::MegaNode>> mNodesToLoad;

private slots:
    void onChildNodesReady(NodeSelectorModelItem *parent);
    void onNodeAdded(NodeSelectorModelItem* childItem);

private:
    virtual void createRootNodes() = 0;
    virtual int rootItemsCount() const = 0;
    virtual bool addToLoadingList(const std::shared_ptr<mega::MegaNode> node);
    virtual void continueLoading(NodeSelectorModelItem* item);
    void createChildItems(std::shared_ptr<mega::MegaNodeList> childNodes, const QModelIndex& index, NodeSelectorModelItem* parent);

    QIcon getFolderIcon(NodeSelectorModelItem* item) const;
    bool fetchMoreRecursively(const QModelIndex& parentIndex);


    std::shared_ptr<const UserAttributes::CameraUploadFolder> mCameraFolderAttribute;
    std::shared_ptr<const UserAttributes::MyChatFilesFolder> mMyChatFilesFolderAttribute;

    QThread* mNodeRequesterThread;
};

Q_DECLARE_METATYPE(std::shared_ptr<mega::MegaNodeList>)
Q_DECLARE_METATYPE(std::shared_ptr<mega::MegaNode>)

#endif // NODESELECTORMODEL_H
