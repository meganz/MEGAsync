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

using namespace  mega;
class NodeSelectorTreeView : public LoadingSceneView<NodeSelectorLoadingDelegate, QTreeView>
{
    Q_OBJECT

public:
    explicit NodeSelectorTreeView(QWidget *parent = nullptr);
    ~NodeSelectorTreeView();

    MegaHandle getSelectedNodeHandle();
    QList<MegaHandle> getMultiSelectionNodeHandle() const;
    void setModel(QAbstractItemModel *model) override;

    QModelIndexList selectedRows() const;

    enum ActionsOrder
    {
        RESTORE = 0,
        SEPARATOR_1,
        MEGA_LINK,
        SYNC,
        UNSYNC,
        SEPARATOR_2,
        RENAME,
        COPY,
        PASTE,
        SEPARATOR_3,
        DELETE_RUBBISH,
        DELETE_PERMANENTLY,
        LEAVE_SHARE
    };
    Q_ENUM(ActionsOrder)

protected:
    void drawBranches(QPainter *painter,
                              const QRect &rect,
                              const QModelIndex &index) const override;

    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent *event) override;

signals:
    void deleteNodeClicked(const QList<MegaHandle>& handles, bool permanently);
    void leaveShareClicked(const QList<MegaHandle>& handles);
    void renameNodeClicked();
    void pasteNodesClicked();
    void getMegaLinkClicked();
    void restoreClicked(const QList<MegaHandle>& handles);
    void nodeSelected();

private slots:
    void deleteNode(const QList<MegaHandle>& handles, bool permanently);
    void renameNode();
    void getMegaLink();
    void restore(const QList<MegaHandle>& handles);
    void onNavigateReady(const QModelIndex& index);
    void onCopyShortcutActivated();
    void onPasteShortcutActivated();
    void onPasteClicked();

private:
    friend class NodeSelectorDelegate;

    bool mousePressorReleaseEvent(QMouseEvent* event);
    bool handleStandardMouseEvent(QMouseEvent* event);
    QModelIndex getIndexFromSourceModel(const QModelIndex& index) const;
    NodeSelectorProxyModel* proxyModel() const;
    std::shared_ptr<MegaNode> getDropNode(const QModelIndex& dropIndex);

    bool areAllEligibleForCopy(const QList<mega::MegaHandle>& handles) const;
    enum class DeletionType
    {
        PERMANENT_REMOVE,
        MOVE_TO_RUBBISH,
        LEAVE_SHARE
    };

    std::optional<NodeSelectorTreeView::DeletionType>
        areAllEligibleForDeletion(const QList<mega::MegaHandle>& handles) const;
    bool areAllEligibleForRestore(const QList<MegaHandle> &handles) const;
    bool isAnyNodeInTheRubbish(const QList<MegaHandle>& handles) const;

    void addPasteMenuAction(QMap<int, QAction*>& actions);
    void addRestoreMenuAction(QMap<int, QAction*>& actions,
                              QList<mega::MegaHandle> selectionHandles);
    void addDeleteMenuAction(QMap<int, QAction*>& actions,
                             QList<mega::MegaHandle> selectionHandles);
    void addDeletePermanently(QMap<int, QAction*>& actions,
                              QList<mega::MegaHandle> selectionHandles);
    void addLeaveInshare(QMap<int, QAction*>& actions, QList<mega::MegaHandle> selectionHandles);
    void addRemoveMenuActions(QMap<int, QAction*>& actions,
                              QList<mega::MegaHandle> selectionHandles);

    // Shortcuts
    QShortcut* mCopyShortcut;
    QShortcut* mPasteShortcut;

    // Copied handles are common for all views
    static QList<mega::MegaHandle> mCopiedHandles;

    MegaApi* mMegaApi;
};

class NodeSelectorTreeViewHeaderView : public QHeaderView
{
    Q_OBJECT
public:
    explicit NodeSelectorTreeViewHeaderView(Qt::Orientation orientation, QWidget* parent = nullptr);

protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;
};

#endif // NODESELECTORTREEVIEW_H
