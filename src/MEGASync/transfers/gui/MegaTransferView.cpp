#include "MegaTransferView.h"
#include "MegaApplication.h"
#include "platform/Platform.h"
#include "control/Utilities.h"
#include "gui/QMegaMessageBox.h"
#include "TransfersWidget.h"

#include <QScrollBar>
#include <QtConcurrent/QtConcurrent>
#include <QIcon>

using namespace mega;

const int MAX_ITEMS_FOR_CONTEXT_MENU = 10;

QString MegaTransferView::cancelAllAskActionText()
{
    return tr("Cancel transfers?\n"
              "All your transfers will be cancelled.");
}

QString MegaTransferView::cancelAndClearAskActionText()
{
    return tr("Cancel transfers?\n"
              "All your transfers in this category will be cancelled and cleared.");
}

QString MegaTransferView::cancelAskActionText()
{
    return tr("Cancel transfers?\n"
              "All your transfers in this category will be cancelled.");
}

QString MegaTransferView::cancelWithSyncAskActionText()
{
    return tr("Cancel transfers?\n"
              "Your incomplete sync transfers won't be cancelled.");
}

QString MegaTransferView::cancelAndClearWithSyncAskActionText()
{
    return tr("Cancel transfers?\n"
              "Your incomplete sync transfers won't be cancelled\n"
              "All the other transfers will be cancelled and cleared.");
}

QString MegaTransferView::clearAllCompletedAskActionText()
{
    return tr("Clear transfers?\n"
              "All your completed transfers will be cleared.");
}

QString MegaTransferView::clearCompletedAskActionText()
{
    return tr("Clear transfers?\n"
              "All your completed transfers in this category will be cleared.");
}

//Multiple seletion
QString MegaTransferView::cancelSelectedAskActionText()
{
    return tr("Cancel transfers?\n"
              "All your selected transfers will be cancelled.");
}

QString MegaTransferView::cancelAndClearSelectedAskActionText()
{
    return tr("Cancel transfers?\n"
              "All your selected transfers will be cancelled and cleared.");
}

QString MegaTransferView::cancelSelectedWithSyncAskActionText()
{
    return tr("Cancel transfers?\n"
              "Your selected incomplete sync transfers won't be cancelled.");
}

QString MegaTransferView::cancelAndClearSelectedWithSyncAskActionText()
{
    return tr("Cancel transfers?\n"
              "Your selected incomplete sync transfers won't be cancelled\n"
              "All the other selected transfers will be cancelled and cleared.");
}

QString MegaTransferView::clearSelectedCompletedAskActionText()
{
    return tr("Clear transfers?\n"
              "All the selected completed transfers in this category will be cleared.");
}

//Single seletion
QString MegaTransferView::cancelSingleActionText()
{
    return (tr("Cancel transfer?"));
}

QString MegaTransferView::clearSingleActionText()
{
    return (tr("Clear transfer?"));
}

//Static messages for context menu
QString MegaTransferView::pauseActionText(int count)
{
    return tr("Pause transfer" , "", count);
}

QString MegaTransferView::resumeActionText(int count)
{
    return tr("Resume transfer" , "", count);
}

QString MegaTransferView::cancelActionText(int count)
{
    return tr("Cancel transfer" , "", count);
}

QString MegaTransferView::clearActionText(int count)
{
    return tr("Clear transfer" , "", count);
}

QString MegaTransferView::cancelAndClearActionText(int count)
{
    return tr("Cancel and clear transfer" , "", count);
}

QMap<QMessageBox::StandardButton, QString> MegaTransferView::getCancelDialogButtons()
{
    return  QMap<QMessageBox::StandardButton, QString>{{QMessageBox::Yes, tr("Yes, cancel")}, {QMessageBox::No, tr("No, continue")}};
}

QMap<QMessageBox::StandardButton, QString> MegaTransferView::getClearDialogButtons()
{
    return  QMap<QMessageBox::StandardButton, QString>{{QMessageBox::Yes,tr("Yes, clear")}, {QMessageBox::No, tr("No, continue")}};
}

//Mega transfer view
MegaTransferView::MegaTransferView(QWidget* parent) :
    QTreeView(parent),
    mDisableLink(false),
    mKeyNavigation(false),
    mParentTransferWidget(nullptr)
{
    setMouseTracking(true);
    setAutoScroll(false);

    verticalScrollBar()->installEventFilter(this);

    connect(&mOpenUrlWatcher, &QFutureWatcher<bool>::finished, this, &MegaTransferView::onOpenUrlFinished);
}

void MegaTransferView::setup()
{
    setContextMenuPolicy(Qt::CustomContextMenu);
}

void MegaTransferView::setup(TransfersWidget* tw)
{
    mParentTransferWidget = tw;

    connect(MegaSyncApp->getTransfersModel(), &TransfersModel::internalMoveStarted, this, &MegaTransferView::onInternalMoveStarted);
    connect(MegaSyncApp->getTransfersModel(), &TransfersModel::internalMoveFinished, this, &MegaTransferView::onInternalMoveFinished);
}

