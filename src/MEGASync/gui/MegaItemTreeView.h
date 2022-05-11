#ifndef MEGAITEMTREEVIEW_H
#define MEGAITEMTREEVIEW_H

#include "megaapi.h"

#include <QTreeView>

class MegaItemProxyModel;


using namespace  mega;
class MegaItemTreeView : public QTreeView
{
    Q_OBJECT

public:
    explicit MegaItemTreeView(QWidget *parent = nullptr);
    MegaHandle getSelectedNodeHandle();

protected:
    void drawBranches(QPainter *painter,
                              const QRect &rect,
                              const QModelIndex &index) const override;

    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    void contextMenuEvent(QContextMenuEvent *event) override;

signals:
    void removeNodeClicked();
    void getMegaLinkClicked();

private slots:
    void removeNode();
    void getMegaLink();

private:

    QModelIndex getIndexFromSourceModel(const QModelIndex& index) const;
    MegaItemProxyModel* proxyModel() const;

    MegaApi* mMegaApi;

};

#endif // MEGAITEMTREEVIEW_H
