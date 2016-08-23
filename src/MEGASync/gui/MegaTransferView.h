#ifndef MEGATRANSFERVIEW_H
#define MEGATRANSFERVIEW_H

#include <QTreeView>
#include <QMenu>
#include <QMouseEvent>
#include "TransferItem.h"
#include "QTransfersModel.h"

class MegaTransferView : public QTreeView
{
    Q_OBJECT

public:
    MegaTransferView(QWidget *parent = 0);

private:
    int last_row;
    TransferItem *lastItemHovered;

    QMenu *contextInProgressMenu;
    QAction *pauseTransfer;
    QAction *moveToTop;
    QAction *moveUp;
    QAction *moveDown;
    QAction *moveToDown;
    QAction *clearTransfer;
    QMenu *contextCompleted;
    QAction *clearCompleted;
    QAction *clearAllCompleted;

    void createContextMenu();
    void createCompletedContextMenu();

protected:
    virtual void mouseMoveEvent(QMouseEvent *event);

private slots:
    void onCustomContextMenu(const QPoint &point);
};

#endif // MEGATRANSFERVIEW_H
