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
    static const QColor UPLOAD_DRAG_COLOR;
    static const QColor DOWNLOAD_DRAG_COLOR;


    MegaTransferView(QWidget* parent = 0);
    void setup();
    void setup(TransfersWidget* tw);
    void disableGetLink(bool disable);
    void disableContextMenus(bool option);

    void onPauseResumeVisibleRows(bool isPaused);
    void onCancelAndClearAllTransfers();

    int getVerticalScrollBarWidth() const;

public slots:
    void onPauseResumeSelection(bool pauseState);
    void onCancelClearVisibleTransfers();
    void onCancelClearSelectedTransfers();
    void onClearCompletedVisibleTransfers();
    void onRetryVisibleTransfers();
    void onCancelClearSelection(bool isClear);

signals:
    void verticalScrollBarVisibilityChanged(bool status);

protected:
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    void changeEvent(QEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;
    bool eventFilter(QObject *object, QEvent *event) override;

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

private:
    friend class TransferManagerDelegateWidget;


    bool mDisableLink;
    bool mDisableMenus;
    bool mKeyNavigation;

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
                           bool enableCancel, bool isTopIndex, bool isBottomIndex);

    QModelIndexList getTransfers(bool onlyVisible, TransferData::TransferStates state = TransferData::TRANSFER_NONE);
    QModelIndexList getSelectedTransfers();
};

#endif // MEGATRANSFERVIEW_H