QModelIndexList MegaTransferView::getTransfers(bool onlyVisible, TransferData::TransferStates state)
{
    QModelIndexList indexes;

    auto proxy(qobject_cast<QSortFilterProxyModel*>(model()));
    if(proxy)
    {
        auto sourceModel = MegaSyncApp->getTransfersModel();

        auto rowCount = onlyVisible ? proxy->rowCount(QModelIndex()) : sourceModel->rowCount(QModelIndex());

        for (auto row (0); row < rowCount; ++row)
        {
            auto index (model()->index(row, 0, QModelIndex()));
            auto d (qvariant_cast<TransferItem>(index.data()).getTransferData());
            if(state == TransferData::TRANSFER_NONE || (d && d->getState() & state))
            {
                if (proxy)
                {
                    index = proxy->mapToSource(index);
                }
                indexes.push_back(index);
            }
        }
    }

    return indexes;
}

QModelIndexList MegaTransferView::getSelectedTransfers()
{
    QModelIndexList indexes;

    if(!selectionModel())
    {
        return indexes;
    }

    auto selection = selectionModel()->selection();

    if (selection.size() > 0)
    {
        auto proxy(qobject_cast<QSortFilterProxyModel*>(model()));
        if (proxy)
        {
            selection = proxy->mapSelectionToSource(selection);
        }
        indexes = selection.indexes();
    }

    return indexes;
}

MegaTransferView::SelectedIndexesInfo MegaTransferView::getVisibleCancelOrClearInfo()
{
    SelectedIndexesInfo info;

    auto proxy (qobject_cast<TransfersManagerSortFilterProxyModel*>(model()));

    if(proxy)
    {
        info.isAnyCancellable = proxy->isAnyCancellable();
        info.areAllCancellable = proxy->areAllCancellable();
        info.areAllSync = proxy->areAllSync();
        auto isAnySync = proxy->isAnySync();
        auto isAnyCompleted = proxy->isAnyCompleted();

        if(info.isAnyCancellable)
        {
            info.buttonsText = getCancelDialogButtons();

            if(!isAnySync)
            {
                if(info.areAllCancellable || !isAnyCompleted)
                {
                    info.actionText = cancelAskActionText();
                }
                else
                {
                    info.actionText = cancelAndClearAskActionText();
                }
            }
            else
            {
                if(isAnyCompleted)
                {
                    info.actionText = cancelAndClearWithSyncAskActionText();
                }
                else
                {
                    info.actionText = cancelWithSyncAskActionText();
                }
            }
        }
        else
        {
            info.actionText = clearCompletedAskActionText();
            info.buttonsText = getClearDialogButtons();
        }
    }

    return info;
}

MegaTransferView::SelectedIndexesInfo MegaTransferView::getSelectedCancelOrClearInfo()
{
    auto indexes = getSelectedTransfers();

    SelectedIndexesInfo info;
    bool isAnyActiveSync(false);
    bool isAnyCompleted(false);

    foreach(auto& index, indexes)
    {
        auto transfer (qvariant_cast<TransferItem>(index.data()).getTransferData());
        if(transfer)
        {
            if(!transfer->isSyncTransfer())
            {
                info.areAllSync = false;

                if(transfer->isActiveOrPending() || transfer->isFailed())
                {
                    info.isAnyCancellable = true;
                }
                else
                {
                    info.areAllCancellable = false;
                }
            }
            else if(!transfer->isCompleted())
            {
                isAnyActiveSync = true;
                info.areAllCancellable = false;
            }

            if(transfer->isCompleted())
            {
                isAnyCompleted = true;
            }
        }
    }


    if(indexes.size() > 1)
    {
        info.buttonsText = getCancelDialogButtons();
        if(isAnyActiveSync)
        {
            if(isAnyCompleted)
            {
                info.actionText = info.isAnyCancellable ? cancelAndClearSelectedWithSyncAskActionText() : clearSelectedCompletedAskActionText();
            }
            else if(info.isAnyCancellable)
            {
                info.actionText = cancelSelectedWithSyncAskActionText();
            }

        }
        else
        {
            if(isAnyCompleted)
            {
                if(info.isAnyCancellable)
                {
                    info.actionText = cancelAndClearSelectedAskActionText();
                }
                else
                {
                    info.actionText = clearSelectedCompletedAskActionText();
                    info.buttonsText = getClearDialogButtons();
                }
            }
            else if(info.isAnyCancellable)
            {
                info.actionText = cancelSelectedAskActionText();
            }
        }
    }
    else
    {
        if(info.isAnyCancellable)
        {
            info.actionText = cancelSingleActionText();
        }
        else
        {
            info.actionText = clearSingleActionText();
        }
    }


    return info;
}

