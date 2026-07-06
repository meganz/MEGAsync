#ifndef NODESELECTORSELECTIONCOORDINATOR_H
#define NODESELECTORSELECTIONCOORDINATOR_H

#include "megaapi.h"

#include <QHash>
#include <QList>
#include <QModelIndex>
#include <QMultiHash>
#include <QObject>
#include <QPointer>
#include <QSet>

#include <functional>
#include <memory>

class NodeSelectorModel;
class NodeSelectorModelItem;
class NodeSelectorProxyModel;
class NodeSelectorTreeView;

class NodeSelectorSelectionCoordinator: public QObject
{
    Q_OBJECT

public:
    struct NewFolderInfo
    {
        mega::MegaHandle handle = mega::INVALID_HANDLE;
        bool recentlyAdded = false;
    };

    struct Objects
    {
        NodeSelectorModel* model = nullptr;
        NodeSelectorProxyModel* proxyModel = nullptr;
        NodeSelectorTreeView* treeView = nullptr;
    };

    struct Policy
    {
        std::function<void(const QModelIndex&, bool, bool)> selectIndex;
        std::function<void()> clearSelection;
        std::function<void(const QModelIndex&)> onItemDoubleClick;
        std::function<void(const QModelIndex&)> onNewFolderAdded;
        std::function<void()> setRootIndexToTop;
        std::function<void(std::function<bool()>)> withSelectionSilenced;
    };

    NodeSelectorSelectionCoordinator(mega::MegaApi* megaApi, Objects objects, Policy policy);

    void setParentOfRestoredNodes(const QSet<mega::MegaHandle>& parentOfRestoredNodes);
    void setMergeFolderHandles(const QMultiHash<mega::MegaHandle, mega::MegaHandle>& handles);
    void resetMergeFolderHandles(const QMultiHash<mega::MegaHandle, mega::MegaHandle>& handles);
    void setNewFolderInfo(const NewFolderInfo& info);

    void setSelectedNodeHandle(const mega::MegaHandle& selectedHandle);
    void expandPendingIndexes();
    void selectPendingIndexes();
    void resetMoveNodesToSelect();
    void reapplyMovedSelection();
    void clearMovedSelection();

    QSet<mega::MegaHandle>& movedHandlesToSelect();
    QSet<mega::MegaHandle>& parentOfRestoredNodes();
    QMultiHash<mega::MegaHandle, mega::MegaHandle>& mergeTargetFolders();

public slots:
    void onItemsMoved();
    void onNodesAdded(const QList<QPointer<NodeSelectorModelItem>>& itemsAdded);

private:
    void checkNewFolderAdded(QPointer<NodeSelectorModelItem> item);
    void triggerTreePathLoad(const std::shared_ptr<mega::MegaNode>& node);

    mega::MegaApi* mMegaApi;
    NodeSelectorModel* mModel;
    NodeSelectorProxyModel* mProxyModel;
    NodeSelectorTreeView* mTreeView;

    std::function<void(const QModelIndex&, bool, bool)> mSelectIndex;
    std::function<void()> mClearSelection;
    std::function<void(const QModelIndex&)> mOnItemDoubleClick;
    std::function<void(const QModelIndex&)> mOnNewFolderAdded;
    std::function<void()> mSetRootIndexToTop;
    std::function<void(std::function<bool()>)> mWithSelectionSilenced;

    QSet<mega::MegaHandle> mMovedHandlesToSelect;
    QSet<mega::MegaHandle> mParentOfRestoredNodes;
    QMultiHash<mega::MegaHandle, mega::MegaHandle> mMergeTargetFolders;
    NewFolderInfo mNewFolderInfo;

    // Only a restore jumps to the top root (its nodes may return to different parents); a
    // move/merge stays in the current folder. Set in onItemsMoved (before mParentOfRestoredNodes
    // is cleared) and consumed in selectPendingIndexes.
    bool mSkipTopRootOnSelect = false;

    // selectPendingIndexes can re-enter itself synchronously (an unresolved handle triggers a
    // loadTreeFromNode that emits blockUi(false) synchronously, which re-invokes it).
    // mResolvingPendingIndexes marks an active call so a synchronous re-entry returns immediately
    // and the recursion stays bounded; mLoadAttempts counts the tree path loads triggered per
    // handle so a handle whose load died (failed synchronously, aborted or clobbered by another
    // load) is retried a bounded number of times and then dropped, instead of being re-queued
    // (or re-loaded) forever.
    QHash<mega::MegaHandle, int> mLoadAttempts;
    bool mResolvingPendingIndexes = false;
};

#endif // NODESELECTORSELECTIONCOORDINATOR_H
