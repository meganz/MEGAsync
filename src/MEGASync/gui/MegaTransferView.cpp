#include "MegaTransferView.h"
#include "MegaApplication.h"
#include "platform/Platform.h"
#include "control/Utilities.h"
#include "gui/QMegaMessageBox.h"
#include <QScrollBar>

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#endif

using namespace mega;

MegaTransferView::MegaTransferView(QWidget *parent) :
    QTreeView(parent)
{
    setMouseTracking(true);
    lastItemHoveredTag = 0;
    contextInProgressMenu = NULL;
    pauseTransfer = NULL;
    resumeTransfer = NULL;
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
    disableLink = false;
    type = 0;

    verticalScrollBar()->setStyleSheet(
                QString::fromUtf8("QScrollBar:vertical {"
                           " background: #f6f6f6;"
                           " width: 15px;"
                           " border-left: 1px solid #E4E4E4;"
                          "}"
                          "QScrollBar::sub-line:vertical, QScrollBar::add-line:vertical {"
                           " border: none;"
                           " background: none;"
                          "}"
                          "QScrollBar::handle:vertical {"
                           " background: #c0c0c0;"
                           " min-height: 20px;"
                           " border-radius: 4px;"
#ifdef Q_OS_MACX
                           " margin: 3px 3px 3px 3px;"
#else
                           " margin: 3px 4px 3px 2px;"
#endif
                          "}"
                 ""));
}

void MegaTransferView::setup(int type)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onCustomContextMenu(const QPoint &)));
    createContextMenu();
    createCompletedContextMenu();
    this->type = type;
}

void MegaTransferView::disableGetLink(bool disable)
{
    disableLink = disable;
    getLink->setEnabled(!disable);
}

int MegaTransferView::getType() const
{
    return type;
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

    if (resumeTransfer)
    {
        resumeTransfer->deleteLater();
        resumeTransfer = NULL;
    }

    resumeTransfer = new QAction(tr("Resume Transfer"), this);
    connect(resumeTransfer, SIGNAL(triggered()), this, SLOT(resumeTransferClicked()));

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
    contextInProgressMenu->addAction(resumeTransfer);
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

void MegaTransferView::customizeContextInProgressMenu(bool enablePause, bool enableResume, bool enableUpMoves, bool enableDownMoves, bool isCancellable)
{
    pauseTransfer->setVisible(enablePause);
    resumeTransfer->setVisible(enableResume);
    moveToTop->setVisible(enableUpMoves);
    moveUp->setVisible(enableUpMoves);
    moveToBottom->setVisible(enableDownMoves);
    moveDown->setVisible(enableDownMoves);
    cancelTransfer->setVisible(isCancellable);
}

void MegaTransferView::customizeCompletedContextMenu(bool enableGetLink, bool enableOpen, bool enableShow)
{
    getLink->setVisible(enableGetLink);
    openItem->setVisible(enableOpen);
    showInFolder->setVisible(enableShow);
}

void MegaTransferView::mouseMoveEvent(QMouseEvent *event)
{
    QTransfersModel *model = (QTransfersModel*)this->model();
    if (model)
    {
        QModelIndex index = indexAt(event->pos());
        if (index.isValid())
        {
            int tag = index.internalId();
            if (tag != lastItemHoveredTag)
            {
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
                else
                {
                    lastItemHoveredTag = 0;
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
                    update();
                }
                lastItemHoveredTag = 0;
            }
        }
    }
    QTreeView::mouseMoveEvent(event);
}

void MegaTransferView::leaveEvent(QEvent *event)
{
    QTransfersModel *model = (QTransfersModel*)this->model();
    if (model)
    {
        if (lastItemHoveredTag)
        {
            TransferItem *lastItemHovered = model->transferItems[lastItemHoveredTag];
            if (lastItemHovered)
            {
                lastItemHovered->mouseHoverTransfer(false);
                update();
            }
            lastItemHoveredTag = 0;
        }
    }
    QTreeView::leaveEvent(event);
}

void MegaTransferView::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        createContextMenu();
        createCompletedContextMenu();
    }
    QWidget::changeEvent(event);
}

void MegaTransferView::onCustomContextMenu(const QPoint &point)
{
    QTransfersModel *model = (QTransfersModel*)this->model();
    if (model)
    {
        bool enablePause = false;
        bool enableResume = false;
        bool enableCancel = false;
        int firstIndex = 0;
        int lastIndex = 0;
        QModelIndexList indexes = selectedIndexes();
        transferTagSelected.clear();
        for (int i = 0; i< indexes.size(); i++)
        {
            if (i == 0)
            {
                firstIndex = indexes[i].row();
            }
            if (i == indexes.size() - 1)
            {
                lastIndex = indexes[i].row();
            }

            transferTagSelected.append(indexes[i].internalId());
            if (!enablePause || !enableResume || !enableCancel)
            {
                TransferItem *item = model->transferItems[indexes[i].internalId()];
                if (!item)
                {
                    enableResume = true;
                    enablePause = true;
                    enableCancel = true;
                }
                else
                {
                    if (item->getRegular())
                    {
                        enableCancel = true;
                    }

                    if (item->getTransferState() == mega::MegaTransfer::STATE_PAUSED)
                    {
                        enableResume = true;
                    }
                    else
                    {
                        enablePause = true;
                    }
                }
            }
        }

        if (transferTagSelected.size())
        {
            if (model->getModelType() == QTransfersModel::TYPE_FINISHED)
            {
                bool failed = false;
                MegaTransfer *transfer = NULL;
                QTransfersModel *model = (QTransfersModel*)this->model();
                if (model)
                {
                    for (int i = 0; i < transferTagSelected.size(); i++)
                    {
                        transfer = model->getTransferByTag(transferTagSelected[i]);
                        if (!transfer)
                        {
                            transferTagSelected.clear();
                            return;
                        }

                        if (transfer->getState() == MegaTransfer::STATE_FAILED)
                        {
                            failed = true;
                        }
                    }
                }

                if (failed)
                {
                    customizeCompletedContextMenu(false, false, false);
                }
                else
                {
                    customizeCompletedContextMenu();
                }
                contextCompleted->exec(mapToGlobal(point));
            }
            else
            {
                customizeContextInProgressMenu(enablePause,
                                               enableResume,
                                               firstIndex > 0,
                                               (model->rowCount(QModelIndex()) - 1) > lastIndex,
                                               enableCancel);
                contextInProgressMenu->exec(mapToGlobal(point));
            }
        }
    }
}

