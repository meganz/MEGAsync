#ifndef NODESELECTORTREEVIEWWIDGET_H
#define NODESELECTORTREEVIEWWIDGET_H

#include "ButtonIconManager.h"
#include "IncomingShareHeaderWidget.h"
#include "megaapi.h"
#include "NodeSelectorBreadcrumbSegment.h"
#include "NodeSelectorModel.h"
#include "NodeSelectorModelUpdateCoordinator.h"
#include "NodeSelectorNodeActions.h"
#include "NodeSelectorSelectionCoordinator.h"
#include "NodeSelectorSelectTypes.h"
#include "NodeSelectorTabTypes.h"
#include "QTMegaListener.h"

#include <QDebug>
#include <QItemSelectionModel>
#include <QMap>
#include <QPersistentModelIndex>
#include <QSet>
#include <QStringList>
#include <QTimer>
#include <QWidget>

#include <memory>
#include <optional>

class NodeSelectorProxyModel;
class NodeSelectorModel;
class NodeSelectorModelItem;
class NodeSelectorTreeView;
class NodeSelectorDelegate;

struct MessageInfo;

namespace Ui
{
class NodeSelectorTreeViewWidget;
}

class NodeSelectorTreeViewWidget: public QWidget
{
    Q_OBJECT

public:
    static const int LOADING_VIEW_THRESSHOLD;
    static const int LABEL_ELIDE_MARGIN;
    static const char* FULL_NAME_PROPERTY;

    enum TabItem
    {
        CLOUD_DRIVE = 0,
        SHARES,
        BACKUPS,
        RUBBISH,
        SEARCH
    };
    Q_ENUM(TabItem)

    enum class ViewType
    {
        VIEW,
        ROOT_EMPTY,
        FOLDER_EMPTY
    };
    Q_ENUM(ViewType)

    explicit NodeSelectorTreeViewWidget(SelectTypeSPtr mode,
                                        TabItem tabType,
                                        QWidget* parent = nullptr);
    ~NodeSelectorTreeViewWidget();

    void init();

    QList<mega::MegaHandle> getMultiSelectionNodeHandle();
    QModelIndexList getSelectedIndexes() const;
    bool containsTakenDownSelected() const;
    mega::MegaHandle getSelectedNodeHandle() const;
    void navigateToItem(const mega::MegaHandle& handle);
    void setSelectedNodeHandle(const mega::MegaHandle& selectedHandle);

    template<class Container>
    void setAsyncSelectedNodeHandle(const Container& selectedHandles)
    {
        clearSelection();
        mModel->selectIndexesByHandleAsync<Container>(selectedHandles);
    }

    void selectPendingIndexes();

    bool clearSelection();

    void abort();
    void moveToTopRootIndex();
    NodeSelectorModelItem* rootItem();
    QModelIndex getCurrentRootIndex() const;
    NodeSelectorProxyModel* getProxyModel();
    void showCurrentRootContextMenu(const QPoint& globalPos);
    QList<NodeSelectorBreadcrumbSegment> navigationBreadcrumbSegments() const;
    bool navigateToBreadcrumbSegment(int segmentIndex);
    bool isShowingEmptyPage() const;
    ViewType currentViewPage() const;
    bool isInRootView() const;
    bool isTopRootEmpty() const;
    bool isAtTopRoot() const;

    bool onNodesUpdate(mega::MegaApi*, mega::MegaNodeList* nodes);

    void enableDragAndDrop(bool enable);

    bool increaseMovingNodes(int number);
    bool decreaseMovingNodes(int number);
    void finishMovingNodes();
    bool areItemsAboutToBeMovedFromHere(mega::MegaHandle firstHandleMoved);

    mega::MegaHandle getHandleByIndex(const QModelIndex& idx) const;

    void addHandleToBeReplaced(mega::MegaHandle handle);
    void setParentOfRestoredNodes(const QSet<mega::MegaHandle>& parentOfRestoredNodes);

    using TargetHandle = mega::MegaHandle;
    using SourceHandle = mega::MegaHandle;
    void setMergeFolderHandles(const QMultiHash<SourceHandle, TargetHandle>& handles);
    void resetMergeFolderHandles(const QMultiHash<SourceHandle, TargetHandle>& handles);

