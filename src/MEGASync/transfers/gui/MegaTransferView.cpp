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
    auto sourceModel(qobject_cast<TransfersModel*>(proxy->sourceModel()));

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

    return indexes;
}

QModelIndexList MegaTransferView::getSelectedTransfers()
{
    auto proxy(qobject_cast<QSortFilterProxyModel*>(model()));
    auto selection = selectionModel()->selection();
    QModelIndexList indexes;

    if (selection.size() > 0)
    {
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
    auto proxy (qobject_cast<TransfersManagerSortFilterProxyModel*>(model()));

    SelectedIndexesInfo info;

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

    auto proxy (qobject_cast<QSortFilterProxyModel*>(model()));
    auto sourceModel(qobject_cast<TransfersModel*>(proxy->sourceModel()));
    sourceModel->pauseTransfers(indexes, pauseState);

    //Use to repaint and update the transfers state
    update();
}

void MegaTransferView::onPauseResumeSelection(bool pauseState)
{
    QModelIndexList indexes = getSelectedTransfers();
    auto proxy (qobject_cast<QSortFilterProxyModel*>(model()));
    auto sourceModel(qobject_cast<TransfersModel*>(proxy->sourceModel()));

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

            auto proxy (qobject_cast<TransfersManagerSortFilterProxyModel*>(model()));
            auto sourceModel(qobject_cast<TransfersModel*>(proxy->sourceModel()));
            sourceModel->cancelAndClearTransfers(indexes, this);
        }
    }
}

void MegaTransferView::onCancelSelectedTransfers()
{
    auto info = getSelectedCancelOrClearInfo();

    if(!info.areAllSync)
    {
        QPointer<MegaTransferView> dialog = QPointer<MegaTransferView>(this);

        if (QMegaMessageBox::warning(this, QString::fromUtf8("MEGAsync"),
                                     info.actionText,
                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::No, info.buttonsText)
                == QMessageBox::Yes
                && dialog)
        {
            QModelIndexList indexes = getSelectedTransfers();

            auto proxy (qobject_cast<QSortFilterProxyModel*>(model()));
            auto sourceModel(qobject_cast<TransfersModel*>(proxy->sourceModel()));
            sourceModel->cancelAndClearTransfers(indexes, this);
        }
    }
}

bool MegaTransferView::onCancelAllTransfers()
{
    bool result(false);
    auto proxy (qobject_cast<TransfersManagerSortFilterProxyModel*>(model()));

    QPointer<MegaTransferView> dialog = QPointer<MegaTransferView>(this);

    if (QMegaMessageBox::warning(this, QString::fromUtf8("MEGAsync"),
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
        auto proxy (qobject_cast<QSortFilterProxyModel*>(model()));
        auto sourceModel(qobject_cast<TransfersModel*>(proxy->sourceModel()));

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
        auto proxy (qobject_cast<QSortFilterProxyModel*>(model()));
        auto sourceModel(qobject_cast<TransfersModel*>(proxy->sourceModel()));

        auto indexes = getTransfers(true, TransferData::FINISHED_STATES_MASK);
        sourceModel->clearTransfers(indexes);
    }
}

void MegaTransferView::clearAllTransfers()
{    
    auto proxy(qobject_cast<QSortFilterProxyModel*>(model()));
    auto sourceModel(qobject_cast<TransfersModel*>(proxy->sourceModel()));

    sourceModel->pauseModelProcessing(true);
    sourceModel->clearAllTransfers();
    sourceModel->pauseModelProcessing(false);
}

void MegaTransferView::cancelAllTransfers()
{
    auto proxy(qobject_cast<QSortFilterProxyModel*>(model()));
    auto sourceModel(qobject_cast<TransfersModel*>(proxy->sourceModel()));

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

    auto proxy (qobject_cast<QSortFilterProxyModel*>(model()));
    auto sourceModel(qobject_cast<TransfersModel*>(proxy->sourceModel()));
    sourceModel->retryTransfers(indexes);
}

void MegaTransferView::onCancelClearSelection(bool isClear)
{
    QModelIndexList indexes = getSelectedTransfers();
    auto proxy (qobject_cast<QSortFilterProxyModel*>(model()));
    auto sourceModel(qobject_cast<TransfersModel*>(proxy->sourceModel()));
    isClear ? sourceModel->clearTransfers(indexes) : sourceModel->cancelAndClearTransfers(indexes, this);
}

void MegaTransferView::enableContextMenu()
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &MegaTransferView::customContextMenuRequested,
            this, &MegaTransferView::onCustomContextMenu);
}