void MegaTransferView::onPauseResumeVisibleRows(bool pauseState)
{
    QModelIndexList indexes = getTransfers(true);

    auto sourceModel = MegaSyncApp->getTransfersModel();
    sourceModel->pauseTransfers(indexes, pauseState);

    //Use to repaint and update the transfers state
    update();
}

void MegaTransferView::onPauseResumeSelection(bool pauseState)
{
    QModelIndexList indexes = getSelectedTransfers();
    auto sourceModel = MegaSyncApp->getTransfersModel();

    sourceModel->pauseTransfers(indexes, pauseState);

    //Use to repaint and update the transfers state
    update();
}

void MegaTransferView::onCancelVisibleTransfers()
{
    QPointer<MegaTransferView> dialog = QPointer<MegaTransferView>(this);

    auto info = getVisibleCancelOrClearInfo();

    if(!info.areAllSync)
    {
        if (QMegaMessageBox::warning(this, QString::fromUtf8("MEGAsync"),
                                     info.actionText,
                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::No, info.buttonsText)
                == QMessageBox::Yes
                && dialog)
        {
            auto indexes = getTransfers(true);

            auto sourceModel = MegaSyncApp->getTransfersModel();
            sourceModel->cancelAndClearTransfers(indexes, this);
        }
    }
}

void MegaTransferView::onCancelSelectedTransfers()
{
    auto info = getSelectedCancelOrClearInfo();
    QPointer<MegaTransferView> dialog = QPointer<MegaTransferView>(this);

    if (QMegaMessageBox::warning(this, QString::fromUtf8("MEGAsync"),
                                 info.actionText,
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::No, info.buttonsText)
            == QMessageBox::Yes
            && dialog)
    {
        QModelIndexList indexes = getSelectedTransfers();

        auto sourceModel = MegaSyncApp->getTransfersModel();
        sourceModel->cancelAndClearTransfers(indexes, this);
    }
}

bool MegaTransferView::onCancelAllTransfers()
{
    bool result(false);
    auto proxy (qobject_cast<TransfersManagerSortFilterProxyModel*>(model()));

    QPointer<MegaTransferView> dialog = QPointer<MegaTransferView>(this);

    if (proxy && QMegaMessageBox::warning(this, QString::fromUtf8("MEGAsync"),
                             proxy->isAnySync() ?  cancelWithSyncAskActionText() : cancelAllAskActionText(),
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No, getCancelDialogButtons())
            == QMessageBox::Yes
            && dialog)
    {
        cancelAllTransfers();
        result = true;
    }

    return result;
}

void MegaTransferView::onClearAllTransfers()
{
    QPointer<MegaTransferView> dialog = QPointer<MegaTransferView>(this);

    if (QMegaMessageBox::warning(this, QString::fromUtf8("MEGAsync"),
                             clearAllCompletedAskActionText(),
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No, getClearDialogButtons())
            == QMessageBox::Yes
            && dialog)
    {
        clearAllTransfers();
    }
}

void MegaTransferView::onCancelAndClearVisibleTransfers()
{
    auto info = getVisibleCancelOrClearInfo();

    QPointer<MegaTransferView> dialog = QPointer<MegaTransferView>(this);

    if (QMegaMessageBox::warning(this, QString::fromUtf8("MEGAsync"),
                             info.actionText,
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No, info.buttonsText)
            == QMessageBox::Yes
            && dialog)
    {
        auto sourceModel = MegaSyncApp->getTransfersModel();

        auto indexes = getTransfers(true, TransferData::FINISHED_STATES_MASK);
        sourceModel->clearTransfers(indexes);

        //Cancel transfers
        auto cancelIndexes = getTransfers(true);
        sourceModel->cancelAndClearTransfers(cancelIndexes, this);
    }
}

void MegaTransferView::onClearVisibleTransfers()
{
    QPointer<MegaTransferView> dialog = QPointer<MegaTransferView>(this);

    if (QMegaMessageBox::warning(this, QString::fromUtf8("MEGAsync"),
                             clearCompletedAskActionText(),
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No, getClearDialogButtons())
            == QMessageBox::Yes
            && dialog)
    {
        auto sourceModel = MegaSyncApp->getTransfersModel();

        auto indexes = getTransfers(true, TransferData::FINISHED_STATES_MASK);
        sourceModel->clearTransfers(indexes);
    }
}

void MegaTransferView::clearAllTransfers()
{    
    auto sourceModel = MegaSyncApp->getTransfersModel();

    sourceModel->pauseModelProcessing(true);
    sourceModel->clearAllTransfers();
    sourceModel->pauseModelProcessing(false);
}

void MegaTransferView::cancelAllTransfers()
{
    auto sourceModel = MegaSyncApp->getTransfersModel();

    sourceModel->pauseModelProcessing(true);
    sourceModel->cancelAllTransfers(this);
    sourceModel->pauseModelProcessing(false);
}