    bool isUiBlocked();
    void dropIntoRootIndex(QDropEvent* event);

    using NewFolderInfo = NodeSelectorSelectionCoordinator::NewFolderInfo;

    void setNewFolderInfo(const NewFolderInfo& newNewFolderInfo);
    void expandNodeByHandle(mega::MegaHandle handle);

    virtual std::optional<IncomingShareHeaderData> incomingShareHeaderData() const
    {
        return std::nullopt;
    }

    virtual QString getRootText() const = 0;

    void setColumnHidden(int column, bool hidden);
    void setNonInteractiveColumns(const QSet<int>& columns);
    void setInitialShowLabelText(bool show);
    void resetAutoColumnWidths();

    virtual bool isCurrentRootIndexReadOnly() const
    {
        return false;
    }

    // Whether a new folder can be created right now. File pickers (which cannot navigate into
    // folders) rely on this selection-driven check instead of the navigation root.
    virtual bool isNewFolderAllowed()
    {
        return !isCurrentRootIndexReadOnly() && !isCurrentSelectionReadOnly();
    }

    TabItem getTabType() const
    {
        return mTabType;
    }

    bool isLoading() const;

public slots:
    virtual void checkViewOnModelChange();
    // Only forwards to checkViewOnModelChange when the current root toggles empty<->non-empty.
    void onModelRowsChanged();
    void setLoadingSceneVisible(bool visible);
    void notifyViewStateChanged();
    void notifyButtonsStateChanged();

signals:
    void enterKeyPressed();
    void onCustomButtonClicked(uint id);
    void newFolderRequested();
    void viewReady(NodeSelectorTreeViewWidget*);
    // Emitted once the proxy model has been reattached to the view after a loading cycle, so the
    // owner can re-apply column visibility (which is no longer buffered in the loading scene).
    void modelReattached(NodeSelectorTreeViewWidget*);
    void uiIsBlocked(bool state);
    void selectionHasChanged();
    void viewStateChanged();
    void currentViewPageChanged(ViewType type);
    void viewButtonsStateChanged();
    void modelModified();
    void rootIndexChanged();
    void nodesRenamed(const QList<mega::MegaHandle>& handles);

protected:
    bool event(QEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void selectionChanged(const QModelIndexList& selected);
    QModelIndex getParentIncomingShareByIndex(QModelIndex idx) const;

    SelectTypeSPtr getSelectType()
    {
        return mSelectType;
    }

    virtual void setViewPage();

    virtual bool showEmptyView()
    {
        return true;
    }

    virtual void makeViewConnections() {}

    virtual bool isNodeCompatibleWithModel(mega::MegaNode*)
    {
        return false;
    }

    virtual QModelIndex getAddedNodeParent(mega::MegaHandle parentHandle);
    QModelIndex getRootIndexFromIndex(const QModelIndex& index);
    void selectIndex(const QModelIndex& index, bool setCurrent, bool exclusiveSelect = false);
    void selectIndex(const mega::MegaHandle& handle, bool setCurrent, bool exclusiveSelect = false);

    enum class NodeState
    {
        EXISTS,
        EXISTS_BUT_PARENT_UNINITIALISED,
        EXISTS_BUT_OUT_OF_VIEW,
        ADD,
        REMOVE,
        MOVED,
        MOVED_OUT_OF_VIEW,
        DOESNT_EXIST
    };

    virtual NodeState getNodeOnModelState(const QModelIndex& index, mega::MegaNode* node);

    virtual SelectType::EmptyPageInfo getEmptyRootPageInfo();
    SelectType::EmptyPageInfo getEmptyFolderPageInfo() const;
    void showRootEmptyState();
    void setCurrentPage(ViewType type);
    bool showLabelText() const;

    virtual NodeSelectorDelegate* createItemDelegate(QObject* parent);
    virtual NodeSelectorDelegate* createLabelDelegate(QObject* parent);

    Ui::NodeSelectorTreeViewWidget* ui;
    std::shared_ptr<NodeSelectorProxyModel> mProxyModel;
    std::unique_ptr<NodeSelectorModel> mModel;
    // Containers used to ignore specific nodes updates. Declared before the coordinators
    // because mModelUpdateCoordinator holds a reference to it and must be destroyed first.
    QSet<mega::MegaHandle> mNodesToBeReplaced;
    std::unique_ptr<NodeSelectorSelectionCoordinator> mSelectionCoordinator;
    std::unique_ptr<NodeSelectorModelUpdateCoordinator> mModelUpdateCoordinator;
    NodeSelectorNodeActions mNodeActions;
    mega::MegaApi* mMegaApi;
    SelectTypeSPtr mSelectType;

protected slots:
    // Invoked after the proxy finishes processing a level load
    virtual void onLevelLoaded();

private slots:
    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void onModelModified();
    void onDeleteClicked(const QList<mega::MegaHandle>& handles,
                         bool permanently,
                         bool showConfirmationMessageBox);
    void onLeaveShareClicked(const QList<mega::MegaHandle>& handles);
    void onRenameClicked();
    void onGenMEGALinkClicked(const QList<mega::MegaHandle>& handles);
    virtual void onItemDoubleClick(const QModelIndex& index);
    void onRemovedIndexAffectsCurrentRoot(const QModelIndex& index);
    void onSectionResized();
    void onUiBlocked(bool state);
    void processCachedNodesUpdated();

private:
    bool mManuallyResizedColumn;
    bool mShowLabelText;
    int mResizeEventsReceived;
    QTimer mResizeEventsTimer;
    // Coalesces view-state refresh requests (see checkViewOnModelChange).
    QTimer mCheckViewOnModelChangeDebounce;

    virtual bool isAllowedToEnterInIndex(const QModelIndex& idx);
    virtual bool isDownloadAllowed() const;
    void setRootIndex(const QModelIndex& proxy_idx);
    void setCurrentViewWidget();
    void executeCheckViewOnModelChange();
    void showFolderEmptyState();
    void applyEmptyState(const SelectType::EmptyPageInfo& info, ViewType type);
    void setEmptyStateButtonsVisibility(const SelectType::EmptyPageInfo& info);
    void updateEmptyStateButtonsVisibility();
    QModelIndex currentFolderIndex() const;
    QModelIndex indexForBreadcrumbSegment(int segmentIndex) const;

    QModelIndex getIndexFromHandle(const mega::MegaHandle& handle);
    virtual std::shared_ptr<NodeSelectorProxyModel> createProxyModel();
    virtual std::unique_ptr<NodeSelectorModel> createModel() = 0;

    virtual bool isSelectionReadOnly(const QModelIndexList&)
    {
        return false;
    }

    virtual bool isCurrentSelectionReadOnly()
    {
        return false;
    }

    void onSelectionHasChanged();

    void checkOkButton(const QModelIndexList& selected);

    // Empty messages
    void initEmptyRootPageMessages();
    void initEmptyFolderMessages();

    // Column width
    QList<int> mVisibleColumns;
    void updateColumnsWidth(bool updateVisibleColumnCounter);
    // Rebuilds mVisibleColumns from the header. No-op while the model is detached (header
    // has 0 columns); widths are recomputed on the next real viewport resize / reattach.
    void rebuildVisibleColumns();
    void updateColumnResizeModes();
    // True when the tree view has a model attached and its header has columns. The loading
    // scene detaches the model, leaving the header with 0 columns; column computations must
    // be skipped until it is reattached.
    bool isTreeViewReady() const;

    // Full-width separator under the tree view column header (declared in the .ui as
    // headerDivider; positioned at runtime).
    void setupHeaderDivider();
    void updateHeaderDividerGeometry();

    ButtonIconManager mButtonIconManager;
    bool first;
    bool mUiBlocked;
    bool mWasEmpty;
    bool mRootWasEmpty = true;
    bool mNewFolderButtonVisible = true;
    bool mViewInitialized = false;
    ViewType mCurrentViewType = ViewType::VIEW;

    QTimer mNodesUpdateTimer;

    TabItem mTabType;

    friend class DownloadType;
    friend class SyncType;
    friend class UploadType;
    friend class StreamType;
    friend class CloudDriveType;
    friend class MoveBackupType;
    friend class SelectType;
};

#endif // NODESELECTORTREEVIEWWIDGET_H