QMenu* MegaTransferView::createContextMenu()
{
    auto contextMenu = new QMenu(this);
    contextMenu->setWindowFlags(contextMenu->windowFlags() | Qt::NoDropShadowWindowHint);

    bool enablePause = false;
    bool enableResume = false;
    bool enableCancel = false;
    bool enableClear = false;
    bool enableMove = false;
    bool isTopIndex(false);
    bool isBottomIndex(false);
    bool showLink (false);
    bool showOpen (false);
    bool showShowInFolder (false);

    QModelIndexList indexes = selectedIndexes();

    for (auto index : qAsConst(indexes))
    {
        if(index.row() == 0)
        {
            isTopIndex = true;
        }

        if(index.row() == (model()->rowCount() -1))
        {
            isBottomIndex = true;
        }

        auto d (qvariant_cast<TransferItem>(index.data()).getTransferData());
        switch (d->getState())
        {
            case TransferData::TRANSFER_ACTIVE:
            case TransferData::TRANSFER_QUEUED:
            case TransferData::TRANSFER_RETRYING:
            {
                enablePause = true;
                enableMove = true;
                enableCancel |= !(d->mType & TransferData::TRANSFER_SYNC);
                break;
            }
            case TransferData::TRANSFER_PAUSED:
            {
                enableResume = true;
                enableMove = true;
                enableCancel |= !(d->mType & TransferData::TRANSFER_SYNC);
                break;
            }
            case TransferData::TRANSFER_FAILED:
            {
                enableCancel |= !(d->mType & TransferData::TRANSFER_SYNC);
            }
            case TransferData::TRANSFER_CANCELLED:
            case TransferData::TRANSFER_COMPLETED:
            {
                enableClear = true;
                break;
            }
            default:
                break;
        }
    }

    bool onlyOneSelected ((indexes.size() == 1));

    if (onlyOneSelected)
    {
        auto d (qvariant_cast<TransferItem>(indexes.first().data()).getTransferData());
        auto state (d->getState());
        auto type ((d->mType & TransferData::TRANSFER_UPLOAD) ?
                       TransferData::TRANSFER_UPLOAD
                     : TransferData::TRANSFER_DOWNLOAD);

        if (state == TransferData::TRANSFER_COMPLETED)
        {
            showLink = true;
            showOpen = true;
            showShowInFolder = true;
        }
        else if (type == TransferData::TRANSFER_UPLOAD)
        {
            showOpen = true;
            showShowInFolder = true;
        }
        else if (type == TransferData::TRANSFER_DOWNLOAD)
        {
            showLink = true;
        }
        else if(type == TransferData::TRANSFER_SYNC)
        {
            //Check if the file exists on local drive. Otherwise, show the OpenInMEGAACtion (as it is on the remote drive)
            auto path = d->path();
            QFileInfo file(path);

            if(file.exists())
            {
                showOpen = true;
                showShowInFolder = true;
            }
            else
            {
                showLink = true;
            }
        }
    }

    bool addSeparator(false);

    if(enablePause)
    {
        auto pauseAction = new QAction(QIcon(QLatin1String(":/images/transfer_manager/context_menu/pause_ico.png")),
                                   pauseActionText(indexes.size()), this);
        connect(pauseAction, &QAction::triggered,
                this, &MegaTransferView::pauseSelectedClicked);

        contextMenu->addAction(pauseAction);

        addSeparator = true;
    }

    if(enableResume)
    {
        auto resumeAction = new QAction(QIcon(QLatin1String(":/images/transfer_manager/context_menu/resume_ico.png")),
                                    resumeActionText(indexes.size()), this);
        connect(resumeAction, &QAction::triggered,
                this, &MegaTransferView::resumeSelectedClicked);

        contextMenu->addAction(resumeAction);

        addSeparator = true;
    }

    addSeparatorToContextMenu(addSeparator, contextMenu);

    if(showOpen)
    {
        auto openItemAction = new QAction(QIcon(QLatin1String(":/images/transfer_manager/context_menu/open_file_ico.png")),
                                      tr("Open"), this);
        connect(openItemAction, &QAction::triggered, this, &MegaTransferView::openItemClicked);

        contextMenu->addAction(openItemAction);
        addSeparator = true;
    }

    if(showShowInFolder)
    {
        //Ico not included in transfer manager folder as it is also used by settingsDialog
        auto showInFolderAction = new QAction(QIcon(QLatin1String(":/images/show_in_folder_ico.png")),
                                          tr("Show in folder"), this);
        connect(showInFolderAction, &QAction::triggered, this, &MegaTransferView::showInFolderClicked);

        contextMenu->addAction(showInFolderAction);
        addSeparator = true;
    }

    if(showLink)
    {
        //Ico not included in transfer manager folder as it is also used by settingsDialog
        auto openInMEGAAction = new QAction(QIcon(QLatin1String(":/images/ico_open_MEGA.png")),
                                        tr("Open in MEGA"), this);
        connect(openInMEGAAction, &QAction::triggered, this, &MegaTransferView::openInMEGAClicked);

        contextMenu->addAction(openInMEGAAction);
        addSeparator = true;
    }

    addSeparatorToContextMenu(addSeparator, contextMenu);

    if(showLink)
    {
        auto getLinkAction = new QAction(QIcon(QLatin1String(":/images/transfer_manager/context_menu/get_link_ico.png")),
                                         tr("Get link"), this);
        connect(getLinkAction, &QAction::triggered, this, &MegaTransferView::getLinkClicked);

        contextMenu->addAction(getLinkAction);
        addSeparator = true;
    }

    addSeparatorToContextMenu(addSeparator, contextMenu);

    if(enableMove)
    {
        if(!isTopIndex)
        {
            auto moveToTopAction = new QAction(QIcon(QLatin1String(":/images/transfer_manager/context_menu/move_top_ico.png")),
                                               tr("Move to top"), this);
            connect(moveToTopAction, &QAction::triggered, this, &MegaTransferView::moveToTopClicked);

            auto moveUpAction = new QAction(QIcon(QLatin1String(":/images/transfer_manager/context_menu/move_up_ico.png")),
                                            tr("Move up"), this);
            connect(moveUpAction, &QAction::triggered, this, &MegaTransferView::moveUpClicked);

            contextMenu->addAction(moveToTopAction);
            contextMenu->addAction(moveUpAction);

            addSeparator = true;
        }

        if(!isBottomIndex)
        {
            auto moveDownAction = new QAction(QIcon(QLatin1String(":/images/transfer_manager/context_menu/move_down_ico.png")),
                                              tr("Move down"), this);
            connect(moveDownAction, &QAction::triggered, this, &MegaTransferView::moveDownClicked);

            auto moveToBottomAction = new QAction(QIcon(QLatin1String(":/images/transfer_manager/context_menu/move_bottom_ico.png")),
                                                  tr("Move to bottom"), this);
            connect(moveToBottomAction, &QAction::triggered, this, &MegaTransferView::moveToBottomClicked);

            contextMenu->addAction(moveDownAction);
            contextMenu->addAction(moveToBottomAction);

            addSeparator = true;
        }

        addSeparatorToContextMenu(addSeparator, contextMenu);
    }

    if(enableCancel)
    {
        auto cancelAction = new QAction(QIcon(QLatin1String(":/images/transfer_manager/context_menu/cancel_transfer_ico.png")),
                                         enableClear ? cancelAndClearActionText(indexes.size()) : cancelActionText(indexes.size()), this);
        connect(cancelAction, &QAction::triggered,
                this, &MegaTransferView::cancelSelectedClicked);

        contextMenu->addAction(cancelAction);

        // Set default action to have it painted red
        contextMenu->setDefaultAction(cancelAction);
    }
    else if(enableClear)
    {
        auto clearAction = new QAction(QIcon(QLatin1String(":/images/transfer_manager/context_menu/ico_clear.png")),
                                   tr("Clear"), this);
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

void MegaTransferView::onCustomContextMenu(const QPoint& point)
{
    auto contextMenu = createContextMenu();

    if(!contextMenu->actions().isEmpty())
    {
        contextMenu->exec(mapToGlobal(point));
    }
    contextMenu->deleteLater();
}

void MegaTransferView::moveToTopClicked()
{
    auto indexes = selectionModel()->selectedRows();
    if(!indexes.isEmpty())
    {
        auto proxy(qobject_cast<QSortFilterProxyModel*>(model()));
        auto sourceModel(qobject_cast<TransfersModel*>(proxy->sourceModel()));

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

    clearSelection();
}

void MegaTransferView::moveUpClicked()
{
    auto indexes = selectionModel()->selectedRows();
    if(!indexes.isEmpty())
    {
        auto proxy(qobject_cast<QSortFilterProxyModel*>(model()));
        auto sourceModel(qobject_cast<TransfersModel*>(proxy->sourceModel()));

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
    clearSelection();
}

void MegaTransferView::moveDownClicked()
{
    auto indexes = selectionModel()->selectedRows();
    if(!indexes.isEmpty())
    {
        auto proxy(qobject_cast<QSortFilterProxyModel*>(model()));
        auto sourceModel(qobject_cast<TransfersModel*>(proxy->sourceModel()));

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

    clearSelection();
}

void MegaTransferView::moveToBottomClicked()
{
    auto indexes = selectionModel()->selectedRows();
    if(!indexes.isEmpty())
    {
        auto proxy(qobject_cast<QSortFilterProxyModel*>(model()));
        auto sourceModel(qobject_cast<TransfersModel*>(proxy->sourceModel()));

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

    const auto indexes (proxy ?
                            proxy->mapSelectionToSource(selectionModel()->selection()).indexes()
                          : selectionModel()->selection().indexes());
    for (auto index : indexes)
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

    const auto indexes (proxy ?
                            proxy->mapSelectionToSource(selectionModel()->selection()).indexes()
                          : selectionModel()->selection().indexes());
    for (auto index : indexes)
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
                    auto openUrlTask = QtConcurrent::run(QDesktopServices::openUrl, QUrl::fromLocalFile(path));
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
    for (auto index : selection)
    {
        if (index.isValid())
        {
            mParentTransferWidget->getModel()->openFolderByIndex(index);
        }
    }
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

