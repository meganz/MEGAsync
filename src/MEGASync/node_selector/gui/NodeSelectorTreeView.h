#ifndef NODESELECTORTREEVIEW_H
#define NODESELECTORTREEVIEW_H

#include "megaapi.h"
#include "NodeSelectorLoadingDelegate.h"
#include "ViewLoadingScene.h"

#include <QHeaderView>
#include <QShortcut>
#include <QTreeView>

#include <optional>

class NodeSelectorProxyModel;
class NodeSelectorModel;
class MegaDelegateHoverManager;

using namespace mega;

class NodeSelectorTreeView: public LoadingSceneView<NodeSelectorLoadingDelegate, QTreeView>
{
    Q_OBJECT

public:
    explicit NodeSelectorTreeView(QWidget* parent = nullptr);
    ~NodeSelectorTreeView();

    MegaHandle getSelectedNodeHandle();
    QList<MegaHandle> getMultiSelectionNodeHandle(const QModelIndexList& selectedRows) const;
    void setModel(QAbstractItemModel* model) override;

    QModelIndexList selectedRows() const;

    void dropEvent(QDropEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;

    enum ActionsOrder
    {
        RESTORE = 0,
        SEPARATOR_1,
        DOWNLOAD,
        SEPARATOR_2,
        MEGA_LINK,
        SYNC,
        UNSYNC,
        SEPARATOR_3,
        RENAME,
        COPY,
        PASTE,
        SEPARATOR_4,
        DELETE_RUBBISH,
        DELETE_PERMANENTLY,
        LEAVE_SHARE
    };
    Q_ENUM(ActionsOrder)

    void setAllowContextMenu(bool newAllowContextMenu);

protected:
    void drawBranches(QPainter* painter,
                      const QRect& rect,
                      const QModelIndex& index) const override;

    void drawRow(QPainter* painter,
                 const QStyleOptionViewItem& option,
                 const QModelIndex& index) const override;

    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

    void startDrag(Qt::DropActions supportedActions) override;

    bool event(QEvent* event) override;

signals:
    void deleteNodeClicked(const QList<MegaHandle>& handles, bool permanently);
    void leaveShareClicked(const QList<MegaHandle>& handles);
    void renameNodeClicked();
    void pasteNodesClicked();
    void getMegaLinkClicked(const QList<MegaHandle>& handles);
    void restoreClicked(const QList<MegaHandle>& handles);
    void nodeSelected();

private slots:
    void deleteNode(const QList<MegaHandle>& handles, bool permanently);
    void renameNode();
    void restore(const QList<MegaHandle>& handles);
    void onNavigateReady(const QModelIndex& index);
    void onCopyShortcutActivated();
    void onPasteShortcutActivated();
    void onPasteClicked(const QModelIndex& selectedIndex);

private:
    friend class NodeSelectorDelegate;

    void expandWithMouseReleaseEvent(QMouseEvent* event);
    void selectFromMouseReleaseEvent(const QModelIndex& index, Qt::KeyboardModifiers modifiers);

    QModelIndex getIndexFromSourceModel(const QModelIndex& index) const;
    NodeSelectorProxyModel* proxyModel() const;
    std::shared_ptr<MegaNode> getDropNode(const QModelIndex& dropIndex);

    // Context menu
    bool areAllEligibleForCopy(const QModelIndexList& selectedIndexes) const;

    enum class DeletionType
    {
        PERMANENT_REMOVE,
        MOVE_TO_RUBBISH,
        LEAVE_SHARE
    };
    std::optional<NodeSelectorTreeView::DeletionType>
        areAllEligibleForDeletion(const QModelIndexList& selectedIndexes) const;
    bool areAllEligibleForLinkShare(const QModelIndexList& selectedIndexes) const;
    bool areAllEligibleForRestore(const QModelIndexList& selectedIndexes) const;

    void addShareLinkMenuAction(QMap<int, QAction*>& actions,
                                const QModelIndexList& selectedIndexes,
                                const QList<MegaHandle>& selectionHandles);
    void addPasteMenuAction(QMap<int, QAction*>& actions, const QModelIndexList& selectedIndexes);
    void addRestoreMenuAction(QMap<int, QAction*>& actions,
                              const QModelIndexList& selectedIndexes,
                              const QList<mega::MegaHandle>& selectionHandles);
    void addDownloadMenuAction(QMap<int, QAction*>& actions,
                               const QList<mega::MegaHandle>& selectionHandles);
    void addRenameMenuAction(QMap<int, QAction*>& actions, const QModelIndex& index);
    void addSyncMenuActions(QMap<int, QAction*>& actions,
                            const QModelIndex& index,
                            MegaHandle selectedHandle);
    void addDeleteMenuAction(QMap<int, QAction*>& actions,
                             QList<mega::MegaHandle> selectionHandles);
    void addDeletePermanently(QMap<int, QAction*>& actions,
                              QList<mega::MegaHandle> selectionHandles);
    void addLeaveInshare(QMap<int, QAction*>& actions,
                         const QList<mega::MegaHandle>& selectionHandles);
    void addRemoveMenuActions(QMap<int, QAction*>& actions,
                              const QModelIndexList& selectedIndexes,
                              const QList<MegaHandle>& selectionHandles);

    bool mAllowContextMenu;

    // Access
    QHash<mega::MegaHandle, int> getNodesAccess(const QList<mega::MegaHandle>& handles) const;

    // Shortcuts
    QShortcut* mCopyShortcut;
    QShortcut* mPasteShortcut;

    // Copied handles are common for all views
    static QList<mega::MegaHandle> mCopiedHandles;

    MegaApi* mMegaApi;

    // Hover event
    std::unique_ptr<MegaDelegateHoverManager> mHoverManager;

    // Branch pixmaps
    mutable QPixmap mRightChevron;
    mutable QPixmap mDownChevron;
};

#endif // NODESELECTORTREEVIEW_H
