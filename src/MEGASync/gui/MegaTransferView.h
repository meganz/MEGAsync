#ifndef MEGATRANSFERVIEW_H
#define MEGATRANSFERVIEW_H

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
    void setup();
    void setup(TransfersWidget* tw);
    void disableGetLink(bool disable);
    void disableContextMenus(bool option);

private:
    bool disableLink;
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
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    void changeEvent(QEvent* event) override;
    void dropEvent(QDropEvent* event) override;

public slots:
    void onPauseResumeVisibleRows(bool pauseState);
    void onPauseResumeAllRows(bool pauseState);
    void onPauseResumeSelection(bool pauseState);
    void onCancelClearAllVisibleTransfers();
    void onClearCompletedVisibleTransfers();
    void onCancelClearAllTransfers();
    void onCancelClearSelection();

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