int MegaTransferView::getVerticalScrollBarWidth() const
{
    return verticalScrollBar()->width();
}

void MegaTransferView::onRetryVisibleTransfers()
{
    QModelIndexList indexes = getTransfers(true, TransferData::TRANSFER_FAILED);

    auto sourceModel = MegaSyncApp->getTransfersModel();
    sourceModel->retryTransfers(indexes);
}

void MegaTransferView::onCancelClearSelection(bool isClear)
{
    QModelIndexList indexes = getSelectedTransfers();

    auto sourceModel = MegaSyncApp->getTransfersModel();
    isClear ? sourceModel->clearTransfers(indexes) : sourceModel->cancelAndClearTransfers(indexes, this);
}

void MegaTransferView::enableContextMenu()
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &MegaTransferView::customContextMenuRequested,
            this, &MegaTransferView::onCustomContextMenu);
}

void MegaTransferView::onCustomContextMenu(const QPoint& point)
{
    auto contextMenu = createContextMenu();

    if(!contextMenu->actions().isEmpty())
    {
        contextMenu->exec(mapToGlobal(point));
    }
}

QMenu* MegaTransferView::createContextMenu()
{
    auto contextMenu = new QMenu(this);
    contextMenu->setWindowFlags(contextMenu->windowFlags() | Qt::NoDropShadowWindowHint);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    Platform::initMenu(contextMenu);

    QModelIndexList indexes = selectedIndexes();
    auto modelSize = model()->rowCount();

    bool isTopIndex = false;
    bool isBottomIndex = false;

    TransferData::TransferStates overallState;
    TransferData::TransferTypes overallType;
    long long int movableTransfers(0);

    //TODO use these containers to open links, open folder...etc
    QList<MegaHandle> handlesToOpenByContextMenu;
    QList<QDir> localFoldersToOpenByContextMenu;
    QList<QFileInfo> localFilesToOpenByContextMenu;

    for (auto index : qAsConst(indexes))
    {
        if(index.row() == 0)
        {
            isTopIndex = true;
        }

        if(index.row() == (modelSize -1))
        {
            isBottomIndex = true;
        }

        auto d (qvariant_cast<TransferItem>(index.data()).getTransferData());

        if(d->isCompleted() || d->mType & TransferData::TRANSFER_DOWNLOAD)
        {
            //Handles to open
            if(handlesToOpenByContextMenu.size() <= MAX_ITEMS_FOR_CONTEXT_MENU)
            {
                auto node = mParentTransferWidget->getModel()->getParentNodeToOpenByRow(index.row());
                if(node && !handlesToOpenByContextMenu.contains(node->getHandle()))
                {
                    handlesToOpenByContextMenu.append(node->getHandle());
                }
            }
        }

        if(d->isCompleted() || d->mType & TransferData::TRANSFER_UPLOAD)
        {
            if(localFoldersToOpenByContextMenu.size() <= MAX_ITEMS_FOR_CONTEXT_MENU || localFilesToOpenByContextMenu.size() <= MAX_ITEMS_FOR_CONTEXT_MENU)
            {
                QFileInfo fileInfo = mParentTransferWidget->getModel()->getFileInfoByIndex(index);
                if(localFilesToOpenByContextMenu.size() <= MAX_ITEMS_FOR_CONTEXT_MENU && !localFilesToOpenByContextMenu.contains(fileInfo))
                {
                    localFilesToOpenByContextMenu.append(fileInfo);
                }

                if(localFoldersToOpenByContextMenu.size() <= MAX_ITEMS_FOR_CONTEXT_MENU)
                {
                    QDir parentFolder(fileInfo.dir());
                    if(!localFoldersToOpenByContextMenu.contains(parentFolder))
                    {
                        localFoldersToOpenByContextMenu.append(parentFolder);
                    }
                }
            }
        }

        auto isMovableRow = [d, modelSize, &movableTransfers]()
        {
            if(modelSize > 1)
            {
                if(d->mType & TransferData::TRANSFER_UPLOAD
                        || d->mType & TransferData::TRANSFER_DOWNLOAD)
                {
                    movableTransfers++;
                }
            }
        };

        switch (d->getState())
        {
            case TransferData::TRANSFER_ACTIVE:
            case TransferData::TRANSFER_QUEUED:
            case TransferData::TRANSFER_RETRYING:
            {
                overallState |= TransferData::TRANSFER_ACTIVE;
                isMovableRow();
                break;
            }
            case TransferData::TRANSFER_PAUSED:
                isMovableRow();

            default:
                overallState |= d->getState();
        }

        overallType |= d->mType;
    }

    enum EnableAction
    {
        NONE = 0,
        PAUSE = 0x1,
        RESUME = 0x2,
        CANCEL = 0x4,
        CLEAR = 0x8,
        MOVE = 0x10,
        LINK = 0x20,
        OPEN = 0x40
    };
    Q_DECLARE_FLAGS(EnableActions, EnableAction);

    EnableActions actionFlag(EnableAction::NONE);

    auto checkActionByType = [overallType, &actionFlag]()
    {
        if((overallType & TransferData::TRANSFER_UPLOAD) && !(overallType & (TransferData::TRANSFER_DOWNLOAD | TransferData::TRANSFER_LTCPDOWNLOAD)))
        {
            actionFlag |= EnableAction::OPEN;
        }
        else if((overallType & TransferData::TRANSFER_DOWNLOAD) && !(overallType & TransferData::TRANSFER_UPLOAD))
        {
            actionFlag |= EnableAction::LINK;
        }
    };

    //Are all completed
    if(overallState == TransferData::TRANSFER_COMPLETED)
    {
        actionFlag |= EnableAction::CLEAR;
        actionFlag |= EnableAction::LINK;
        actionFlag |= EnableAction::OPEN;
    }
    else if(overallState == TransferData::TRANSFER_FAILED)
    {
        actionFlag |= EnableAction::CLEAR;
        checkActionByType();
    }
    else if(overallState & TransferData::TRANSFER_PAUSED || overallState & TransferData::TRANSFER_ACTIVE)
    {
        if(overallState & TransferData::TRANSFER_COMPLETED)
        {
            actionFlag |= EnableAction::CLEAR;
        }
        else
        {
            //only if all selected indexes can be moved, the move action is enabled
            if(movableTransfers == indexes.size())
            {
                actionFlag |= EnableAction::MOVE;
            }

            if(overallState & TransferData::TRANSFER_ACTIVE)
            {
                actionFlag |= EnableAction::PAUSE;
            }
            else if(overallState & TransferData::TRANSFER_PAUSED)
            {
                actionFlag |= EnableAction::RESUME;
            }
        }

        if(!(overallType & TransferData::TRANSFER_SYNC))
        {
            actionFlag |= EnableAction::CANCEL;
        }

        checkActionByType();
    }

    bool addSeparator(false);

    if(actionFlag & EnableAction::PAUSE)
    {
        auto pauseAction = new MenuItemAction(pauseActionText(indexes.size()),QIcon(QLatin1String(":/images/transfer_manager/context_menu/pause_ico.png")), contextMenu);
        connect(pauseAction, &QAction::triggered,
                this, &MegaTransferView::pauseSelectedClicked);

        contextMenu->addAction(pauseAction);
        addSeparator = true;
    }

    if(actionFlag & EnableAction::RESUME)
    {
        auto resumeAction = new MenuItemAction(resumeActionText(indexes.size()), QIcon(QLatin1String(":/images/transfer_manager/context_menu/resume_ico.png")), contextMenu);
        connect(resumeAction, &QAction::triggered,
                this, &MegaTransferView::resumeSelectedClicked);

        contextMenu->addAction(resumeAction);

        addSeparator = true;
    }

    addSeparatorToContextMenu(addSeparator, contextMenu);

    if(actionFlag & EnableAction::OPEN)
    {
        if(localFilesToOpenByContextMenu.size() <= MAX_ITEMS_FOR_CONTEXT_MENU)
        {
            auto openItemAction = new MenuItemAction(tr("Open"), QIcon(QLatin1String(":/images/transfer_manager/context_menu/open_file_ico.png")), contextMenu);
            connect(openItemAction, &QAction::triggered, this, &MegaTransferView::openItemClicked);

            contextMenu->addAction(openItemAction);
        }

        if(localFoldersToOpenByContextMenu.size() <= MAX_ITEMS_FOR_CONTEXT_MENU)
        {
            //Ico not included in transfer manager folder as it is also used by settingsDialog
            auto showInFolderAction = new MenuItemAction(tr("Show in folder"), QIcon(QLatin1String(":/images/show_in_folder_ico.png")),
                                                         contextMenu);
            connect(showInFolderAction, &QAction::triggered, this, &MegaTransferView::showInFolderClicked);

            contextMenu->addAction(showInFolderAction);
            addSeparator = true;
        }
    }

    addSeparatorToContextMenu(addSeparator, contextMenu);

    if(actionFlag & EnableAction::LINK)
    {
        if(handlesToOpenByContextMenu.size() <= MAX_ITEMS_FOR_CONTEXT_MENU)
        {
            //Ico not included in transfer manager folder as it is also used by settingsDialog
            auto openInMEGAAction = new MenuItemAction(tr("Open in MEGA"), QIcon(QLatin1String(":/images/ico_open_MEGA.png")), contextMenu);
            connect(openInMEGAAction, &QAction::triggered, this, &MegaTransferView::openInMEGAClicked);

            contextMenu->addAction(openInMEGAAction);
        }

        auto getLinkAction = new MenuItemAction(tr("Get link"), QIcon(QLatin1String(":/images/transfer_manager/context_menu/get_link_ico.png")), contextMenu);
        connect(getLinkAction, &QAction::triggered, this, &MegaTransferView::getLinkClicked);

        contextMenu->addAction(getLinkAction);
        addSeparator = true;
    }

    addSeparatorToContextMenu(addSeparator, contextMenu);

    if(actionFlag & EnableAction::MOVE)
    {
        if(!isTopIndex)
        {
            auto moveToTopAction = new MenuItemAction(tr("Move to top"), QIcon(QLatin1String(":/images/transfer_manager/context_menu/move_top_ico.png")), contextMenu);
            connect(moveToTopAction, &QAction::triggered, this, &MegaTransferView::moveToTopClicked);

            auto moveUpAction = new MenuItemAction(tr("Move up"), QIcon(QLatin1String(":/images/transfer_manager/context_menu/move_up_ico.png")), contextMenu);
            connect(moveUpAction, &QAction::triggered, this, &MegaTransferView::moveUpClicked);

            contextMenu->addAction(moveToTopAction);
            contextMenu->addAction(moveUpAction);

            addSeparator = true;
        }

        if(!isBottomIndex)
        {
            auto moveDownAction = new MenuItemAction(tr("Move down"), QIcon(QLatin1String(":/images/transfer_manager/context_menu/move_down_ico.png")), contextMenu);
            connect(moveDownAction, &QAction::triggered, this, &MegaTransferView::moveDownClicked);

            auto moveToBottomAction = new MenuItemAction(tr("Move to bottom"), QIcon(QLatin1String(":/images/transfer_manager/context_menu/move_bottom_ico.png")), contextMenu);
            connect(moveToBottomAction, &QAction::triggered, this, &MegaTransferView::moveToBottomClicked);

            contextMenu->addAction(moveDownAction);
            contextMenu->addAction(moveToBottomAction);

            addSeparator = true;
        }

        addSeparatorToContextMenu(addSeparator, contextMenu);
    }

    if(actionFlag & EnableAction::CANCEL)
    {
        auto cancelAction = new MenuItemAction(actionFlag & EnableAction::CLEAR ? cancelAndClearActionText(indexes.size()) : cancelActionText(indexes.size()),
                                               QIcon(QLatin1String(":/images/transfer_manager/context_menu/cancel_transfer_ico.png")), contextMenu);
        connect(cancelAction, &QAction::triggered,
                this, &MegaTransferView::cancelSelectedClicked);

        contextMenu->addAction(cancelAction);

        // Set default action to have it painted red
        contextMenu->setDefaultAction(cancelAction);
    }
    else if(actionFlag & EnableAction::CLEAR)
    {
        auto clearAction = new MenuItemAction(tr("Clear"), QIcon(QLatin1String(":/images/transfer_manager/context_menu/ico_clear.png")), contextMenu);
        connect(clearAction, &QAction::triggered,
                this, &MegaTransferView::clearSelectedClicked);

        contextMenu->addAction(clearAction);
    }

    return contextMenu;
}


