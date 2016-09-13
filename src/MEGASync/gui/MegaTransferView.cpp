#include "MegaTransferView.h"
#include "MegaApplication.h"
#include "platform/Platform.h"

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#endif

using namespace mega;

MegaTransferView::MegaTransferView(QWidget *parent) :
    QTreeView(parent), last_row(-1)
{
    setMouseTracking(true);
    lastItemHoveredTag = 0;
    contextInProgressMenu = NULL;
    pauseTransfer = NULL;
    moveToTop = NULL;
    moveUp = NULL;
    moveDown = NULL;
    moveToBottom = NULL;
    cancelTransfer = NULL;
    contextCompleted = NULL;
    getLink = NULL;
    openItem = NULL;
    showInFolder = NULL;
    clearCompleted = NULL;
    clearAllCompleted = NULL;
    transferTagSelected = 0;
    transferStateSelected = 0;
    disableLink = false;
}

void MegaTransferView::setup(int type)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onCustomContextMenu(const QPoint &)));
    createContextMenu();
    createCompletedContextMenu();
}

void MegaTransferView::disableGetLink(bool disable)
{
    disableLink = disable;
    getLink->setEnabled(!disable);
}

void MegaTransferView::createContextMenu()
{
    if (!contextInProgressMenu)
    {
        contextInProgressMenu = new QMenu(this);
        contextInProgressMenu->setStyleSheet(QString::fromAscii(
                                   "QMenu {background: #ffffff;}"
                                   "QMenu::item {font-family: Source Sans Pro; margin: 5px 9px 5px 9px; color: #777777; padding: 5px 8px;} "
                                   "QMenu::item:selected {background: #aaaaaa; border: 1px solid #aaaaaa; border-radius: 2px; margin-left: 9px; margin-right: 9px; color: #ffffff; padding: 5px 8px;}"
                                   "QMenu::separator {height: 1px; margin: 6px 0px 6px 0px; background-color: rgba(0, 0, 0, 0.1);}"
                                   "QMenu::item:disabled {background-color: #ffffff; color: rgba(119,119,119,0.3); border: none; margin: 5px 9px 5px 9px;}"));
    }
    else
    {
        QList<QAction *> actions = contextInProgressMenu->actions();
        for (int i = 0; i < actions.size(); i++)
        {
            contextInProgressMenu->removeAction(actions[i]);
        }
    }

    if (pauseTransfer)
    {
        pauseTransfer->deleteLater();
        pauseTransfer = NULL;
    }

    pauseTransfer = new QAction(tr("Pause Transfer"), this);
    connect(pauseTransfer, SIGNAL(triggered()), this, SLOT(pauseTransferClicked()));

    if (moveToTop)
    {
        moveToTop->deleteLater();
        moveToTop = NULL;
    }

    moveToTop = new QAction(tr("Move to top"), this);
    connect(moveToTop, SIGNAL(triggered()), this, SLOT(moveToTopClicked()));

    if (moveUp)
    {
        moveUp->deleteLater();
        moveUp = NULL;
    }

    moveUp = new QAction(tr("Move up"), this);
    connect(moveUp, SIGNAL(triggered()), this, SLOT(moveUpClicked()));

    if (moveDown)
    {
        moveDown->deleteLater();
        moveDown = NULL;
    }

    moveDown = new QAction(tr("Move down"), this);
    connect(moveDown, SIGNAL(triggered()), this, SLOT(moveDownClicked()));

    if (moveToBottom)
    {
        moveToBottom->deleteLater();
        moveToBottom = NULL;
    }

    moveToBottom = new QAction(tr("Move to bottom"), this);
    connect(moveToBottom, SIGNAL(triggered()), this, SLOT(moveToBottomClicked()));

    if (cancelTransfer)
    {
        cancelTransfer->deleteLater();
        cancelTransfer = NULL;
    }

    cancelTransfer = new QAction(tr("Cancel"), this);
    connect(cancelTransfer, SIGNAL(triggered()), this, SLOT(cancelTransferClicked()));

    contextInProgressMenu->addAction(pauseTransfer);
    contextInProgressMenu->addSeparator();
    contextInProgressMenu->addAction(moveToTop);
    contextInProgressMenu->addAction(moveUp);
    contextInProgressMenu->addAction(moveDown);
    contextInProgressMenu->addAction(moveToBottom);
    contextInProgressMenu->addSeparator();
    contextInProgressMenu->addAction(cancelTransfer);
}

