#include "MegaTransferView.h"

MegaTransferView::MegaTransferView(QWidget *parent) :
    QTreeView(parent), last_row(-1)
{
    setMouseTracking(true);
    lastItemHovered = NULL;
    contextInProgressMenu = NULL;
    pauseTransfer = NULL;
    moveToTop = NULL;
    moveUp = NULL;
    moveDown = NULL;
    moveToBottom = NULL;
    cancelTransfer = NULL;
    contextCompleted = NULL;
    clearCompleted = NULL;
    clearAllCompleted = NULL;
    transferTagSelected = 0;
    transferStateSelected = 0;

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onCustomContextMenu(const QPoint &)));
    createContextMenu();
    createCompletedContextMenu();
}

void MegaTransferView::createContextMenu()
{
    if (!contextInProgressMenu)
    {
        contextInProgressMenu = new QMenu(this);
        contextInProgressMenu->setStyleSheet(QString::fromAscii(
                                   "QMenu {background: #ffffff;}"
                                   "QMenu::item {font-family: Source Sans Pro; margin-left: 8px; margin-right: 8px; color: #777777; padding: 5px 8px;} "
                                   "QMenu::item:selected {background: #aaaaaa; border: 1px solid #aaaaaa; border-radius: 2px; margin-left: 7px; margin-right: 7px; color: #ffffff; padding: 5px 8px;}"
                                   "QMenu::item:disabled {background-color: #ffffff; color: rgba(119,119,119,0.3); border: none; margin-left: 7px; margin-right: 7px;}"));
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
    QAbstractItemModel *model = this->model();
    if (model)
    {
        QModelIndex index = indexAt(event->pos());
        if (index.isValid())
        {
             if (index.row() != last_row)
             {
                last_row = index.row();
                if (lastItemHovered)
                {
                    lastItemHovered->mouseHoverTransfer(false);
                }
                TransferItem *ti = qvariant_cast<TransferItem*>(index.data());
                if (ti)
                {
                    lastItemHovered = ti;
                    ti->mouseHoverTransfer(true);
                }
             }
        }
        else
        {
            if (lastItemHovered)
            {
                lastItemHovered->mouseHoverTransfer(false);
                last_row = -1;
                lastItemHovered = NULL;
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
        if (index.isValid())
        {
            transferTagSelected = model->data(index, TagRole).toInt();

            if (model->getModelType() == QTransfersModel::TYPE_FINISHED)
            {
                contextCompleted->exec(mapToGlobal(point));
            }
            else
            {
                transferStateSelected = model->data(index, TransferStatusRole).toInt();
                bool isRegularTransfer = model->data(index, IsRegularTransferRole).toBool();
                customizeContextInProgressMenu(transferStateSelected == mega::MegaTransfer::STATE_PAUSED, model->rowCount(QModelIndex()) > 1, model->rowCount(QModelIndex()) > 1, isRegularTransfer);
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
    QTransfersModel *model = (QTransfersModel*)this->model();
    if (model)
    {
        if (transferTagSelected)
        {
            model->onTransferCancel(transferTagSelected);
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
