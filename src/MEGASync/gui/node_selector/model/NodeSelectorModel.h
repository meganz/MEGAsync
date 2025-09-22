#ifndef NODESELECTORMODEL_H
#define NODESELECTORMODEL_H

#include "DuplicatedNodeInfo.h"
#include "megaapi.h"
#include "MegaApplication.h"
#include "MessageDialogOpener.h"
#include "NodeSelectorModelItem.h"
#include "Utilities.h"

#include <QAbstractItemModel>
#include <QIcon>
#include <QList>
#include <QPointer>
#include <QQueue>
#include <QReadWriteLock>

#include <memory>
#include <optional>

namespace UserAttributes{
class CameraUploadFolder;
class MyChatFilesFolder;
}

class SyncSettings;

enum class NodeSelectorModelRoles
{
    DATE_ROLE = Qt::UserRole,
    IS_FILE_ROLE,
    IS_SYNCABLE_FOLDER_ROLE,
    STATUS_ROLE,
    ACCESS_ROLE,
    HANDLE_ROLE,
    MODEL_ITEM_ROLE,
    NODE_ROLE,
    EXTRA_ROW_ROLE,
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

enum MoveActionType
{
    MOVE = 0,
    COPY,
    RESTORE,
    EMPTY_MERGE,
    DELETE_RUBBISH,
    DELETE_PERMANENTLY
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

 private slots:
     void onSearchItemTypeChanged(NodeSelectorModelItemSearch::Types type);

 private:
     bool isAborted();
     NodeSelectorModelItem* createSearchItem(mega::MegaNode* node,
                                             NodeSelectorModelItemSearch::Types typesAllowed);

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

class AddNodesQueue: public QObject
{
    Q_OBJECT

public:
    AddNodesQueue(NodeSelectorModel* model);

    void addStep(const QList<std::shared_ptr<mega::MegaNode>>& nodes,
                 const QModelIndex& parentIndex);

private slots:
    void onNodesAdded(bool state);

private:
    struct Info
    {
        QList<std::shared_ptr<mega::MegaNode>> nodesToAdd;
        QModelIndex parentIndex;
    };

    QQueue<Info> mSteps;
    NodeSelectorModel* mModel;
};

class RemoveNodesQueue: public QObject
{
    Q_OBJECT

public:
    RemoveNodesQueue(NodeSelectorModel* model);

    void addStep(const mega::MegaHandle& handle);

signals:
    void startBeginRemoveRows(const mega::MegaHandle& handle);

private slots:
    void onRowsRemoved();

private:
    QQueue<mega::MegaHandle> mSteps;
    NodeSelectorModel* mModel;
};

struct NodeSelectorMergeInfo
{
    std::shared_ptr<mega::MegaNode> nodeToMerge;
    std::shared_ptr<mega::MegaNode> nodeTarget;
    std::shared_ptr<mega::MegaNode> parentNode;
    MoveActionType type;

    enum RestoreMergeType
    {
        MERGE_AND_MOVE_TO_TARGET,
        MERGE_ON_EXISTING_TARGET
    };

    std::optional<RestoreMergeType> restoreMergeType;
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
      ACCESS,
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

    void setCurrentRootIndex(const QModelIndex& rootIndex);
    QModelIndex rootIndex(const QModelIndex& visualRootIndex) const;
    virtual QModelIndex getTopRootIndex() const;

    bool isRequestingNodes() const;

    void setDisableFolders(bool option);
    void setSyncSetupMode(bool value);

    virtual bool addNodes(QList<std::shared_ptr<mega::MegaNode>> node, const QModelIndex& parent);
    void deleteNodeFromModel(const QModelIndex& index);

    int getNodeAccess(mega::MegaNode* node);

    std::shared_ptr<mega::MegaNode> getNodeToRemove(mega::MegaHandle handle);
    void deleteNodes(const QList<mega::MegaHandle>& nodeHandles, bool permanently);
    bool areAllNodesEligibleForDeletion(const QList<mega::MegaHandle>& handles);
    bool areAllNodesEligibleForRestore(const QList<mega::MegaHandle> &handles) const;

