#ifndef NODESELECTORMODEL_H
#define NODESELECTORMODEL_H

#include "NodeSelectorModelItem.h"
#include "Utilities.h"
#include <MegaApplication.h>
#include "QMegaMessageBox.h"
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

class DuplicatedNodeInfo;

enum class NodeSelectorModelRoles
{
    DATE_ROLE = Qt::UserRole,
    IS_FILE_ROLE,
    IS_SYNCABLE_FOLDER_ROLE,
    STATUS_ROLE,
    HANDLE_ROLE,
    MODEL_ITEM_ROLE,
    NODE_ROLE,
    last
};

enum class NodeRowDelegateRoles
{
    INDENT_ROLE = toInt(NodeSelectorModelRoles::last),  //ALWAYS use last enum value from previous enum class for new enums
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
struct MessageInfo;

class NodeRequester : public QObject
{
    Q_OBJECT

public:
    NodeRequester(NodeSelectorModel* model);
    ~NodeRequester();

    void setShowFiles(bool show);
    void setShowReadOnlyFolders(bool show);
    void setSyncSetupMode(bool value);
    void lockDataMutex(bool state) const;
    bool isRequestingNodes() const;
    int rootIndexSize() const;
    int rootIndexOf(NodeSelectorModelItem *item);
    NodeSelectorModelItem* getRootItem(int index) const;

    bool trySearchLock() const;
    void lockSearchMutex(bool state) const;

    void cancelCurrentRequest();
    void restartSearch();

    const NodeSelectorModelItemSearch::Types &searchedTypes() const;

    bool showFiles() const;

    bool isIncomingShareCompatible(mega::MegaNode* node);

public slots:
    void requestNodeAndCreateChildren(NodeSelectorModelItem* item, const QModelIndex& parentIndex);
    void search(const QString& text, NodeSelectorModelItemSearch::Types typesAllowed);
    void createCloudDriveRootItem();
    void createIncomingSharesRootItems(std::shared_ptr<mega::MegaNodeList> nodeList);
    void createRubbishRootItems();
    void addIncomingSharesRootItem(std::shared_ptr<mega::MegaNode> node);
    void addSearchRootItem(QList<std::shared_ptr<mega::MegaNode> > nodes, NodeSelectorModelItemSearch::Types typesAllowed);
    void createBackupRootItems(mega::MegaHandle backupsHandle);

    void removeRootItem(NodeSelectorModelItem* item);
    void removeRootItem(std::shared_ptr<mega::MegaNode> node);

    void onAddNodesRequested(QList<std::shared_ptr<mega::MegaNode>> newNodes, const QModelIndex& parentIndex, NodeSelectorModelItem *parentItem);
    void removeItem(NodeSelectorModelItem *item);
    void abort();

signals:
     void nodesReady(NodeSelectorModelItem* parent);
     void megaCloudDriveRootItemCreated();
     void megaIncomingSharesRootItemsCreated();
     void megaRubbishRootItemsCreated();
     void rootItemsAdded();
     void rootItemsDeleted();
     void megaBackupRootItemsCreated();
     void searchItemsCreated();
     void nodeAdded(NodeSelectorModelItem* item);
     void nodesAdded(QList<QPointer<NodeSelectorModelItem>> item);
     void updateLoadingMessage(std::shared_ptr<MessageInfo> message);

private:
     bool isAborted();
    NodeSelectorModelItem* createSearchItem(mega::MegaNode* node, NodeSelectorModelItemSearch::Types typesAllowed);

     std::atomic<bool> mShowFiles{true};
     std::atomic<bool> mShowReadOnlyFolders{true};
     std::atomic<bool> mAborted{false};
     std::atomic<bool> mSearchCanceled{false};
     std::atomic<bool> mSyncSetupMode{false};
     std::atomic<bool> mNodesRequested{false};
     NodeSelectorModel* mModel;
     QList<NodeSelectorModelItem*> mRootItems;
     mutable QMutex mDataMutex;
     mutable QMutex mSearchMutex;
     std::shared_ptr<mega::MegaCancelToken> mCancelToken;
     NodeSelectorModelItemSearch::Types mSearchedTypes;
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