void MegaTransferView::pauseTransferClicked()
{
    QTransfersModel *model = (QTransfersModel*)this->model();
    if (model)
    {
        for (int i = 0; i < transferTagSelected.size(); i++)
        {
            model->megaApi->pauseTransferByTag(transferTagSelected[i], true);
        }
    }
}

void MegaTransferView::resumeTransferClicked()
{
    QTransfersModel *model = (QTransfersModel*)this->model();
    if (model)
    {
        for (int i = 0; i < transferTagSelected.size(); i++)
        {
            model->megaApi->pauseTransferByTag(transferTagSelected[i], false);
        }
    }
}

void MegaTransferView::moveToTopClicked()
{
    QTransfersModel *model = (QTransfersModel*)this->model();
    if (model)
    {
        for (int i = transferTagSelected.size() - 1; i >= 0; i--)
        {
            model->megaApi->moveTransferToFirstByTag(transferTagSelected[i]);
        }
    }
}

void MegaTransferView::moveUpClicked()
{
    QTransfersModel *model = (QTransfersModel*)this->model();
    if (model)
    {
        for (int i = 0; i < transferTagSelected.size(); i++)
        {
            model->megaApi->moveTransferUpByTag(transferTagSelected[i]);
        }
    }
}

void MegaTransferView::moveDownClicked()
{
    QTransfersModel *model = (QTransfersModel*)this->model();
    if (model)
    {
        for (int i = transferTagSelected.size() - 1; i >= 0 ; i--)
        {
            model->megaApi->moveTransferDownByTag(transferTagSelected[i]);
        }
    }
}

void MegaTransferView::moveToBottomClicked()
{
    QTransfersModel *model = (QTransfersModel*)this->model();
    if (model)
    {
        for (int i = 0; i < transferTagSelected.size(); i++)
        {
            model->megaApi->moveTransferToLastByTag(transferTagSelected[i]);
        }
    }
}

void MegaTransferView::cancelTransferClicked()
{
    if (QMegaMessageBox::warning(0,
                             QString::fromUtf8("MEGAsync"),
                             tr("Are you sure you want to cancel this transfer?"), Utilities::getDevicePixelRatio(),
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
    {
        return;
    }

    QTransfersModel *model = (QTransfersModel*)this->model();
    if (model)
    {
        for (int i = 0; i < transferTagSelected.size(); i++)
        {
            model->megaApi->cancelTransferByTag(transferTagSelected[i]);
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
        QList<MegaHandle> exportList;
        QStringList linkList;
        for (int i = 0; i < transferTagSelected.size(); i++)
        {
            transfer = model->getTransferByTag(transferTagSelected[i]);
            if (transfer)
            {
                MegaNode *node = transfer->getPublicMegaNode();
                if (!node || !node->isPublic())
                {
                    exportList.push_back(transfer->getNodeHandle());
                }
                else
                {
                    char *handle = node->getBase64Handle();
                    char *key = node->getBase64Key();
                    if (handle && key)
                    {
                        QString link = QString::fromUtf8("https://mega.nz/#!%1!%2")
                                .arg(QString::fromUtf8(handle)).arg(QString::fromUtf8(key));
                        linkList.append(link);
                    }
                    delete [] handle;
                    delete [] key;
                }
                delete node;
            }
        }

        if (exportList.size() || linkList.size())
        {
            ((MegaApplication*)qApp)->exportNodes(exportList, linkList);
        }
    }
}

void MegaTransferView::openItemClicked()
{
    MegaTransfer *transfer = NULL;
    QTransfersModel *model = (QTransfersModel*)this->model();
    if (model)
    {
        for (int i = 0; i < transferTagSelected.size(); i++)
        {
            transfer = model->getTransferByTag(transferTagSelected[i]);
            if (transfer && transfer->getPath())
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
        for (int i = 0; i < transferTagSelected.size(); i++)
        {
            transfer = model->getTransferByTag(transferTagSelected[i]);
            if (transfer && transfer->getPath())
            {
                QString localPath = QString::fromUtf8(transfer->getPath());
                #ifdef WIN32
                if (localPath.startsWith(QString::fromAscii("\\\\?\\")))
                {
                    localPath = localPath.mid(4);
                }
                #endif
                Platform::showInFolder(localPath);
            }
        }
    }
}

void MegaTransferView::clearTransferClicked()
{
    QTransfersModel *model = (QTransfersModel*)this->model();
    if (model)
    {
        for (int i = 0; i < transferTagSelected.size(); i++)
        {
            model->removeTransferByTag(transferTagSelected[i]);
        }
    }
}

void MegaTransferView::clearAllTransferClicked()
{
    QTransfersModel *model = (QTransfersModel*)this->model();
    if (model)
    {
        model->removeAllTransfers();
    }
}
