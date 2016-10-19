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
    void setup(int type);
    void disableGetLink(bool disable);

private:
    int last_row;
    int lastItemHoveredTag;
    int transferTagSelected;
    int transferStateSelected;
    bool disableLink;

    QMenu *contextInProgressMenu;
    QAction *pauseTransfer;
    QAction *moveToTop;
    QAction *moveUp;
    QAction *moveDown;
    QAction *moveToBottom;
    QAction *cancelTransfer;
    QMenu *contextCompleted;
    QAction *getLink;
    QAction *openItem;
    QAction *showInFolder;
    QAction *clearCompleted;
    QAction *clearAllCompleted;

    void createContextMenu();
    void createCompletedContextMenu();
    void customizeContextInProgressMenu(bool paused, bool enableUpMoves, bool enableDownMoves, bool isCancellable);

protected:
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void leaveEvent(QEvent* event);

private slots:
    void onCustomContextMenu(const QPoint &point);
    void pauseTransferClicked();
    void moveToTopClicked();
    void moveUpClicked();
    void moveDownClicked();
    void moveToBottomClicked();
    void cancelTransferClicked();
    void getLinkClicked();
    void openItemClicked();
    void showInFolderClicked();
    void clearTransferClicked();
    void clearAllTransferClicked();
};

#endif // MEGATRANSFERVIEW_H