void MegaTransferView::addSeparatorToContextMenu(bool& addSeparator, QMenu* contextMenu)
{
    if(addSeparator)
    {
        contextMenu->addSeparator();
        addSeparator = false;
    }
}

void MegaTransferView::mouseReleaseEvent(QMouseEvent* event)
{
    auto pressedIndex = indexAt(event->pos());
    if(!pressedIndex.isValid())
    {
        clearSelection();
    }

    QTreeView::mouseReleaseEvent(event);
}

void MegaTransferView::mouseMoveEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        return;
    }

    QTreeView::mouseMoveEvent(event);
}

void MegaTransferView::changeEvent(QEvent* event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        viewport()->update();
    }

    QTreeView::changeEvent(event);
}

void MegaTransferView::dropEvent(QDropEvent* event)
{
    QAbstractItemView::dropEvent(event);
    event->acceptProposedAction();
    clearSelection();
}

void MegaTransferView::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Delete)
    {
        onCancelSelectedTransfers();
    }
    else if(event->key() == Qt::Key_Down || event->key() == Qt::Key_Up)
    {
        mKeyNavigation = true;
    }

    QTreeView::keyPressEvent(event);

    if(mKeyNavigation)
    {
        mKeyNavigation = false;
    }
}

void MegaTransferView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    if(mKeyNavigation)
    {
        auto selectedIndexes = selected.indexes();
        auto deselectedIndexes = deselected.indexes();

        if(!selectedIndexes.isEmpty())
        {
            scrollTo(selectedIndexes.last(), QAbstractItemView::PositionAtCenter);
        }
        else if(!deselectedIndexes.isEmpty())
        {
            scrollTo(deselectedIndexes.last(), QAbstractItemView::PositionAtCenter);
        }
    }

    QTreeView::selectionChanged(selected, deselected);
}

