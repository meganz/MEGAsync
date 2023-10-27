#ifndef NODESELECTORTREEVIEW_H
#define NODESELECTORTREEVIEW_H

#include "megaapi.h"
#include "ViewLoadingScene.h"
#include "NodeSelectorLoadingDelegate.h"

#include <QTreeView>
#include <QHeaderView>

class NodeSelectorProxyModel;


using namespace  mega;
class NodeSelectorTreeView : public LoadingSceneView<NodeSelectorLoadingDelegate, QTreeView>
{
    Q_OBJECT

public:
    explicit NodeSelectorTreeView(QWidget *parent = nullptr);
    MegaHandle getSelectedNodeHandle();
    void setModel(QAbstractItemModel *model) override;

protected:
    void drawBranches(QPainter *painter,
                              const QRect &rect,
                              const QModelIndex &index) const override;

    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

signals:
    void removeNodeClicked();
    void renameNodeClicked();
    void getMegaLinkClicked();
    void restoreClicked();
    void nodeSelected();

private slots:
    void removeNode();
    void renameNode();
    void getMegaLink();
    void restore();
    void onNavigateReady(const QModelIndex& index);

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    void onCurrentRowChanged(const QModelIndex &current, const QModelIndex &previous);
#endif

private:
    bool mousePressorReleaseEvent(QMouseEvent* event);
    bool handleStandardMouseEvent(QMouseEvent* event);
    QModelIndex getIndexFromSourceModel(const QModelIndex& index) const;
    NodeSelectorProxyModel* proxyModel() const;

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
