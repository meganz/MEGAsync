#ifndef NODESELECTORTREEVIEWWIDGET_H
#define NODESELECTORTREEVIEWWIDGET_H

#include "ButtonIconManager.h"
#include "megaapi.h"
#include "NodeSelectorModel.h"
#include "QTMegaListener.h"

#include <QDebug>
#include <QItemSelectionModel>
#include <QPersistentModelIndex>
#include <QPushButton>
#include <QTimer>
#include <QWidget>

#include <memory>

class NodeSelectorProxyModel;
class NodeSelectorModel;
class NodeSelectorModelItem;
class NodeSelectorTreeView;
class SelectType;
typedef std::shared_ptr<SelectType> SelectTypeSPtr;

struct MessageInfo;

namespace Ui
{
class NodeSelectorTreeViewWidget;
}

class NodeSelectorTreeViewWidget: public QWidget
{
    Q_OBJECT

    struct Navigation
    {
        QList<mega::MegaHandle> forwardHandles = QList<mega::MegaHandle>();
        QList<mega::MegaHandle> backwardHandles = QList<mega::MegaHandle>();

        void removeFromForward(const mega::MegaHandle& handle);
        void remove(const mega::MegaHandle& handle);

        void appendToBackward(const mega::MegaHandle& handle);
        void appendToForward(const mega::MegaHandle& handle);
        void clear();
    };

public:
    static const int LOADING_VIEW_THRESSHOLD;
    static const int LABEL_ELIDE_MARGIN;
    static const char* FULL_NAME_PROPERTY;

    explicit NodeSelectorTreeViewWidget(SelectTypeSPtr mode, QWidget* parent = nullptr);
    ~NodeSelectorTreeViewWidget();

    void init();

    mega::MegaHandle getSelectedNodeHandle();
    QList<mega::MegaHandle> getMultiSelectionNodeHandle();
    QModelIndexList getSelectedIndexes() const;
    void navigateToItem(const mega::MegaHandle& handle);
    void setSelectedNodeHandle(const mega::MegaHandle& selectedHandle);

    template<class Container>
    void setAsyncSelectedNodeHandle(const Container& selectedHandles)
    {
        clearSelection();
        mModel->selectIndexesByHandleAsync<Container>(selectedHandles);
    }

    void selectPendingIndexes();

    virtual void setTitleText(const QString& nodeName);

    void clearSelection();
    bool isSelectionCorrect();

    void abort();
    NodeSelectorModelItem* rootItem();
    NodeSelectorProxyModel* getProxyModel();
    bool isInRootView() const;
    bool isEmpty() const;

    bool onNodesUpdate(mega::MegaApi*, mega::MegaNodeList* nodes);

    void enableDragAndDrop(bool enable);

    bool increaseMovingNodes(int number);
    bool decreaseMovingNodes(int number);
    bool areItemsAboutToBeMovedFromHere(mega::MegaHandle firstHandleMoved);

    mega::MegaHandle getHandleByIndex(const QModelIndex& idx);

    void addHandleToBeReplaced(mega::MegaHandle handle);
    void setParentOfRestoredNodes(const QSet<mega::MegaHandle>& parentOfRestoredNodes);

    using TargetHandle = mega::MegaHandle;
    using SourceHandle = mega::MegaHandle;
    void setMergeFolderHandles(const QMultiHash<SourceHandle, TargetHandle>& handles);
    void resetMergeFolderHandles(const QMultiHash<SourceHandle, TargetHandle>& handles);

