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
    clearTransfer = NULL;
    contextCompleted = NULL;
    clearCompleted = NULL;
    clearAllCompleted = NULL;

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
                                   "QMenu::item:selected {background: #aaaaaa; border: 1px solid #aaaaaa; border-radius: 2px; margin-left: 7px; margin-right: 7px; color: #ffffff; padding: 5px 8px;}"));
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

    if (clearTransfer)
    {
        clearTransfer->deleteLater();
        clearTransfer = NULL;
    }

    clearTransfer = new QAction(tr("Clear"), this);
    connect(clearTransfer, SIGNAL(triggered()), this, SLOT(clearTransferClicked()));

    contextInProgressMenu->addAction(pauseTransfer);
    contextInProgressMenu->addSeparator();
    contextInProgressMenu->addAction(moveToTop);
    contextInProgressMenu->addAction(moveUp);
    contextInProgressMenu->addAction(moveDown);
    contextInProgressMenu->addAction(moveToBottom);
    contextInProgressMenu->addSeparator();
    contextInProgressMenu->addAction(clearTransfer);
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

    if (clearAllCompleted)
    {
        clearAllCompleted->deleteLater();
        clearAllCompleted = NULL;
    }

    clearAllCompleted = new QAction(tr("Clear All"), this);

    contextCompleted->addAction(clearCompleted);
    contextCompleted->addAction(clearAllCompleted);
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
            if (model->getModelType() == QTransfersModel::TYPE_FINISHED)
            {
                contextCompleted->exec(mapToGlobal(point));
            }
            else
            {
                contextInProgressMenu->exec(mapToGlobal(point));
            }
        }
    }
}

void MegaTransferView::pauseTransferClicked()
{
    QModelIndex index = currentIndex();
    if (index.isValid())
    {

    }

}

void MegaTransferView::moveToTopClicked()
{
    QModelIndex index = currentIndex();
    if (index.isValid())
    {

    }

}

void MegaTransferView::moveUpClicked()
{
    QModelIndex index = currentIndex();
    if (index.isValid())
    {

    }

}

void MegaTransferView::moveDownClicked()
{
    QModelIndex index = currentIndex();
    if (index.isValid())
    {

    }

}

void MegaTransferView::moveToBottomClicked()
{
    QModelIndex index = currentIndex();
    if (index.isValid())
    {

    }

}

void MegaTransferView::clearTransferClicked()
{
    QModelIndex index = currentIndex();
    if (index.isValid())
    {

    }

}