    enum class MovedItemsType
    {
        NONE = 0x0,
        FILES = 0x1,
        FOLDERS = 0x2,
        BOTH = FILES | FOLDERS
    };
    Q_DECLARE_FLAGS(MovedItemsTypes, MovedItemsType)

    struct IndexesActionInfo
    {
        bool needsToBeSelected = false;
        QList<QPair<mega::MegaHandle, QModelIndex>> indexesToBeExpanded;
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

    bool isRequestingNodes() const;

    void setDisableFolders(bool option);
    void setSyncSetupMode(bool value);

    virtual void addNodes(QList<std::shared_ptr<mega::MegaNode>> node, const QModelIndex &parent);
    void removeNodeFromModel(const QModelIndex &index);

    int getNodeAccess(mega::MegaNode* node);

    std::shared_ptr<mega::MegaNode> getNodeToRemove(mega::MegaHandle handle);
    void removeNodes(const QList<mega::MegaHandle>& nodeHandles, bool permanently);
    bool areAllNodesEligibleForDeletion(const QList<mega::MegaHandle>& handles);
    bool areAllNodesEligibleForRestore(const QList<mega::MegaHandle> &handles) const;

    void moveFolderAndMerge(std::shared_ptr<mega::MegaNode> moveFolder, std::shared_ptr<mega::MegaNode> conflictTargetFolder, std::shared_ptr<mega::MegaNode> targetParentFolder);
    void moveFileAndReplace(std::shared_ptr<mega::MegaNode> moveFile, std::shared_ptr<mega::MegaNode> conflictTargetFile, std::shared_ptr<mega::MegaNode> targetParentFolder);
    void moveNodeAndRename(std::shared_ptr<mega::MegaNode> moveNode, const QString& newName, std::shared_ptr<mega::MegaNode> targetParentFolder);
    void moveNode(std::shared_ptr<mega::MegaNode> moveNode, std::shared_ptr<mega::MegaNode> targetParentFolder);
    void moveNodesAfterConflictCheck(std::shared_ptr<ConflictTypes> conflicts);

    void showFiles(bool show);
    void showReadOnlyFolders(bool show);

    QVariant getIcon(const QModelIndex &index, NodeSelectorModelItem* item) const;
    QVariant getText(const QModelIndex &index, NodeSelectorModelItem* item) const;
    void setFetchStep(int step);

    void loadTreeFromNode(const std::shared_ptr<mega::MegaNode> node);
    QModelIndex getIndexFromNode(const std::shared_ptr<mega::MegaNode> node, const QModelIndex& parent);
    QModelIndex findItemByNodeHandle(const mega::MegaHandle &handle, const QModelIndex& parent);

    static NodeSelectorModelItem *getItemByIndex(const QModelIndex& index);
    void updateItemNode(const QModelIndex& indexToUpdate, std::shared_ptr<mega::MegaNode> node);
    void updateRow(const QModelIndex &indexToUpdate);

    virtual void firstLoad() = 0;
    void rootItemsLoaded();

    virtual void proxyInvalidateFinished(){}

    IndexesActionInfo needsToBeExpandedAndSelected();
    void clearIndexesNodeInfo(bool select = false);
    void abort();

    virtual bool canBeDeleted() const;
    virtual bool rootNodeUpdated(mega::MegaNode*){return false;}
    virtual bool isNodeAccepted(mega::MegaNode* node){return !MegaSyncApp->getMegaApi()->isInRubbish(node);}
    virtual bool showsSyncStates() {return false;}

    bool showFiles() const;

    bool isBeingModified() const {return mIsBeingModified;}
    void setIsModelBeingModified(bool state) {mIsBeingModified = state;}