    bool isUiBlocked();

public slots:
    virtual void checkViewOnModelChange();
    void setLoadingSceneVisible(bool visible);

signals:
    void okBtnClicked();
    void onCustomButtonClicked(uint id);
    void viewReady();
    void uiIsBlocked(bool state);
    void selectionIsCorrect(bool state);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    bool event(QEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void setTitle(const QString& title);
    void selectionChanged(const QModelIndexList& selected);
    QModelIndex getParentIncomingShareByIndex(QModelIndex idx);

    SelectTypeSPtr getSelectType()
    {
        return mSelectType;
    }

    virtual void setViewPage();

    virtual bool showEmptyView()
    {
        return true;
    }

    virtual void makeCustomConnections() {}

    virtual bool isNodeCompatibleWithModel(mega::MegaNode*)
    {
        return false;
    }

    virtual void onRootIndexChanged(const QModelIndex& source_idx);
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

    struct EmptyLabelInfo
    {
        QString title;
        QString description;
    };

    virtual EmptyLabelInfo getEmptyLabel();

    Ui::NodeSelectorTreeViewWidget* ui;
    std::unique_ptr<NodeSelectorProxyModel> mProxyModel;
    std::unique_ptr<NodeSelectorModel> mModel;
    Navigation mNavigationInfo;
    mega::MegaApi* mMegaApi;
    SelectTypeSPtr mSelectType;

protected slots:
    // Title
    void updateRootTitle();

private slots:
    void onbNewFolderClicked();
    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void onModelModified();
    void onDeleteClicked(const QList<mega::MegaHandle>& handles,
                         bool permanently,
                         bool showConfirmationMessageBox);
    void onLeaveShareClicked(const QList<mega::MegaHandle>& handles);
    void onRenameClicked();
    void onGenMEGALinkClicked(const QList<mega::MegaHandle>& handles);
    virtual void onItemDoubleClick(const QModelIndex& index);
    void onGoForwardClicked();
    void onGoBackClicked();
    void onRemoveIndexFromGoBack(const QModelIndex& index);
    void onSectionResized();
    void onExpandReady();
    void onUiBlocked(bool state);
    void processCachedNodesUpdated();
    void removeItemByHandle(mega::MegaHandle handle);
    void onItemsMoved();
    void onNodesAdded(const QList<QPointer<NodeSelectorModelItem>>& itemsAdded);

private:
    bool mManuallyResizedColumn;
    int mResizeEventsReceived;
    QTimer mResizeEventsTimer;

    virtual bool isAllowedToEnterInIndex(const QModelIndex& idx);
    void checkBackForwardButtons();
    void setRootIndex(const QModelIndex& proxy_idx);
    virtual QIcon getEmptyIcon();
    void setEmptyFolderPage();

    QModelIndex getIndexFromHandle(const mega::MegaHandle& handle);
    void checkButtonsVisibility();
    void addCustomButtons(NodeSelectorTreeViewWidget* wdg);
    virtual QString getRootText() = 0;
    virtual std::unique_ptr<NodeSelectorProxyModel> createProxyModel();
    virtual std::unique_ptr<NodeSelectorModel> createModel() = 0;

    virtual bool isCurrentRootIndexReadOnly()
    {
        return false;
    }

    virtual bool newFolderBtnCanBeVisisble()
    {
        return true;
    }

    void setNewFolderButtonVisibility(bool state);

    virtual bool isSelectionReadOnly(const QModelIndexList&)
    {
        return false;
    }

    void selectionHasChanged(const QModelIndexList& selected);

    virtual bool isCurrentSelectionReadOnly()
    {
        return false;
    }

    void checkOkButton(const QModelIndexList& selected);
    bool shouldUpdateImmediately();
    bool areThereNodesToUpdate();

    // Expand and select
    void expandPendingIndexes();
    void resetMoveNodesToSelect();

    // Empty messages
    void initEmptyMessages();

    // Column width
    QList<int> mVisibleColumns;
    void updateColumnsWidth(bool updateVisibleColumnCounter);

    ButtonIconManager mButtonIconManager;
    bool first;
    bool mUiBlocked;

    struct UpdateNodesInfo
    {
        UpdateNodesInfo(mega::MegaNode* node, const QModelIndex& index):
            parentHandle(node->getParentHandle()),
            handle(node->getHandle()),
            node(std::shared_ptr<mega::MegaNode>(node->copy())),
            index(index)
        {}

        UpdateNodesInfo() {}

        mega::MegaHandle parentHandle = mega::INVALID_HANDLE;
        mega::MegaHandle handle = mega::INVALID_HANDLE;
        std::shared_ptr<mega::MegaNode> node;
        QModelIndex index;
    };

    void updateNode(const UpdateNodesInfo& info, bool scrollTo = false);

    // Update Containers
    QList<UpdateNodesInfo> mRenamedNodesByHandle;
    QList<UpdateNodesInfo> mUpdatedNodes;
    QMultiMap<mega::MegaHandle, UpdateNodesInfo> mAddedNodesByParentHandle;
    QMap<mega::MegaHandle, UpdateNodesInfo> mUpdatedNodesBeforeAdded;
    QList<UpdateNodesInfo> mRemovedNodes;
    QList<UpdateNodesInfo> mRemoveMovedNodes;
    QList<UpdateNodesInfo> mUpdatedButInvisibleNodes;
    QList<UpdateNodesInfo> mMergeSourceFolderRemoved;

    // Select when moving finished
    QSet<mega::MegaHandle> mMovedHandlesToSelect;

    // Containers used to ignore specific nodes updates
    QSet<mega::MegaHandle> mParentOfRestoredNodes;
    QMultiHash<SourceHandle, TargetHandle> mMergeTargetFolders;
    QSet<mega::MegaHandle> mNodesToBeReplaced;

    QTimer mNodesUpdateTimer;

    void checkNewFolderAdded(QPointer<NodeSelectorModelItem> item);
    mega::MegaHandle mNewFolderHandle;
    bool mNewFolderAdded;

    friend class DownloadType;
    friend class SyncType;
    friend class UploadType;
    friend class StreamType;
    friend class CloudDriveType;
    friend class SelectType;
};

class SelectType
{
public:
    explicit SelectType() = default;
    virtual ~SelectType() = default;

