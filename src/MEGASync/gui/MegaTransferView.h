#ifndef MEGATRANSFERVIEW_H
#define MEGATRANSFERVIEW_H

#include "TransferItem.h"
#include "QTransfersModel.h"
#include "TransfersWidget.h"

#include <QGraphicsEffect>
#include <QTreeView>
#include <QMenu>
#include <QMouseEvent>

class MegaTransferView : public QTreeView
{
    Q_OBJECT

public:
    MegaTransferView(QWidget* parent = 0);
    void setup(int type);
    void setup(TransfersWidget* tw);
    void disableGetLink(bool disable);
    void disableContextMenus(bool option);
    int getType() const;

private:
    int lastItemHoveredTag;
    QList<int> transferTagSelected;
    bool disableLink;
    int type;
    bool disableMenus;

    TransfersWidget* mParentTransferWidget;

    QMenu* mContextMenu;
    QAction* mPauseAction;
    QAction* mResumeAction;
    QAction* mMoveToTopAction;
    QAction* mMoveUpAction;
    QAction* mMoveDownAction;
    QAction* mMoveToBottomAction;
    QAction* mCancelAction;
    QAction* mGetLinkAction;
    QAction* mOpenItemAction;
    QAction* mShowInFolderAction;
    QAction* mClearAction;

    void createContextMenu();
    void updateContextMenu(bool enablePause, bool enableResume, bool enableMove, bool enableClear,
                           bool enableCancel);

protected:
    virtual void mouseReleaseEvent(QMouseEvent* event );
    void changeEvent(QEvent* event);
    void dropEvent(QDropEvent* event);

public slots:
    void onPauseResumeAllRows(bool pauseState);
    void onPauseResumeSelection(bool pauseState);
    void onCancelClearAllRows(bool cancel, bool clear);
    void onCancelClearSelection(bool cancel, bool clear);

private slots:
    void onCustomContextMenu(const QPoint& point);
    void moveToTopClicked();
    void moveUpClicked();
    void moveDownClicked();
    void moveToBottomClicked();
    void getLinkClicked();
    void openItemClicked();
    void showInFolderClicked();
    void showInMegaClicked();
    void cancelSelectedClicked();
    void clearSelectedClicked();
    void pauseSelectedClicked();
    void resumeSelectedClicked();

signals:
    void showContextMenu(QPoint pos);
};

#endif // MEGATRANSFERVIEW_H
