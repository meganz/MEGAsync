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

    static const int CANCEL_MESSAGE_THRESHOLD;


    MegaTransferView(QWidget* parent = 0);
    void setup();
    void setup(TransfersWidget* tw);
    void enableContextMenu();

    void onPauseResumeVisibleRows(bool isPaused);
    bool onCancelAllTransfers();
    void onClearAllTransfers();
    void onCancelAndClearVisibleTransfers();

    int getVerticalScrollBarWidth() const;

    QString getVisibleCancelOrClearText();
    QString getSelectedCancelOrClearText();

public slots:
    void onPauseResumeSelection(bool pauseState);
    void onCancelVisibleTransfers();
    void onCancelSelectedTransfers();
    void onRetryVisibleTransfers();
    void onCancelClearSelection(bool isClear);

signals:
    void verticalScrollBarVisibilityChanged(bool status);
    void pauseResumeTransfersByContextMenu(bool pause);

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
    void openInMEGAClicked();
    void openItemClicked();
    void showInFolderClicked();
    void showInMegaClicked();
    void cancelSelectedClicked();
    void clearSelectedClicked();
    void pauseSelectedClicked();
    void resumeSelectedClicked();
    void onInternalMoveStarted();
    void onInternalMoveFinished();

private:
    friend class TransferManagerDelegateWidget;


    bool mDisableLink;
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
    QAction* mOpenInMEGAAction;
    QAction* mGetLinkAction;
    QAction* mOpenItemAction;
    QAction* mShowInFolderAction;
    QAction* mClearAction;

    void createContextMenu();
    void updateContextMenu(bool enablePause, bool enableResume, bool enableMove, bool enableClear,
                           bool enableCancel, bool isTopIndex, bool isBottomIndex);
    void clearAllTransfers();
    void cancelAllTransfers();

    QModelIndexList getTransfers(bool onlyVisible, TransferData::TransferStates state = TransferData::TRANSFER_NONE);
    QModelIndexList getSelectedTransfers();
    bool isSingleSelectedTransfers();
};

#endif // MEGATRANSFERVIEW_H