    bool startProcessingNodes(const QMimeData* data,
                              const QModelIndex& parent,
                              MoveActionType type);
    void processNodesAfterConflictCheck(std::shared_ptr<ConflictTypes> conflicts,
                                        MoveActionType type);
    bool processNodesAndCheckConflicts(
        const QList<QPair<mega::MegaHandle, std::shared_ptr<mega::MegaNode>>>& handleAndTarget,
        std::shared_ptr<mega::MegaNode> sourceNode,
        MoveActionType type);

    bool increaseMovingNodes(int number);
    bool isMovingNodes() const;
    bool moveProcessedByNumber(int number);

    int getMoveRequestsCounter()
    {
        return mMoveRequestsCounter;
    }

    // Copy logic
    bool pasteNodes(const QList<mega::MegaHandle>& nodesToCopy, const QModelIndex& targetIndex);
    bool canPasteNodes(const QList<mega::MegaHandle>& nodesToCopy, const QModelIndex& indexToPaste);
    virtual bool canCopyNodes() const;

    void moveFileAndReplace(std::shared_ptr<mega::MegaNode> moveFile,
                            std::shared_ptr<mega::MegaNode> conflictTargetFile,
                            std::shared_ptr<mega::MegaNode> targetParentFolder);
    void copyFileAndReplace(std::shared_ptr<mega::MegaNode> copyItem,
                            std::shared_ptr<mega::MegaNode> conflictTargetFile,
                            std::shared_ptr<mega::MegaNode> targetParentFolder);
    void moveNodeAndRename(std::shared_ptr<mega::MegaNode> moveNode,
                           const QString& newName,
                           std::shared_ptr<mega::MegaNode> targetParentFolder);
    void copyNodeAndRename(std::shared_ptr<mega::MegaNode> copyNode,
                           const QString& newName,
                           std::shared_ptr<mega::MegaNode> targetParentFolder);
    void moveNode(std::shared_ptr<mega::MegaNode> moveNode,
                  std::shared_ptr<mega::MegaNode> targetParentFolder);
    void copyNode(std::shared_ptr<mega::MegaNode> copyNode,
                  std::shared_ptr<mega::MegaNode> targetParentFolder);
    //

    void showFiles(bool show);
    void showReadOnlyFolders(bool show);

    QVariant getIcon(const QModelIndex &index, NodeSelectorModelItem* item) const;
    QVariant getText(const QModelIndex &index, NodeSelectorModelItem* item) const;
    void setFetchStep(int step);

    void loadTreeFromNode(const std::shared_ptr<mega::MegaNode> node);
    QModelIndex getIndexFromNode(const std::shared_ptr<mega::MegaNode> node, const QModelIndex& parent);
    QModelIndex getIndexFromHandle(const mega::MegaHandle& handle, const QModelIndex& parent);
    QModelIndex findIndexByNodeHandle(const mega::MegaHandle& handle, const QModelIndex& parent);

    static NodeSelectorModelItem *getItemByIndex(const QModelIndex& index);
    void updateItemNode(const QModelIndex& indexToUpdate, std::shared_ptr<mega::MegaNode> node);
    void updateRow(const QModelIndex &indexToUpdate);

    virtual void firstLoad() = 0;
    void rootItemsLoaded();

    virtual void proxyInvalidateFinished(){}

    QList<QPair<mega::MegaHandle, QModelIndex>> needsToBeExpanded();
    QList<QPair<mega::MegaHandle, QModelIndex>> needsToBeSelected();

    void abort();

    virtual bool rootNodeUpdated(mega::MegaNode*)
    {
        return false;
    }

    virtual bool canBeDeleted() const;
    virtual bool isNodeAccepted(mega::MegaNode* node){return !MegaSyncApp->getMegaApi()->isInRubbish(node);}
    virtual bool showsSyncStates() {return false;}

    bool showFiles() const;

    bool isBeingModified() const
    {
        return mIsBeingModified;
    }

    void setIsModelBeingModified(bool state);