void MegaTransferView::createCompletedContextMenu()
{
    if (!contextCompleted)
    {
        contextCompleted = new QMenu(this);
        contextCompleted->setStyleSheet(QString::fromAscii(
                                   "QMenu {background: #ffffff;}"
                                   "QMenu::item {font-family: Source Sans Pro; margin-left: 8px; margin-right: 8px; color: #777777; padding: 5px 8px;} "
                                   "QMenu::item:selected {background: #aaaaaa; border: 1px solid #aaaaaa; border-radius: 2px; margin-left: 7px; margin-right: 7px; color: #ffffff; padding: 5px 8px;}"));
    }
    else
    {
        QList<QAction *> actions = contextCompleted->actions();
        for (int i = 0; i < actions.size(); i++)
        {
            contextCompleted->removeAction(actions[i]);
        }
    }

    if (getLink)
    {
        getLink->deleteLater();
        getLink = NULL;
    }

    getLink = new QAction(tr("Get MEGA link"), this);
    connect(getLink, SIGNAL(triggered()), this, SLOT(getLinkClicked()));

    if (openItem)
    {
        openItem->deleteLater();
        openItem = NULL;
    }

    openItem = new QAction(tr("Open"), this);
    connect(openItem, SIGNAL(triggered()), this, SLOT(openItemClicked()));

    if (showInFolder)
    {
        showInFolder->deleteLater();
        showInFolder = NULL;
    }

    showInFolder = new QAction(tr("Show in folder"), this);
    connect(showInFolder, SIGNAL(triggered()), this, SLOT(showInFolderClicked()));

    if (clearCompleted)
    {
        clearCompleted->deleteLater();
        clearCompleted = NULL;
    }

    clearCompleted = new QAction(tr("Clear"), this);
    connect(clearCompleted, SIGNAL(triggered()), this, SLOT(clearTransferClicked()));

    if (clearAllCompleted)
    {
        clearAllCompleted->deleteLater();
        clearAllCompleted = NULL;
    }

    clearAllCompleted = new QAction(tr("Clear All"), this);
    connect(clearAllCompleted, SIGNAL(triggered()), this, SLOT(clearAllTransferClicked()));

    contextCompleted->addAction(getLink);
    contextCompleted->addAction(openItem);
    contextCompleted->addAction(showInFolder);
    contextCompleted->addSeparator();
    contextCompleted->addAction(clearCompleted);
    contextCompleted->addAction(clearAllCompleted);
}

void MegaTransferView::customizeContextInProgressMenu(bool paused, bool enableUpMoves, bool enableDownMoves, bool isCancellable)
{
    paused ?  pauseTransfer->setText(tr("Resume Transfer")) :  pauseTransfer->setText(tr("Pause Transfer"));
    moveToTop->setEnabled(enableUpMoves);
    moveUp->setEnabled(enableUpMoves);
    moveToBottom->setEnabled(enableDownMoves);
    moveDown->setEnabled(enableDownMoves);
    cancelTransfer->setEnabled(isCancellable);
}

void MegaTransferView::mouseMoveEvent(QMouseEvent *event)
{
    QTransfersModel *model = (QTransfersModel*)this->model();
    if (model)
    {
        QModelIndex index = indexAt(event->pos());
        int tag = index.internalId();

        if (index.isValid())
        {
             if (index.row() != last_row)
             {
                last_row = index.row();
                if (lastItemHoveredTag)
                {
                    TransferItem *lastItemHovered = model->transferItems[lastItemHoveredTag];
                    if (lastItemHovered)
                    {
                        lastItemHovered->mouseHoverTransfer(false);
                    }
                }

                TransferItem *item = model->transferItems[tag];
                if (item)
                {
                    lastItemHoveredTag = item->getTransferTag();
                    item->mouseHoverTransfer(true);
                }
             }
        }
        else
        {
            if (lastItemHoveredTag)
            {
                TransferItem *lastItemHovered = model->transferItems[lastItemHoveredTag];
                if (lastItemHovered)
                {
                    lastItemHovered->mouseHoverTransfer(false);
                    last_row = -1;
                    lastItemHoveredTag = 0;
                }
            }
        }
    }
    QTreeView::mouseMoveEvent(event);
}

