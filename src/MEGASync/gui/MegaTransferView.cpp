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
    showInMEGA = NULL;
    clearCompleted = NULL;
    clearAllCompleted = NULL;
    disableLink = false;
    disableMenus = false;
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
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
}

void MegaTransferView::setup(int type)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    // Disable and find out alternative way to position context menu,
    // since main parent widget is flagged as popup (InfoDialog), and coordinates does not work properly
    // connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onCustomContextMenu(const QPoint &)));
    connect(this, SIGNAL(showContextMenu(const QPoint &)), this, SLOT(onCustomContextMenu(const QPoint &)));
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

void MegaTransferView::disableContextMenus(bool option)
{
    disableMenus = option;
}

void MegaTransferView::createContextMenu()
{
    if (!contextInProgressMenu)
    {
        contextInProgressMenu = new QMenu(this);
        Platform::initMenu(contextInProgressMenu);
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
        Platform::initMenu(contextCompleted);
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

    if (showInMEGA)
    {
        showInMEGA->deleteLater();
        showInMEGA = NULL;
    }

    showInMEGA = new QAction(tr("View on MEGA"), this);
    connect(showInMEGA, SIGNAL(triggered()), this, SLOT(showInMEGAClicked()));


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
    contextCompleted->addAction(showInMEGA);
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

void MegaTransferView::customizeCompletedContextMenu(bool enableGetLink, bool enableOpen, bool enableShow, bool enableShowInMEGA)
{
    getLink->setVisible(enableGetLink);
    openItem->setVisible(enableOpen);
    showInFolder->setVisible(enableShow);
    showInMEGA->setVisible(enableShowInMEGA);
}

void MegaTransferView::mouseMoveEvent(QMouseEvent *event)
{
    QTransfersModel *model = (QTransfersModel*)this->model();
    if (model)
    {
        QModelIndex index = indexAt(event->pos());
        if (index.isValid())
        {
            const int tag = static_cast<int>(index.internalId());
            if (lastItemHoveredTag)
            {
                TransferItem *lastItemHovered = model->transferItems[lastItemHoveredTag];
                if (lastItemHovered)
                {
                    lastItemHovered->mouseHoverTransfer(false, event->pos() - visualRect(index).topLeft());
                }
            }

            TransferItem *item = model->transferItems[tag];
            if (item)
            {
                lastItemHoveredTag = item->getTransferTag();
                item->mouseHoverTransfer(true, event->pos() - visualRect(index).topLeft());
            }
            else
            {
                lastItemHoveredTag = 0;
            }
        }
        else
        {
            if (lastItemHoveredTag)
            {
                TransferItem *lastItemHovered = model->transferItems[lastItemHoveredTag];
                if (lastItemHovered)
                {
                    lastItemHovered->mouseHoverTransfer(false, event->pos() - visualRect(index).topLeft());
                    update();
                }
                lastItemHoveredTag = 0;
            }
        }
    }
    QTreeView::mouseMoveEvent(event);
}

void MegaTransferView::mouseReleaseEvent(QMouseEvent *event)
{
    if (!(event->button() == Qt::RightButton))
    {
        QTreeView::mouseReleaseEvent(event);
        return;
    }

    if (!disableMenus)
    {
        emit showContextMenu(QPoint(event->x(), event->y()));
    }
    QTreeView::mouseReleaseEvent(event);
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
                lastItemHovered->mouseHoverTransfer(false, QPoint(-1,-1));
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

void MegaTransferView::paintEvent(QPaintEvent * e)
{
    auto app = static_cast<MegaApplication*>(qApp);
    app->megaApiLock.reset(app->getMegaApi()->getMegaApiLock(false));
    QTreeView::paintEvent(e);
    app->megaApiLock.reset();
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

            const int internalId = static_cast<int>(indexes[i].internalId());
            transferTagSelected.append(internalId);
            if (!enablePause || !enableResume || !enableCancel)
            {
                TransferItem *item = model->transferItems[internalId];
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
            int modelType = model->getModelType();
            if (modelType == QTransfersModel::TYPE_FINISHED)
            {
                bool failed = false;
                bool linkAvailable = true;
                bool showInMega = true;
                bool showInFolder = true;

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

                        TransferItem *item = model->transferItems[transferTagSelected[i]];

                        if (!item || !item->getIsLinkAvailable())
                        {
                            linkAvailable = false;
                        }

                        const bool transferIsDownloadType{transfer->getType() == MegaTransfer::TYPE_DOWNLOAD};
                        const bool unkownAccess{!item || item->getNodeAccess() == MegaShare::ACCESS_UNKNOWN};
                        if (unkownAccess || transferIsDownloadType)
                        {
                            showInMega = false;
                        }

                        const bool transferIsUploadType{transfer->getType() == MegaTransfer::TYPE_UPLOAD};
                        if(transferIsUploadType)
                        {
                            showInFolder = false;
                        }

                        delete transfer;
                    }
                }

                if (failed)
                {
                    customizeCompletedContextMenu(false, false, false, false);
                }
                else
                {
                    customizeCompletedContextMenu(linkAvailable, true, showInFolder, showInMega);
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
    QPointer<MegaTransferView> view = QPointer<MegaTransferView>(this);
    if (QMegaMessageBox::warning(0,
                             QString::fromUtf8("MEGAsync"),
                             tr("Are you sure you want to cancel this transfer?"),
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes
            || !view)
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
                        QString link = Preferences::BASE_URL + QString::fromUtf8("/#!%1!%2")
                                .arg(QString::fromUtf8(handle)).arg(QString::fromUtf8(key));
                        linkList.append(link);
                    }
                    delete [] handle;
                    delete [] key;
                }
                delete node;
                delete transfer;
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
            delete transfer;
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
            delete transfer;
        }
    }
}

void MegaTransferView::showInMEGAClicked()
{
    MegaTransfer *transfer = NULL;
    QTransfersModel *model = (QTransfersModel*)this->model();
    if (model)
    {
        for (int i = 0; i < transferTagSelected.size(); i++)
        {
            transfer = model->getTransferByTag(transferTagSelected[i]);
            if (transfer)
            {
                MegaHandle handle = transfer->getParentHandle();
                if (handle != INVALID_HANDLE)
                {
                    MegaApplication* app{((MegaApplication *)qApp)};
                    constexpr bool versions{false};
                    app->shellViewOnMega(handle, versions);
                }
                delete transfer;
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