bool MegaTransferView::eventFilter(QObject *object, QEvent *event)
{
    if(object == verticalScrollBar())
    {
        if(event->type() == QEvent::Show)
        {
            emit verticalScrollBarVisibilityChanged(true);
        }
        else if(event->type() == QEvent::Hide)
        {
            emit verticalScrollBarVisibilityChanged(false);
        }
    }

    return QTreeView::eventFilter(object, event);
}

void MegaTransferView::moveToTopClicked()
{
    auto proxy(qobject_cast<QSortFilterProxyModel*>(model()));
    if(proxy)
    {
        if(!selectionModel())
        {
            return;
        }

        auto indexes = selectionModel()->selectedRows();
        if(!indexes.isEmpty())
        {
            auto sourceModel = MegaSyncApp->getTransfersModel();

            // Sort to keep items in the same order
            std::sort(indexes.begin(), indexes.end(),[](QModelIndex check1, QModelIndex check2){
                return check1.row() > check2.row();
            });

            auto firstToMove = indexes.first();

            if(firstToMove.row() == 0)
            {
                return;
            }

            for (int item = 0; item < indexes.size(); ++item)
            {
                auto index = indexes.at(item);
                auto sourceIndex = proxy->mapToSource(index);

                sourceModel->moveTransferPriority(QModelIndex(), QList<int>() << sourceIndex.row(), QModelIndex(), -1);
            }
        }
    }

    clearSelection();
}

