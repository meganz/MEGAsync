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
    int transferTagSelected;
    int transferStateSelected;

    QMenu *contextInProgressMenu;
    QAction *pauseTransfer;
    QAction *moveToTop;
    QAction *moveUp;
    QAction *moveDown;
    QAction *moveToBottom;
    QAction *cancelTransfer;
    QMenu *contextCompleted;
    QAction *clearCompleted;
    QAction *clearAllCompleted;

    void createContextMenu();
    void createCompletedContextMenu();
    void customizeContextInProgressMenu(bool paused, bool enableUpMoves, bool enableDownMoves, bool isCancellable);

protected:
    virtual void mouseMoveEvent(QMouseEvent *event);

private slots:
    void onCustomContextMenu(const QPoint &point);
    void pauseTransferClicked();
    void moveToTopClicked();
    void moveUpClicked();
    void moveDownClicked();
    void moveToBottomClicked();
    void cancelTransferClicked();
    void clearTransferClicked();
    void clearAllTransferClicked();
};

#endif // MEGATRANSFERVIEW_H