    virtual bool isContextMenuAllowed()
    {
        return false;
    }

    virtual bool isAllowedToNavigateInside(const QModelIndex& index);
    virtual void init(NodeSelectorTreeViewWidget* wdg) = 0;
    virtual bool okButtonEnabled(NodeSelectorTreeViewWidget* wdg,
                                 const QModelIndexList& selected) = 0;

    virtual void selectionHasChanged(NodeSelectorTreeViewWidget*) {}

    virtual void newFolderButtonVisibility(NodeSelectorTreeViewWidget* wdg);

    virtual QMap<uint, QPushButton*> addCustomButtons(NodeSelectorTreeViewWidget*)
    {
        return QMap<uint, QPushButton*>();
    }

    virtual void updateCustomButtonsText(NodeSelectorTreeViewWidget*) {}

    virtual NodeSelectorModelItemSearch::Types allowedTypes() = 0;

    virtual bool footerVisible() const
    {
        return true;
    }

    virtual void makeViewCustomConnections(NodeSelectorTreeView*, NodeSelectorTreeViewWidget*) {}

protected:
    bool cloudDriveIsCurrentRootIndex(NodeSelectorTreeViewWidget* wdg);

    QPushButton* createCustomButton(const QString& type,
                                    const QString& text,
                                    const QString& iconFile);
};

class DownloadType: public SelectType
{
public:
    explicit DownloadType() = default;
    void init(NodeSelectorTreeViewWidget* wdg) override;
    bool okButtonEnabled(NodeSelectorTreeViewWidget*, const QModelIndexList& selected) override;
    NodeSelectorModelItemSearch::Types allowedTypes() override;
};

class SyncType: public SelectType
{
public:
    explicit SyncType() = default;
    bool isAllowedToNavigateInside(const QModelIndex& index) override;
    void init(NodeSelectorTreeViewWidget* wdg) override;
    bool okButtonEnabled(NodeSelectorTreeViewWidget* wdg, const QModelIndexList& selected) override;
    NodeSelectorModelItemSearch::Types allowedTypes() override;
};

class StreamType: public SelectType
{
public:
    explicit StreamType() = default;
    void init(NodeSelectorTreeViewWidget* wdg) override;
    bool okButtonEnabled(NodeSelectorTreeViewWidget*, const QModelIndexList& selected) override;
    NodeSelectorModelItemSearch::Types allowedTypes() override;
};

class UploadType: public SelectType
{
public:
    explicit UploadType() = default;
    void init(NodeSelectorTreeViewWidget* wdg) override;
    bool okButtonEnabled(NodeSelectorTreeViewWidget* wdg, const QModelIndexList& selected) override;
    NodeSelectorModelItemSearch::Types allowedTypes() override;
};

class CloudDriveType: public SelectType
{
public:
    enum ButtonId : uint
    {
        Upload,
        ClearRubbish
    };

    explicit CloudDriveType() = default;

    bool isContextMenuAllowed() override
    {
        return true;
    }

    void init(NodeSelectorTreeViewWidget* wdg) override;
    void selectionHasChanged(NodeSelectorTreeViewWidget* wdg) override;
    QMap<uint, QPushButton*> addCustomButtons(NodeSelectorTreeViewWidget* wdg) override;
    void updateCustomButtonsText(NodeSelectorTreeViewWidget* wdg) override;
    void newFolderButtonVisibility(NodeSelectorTreeViewWidget* wdg) override;

    bool okButtonEnabled(NodeSelectorTreeViewWidget*, const QModelIndexList& selected) override;
    NodeSelectorModelItemSearch::Types allowedTypes() override;
    bool footerVisible() const override;

    void makeViewCustomConnections(NodeSelectorTreeView* view,
                                   NodeSelectorTreeViewWidget* wdg) override;

private:
    QString getCustomButtonText(uint buttonId) const;

    QMap<QWidget*, QMap<uint, QPushButton*>> mCustomButtons;
};

class MoveBackupType: public UploadType
{
public:
    explicit MoveBackupType() = default;
    NodeSelectorModelItemSearch::Types allowedTypes() override;
};

#endif // NODESELECTORTREEVIEWWIDGET_H