void MegaTransferView::onCustomContextMenu(const QPoint &point)
{
    QTransfersModel *model = (QTransfersModel*)this->model();
    if (model)
    {
        QModelIndex index = indexAt(point);
        int tag = index.internalId();
        TransferItem *item = model->transferItems[tag];
        if (index.isValid() && item)
        {
            transferTagSelected = tag;
            if (model->getModelType() == QTransfersModel::TYPE_FINISHED)
            {
                contextCompleted->exec(mapToGlobal(point));
            }
            else if (model->getModelType() == QTransfersModel::TYPE_ALL)
            {
                transferStateSelected = item->getTransferState();
                bool isRegularTransfer = item->getRegular();
                customizeContextInProgressMenu(transferStateSelected == mega::MegaTransfer::STATE_PAUSED,
                                               false,
                                               false,
                                               isRegularTransfer);
                contextInProgressMenu->exec(mapToGlobal(point));
            }
            else
            {
                transferStateSelected = item->getTransferState();
                bool isRegularTransfer = item->getRegular();
                customizeContextInProgressMenu(transferStateSelected == mega::MegaTransfer::STATE_PAUSED,
                                               index.row() > 0,
                                               (model->rowCount(QModelIndex()) - 1) > index.row(),
                                               isRegularTransfer);
                contextInProgressMenu->exec(mapToGlobal(point));
            }
            transferTagSelected = 0;
            transferStateSelected = 0;
        }
    }
}

void MegaTransferView::pauseTransferClicked()
{
    QTransfersModel *model = (QTransfersModel*)this->model();
    if (model)
    {
        if (transferTagSelected)
        {
            model->onTransferPaused(transferTagSelected, !(transferStateSelected == mega::MegaTransfer::STATE_PAUSED));
        }
    }
}

void MegaTransferView::moveToTopClicked()
{
    QTransfersModel *model = (QTransfersModel*)this->model();
    if (model)
    {
        if (transferTagSelected)
        {
            model->onMoveTransferToFirst(transferTagSelected);
        }
    }
}

void MegaTransferView::moveUpClicked()
{
    QTransfersModel *model = (QTransfersModel*)this->model();
    if (model)
    {
        if (transferTagSelected)
        {
            model->onMoveTransferUp(transferTagSelected);
        }
    }
}

void MegaTransferView::moveDownClicked()
{
    QTransfersModel *model = (QTransfersModel*)this->model();
    if (model)
    {
        if (transferTagSelected)
        {
            model->onMoveTransferDown(transferTagSelected);
        }
    }
}

void MegaTransferView::moveToBottomClicked()
{
    QTransfersModel *model = (QTransfersModel*)this->model();
    if (model)
    {
        if (transferTagSelected)
        {
            model->onMoveTransferToLast(transferTagSelected);
        }
    }
}

void MegaTransferView::cancelTransferClicked()
{
    if(QMessageBox::question(this,
                             QString::fromUtf8("MEGAsync"),
                             tr("Are you sure you want to cancel this transfer?"),
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
    {
        return;
    }

    QTransfersModel *model = (QTransfersModel*)this->model();
    if (model)
    {
        if (transferTagSelected)
        {
            model->onTransferCancel(transferTagSelected);
        }
    }
}

void MegaTransferView::getLinkClicked()
{
    if (disableLink)
    {
        return;
    }

    MegaTransfer *transfer = NULL;
    QTransfersModel *model = (QTransfersModel*)this->model();
    if (model)
    {
        if (transferTagSelected)
        {
            transfer = model->getFinishedTransferByTag(transferTagSelected);
            if (transfer->getNodeHandle() != mega::INVALID_HANDLE)
            {
                ((MegaApplication*)qApp)->copyFileLink(transfer->getNodeHandle(), QString());
            }
        }
    }
}

void MegaTransferView::openItemClicked()
{
    MegaTransfer *transfer = NULL;
    QTransfersModel *model = (QTransfersModel*)this->model();
    if (model)
    {
        if (transferTagSelected)
        {
            transfer = model->getFinishedTransferByTag(transferTagSelected);
            if (transfer->getPath())
            {
                QtConcurrent::run(QDesktopServices::openUrl, QUrl::fromLocalFile(QString::fromUtf8(transfer->getPath())));
            }
        }
    }
}

void MegaTransferView::showInFolderClicked()
{
    MegaTransfer *transfer = NULL;
    QTransfersModel *model = (QTransfersModel*)this->model();
    if (model)
    {
        if (transferTagSelected)
        {
            transfer = model->getFinishedTransferByTag(transferTagSelected);
            if (transfer->getPath())
            {
                Platform::showInFolder(QString::fromUtf8(transfer->getPath()));
            }
        }
    }
}

void MegaTransferView::clearTransferClicked()
{
    QTransfersModel *model = (QTransfersModel*)this->model();
    if (model)
    {
        if (transferTagSelected)
        {
            model->removeTransferByTag(transferTagSelected);
        }
    }
}

void MegaTransferView::clearAllTransferClicked()
{
    QTransfersModel *model = (QTransfersModel*)this->model();
    if (model)
    {
        if (transferTagSelected)
        {
            model->removeAllTransfers();
        }
    }
}
