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
    int getType() const;

private:
    int lastItemHoveredTag;
    QList<int> transferTagSelected;
    bool disableLink;
    int type;

    QMenu *contextInProgressMenu;
    QAction *pauseTransfer;
    QAction *resumeTransfer;
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
    void customizeContextInProgressMenu(bool enablePause, bool enableResume, bool enableUpMoves, bool enableDownMoves, bool isCancellable);
    void customizeCompletedContextMenu(bool enableGetLink = true, bool enableOpen = true, bool enableShow = true);

protected:
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void leaveEvent(QEvent* event);
    void changeEvent(QEvent *event);

private slots:
    void onCustomContextMenu(const QPoint &point);
    void pauseTransferClicked();
    void resumeTransferClicked();
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
