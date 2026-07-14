#ifndef NODESELECTORTREEVIEW_H
#define NODESELECTORTREEVIEW_H

#include "megaapi.h"
#include "NodeSelectorLoadingDelegate.h"
#include "ViewLoadingScene.h"

#include <QHeaderView>
#include <QHelpEvent>
#include <QPointer>
#include <QSet>
#include <QShortcut>
#include <QTreeView>

#include <optional>

class NodeSelectorProxyModel;
class NodeSelectorModel;
class MegaDelegateHoverManager;
class ArrowTooltip;

// Styled tooltip shared across the node selector (tree rows and header sections): Caption/Regular
// text in an ArrowTooltip with an up-pointing arrow, anchored just below the hovered element
// and horizontally centered on the cursor.
class NodeSelectorStyledTooltip
{
public:
    // Element under the cursor that the tooltip should describe, returned by the resolver passed
    // to handleViewportEvent(). An empty text means "no tooltip here"; id identifies the element
    // (compared with Id::operator!=) so the tooltip is re-anchored only when moving onto a
    // different one, which avoids the cursor-following motion-blur.
    template<typename Id>
    struct Target
    {
        QString text;
        int anchorBottomGlobalY = 0;
        Id id = {};
    };

    // Drives the tooltip from a view's viewportEvent: on QEvent::ToolTip it calls
    // resolve(viewportPos) -> Target and shows/hides accordingly; Leave/Wheel hide it. Returns
    // true when the event was consumed (the caller should then return true without calling base).
    template<typename Id, typename Resolver>
    bool handleViewportEvent(QEvent* e, QWidget* parent, Id& lastId, Resolver resolve)
    {
        if (e->type() == QEvent::ToolTip)
        {
            auto* helpEvent = static_cast<QHelpEvent*>(e);
            const auto target = resolve(helpEvent->pos());
            if (!target.text.isEmpty())
            {
                if (!isVisible() || lastId != target.id)
                {
                    show(parent, target.text, helpEvent->globalPos(), target.anchorBottomGlobalY);
                    lastId = target.id;
                }
                return true;
            }
            hide();
        }
        else if (e->type() == QEvent::Leave || e->type() == QEvent::Wheel)
        {
            hide();
        }
        return false;
    }

    void hide();

    bool isVisible() const
    {
        return !mTooltip.isNull();
    }

private:
    void show(QWidget* parent,
              const QString& text,
              const QPoint& globalCursorPos,
              int anchorBottomGlobalY);

    static constexpr int V_GAP = 8;
    QPointer<ArrowTooltip> mTooltip;
};

class NodeSelectorHeaderView: public QHeaderView
{
    Q_OBJECT

public:
    explicit NodeSelectorHeaderView(Qt::Orientation orientation, QWidget* parent = nullptr);

    void setNonInteractiveSections(const QSet<int>& sections);

protected:
    void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    bool event(QEvent* e) override;
    bool viewportEvent(QEvent* e) override;

private:
    QStyleOptionHeader::SectionPosition sectionPosition(int logicalIndex) const;
    QPixmap sortArrowPixmap(Qt::SortOrder order) const;

    QSet<int> mNonInteractiveSections;
    mutable QPixmap mAscendingSortArrow;
    mutable QPixmap mDescendingSortArrow;

    NodeSelectorStyledTooltip mTooltip;
    int mTooltipSection = -1; // Header section the tooltip is currently anchored to.
};

using namespace mega;

class NodeSelectorTreeView: public LoadingSceneView<NodeSelectorLoadingDelegate, QTreeView>
{
    Q_OBJECT

public:
    explicit NodeSelectorTreeView(QWidget* parent = nullptr);
    ~NodeSelectorTreeView();

    QList<MegaHandle> getMultiSelectionNodeHandle(const QModelIndexList& selectedRows) const;
    void setModel(QAbstractItemModel* model) override;
    void setRootIndexReadOnly(bool state);

    QModelIndexList selectedRows() const;