void MegaTransferView::moveUpClicked()
{
    QModelIndexList indexes;
    if(selectionModel())
    {
        indexes = selectionModel()->selectedRows();
    }

    if(!indexes.isEmpty())
    {
        auto proxy(qobject_cast<QSortFilterProxyModel*>(model()));
        if(proxy)
        {
            auto sourceModel = MegaSyncApp->getTransfersModel();

            // Sort to keep items in the same order
            std::sort(indexes.begin(), indexes.end(),[](QModelIndex check1, QModelIndex check2){
                return check1.row() < check2.row();
            });

            auto firstToMove = indexes.first();

            if(firstToMove.row() == 0)
            {
                return;
            }

            auto proxyTargetIndex = proxy->index(indexes.first().row() - 1,0);
            auto sourceTargetIndex = proxy->mapToSource(proxyTargetIndex);
            auto rowTarget = sourceTargetIndex.row();

            for (int item = 0; item < indexes.size(); ++item)
            {
                auto index = indexes.at(item);
                auto sourceIndex = proxy->mapToSource(index);

                if(item != 0)
                {
                    auto previousIndex = indexes.at(item-1);

                    if(index.row() - previousIndex.row() != 1)
                    {
                        proxyTargetIndex = proxy->index(index.row()-1,0);
                        sourceTargetIndex = proxy->mapToSource(proxyTargetIndex);
                        rowTarget = sourceTargetIndex.row();
                    }
                }

                sourceModel->moveTransferPriority(QModelIndex(), QList<int>() << sourceIndex.row(), QModelIndex(), rowTarget);
            }
        }
    }
    clearSelection();
}

void MegaTransferView::moveDownClicked()
{
    QModelIndexList indexes;
    if(selectionModel())
    {
        indexes = selectionModel()->selectedRows();
    }

    if(!indexes.isEmpty())
    {
        auto proxy(qobject_cast<QSortFilterProxyModel*>(model()));
        if(proxy)
        {
            auto sourceModel = MegaSyncApp->getTransfersModel();

            // Sort to keep items in the same order
            std::sort(indexes.begin(), indexes.end(),[](QModelIndex check1, QModelIndex check2){
                return check1.row() > check2.row();
            });

            auto firstToMove = indexes.first();

            if(firstToMove.row() == model()->rowCount())
            {
                return;
            }

            auto proxyTargetIndex = proxy->index(indexes.first().row() + 1,0);
            auto sourceTargetIndex = proxy->mapToSource(proxyTargetIndex);
            auto rowTarget = sourceTargetIndex.row();

            sourceModel->inverseMoveRowsSignal(true);

            for (int item = 0; item < indexes.size(); ++item)
            {
                auto index = indexes.at(item);
                auto sourceIndex = proxy->mapToSource(index);

                if(item != 0)
                {
                    auto previousIndex = indexes.at(item-1);

                    if(previousIndex.row() - index.row() != 1)
                    {
                        proxyTargetIndex = proxy->index(index.row() + 1,0);
                        sourceTargetIndex = proxy->mapToSource(proxyTargetIndex);
                        rowTarget = sourceTargetIndex.row();
                    }
                }
                sourceModel->moveTransferPriority(QModelIndex(), QList<int>() << rowTarget, QModelIndex(), sourceIndex.row());
            }

            sourceModel->inverseMoveRowsSignal(false);
        }
    }

    clearSelection();
}

