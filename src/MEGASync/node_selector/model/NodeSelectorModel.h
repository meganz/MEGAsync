#ifndef NODESELECTORMODEL_H
#define NODESELECTORMODEL_H

#include "DuplicatedNodeInfo.h"
#include "megaapi.h"
#include "MegaApplication.h"
#include "MessageDialogData.h"
#include "NodeSelectorModelItem.h"
#include "NodeSelectorOperationTracker.h"
#include "Utilities.h"

#include <QAbstractItemModel>
#include <QHash>
#include <QIcon>
#include <QList>
#include <QMutex>
#include <QPointer>
#include <QQueue>
#include <QRecursiveMutex>

#include <atomic>
#include <functional>
#include <memory>
#include <optional>

namespace UserAttributes
{
class CameraUploadFolder;
class MyChatFilesFolder;
}

class SyncSettings;

enum class NodeSelectorModelRoles
{
    DATE_ROLE = Qt::UserRole,
    IS_FILE_ROLE,
    IS_TAKEN_DOWN_ROLE,
    IS_SYNCABLE_FOLDER_ROLE,
    STATUS_ROLE,
    ACCESS_ROLE,
    HANDLE_ROLE,
    MODEL_ITEM_ROLE,
    NODE_ROLE,
    ICON_SIZE_ROLE,
    EXTRA_ROW_ROLE,
    LABEL_COLOR_ROLE,
    LABEL_ORDER_ROLE,
    IS_EXPORTED_ROLE,
    last
};

enum class NodeRowDelegateRoles
{
    INIT_ROLE = toInt(NodeSelectorModelRoles::last), // ALWAYS use last enum value from previous
                                                     // enum class for new enums
    last
};

enum class HeaderRoles
{
    ICON_ROLE = toInt(NodeRowDelegateRoles::last), // ALWAYS use last enum value from previous enum
                                                   // class for new enums
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

class NodeRequester: public QObject
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
    int rootIndexOf(NodeSelectorModelItem* item);
    NodeSelectorModelItem* getRootItem(int index) const;

    bool trySearchLock() const;
    void lockSearchMutex(bool state) const;

    void cancelCurrentRequest();
    void restartSearch();

    const TabTypes& searchedTypes() const;
    int lastSearchResultCount() const;

    bool showFiles() const;

    bool isIncomingShareCompatible(mega::MegaNode* node);

public slots:
    void requestNodeAndCreateChildren(NodeSelectorModelItem* item, const QModelIndex& parentIndex);
    void search(const QString& text, TabTypes typesAllowed, bool flatten);
    void createCloudDriveRootItem();
    void createIncomingSharesRootItems(std::shared_ptr<mega::MegaNodeList> nodeList);
    void createRubbishRootItems();
    void addIncomingSharesRootItem(std::shared_ptr<mega::MegaNode> node);
    void addSearchRootItem(QList<std::shared_ptr<mega::MegaNode>> nodes, TabTypes typesAllowed);
    void addSearchPathItems(QList<std::shared_ptr<mega::MegaNode>> nodes, TabTypes typesAllowed);
    void createBackupRootItems(mega::MegaHandle backupsHandle);

    void removeRootItem(NodeSelectorModelItem* item);
    void removeRootItem(std::shared_ptr<mega::MegaNode> node);

    QList<QPointer<NodeSelectorModelItem>>
        onAddNodesRequested(QList<std::shared_ptr<mega::MegaNode>> newNodes,
                            const QModelIndex& parentIndex,
                            NodeSelectorModelItem* parentItem);
    void removeItem(NodeSelectorModelItem* item);
    void abort();

signals:
    void nodesReady(NodeSelectorModelItem* parent, int insertedCount);
    void megaCloudDriveRootItemCreated();
    void megaIncomingSharesRootItemsCreated();
    void megaRubbishRootItemsCreated();
    void rootItemsAdded();
    void rootItemsDeleted();
    void megaBackupRootItemsCreated();
    void searchItemsCreated();
    void searchPathItemsAdded();
    void nodeAdded(NodeSelectorModelItem* item);
    void nodesAdded(QList<QPointer<NodeSelectorModelItem>> item);

private slots:
    void onSearchItemTypeChanged(TabTypes type);

private:
    bool isAborted();
    void appendRootItems(const QList<NodeSelectorModelItem*>& items);

    NodeSelectorModelItem* createSearchItem(mega::MegaNode* node, TabTypes typesAllowed);
    NodeSelectorModelItemSearch* createSearchTreeItem(mega::MegaNode* node, TabTypes type);
    bool canCreateSearchItem(mega::MegaNode* node);
    QList<std::shared_ptr<mega::MegaNode>> createSearchPath(mega::MegaNode* node,
                                                            TabTypes type) const;
    using AppendChildrenFn = std::function<QList<QPointer<NodeSelectorModelItem>>(
        NodeSelectorModelItem*,
        const QList<std::shared_ptr<mega::MegaNode>>&)>;