    void dropEvent(QDropEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;

    enum ActionsOrder
    {
        RESTORE = 0,
        SEPARATOR_1,
        UPLOAD,
        DOWNLOAD,
        SEPARATOR_2,
        MEGA_LINK,
        SYNC,
        UNSYNC,
        SEPARATOR_3,
        NEW_FOLDER,
        SEPARATOR_4,
        RENAME,
        COPY,
        PASTE,
        SEPARATOR_5,
        DISPUTE_TAKEDOWN,
        SEPARATOR_6,
        DELETE_RUBBISH,
        DELETE_PERMANENTLY,
        LEAVE_SHARE
    };
    Q_ENUM(ActionsOrder)

    void setAllowContextMenu(bool newAllowContextMenu);
    void setAllowNewFolderContextMenuItem(bool newAllowNewFolderContextMenuItem);
    void contextMenuEvent(QContextMenuEvent* event) override;

    bool containsTakenDownItem(const QModelIndexList& selectedIndexes) const;
    // Builds and shows the node context menu for the given target. clickedEmptySpace means the
    // click did not land on a row (it targets the current root folder), enabling folder-level
    // actions (upload, new folder). ignoreEmptySpaceBlock additionally treats the current root as
    // the selected node, so node-level actions (copy, sync, move to rubbish, share link...) are
    // also offered. Called from contextMenuEvent (right-click) and from the breadcrumb chevron.
    void showContextMenu(const QModelIndexList& selectedRowsList,
                         const QModelIndex& clickedIndex,
                         bool clickedEmptySpace,
                         const QPoint& globalPos,
                         bool ignoreEmptySpaceBlock = false);

protected:
    // Detach the model while the loading scene is shown, so the view never queries the
    // proxy while it is being re-sorted in another thread (avoids broken layouts).
    bool detachModelDuringLoading() const override
    {
        return true;
    }

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

    void startDrag(Qt::DropActions supportedActions) override;

    bool event(QEvent* event) override;
    bool viewportEvent(QEvent* event) override;

signals:
    void deleteNodeClicked(const QList<MegaHandle>& handles,
                           bool permanently,
                           bool showConfirmationMessageBo);
    void leaveShareClicked(const QList<MegaHandle>& handles);
    void renameNodeClicked();
    void pasteNodesClicked();
    void getMegaLinkClicked(const QList<MegaHandle>& handles);
    void restoreClicked(const QList<MegaHandle>& handles);
    void enterKeyPressed();
    void newFolderClicked();
    void uploadClicked();

private slots:
    void deleteNode(const QList<MegaHandle>& handles,
                    bool permanently,
                    bool showConfirmationMessageBox = true);
    void renameNode();
    void restore(const QList<MegaHandle>& handles);
    void onNavigateReady(const QModelIndex& index);
    void onCopyShortcutActivated();
    void onPasteShortcutActivated();
    void onPasteClicked(const QModelIndex& selectedIndex);

private:
    friend class NodeSelectorDelegate;

    static const int ROW_SIDE_MARGIN;
    static const int ROW_RIGHT_MARGIN;
    static const int ROW_VERTICAL_MARGIN;
    static const qreal ROW_RADIUS;

    void selectFromMouseEvent(const QModelIndex& index, Qt::KeyboardModifiers modifiers);
    // Sets the row highlighted as the current drop target and repaints only on change.
    void setDropHoverIndex(const QModelIndex& index);

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
    bool areAllEligibleForDownload(const QModelIndexList& selectedIndexes) const;

    void addShareLinkMenuAction(QMap<int, QAction*>& actions,
                                const QModelIndexList& selectedIndexes,
                                const QList<MegaHandle>& selectionHandles);
    void addPasteMenuAction(QMap<int, QAction*>& actions, const QModelIndexList& selectedIndexes);
    void addRestoreMenuAction(QMap<int, QAction*>& actions,
                              const QModelIndexList& selectedIndexes,
                              const QList<mega::MegaHandle>& selectionHandles);
    void addDownloadMenuAction(QMap<int, QAction*>& actions,
                               const QModelIndexList& selectedIndexes,
                               const QList<mega::MegaHandle>& selectionHandles);
    void addUploadMenuAction(QMap<int, QAction*>& actions);
    void addNewFolderMenuAction(QMap<int, QAction*>& actions);
    void addRenameMenuAction(QMap<int, QAction*>& actions, const QModelIndex& index);
    void addSyncMenuActions(QMap<int, QAction*>& actions,
                            const QModelIndex& index,
                            MegaHandle selectedHandle);
    void addDisputeTakedownMenuAction(QMap<int, QAction*>& actions);
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
    bool mAllowNewFolderContextMenuItem;

    // Access
    QHash<mega::MegaHandle, int> getNodesAccess(const QList<mega::MegaHandle>& handles) const;

    // Read-Only root index
    bool mRootIndexReadOnly;

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

    // All NodeSelector row tooltips are rendered through this styled tooltip instead of native
    // QToolTip. The whole-row access role takes priority over the per-cell ToolTipRole.
    NodeSelectorStyledTooltip mTooltip;
    QPersistentModelIndex mTooltipIndex; // Row the styled tooltip is currently anchored to.
    QPersistentModelIndex mDropHoverIndex; // Row highlighted as the current drop target.
};

#endif // NODESELECTORTREEVIEW_H