    void setAcceptDragAndDrop(bool newAcceptDragAndDrop);
    bool acceptDragAndDrop(const QMimeData* data);

    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int, int, const QModelIndex &parent) override;
    virtual bool canDropMimeData(const QMimeData* data,
        Qt::DropAction action,
        int row,
        int column,
        const QModelIndex& parent) const override;

    void onRequestFinish(mega::MegaRequest* request, mega::MegaError* e);

signals:
    void levelsAdded(const QList<QPair<mega::MegaHandle, QModelIndex>>& parent, bool force = false);
    void requestChildNodes(NodeSelectorModelItem* parent, const QModelIndex& parentIndex);
    void firstLoadFinished(const QModelIndex& parent);
    void requestAddNodes(QList<std::shared_ptr<mega::MegaNode>> newNodes, const QModelIndex& parentIndex, NodeSelectorModelItem* parent);
    void removeItem(NodeSelectorModelItem* items);
    void removeRootItem(NodeSelectorModelItem* items);
    void deleteWorker();
    void blockUi(bool state);
    void forceFilter();
    void updateLoadingMessage(std::shared_ptr<MessageInfo> message);
    void showMessageBox(QMegaMessageBox::MessageBoxInfo info);
    void showDuplicatedNodeDialog(std::shared_ptr<ConflictTypes> conflicts);

protected:
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    Qt::DropActions supportedDropActions() const;
    QMimeData* mimeData(const QModelIndexList& indexes) const;

    QStringList mimeTypes() const;

    void fetchItemChildren(const QModelIndex& parent);
    void addRootItems();
    virtual void loadLevelFinished();
    bool continueWithNextItemToLoad(const QModelIndex &parentIndex);

    int mRequiredRights;
    bool mDisplayFiles;
    bool mSyncSetupMode;
    mutable IndexesActionInfo mIndexesActionInfo;
    NodeRequester* mNodeRequesterWorker;
    QList<std::shared_ptr<mega::MegaNode>> mNodesToLoad;

protected slots:
    virtual void onRootItemAdded();
    virtual void onRootItemDeleted();

private slots:
    void onChildNodesReady(NodeSelectorModelItem *parent);
    void onNodesAdded(QList<QPointer<NodeSelectorModelItem> > childrenItem);
    void onSyncStateChanged(std::shared_ptr<SyncSettings> sync);

private:
    virtual void createRootNodes() = 0;
    virtual int rootItemsCount() const = 0;
    virtual bool addToLoadingList(const std::shared_ptr<mega::MegaNode> node);
    void createChildItems(std::shared_ptr<mega::MegaNodeList> childNodes, const QModelIndex& index, NodeSelectorModelItem* parent);
    void protectModelWhenPerformingActions();

    QIcon getFolderIcon(NodeSelectorModelItem* item) const;
    bool fetchMoreRecursively(const QModelIndex& parentIndex);

    std::shared_ptr<const UserAttributes::CameraUploadFolder> mCameraFolderAttribute;
    std::shared_ptr<const UserAttributes::MyChatFilesFolder> mMyChatFilesFolderAttribute;

    std::shared_ptr<mega::MegaRequestListener> mListener;

    QThread* mNodeRequesterThread;
    bool mIsBeingModified; //Used to know if the model is being modified in order to avoid nesting beginInsertRows and any other begin* methods
    bool mAcceptDragAndDrop;

    //Variables related to move (including moving to rubbish bin or remove)
    QMap<mega::MegaHandle, int> mRequestByHandle;
    QMap<mega::MegaHandle, int> mRequestFailedByHandle;
    MovedItemsTypes mMovedItemsType;
};

Q_DECLARE_METATYPE(std::shared_ptr<mega::MegaNodeList>)
Q_DECLARE_METATYPE(std::shared_ptr<mega::MegaNode>)

#endif // NODESELECTORMODEL_H