    void setAcceptDragAndDrop(bool newAcceptDragAndDrop);
    bool acceptDragAndDrop(const QMimeData* data);

    QMimeData* mimeData(const QModelIndexList& indexes) const override;
    QMimeData* mimeData(const QList<mega::MegaHandle>& handles) const;

    bool dropMimeData(const QMimeData* data,
                      Qt::DropAction action,
                      int row,
                      int column,
                      const QModelIndex& parent) override;
    // specific cases
    virtual bool canDropMimeData(const QMimeData* data,
        Qt::DropAction action,
        int row,
        int column,
        const QModelIndex& parent) const override;
    // General cases
    virtual bool canDropMimeData() const;
    bool checkDraggedMimeData(const QMimeData* data) const;

    void onRequestFinish(mega::MegaRequest* request, mega::MegaError* e);

    void sendBlockUiSignal(bool state);

    template<class Container>
    void selectIndexesByHandleAsync(const Container& handles)
    {
        for (const auto& handle: handles)
        {
            mIndexesToBeSelected.append(qMakePair(handle, QModelIndex()));
        }
    }

signals:
    void levelsAdded(const QList<QPair<mega::MegaHandle, QModelIndex>>& parent, bool force = false);
    void nodesAdded(const QList<QPointer<NodeSelectorModelItem>>& itemsAdded);
    void requestChildNodes(NodeSelectorModelItem* parent, const QModelIndex& parentIndex);
    void firstLoadFinished(const QModelIndex& parent);
    void requestAddNodes(QList<std::shared_ptr<mega::MegaNode>> newNodes, const QModelIndex& parentIndex, NodeSelectorModelItem* parent);
    void removeItem(NodeSelectorModelItem* items);
    void removeRootItem(NodeSelectorModelItem* items);
    void deleteWorker();
    void blockUi(bool state, QPrivateSignal);
    void updateLoadingMessage(std::shared_ptr<MessageInfo> message);
    void showMessageBox(MessageDialogInfo info) const;
    void showDuplicatedNodeDialog(std::shared_ptr<ConflictTypes> conflicts, MoveActionType type);
    void modelIsBeingModifiedChanged(bool status);
    void modelModified();
    void itemsMoved();
    void itemsAboutToBeMoved(const QList<mega::MegaHandle> handles, int actionType);
    void itemsAboutToBeMovedFailed(const QList<mega::MegaHandle> handles, int actionType);
    void itemsAboutToBeRestored(const QSet<mega::MegaHandle>& targetFolders);
    void itemAboutToBeReplaced(mega::MegaHandle replacedHandle);
    void itemsAboutToBeMerged(const QList<std::shared_ptr<NodeSelectorMergeInfo>>& targetFolders,
                              int actionType);
    void itemsAboutToBeMergedFailed(
        const QList<std::shared_ptr<NodeSelectorMergeInfo>>& targetFolders,
        int actionType);
    void finishAsyncRequest(mega::MegaHandle handle, int error);

protected:
    void beginRemoveRowsAsync(const mega::MegaHandle& handle);

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    Qt::DropActions supportedDropActions() const override;

    QStringList mimeTypes() const override;

    void fetchItemChildren(const QModelIndex& parent);
    void addRootItems();
    virtual void loadLevelFinished();
    bool continueWithNextItemToLoad(const QModelIndex& parentIndex);

    virtual void ignoreDuplicatedNodeOptions(std::shared_ptr<mega::MegaNode>) {}

    virtual bool showAccess(mega::MegaNode* node) const;

    void executeExtraSpaceLogic();

    bool mSyncSetupMode;
    NodeRequester* mNodeRequesterWorker;
    QList<std::shared_ptr<mega::MegaNode>> mNodesToLoad;

    // Indexes to be selected as they are loaded
    QList<QPair<mega::MegaHandle, QModelIndex>> mIndexesToBeSelected;

