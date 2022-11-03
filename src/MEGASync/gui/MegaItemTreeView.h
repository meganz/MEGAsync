#ifndef MEGAITEMTREEVIEW_H
#define MEGAITEMTREEVIEW_H

#include "megaapi.h"

#include <QTreeView>
#include <QHeaderView>

class MegaItemProxyModel;


using namespace  mega;
class MegaItemTreeView : public QTreeView
{
    Q_OBJECT

public:
    explicit MegaItemTreeView(QWidget *parent = nullptr);
    MegaHandle getSelectedNodeHandle();
    void setModel(QAbstractItemModel *model) override;

protected:
    void drawBranches(QPainter *painter,
                              const QRect &rect,
                              const QModelIndex &index) const override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

    void contextMenuEvent(QContextMenuEvent *event) override;

    void verticalScrollbarValueChanged(int value) override;
    bool eventFilter(QObject* obj, QEvent* evnt) override;
    bool viewportEvent(QEvent *event) override;

signals:
    void removeNodeClicked();
    void getMegaLinkClicked();

private slots:
    void removeNode();
    void getMegaLink();
    void onNavigateReady(const QModelIndex& index);

private:
    bool mousePressorReleaseEvent(QMouseEvent* event);
    QModelIndex getIndexFromSourceModel(const QModelIndex& index) const;
    MegaItemProxyModel* proxyModel() const;
    //QModelIndex mIndexToEnter;
    MegaApi* mMegaApi;

};

class MegaItemHeaderView : public QHeaderView
{
    Q_OBJECT
public:
    explicit MegaItemHeaderView(Qt::Orientation orientation, QWidget* parent = nullptr);

protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;

};

#endif // MEGAITEMTREEVIEW_H
