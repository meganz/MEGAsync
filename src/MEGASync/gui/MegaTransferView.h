#ifndef MEGATRANSFERVIEW_H
#define MEGATRANSFERVIEW_H

#include "TransferItem.h"
#include "QTransfersModel.h"
#include "TransfersWidget.h"

#include <QTreeView>
#include <QMenu>
#include <QMouseEvent>

class MegaTransferView : public QTreeView
{
    Q_OBJECT

public:
    MegaTransferView(QWidget *parent = 0);
    void setup(int type);
    void setup(TransfersWidget* tw);
    void disableGetLink(bool disable);
    void disableContextMenus(bool option);
    int getType() const;

    void pauseResumeSelection(bool pauseState);
    void cancelClearSelection();

private:
    int lastItemHoveredTag;
    QList<int> transferTagSelected;
    bool disableLink;
    int type;
    bool disableMenus;

    TransfersWidget* mParentTransferWidget;

    QMenu *mContextMenu;
    QAction *mPauseAction;
    QAction *mResumeAction;
    QAction *mMoveToTopAction;
    QAction *mMoveUpAction;
    QAction *mMoveDownAction;
    QAction *mMoveToBottomAction;
    QAction *mCancelAction;
    QAction *mGetLinkAction;
    QAction *mOpenItemAction;
    QAction *mShowInFolderAction;
    QAction *mShowInMegaAction;
    QAction *mClearAction;

    void createContextMenu();
    void updateContextMenu(bool enablePause, bool enableResume, bool enableMove, bool enableClear,
                           bool enableCancel);

protected:
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent (QMouseEvent * event );
    virtual void leaveEvent(QEvent* event);
    void changeEvent(QEvent *event);
    void dropEvent(QDropEvent *event);

private slots:
    void onCustomContextMenu(const QPoint &point);
    void moveToTopClicked();
    void moveUpClicked();
    void moveDownClicked();
    void moveToBottomClicked();
    void getLinkClicked();
    void openItemClicked();
    void showInFolderClicked();
    void showInMegaClicked();

signals:
    void showContextMenu(QPoint pos);
};

#endif // MEGATRANSFERVIEW_H