    void addSearchPath(
        QList<NodeSelectorModelItem*>& items,
        const QList<std::shared_ptr<mega::MegaNode>>& path,
        TabTypes type,
        AppendChildrenFn appendChildren = {},
        QHash<mega::MegaHandle, NodeSelectorModelItem*>* alreadyProcessedItemsByHandle = nullptr);
    NodeSelectorModelItem* findSearchItem(const QList<NodeSelectorModelItem*>& items,
                                          mega::MegaHandle handle) const;
    NodeSelectorModelItem* findSearchChild(NodeSelectorModelItem* parent,
                                           mega::MegaHandle handle) const;
    bool isSearchRootNode(mega::MegaNode* node, TabTypes type) const;

    std::atomic<bool> mShowFiles{true};
    std::atomic<bool> mShowReadOnlyFolders{true};
    std::atomic<bool> mAborted{false};
    std::atomic<bool> mSearchCanceled{false};
    std::atomic<bool> mSyncSetupMode{false};
    std::atomic<bool> mNodesRequested{false};
    NodeSelectorModel* mModel;
    QList<NodeSelectorModelItem*> mRootItems;
    // Recursive so a single caller (e.g. the proxy's async sort) can hold it across an
    // operation that itself calls the per-access readers below, which re-lock it.
    mutable QRecursiveMutex mDataMutex;
    mutable QMutex mSearchMutex;
    std::shared_ptr<mega::MegaCancelToken> mCancelToken;
    TabTypes mSearchedTypes;
    std::atomic<int> mLastSearchResultCount{0};
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
    void skipCurrentStep();

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

class NodeSelectorModel: public QAbstractItemModel
{
    Q_OBJECT

public:
    static const int ROW_HEIGHT;

    enum Column
    {
        NODE = 0,
        LABEL,
        USER,
        ACCESS,
        ADDED_DATE,
        LAST_MODIFIED_DATE,
        IS_EXPORTED,
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

    explicit NodeSelectorModel(QObject* parent = 0);
    virtual ~NodeSelectorModel();

    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row,
                      int column,
                      const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    bool canFetchMore(const QModelIndex& parent) const override;

    void setCurrentRootIndex(const QModelIndex& rootIndex);
    QModelIndex getCurrentRootIndex() const;
    QModelIndex rootIndex(const QModelIndex& visualRootIndex) const;
    virtual QModelIndex getTopRootIndex() const;

    bool isRequestingNodes() const;

    void setDisableFolders(bool option);
    void setSyncSetupMode(bool value);

    virtual bool addNodes(QList<std::shared_ptr<mega::MegaNode>> node, const QModelIndex& parent);
    bool deleteNodeFromModel(const QModelIndex& index);

    int getNodeAccess(mega::MegaNode* node);

    std::shared_ptr<mega::MegaNode> getNodeToRemove(mega::MegaHandle handle);
    void deleteNodes(const QList<mega::MegaHandle>& nodeHandles, bool permanently);
    bool areAllNodesEligibleForDeletion(const QList<mega::MegaHandle>& handles);
    bool areAllNodesEligibleForRestore(const QList<mega::MegaHandle>& handles) const;

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
    void finishMovingNodes();