    // Indexes to be expanded as they are loaded
    QList<QPair<mega::MegaHandle, QModelIndex>> mIndexesToBeExpanded;

protected slots:
    void onRootItemAdded();

private slots:
    void onChildNodesReady(NodeSelectorModelItem *parent);
    void onNodesAdded(QList<QPointer<NodeSelectorModelItem> > childrenItem);
    void onSyncStateChanged(std::shared_ptr<SyncSettings> sync);
    void resetMoveProcessing();
    void checkFinishedRequest(mega::MegaHandle handle, int errorCode);
    void onStartBeginRemoveRowsAsync(const mega::MegaHandle& handle);

private:
    virtual void createRootNodes() = 0;
    virtual int rootItemsCount() const = 0;
    virtual bool addToLoadingList(const std::shared_ptr<mega::MegaNode> node);
    void createChildItems(std::shared_ptr<mega::MegaNodeList> childNodes, const QModelIndex& index, NodeSelectorModelItem* parent);
    void protectModelWhenPerformingActions();

    void executeRemoveExtraSpaceLogic(const QModelIndex& previousIndex);
    void executeAddExtraSpaceLogic(const QModelIndex& currentIndex);

    QIcon getFolderIcon(NodeSelectorModelItem* item) const;
    bool fetchMoreRecursively(const QModelIndex& parentIndex);

    std::shared_ptr<const UserAttributes::CameraUploadFolder> mCameraFolderAttribute;
    std::shared_ptr<const UserAttributes::MyChatFilesFolder> mMyChatFilesFolderAttribute;

    std::shared_ptr<mega::MegaRequestListener> mListener;

    QThread* mNodeRequesterThread;
    bool mIsBeingModified; //Used to know if the model is being modified in order to avoid nesting beginInsertRows and any other begin* methods
    bool mIsProcessingMoves; // Used to know if the user moved nodes
    bool mAcceptDragAndDrop;

    // Variables related to move (including moving to rubbish bin or remove)
    QReadWriteLock mRequestCounterLock;

    struct RequestsBeingProcessed
    {
        int type = -1;
        int counter = 0;

        void clear()
        {
            type = -1;
            counter = 0;
        }
    };

    RequestsBeingProcessed mRequestsBeingProcessed;
    void initRequestsBeingProcessed(int type, int counter);
    int requestFinished();

    QList<mega::MegaHandle> mExpectedNodesUpdates;
    QMultiMap<mega::MegaHandle, int> mRequestFailedByHandle;
    MovedItemsTypes mMovedItemsType;

    // Move nodes
    bool checkMoveProcessing();
    void checkForDuplicatedSourceFilesWhenRestoring(std::shared_ptr<ConflictTypes> conflicts);
    void checkRestoreNodesTargetFolder(std::shared_ptr<ConflictTypes> conflicts);

    QQueue<std::shared_ptr<NodeSelectorMergeInfo>> mMergeQueue;
    QList<std::shared_ptr<NodeSelectorMergeInfo>> mFailedMerges;
    void processMergeQueue(MoveActionType type);
    std::optional<NodeSelectorMergeInfo::RestoreMergeType>
        checkForFoldersToMergeWhenRestoring(std::shared_ptr<ConflictTypes> conflicts);

    int mMoveRequestsCounter;

    // If the model is being modified, queue the nodes to add or to remove
    AddNodesQueue mAddNodesQueue;
    RemoveNodesQueue mRemoveNodesQueue;

    // Current root index
    QModelIndex mCurrentRootIndex;
    QModelIndex mAddedIndex;
    bool mAddExpaceWhenLoadingFinish = false;
    QModelIndex mPendingRootIndex;
    bool mExtraSpaceAdded;
    bool mExtraSpaceRemoved;
    bool mRemovingPreviousExtraSpace;
};

Q_DECLARE_METATYPE(std::shared_ptr<mega::MegaNodeList>)
Q_DECLARE_METATYPE(std::shared_ptr<mega::MegaNode>)
Q_DECLARE_METATYPE(QList<mega::MegaHandle>)
Q_DECLARE_METATYPE(QSet<mega::MegaHandle>)
Q_DECLARE_METATYPE(QList<std::shared_ptr<NodeSelectorMergeInfo>>)

#endif // NODESELECTORMODEL_H