void MegaTransferView::moveToBottomClicked()
{
    QModelIndexList indexes;
    if(selectionModel())
    {
        indexes = selectionModel()->selectedRows();
    }

    if(!indexes.isEmpty())
    {
        auto proxy(qobject_cast<QSortFilterProxyModel*>(model()));
        if(proxy)
        {
            auto sourceModel = MegaSyncApp->getTransfersModel();

            // Sort to keep items in the same order
            std::sort(indexes.begin(), indexes.end(),[](QModelIndex check1, QModelIndex check2){
                return check1.row() < check2.row();
            });

            auto firstToMove = indexes.last();

            if(firstToMove.row() == model()->rowCount())
            {
                return;
            }

            for (int item = 0; item < indexes.size(); ++item)
            {
                auto index = indexes.at(item);
                auto sourceIndex = proxy->mapToSource(index);

                sourceModel->moveTransferPriority(QModelIndex(), QList<int>() << sourceIndex.row(), QModelIndex(), -2);
            }
        }
    }

    clearSelection();
}

void MegaTransferView::getLinkClicked()
{
    if (mDisableLink)
    {
        return;
    }

    QList<int> rows;
    auto proxy(qobject_cast<QSortFilterProxyModel*>(model()));

    QModelIndexList indexes;
    if(selectionModel())
    {
        if(proxy)
        {
            indexes = proxy->mapSelectionToSource(selectionModel()->selection()).indexes();
        }
        else
        {
            indexes = selectionModel()->selection().indexes();
        }
    }

    for (auto index : qAsConst(indexes))
    {
        rows.push_back(index.row());
    }

    if (!rows.isEmpty())
    {
        mParentTransferWidget->getModel()->getLinks(rows);
    }

    clearSelection();
}

void MegaTransferView::openInMEGAClicked()
{
    if (mDisableLink)
    {
        return;
    }

    QList<int> rows;
    auto proxy(qobject_cast<QSortFilterProxyModel*>(model()));

    QModelIndexList indexes;
    if(selectionModel())
    {
        if(proxy)
        {
            indexes = proxy->mapSelectionToSource(selectionModel()->selection()).indexes();
        }
        else
        {
            indexes = selectionModel()->selection().indexes();
        }
    }

    for (auto index : qAsConst(indexes))
    {
        rows.push_back(index.row());
    }

    if (!rows.isEmpty())
    {
        mParentTransferWidget->getModel()->openInMEGA(rows);
    }

    clearSelection();
}

void MegaTransferView::openItemClicked()
{
    const QModelIndexList selection (selectedIndexes());
    for (auto index : selection)
    {
        if (index.isValid())
        {
            const auto transferItem (
                        qvariant_cast<TransferItem>(index.data(Qt::DisplayRole)));
            auto d (transferItem.getTransferData());
            auto path = d->path();
            if (!path.isEmpty())
            {
                QFileInfo info(path);
                if(info.exists())
                {
                    auto openUrlTask = Utilities::openUrl(QUrl::fromLocalFile(path));
                    mOpenUrlWatcher.setFuture(openUrlTask);
                }
                else
                {
                    showOpeningFileError();
                }
            }
        }
    }
    clearSelection();
}

void MegaTransferView::showInFolderClicked()
{
    const QModelIndexList selection (selectedIndexes());
    mParentTransferWidget->getModel()->openFoldersByIndexes(selection);
    clearSelection();
}

void MegaTransferView::showInMegaClicked()
{
    const QModelIndexList selection (selectedIndexes());
    for (auto index : selection)
    {
        if (index.isValid())
        {
            const auto transferItem (
                        qvariant_cast<TransferItem>(index.data(Qt::DisplayRole)));
            auto d (transferItem.getTransferData());

            if (d->mParentHandle != mega::INVALID_HANDLE)
            {
                qobject_cast<MegaApplication*>(qApp)->shellViewOnMega(d->mParentHandle, false);
            }
        }
    }
    clearSelection();
}

void MegaTransferView::cancelSelectedClicked()
{
    onCancelSelectedTransfers();
}

void MegaTransferView::clearSelectedClicked()
{
    onCancelSelectedTransfers();
}

void MegaTransferView::pauseSelectedClicked()
{
    emit pauseResumeTransfersByContextMenu(true);
}

void MegaTransferView::resumeSelectedClicked()
{
    emit pauseResumeTransfersByContextMenu(false);
}

void MegaTransferView::onInternalMoveStarted()
{
     setAutoScroll(true);
}

void MegaTransferView::onInternalMoveFinished()
{
    setAutoScroll(false);
}

void MegaTransferView::onOpenUrlFinished()
{
    auto result = mOpenUrlWatcher.result();
    if(!result)
    {
        showOpeningFileError();
    }
}

void MegaTransferView::showOpeningFileError()
{
    QMegaMessageBox::warning(nullptr, tr("Error"), tr("Error opening file"), QMessageBox::Ok);
}