    int getMoveRequestsCounter()
    {
        return mOperationTracker.pendingMoveItems();
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

    QVariant getIcon(const QModelIndex& index, NodeSelectorModelItem* item) const;
    QVariant getText(const QModelIndex& index, NodeSelectorModelItem* item) const;
    virtual QVariant getDisplayText(NodeSelectorModelItem* item) const;
    virtual QVariant getLabelText(NodeSelectorModelItem* item) const;
    virtual QVariant getAddedDateText(NodeSelectorModelItem* item) const;
    virtual QVariant getLastModifiedDateText(NodeSelectorModelItem* item) const;
    virtual QVariant getAccessText(NodeSelectorModelItem* item) const;
    virtual QVariant getUserText(NodeSelectorModelItem* item) const;

    void setFetchStep(int step);

    void loadTreeFromNode(const std::shared_ptr<mega::MegaNode> node);
    bool isLoadingTreePath() const;
    QModelIndex getIndexFromNode(const std::shared_ptr<mega::MegaNode> node,
                                 const QModelIndex& parent);
    QModelIndex getIndexFromHandle(const mega::MegaHandle& handle, const QModelIndex& parent);
    QModelIndex findIndexByNodeHandle(const mega::MegaHandle& handle, const QModelIndex& parent);

    static NodeSelectorModelItem* getItemByIndex(const QModelIndex& index);

    // Hold the worker's data mutex across a whole operation. The proxy uses this to keep the
    // source item tree stable while its async sort rebuilds the source-to-proxy mapping, so the
    // NodeRequester worker cannot structurally mutate mChildItems/mRootItems mid-build (which
    // would desync the mapping and later fault in proxy_to_source() during paint).
    void lockDataMutex(bool lock) const
    {
        mNodeRequesterWorker->lockDataMutex(lock);
    }

    void updateItemNode(const QModelIndex& indexToUpdate, std::shared_ptr<mega::MegaNode> node);
    void updateRow(const QModelIndex& indexToUpdate);

    virtual void firstLoad() = 0;
    void rootItemsLoaded();

    virtual bool hasTopRootIndex()
    {
        return true;
    }

    virtual void proxyInvalidateFinished() {}

    QList<QPair<mega::MegaHandle, QModelIndex>> needsToBeExpanded();
    QList<QPair<mega::MegaHandle, QModelIndex>> needsToBeSelected();

    void abort();

    virtual bool rootNodeUpdated(mega::MegaNode*)
    {
        return false;
    }

    virtual bool canBeDeleted() const;

    virtual bool isNodeAccepted(mega::MegaNode* node)
    {
        return !MegaSyncApp->getMegaApi()->isInRubbish(node);
    }

    virtual bool showsSyncStates()
    {
        return false;
    }

    bool showFiles() const;

    bool isBeingModified() const
    {
        return mIsBeingModified;
    }

    void setIsModelBeingModified(bool state);

    void setAcceptDragAndDrop(bool newAcceptDragAndDrop);
    bool acceptDragAndDrop(const QMimeData* data);
    // Controls the phantom extra space row at the bottom of a folder's children, used as a drop
    // target and as the right-click (context menu) area. Disabled for selectors without those
    // interactions (e.g. file pickers), where it would only be dead space.
    void setExtraSpaceEnabled(bool enabled);

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
    bool checkDraggedMimeData(const QMimeData* data, const QModelIndex& dropIndex) const;

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
    void requestAddNodes(QList<std::shared_ptr<mega::MegaNode>> newNodes,
                         const QModelIndex& parentIndex,
                         NodeSelectorModelItem* parent);
    void removeItem(NodeSelectorModelItem* items);
    void removeRootItem(NodeSelectorModelItem* items);
    void deleteWorker();
    void blockUi(bool state, QPrivateSignal);
    void showMessageBox(MessageDialogInfo info) const;
    void showDuplicatedNodeDialog(std::shared_ptr<ConflictTypes> conflicts, MoveActionType type);
    void modelIsBeingModifiedChanged(bool status);
    void modelModified();
    void currentRootIndexChanged();
    // Emitted when a batch of nodes changes its display name without a path change (e.g. a device
    // name update in Backups), so the navigation breadcrumb can refresh the affected segments.
    void nodesRenamed(const QList<mega::MegaHandle>& handles);
    // Emitted right before a root node is removed (e.g. an incoming share un-shared) while it
    // still exists, so the view can navigate out of it first if it is the open folder.
    void rootNodeAboutToBeRemoved(const QModelIndex& sourceIndex);
    void itemsMoved();
    void itemsAboutToBeMoved(const QList<mega::MegaHandle> handles, int actionType);
    void itemsAboutToBeMovedFailed(const QList<mega::MegaHandle> handles, int actionType);
    void itemRequestsFinished(int actionType);
    void itemsAboutToBeRestored(const QSet<mega::MegaHandle>& targetFolders);
    void itemAboutToBeReplaced(mega::MegaHandle replacedHandle);
    void itemsAboutToBeMerged(const QList<std::shared_ptr<NodeSelectorMergeInfo>>& targetFolders,
                              int actionType);
    void itemMergeFinished(mega::MegaHandle sourceHandle,
                           mega::MegaHandle targetHandle,
                           int actionType);
    void itemsAboutToBeMergedFailed(
        const QList<std::shared_ptr<NodeSelectorMergeInfo>>& targetFolders,
        int actionType);
    void finishAsyncRequest(mega::MegaHandle handle, int error);

public slots:
    void beginRootItemsInsertion(int first, int last);
    void beginChildRowsInsertion(const QModelIndex& parent, int first, int last);

protected:
    void beginRemoveRowsAsync(const mega::MegaHandle& handle);
    MessageDialogInfo buildFailedRequestMessage(
        int requestType,
        const QList<mega::MegaHandle>& failedHandles,
        NodeSelectorOperationTracker::FinishedRequestGroup finishedRequestGroup) const;

    Qt::ItemFlags flags(const QModelIndex& index) const override;
    Qt::DropActions supportedDropActions() const override;

    QStringList mimeTypes() const override;

    bool fetchItemChildren(const QModelIndex& parent);
    void addRootItems();
    virtual void loadLevelFinished();
    bool continueWithNextItemToLoad(const QModelIndex& parentIndex);

    virtual void ignoreDuplicatedNodeOptions(std::shared_ptr<mega::MegaNode>) {}

    virtual bool showAccess(mega::MegaNode* node) const;

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
    void onChildNodesReady(NodeSelectorModelItem* parent, int insertedCount);
    void onNodesAdded(QList<QPointer<NodeSelectorModelItem>> childrenItem);
    void cancelPendingModification();
    void onSyncStateChanged(std::shared_ptr<SyncSettings> sync);
    void resetMoveProcessing();
    void checkFinishedRequest(mega::MegaHandle handle, int errorCode);
    void onStartBeginRemoveRowsAsync(const mega::MegaHandle& handle);

private:
    virtual void createRootNodes() = 0;
    virtual int rootItemsCount() const = 0;
    virtual bool addToLoadingList(const std::shared_ptr<mega::MegaNode> node);
    void createChildItems(std::shared_ptr<mega::MegaNodeList> childNodes,
                          const QModelIndex& index,
                          NodeSelectorModelItem* parent);
    void protectModelWhenPerformingActions();

    void executeRemoveExtraSpaceLogic(const QModelIndex& previousIndex);
    void executeAddExtraSpaceLogic(const QModelIndex& currentIndex);
    bool isExtraSpaceIndex(const QModelIndex& index) const;

    // Single guarded mutation point for the current root. An invalid index is coerced to the top
    // root index, so the current root never becomes QModelIndex(-1,-1) while a valid top root
    // exists. Returns the value actually stored. Callers remain responsible for emitting
    // currentRootIndexChanged().
    QModelIndex commitCurrentRootIndex(const QModelIndex& index);

    QPair<QIcon, QString> getFolderIcon(NodeSelectorModelItem* item) const;
    bool fetchMoreRecursively(const QModelIndex& parentIndex);

    std::shared_ptr<const UserAttributes::CameraUploadFolder> mCameraFolderAttribute;
    std::shared_ptr<const UserAttributes::MyChatFilesFolder> mMyChatFilesFolderAttribute;

    std::shared_ptr<mega::MegaRequestListener> mListener;

    QThread* mNodeRequesterThread;
    bool mIsBeingModified; // Used to know if the model is being modified in order to avoid nesting
                           // beginInsertRows and any other begin* methods
    bool mIsProcessingMoves; // Used to avoid duplicate move completion notifications
    bool mAcceptDragAndDrop;
    NodeSelectorOperationTracker mOperationTracker;

    QList<mega::MegaHandle> mExpectedNodesUpdates;

    // Move nodes
    bool checkMoveProcessing();
    void checkForDuplicatedSourceFilesWhenRestoring(std::shared_ptr<ConflictTypes> conflicts);
    void checkRestoreNodesTargetFolder(std::shared_ptr<ConflictTypes> conflicts);

    QQueue<std::shared_ptr<NodeSelectorMergeInfo>> mMergeQueue;
    QList<std::shared_ptr<NodeSelectorMergeInfo>> mFailedMerges;
    void processMergeQueue(MoveActionType type);
    std::optional<NodeSelectorMergeInfo::RestoreMergeType>
        checkForFoldersToMergeWhenRestoring(std::shared_ptr<ConflictTypes> conflicts);

    // If the model is being modified, queue the nodes to add or to remove
    AddNodesQueue mAddNodesQueue;
    RemoveNodesQueue mRemoveNodesQueue;

    // Current root index
    QModelIndex mCurrentRootIndex;
    bool mAddExpaceWhenLoadingFinish = false;
    // Set only when a root change is deferred because the model was mid-change; holds the root to
    // commit once loading finishes. Invalid means the root was already committed and only the
    // phantom-row add was deferred.
    QModelIndex mPendingRootIndex;
    bool mExtraSpaceAdded;
    bool mExtraSpaceRemoved;
    bool mRemovingPreviousExtraSpace;
    bool mExtraSpaceEnabled;
};

Q_DECLARE_METATYPE(std::shared_ptr<mega::MegaNodeList>)
Q_DECLARE_METATYPE(std::shared_ptr<mega::MegaNode>)
Q_DECLARE_METATYPE(QSet<mega::MegaHandle>)
Q_DECLARE_METATYPE(QList<std::shared_ptr<NodeSelectorMergeInfo>>)

#endif // NODESELECTORMODEL_H
