#include "TransfersWidget.h"
#include "ui_TransfersWidget.h"
#include "MegaApplication.h"
#include "QMegaMessageBox.h"

#include <QTimer>
#include <QtConcurrent/QtConcurrent>
#include <QScrollBar>

TransfersWidget::TransfersWidget(QWidget* parent) :
    QWidget (parent),
    ui (new Ui::TransfersWidget),
    tDelegate (nullptr),
    app (qobject_cast<MegaApplication*>(qApp)),
    mCurrentTab(NO_TAB),
    mScanningIsActive(false)
{
    ui->setupUi(this);

    ui->statusColumn->setSortOrder(Qt::DescendingOrder);

    connect(ui->nameColumn, &TransferWidgetHeaderItem::toggled, this, &TransfersWidget::onHeaderItemClicked);
    connect(ui->sizeColumn, &TransferWidgetHeaderItem::toggled, this, &TransfersWidget::onHeaderItemClicked);
    connect(ui->speedColumn, &TransferWidgetHeaderItem::toggled, this, &TransfersWidget::onHeaderItemClicked);
    connect(ui->statusColumn, &TransferWidgetHeaderItem::toggled, this, &TransfersWidget::onHeaderItemClicked);
    connect(ui->timeColumn, &TransferWidgetHeaderItem::toggled, this, &TransfersWidget::onHeaderItemClicked);

    mModel = app->getTransfersModel();

    //Align header pause/cancel buttons to view pause/cancel button
    connect(ui->tvTransfers, &MegaTransferView::verticalScrollBarVisibilityChanged, this, &TransfersWidget::onVerticalScrollBarVisibilityChanged);
    connect(ui->tvTransfers, &MegaTransferView::pauseResumeTransfersByContextMenu, this, &TransfersWidget::onPauseResumeTransfer);

    connect(mModel, &TransfersModel::transfersProcessChanged, this, &TransfersWidget::onCheckPauseResumeButton);
    connect(mModel, &TransfersModel::transfersProcessChanged, this, &TransfersWidget::onCheckCancelClearButton);

    ui->tCancelClearVisible->installEventFilter(this);
    ui->tPauseResumeVisible->installEventFilter(this);
    ui->tvTransfers->installEventFilter(this);

    auto leftPaneButtons = ui->wTableHeader->findChildren<QAbstractButton*>();
    foreach(auto& button, leftPaneButtons)
    {
        mButtonIconManager.addButton(button);
    }
}
void TransfersWidget::setupTransfers()
{
    mProxyModel = new TransfersManagerSortFilterProxyModel(ui->tvTransfers);
    mProxyModel->setSourceModel(app->getTransfersModel());
    mProxyModel->initProxyModel(SortCriterion::PRIORITY, Qt::DescendingOrder);

    connect(mProxyModel, &TransfersManagerSortFilterProxyModel::modelAboutToBeChanged, this, &TransfersWidget::onUiBlocked);
    connect(mProxyModel, &TransfersManagerSortFilterProxyModel::modelChanged, this, &TransfersWidget::onModelChanged);
    connect(mProxyModel, &TransfersManagerSortFilterProxyModel::pauseResumeTransfer, this, &TransfersWidget::onPauseResumeTransfer);
    connect(mProxyModel, &TransfersManagerSortFilterProxyModel::transferCancelClear, this, &TransfersWidget::onCancelClearButtonPressedOnDelegate);
    connect(mProxyModel, &TransfersManagerSortFilterProxyModel::transferRetry, this, &TransfersWidget::onRetryButtonPressedOnDelegate);
    connect(app->getTransfersModel(), &TransfersModel::blockUi, this, &TransfersWidget::onUiBlocked);
    connect(app->getTransfersModel(), &TransfersModel::unblockUi, this, &TransfersWidget::onUiUnblocked);
    connect(app->getTransfersModel(), &TransfersModel::unblockUiAndFilter, this, &TransfersWidget::onUiUnblockedAndFilter);
    connect(app->getTransfersModel(), &TransfersModel::rowsAboutToBeMoved, this, &TransfersWidget::onRowsAboutToBeMoved);

    configureTransferView();
}

TransfersWidget::~TransfersWidget()
{
    mLoadingScene.toggleLoadingScene(false);
    delete ui;
    if (tDelegate) delete tDelegate;
    if (mProxyModel) delete mProxyModel;
}

void TransfersWidget::configureTransferView()
{
    if (!mModel)
    {
        return;
    }

    tDelegate = new MegaTransferDelegate(mProxyModel, ui->tvTransfers);
    ui->tvTransfers->setup(this);
    ui->tvTransfers->setItemDelegate(tDelegate);
    ui->tvTransfers->setDragEnabled(true);
    ui->tvTransfers->viewport()->setAcceptDrops(true);
    ui->tvTransfers->setDropIndicatorShown(true);
    ui->tvTransfers->setDragDropMode(QAbstractItemView::InternalMove);
    ui->tvTransfers->enableContextMenu();

    mLoadingScene.setView(ui->tvTransfers);
    mDelegateHoverManager.setView(ui->tvTransfers);
}

TransfersModel* TransfersWidget::getModel()
{
    return app->getTransfersModel();
}

void TransfersWidget::onHeaderItemClicked(int sortBy, Qt::SortOrder order)
{
    mProxyModel->sort(sortBy, order);
}

void TransfersWidget::on_tCancelClearVisible_clicked()
{
    if(getCurrentTab() == TransfersWidget::ALL_TRANSFERS_TAB)
    {
        if(ui->tvTransfers->onCancelAllTransfers())
        {
            emit changeToAllTransfersTab();
        }
    }
    else if(getCurrentTab() == TransfersWidget::COMPLETED_TAB)
    {
        ui->tvTransfers->onClearAllTransfers();
    }
    else if((getCurrentTab() > TransfersWidget::TYPES_TAB_BASE && getCurrentTab() < TransfersWidget::TYPES_LAST)
            || getCurrentTab() == TransfersWidget::SEARCH_TAB)
    {
        if(mProxyModel->areAllSync())
        {
            ui->tvTransfers->onClearVisibleTransfers();
        }
        else
        {
            ui->tvTransfers->onCancelAndClearVisibleTransfers();
        }
    }
    else
    {
        ui->tvTransfers->onCancelVisibleTransfers();
    }

    //Use to repaint and update the transfers state
    ui->tvTransfers->update();
}

void TransfersWidget::onCheckPauseResumeButton()
{
    //SHow when there is at least one active or there are no transfers (at the beginning)
    ui->tPauseResumeVisible->setVisible(mProxyModel->isAnyActive());

    if(ui->tPauseResumeVisible->isVisible())
    {
        auto pauseState(mProxyModel->areAllPaused());
        if(ui->tPauseResumeVisible->isChecked() != pauseState)
        {
            togglePauseResumeButton(pauseState);
        }
    }
}

void TransfersWidget::togglePauseResumeButton(bool state)
{
    ui->tPauseResumeVisible->blockSignals(true);
    ui->tPauseResumeVisible->setChecked(state);
    ui->tPauseResumeVisible->blockSignals(false);

    //Use to repaint and update the transfers state
    ui->tvTransfers->update();
}

void TransfersWidget::onCheckCancelClearButton()
{
    bool areAllTransfersCompleted(mProxyModel->areAllCompleted());
    bool areAllSync(mProxyModel->areAllSync());
    bool isAnyTransferCompleted(mProxyModel->isAnyCompleted());
    bool isAnyTransferActive(mProxyModel->isAnyCancellable());

    auto buttonVisible((isAnyTransferActive && !areAllSync) || areAllTransfersCompleted || isAnyTransferCompleted);
    if(mCancelClearInfo.visible != buttonVisible)
    {
        mCancelClearInfo.visible = buttonVisible;
        ui->tCancelClearVisible->setVisible(mCancelClearInfo.visible);
    }

    if (mCurrentTab == TransfersWidget::COMPLETED_TAB)
    {        
        mCancelClearInfo.clearAction = true;

    }
    else if ((mCurrentTab > TransfersWidget::TYPES_TAB_BASE && mCurrentTab < TransfersWidget::TYPES_LAST) || mCurrentTab == TransfersWidget::SEARCH_TAB)
    {        
        bool showClear(false);

        if(areAllTransfersCompleted)
        {
            showClear = true;
        }
        else if(isAnyTransferCompleted)
        {
            if(areAllSync || !isAnyTransferActive)
            {
                showClear = true;
            }
        }

        if(mCancelClearInfo.clearAction != showClear)
        {
            mCancelClearInfo.clearAction = showClear;
        }
    }
    else
    {
        mCancelClearInfo.clearAction = false;
    }

    if(mCancelClearInfo.visible)
    {
        if(mCancelClearInfo.clearAction)
        {
            ui->tCancelClearVisible->setProperty("default_icon",
                                                 QString::fromStdString("qrc:/images/transfer_manager/transfers_actions/lists_minus_all_ico_default.png"));
        }
        else
        {
            ui->tCancelClearVisible->setProperty("default_icon",
                                                 QString::fromStdString("qrc:/images/transfer_manager/transfers_actions/lists_cancel_all_ico_default.png"));
        }
    }
}

void TransfersWidget::updateCancelClearButtonTooltip()
{
    bool areAllTransfersCompleted(mProxyModel->areAllCompleted());
    bool areAllSync(mProxyModel->areAllSync());
    bool isAnyTransferCompleted(mProxyModel->isAnyCompleted());
    bool isAnyTransferActive(mProxyModel->isAnyCancellable());

    if (mCurrentTab == TransfersWidget::COMPLETED_TAB)
    {
        mCancelClearInfo.cancelClearTooltip = getClearTooltip(mCurrentTab);
    }
    else if ((mCurrentTab > TransfersWidget::TYPES_TAB_BASE && mCurrentTab < TransfersWidget::TYPES_LAST) || mCurrentTab == TransfersWidget::SEARCH_TAB)
    {
        if(areAllTransfersCompleted)
        {
            mCancelClearInfo.cancelClearTooltip = getClearTooltip(mCurrentTab);
        }
        else if(isAnyTransferCompleted)
        {
            if(areAllSync || !isAnyTransferActive)
            {
                mCancelClearInfo.cancelClearTooltip = getClearTooltip(mCurrentTab);
            }
            else
            {
                mCancelClearInfo.cancelClearTooltip = getCancelAndClearTooltip(mCurrentTab);
            }
        }
        else
        {
            mCancelClearInfo.cancelClearTooltip = getCancelTooltip(mCurrentTab);
        }
    }
    //For example, FAILED TRANSFERS
    else
    {
        mCancelClearInfo.cancelClearTooltip = getCancelTooltip(mCurrentTab);
    }

    ui->tCancelClearVisible->setToolTip(mCancelClearInfo.cancelClearTooltip);
}

void TransfersWidget::updatePauseResumeButtonTooltip()
{
    ui->tPauseResumeVisible->isChecked() ?  ui->tPauseResumeVisible->setToolTip(getResumeTooltip(mCurrentTab))
                                          : ui->tPauseResumeVisible->setToolTip(getPauseTooltip(mCurrentTab));
}

void TransfersWidget::textFilterChanged(const QString& pattern)
{
    mProxyModel->setFilterFixedString(pattern);
    ui->tvTransfers->scrollToTop();
}

void TransfersWidget::textFilterTypeChanged(const TransferData::TransferTypes transferTypes)
{
    mProxyModel->setFilters(transferTypes,{},{});
    mProxyModel->textSearchTypeChanged();
}

void TransfersWidget::filtersChanged(const TransferData::TransferTypes transferTypes,
                                     const TransferData::TransferStates transferStates,
                                     const Utilities::FileTypes fileTypes)
{
    mProxyModel->setFilters(transferTypes, transferStates, fileTypes);
}

void TransfersWidget::transferFilterReset()
{
    mProxyModel->resetAllFilters();
}

void TransfersWidget::mouseRelease(const QPoint &point)
{
   if(ui->tvTransfers->isVisible())
   {
       if(!ui->tvTransfers->selectionModel()->selection().isEmpty())
       {
           auto viewGlobalPos = parentWidget()->mapToGlobal(ui->tvTransfers->pos());
           QRect viewGlobalRect(viewGlobalPos, ui->tvTransfers->size());

           auto pressedOnView = viewGlobalRect.contains(point);
           if(!pressedOnView)
           {
               ui->tvTransfers->clearSelection();
           }
       }
   }
}

void TransfersWidget::setCurrentTab(TM_TAB tab)
{
    mCurrentTab = tab;
}

TransfersWidget::TM_TAB TransfersWidget::getCurrentTab()
{
    return mCurrentTab;
}

void TransfersWidget::updateHeaders()
{
    //Now, you can check the buttons as the filter is finished
    updateHeaderItems();

    onCheckCancelClearButton();
    onCheckPauseResumeButton();
}

void TransfersWidget::updateHeaderItems()
{
    // Show pause button on tab except completed tab,
    // and set Clear All button string,
    // Emit wether we are showing completed or not
    if (mCurrentTab == TransfersWidget::ALL_TRANSFERS_TAB)
    {
        mHeaderInfo.headerTime = tr("Time left");
        mHeaderInfo.headerSpeed = tr("Speed");
    }
    else
    {
        if (mCurrentTab == TransfersWidget::COMPLETED_TAB)
        {
            mHeaderInfo.headerTime = tr("Time completed");
            mHeaderInfo.headerSpeed = tr("Avg. speed");
        }
        else if (mCurrentTab == TransfersWidget::FAILED_TAB)
        {
            mHeaderInfo.headerTime = tr("Time completed");
            mHeaderInfo.headerSpeed = tr("Avg. speed");
        }
        else if (mCurrentTab == TransfersWidget::SEARCH_TAB || (mCurrentTab > TransfersWidget::TYPES_TAB_BASE && mCurrentTab < TransfersWidget::TYPES_LAST))
        {
            mHeaderInfo.headerTime = tr("Time");
            mHeaderInfo.headerSpeed = tr("Speed");
        }
        //UPLOAD // DOWNLOAD
        else
        {
            mHeaderInfo.headerTime = tr("Time left");
            mHeaderInfo.headerSpeed = tr("Speed");
        }
    }

    ui->timeColumn->setTitle(mHeaderInfo.headerTime);
    ui->speedColumn->setTitle(mHeaderInfo.headerSpeed);
}

void TransfersWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        updateHeaderItems();
    }

    QWidget::changeEvent(event);
}

bool TransfersWidget::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->tCancelClearVisible && event->type() == QEvent::ToolTip)
    {
        updateCancelClearButtonTooltip();
    }
    else if(watched == ui->tPauseResumeVisible && event->type() == QEvent::ToolTip)
    {
        updatePauseResumeButtonTooltip();
    }
    else if(watched == ui->tvTransfers && event->type() == QEvent::Show)
    {
        onVerticalScrollBarVisibilityChanged(ui->tvTransfers->verticalScrollBar()->isVisible());
    }

    return QWidget::eventFilter(watched, event);
}

bool TransfersWidget::isLoadingViewSet()
{
    return mLoadingScene.isLoadingViewSet();
}

void TransfersWidget::setScanningWidgetVisible(bool state)
{
    mScanningIsActive = state;

    if(!state && mLoadingScene.isLoadingViewSet())
    {
        emit disableTransferManager(true);
    }
    else if(state && mLoadingScene.isLoadingViewSet())
    {
        emit disableTransferManager(false);
    }
}

void TransfersWidget::onUiBlocked()
{
    ui->tvTransfers->blockSignals(true);
    ui->tvTransfers->header()->blockSignals(true);

    mLoadingScene.toggleLoadingScene(true);

    if(!mScanningIsActive)
    {
        emit disableTransferManager(true);
    }
}

void TransfersWidget::onUiUnblocked()
{
    //Set the model to the tvTransfers when the proxy model finishes filtering the first time
    if(!ui->tvTransfers->model())
    {
        ui->tvTransfers->setModel(mProxyModel);
    }

    ui->tvTransfers->header()->blockSignals(false);
    ui->tvTransfers->blockSignals(false);

    mLoadingScene.toggleLoadingScene(false);

    emit disableTransferManager(false);

    mModel->uiUnblocked();
}

void TransfersWidget::onUiUnblockedAndFilter()
{
    if(mLoadingScene.isLoadingViewSet())
    {
        mProxyModel->refreshFilterFixedString();
    }
}

void TransfersWidget::onModelAboutToBeChanged()
{
    onUiBlocked();
}

void TransfersWidget::onModelChanged()
{
    onUiUnblocked();
    updateHeaders();

    selectAndScrollToMovedTransfer();
}

void TransfersWidget::onRowsAboutToBeMoved(int scrollTo)
{
    mScrollToAfterMovingRow.append(scrollTo);

    if(mProxyModel->getSortCriterion() != static_cast<int>(SortCriterion::PRIORITY))
    {
        ui->statusColumn->forceClick();
    }
    else
    {
        selectAndScrollToMovedTransfer(QAbstractItemView::EnsureVisible);
    }
}

void TransfersWidget::selectAndScrollToMovedTransfer(QAbstractItemView::ScrollHint scrollHint)
{
    QTimer::singleShot(200, this, [this, scrollHint]()
    {
        if(!mScrollToAfterMovingRow.isEmpty())
        {
            foreach(auto row, mScrollToAfterMovingRow)
            {
                auto d = app->getTransfersModel()->getTransferByTag(row);
                if(d)
                {
                    auto rowIndex = app->getTransfersModel()->index(app->getTransfersModel()->getRowByTransferTag(d->mTag),0);
                    if(rowIndex.isValid())
                    {
                        auto proxyIndex = mProxyModel->mapFromSource(rowIndex);
                        ui->tvTransfers->selectionModel()->select(proxyIndex, QItemSelectionModel::SelectionFlag::Select);
                        ui->tvTransfers->scrollTo(proxyIndex, scrollHint);
                    }
                }
            }

            mScrollToAfterMovingRow.clear();
        }
    });
}

void TransfersWidget::onPauseResumeTransfer(bool pause)
{
    ui->tvTransfers->onPauseResumeSelection(pause);

    emit transferPauseResumeStateChanged(pause);
}

void TransfersWidget::onCancelClearButtonPressedOnDelegate()
{
    auto selection = ui->tvTransfers->selectionModel()->selection();
    auto sourceSelection= mProxyModel->mapSelectionToSource(selection);
    auto sourceSelectionIndexes = sourceSelection.indexes();

    auto info = ui->tvTransfers->getSelectedCancelOrClearInfo();

    QPointer<TransfersWidget> dialog = QPointer<TransfersWidget>(this);

    if (QMegaMessageBox::warning(this, QString::fromUtf8("MEGAsync"),
                             info.actionText,
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No, info.buttonsText)
            != QMessageBox::Yes
            || !dialog)
    {
        return;
    }

    getModel()->cancelAndClearTransfers(sourceSelectionIndexes, this);
}

void TransfersWidget::onRetryButtonPressedOnDelegate()
{
    auto selection = ui->tvTransfers->selectionModel()->selection();
    auto sourceSelection= mProxyModel->mapSelectionToSource(selection);
    auto sourceSelectionIndexes = sourceSelection.indexes();

    getModel()->retryTransfers(sourceSelectionIndexes);
}

void TransfersWidget::on_tPauseResumeVisible_toggled(bool state)
{
    emit pauseResumeVisibleRows(state);
}

void TransfersWidget::onVerticalScrollBarVisibilityChanged(bool state)
{
    if(ui->tvTransfers->isVisible())
    {
        if(state)
        {
            int sliderWidth = ui->tvTransfers->verticalScrollBar()->width();
            ui->rightMargin->changeSize(sliderWidth,0,QSizePolicy::Fixed, QSizePolicy::Preferred);
        }
        else
        {
            ui->rightMargin->changeSize(0,0,QSizePolicy::Fixed, QSizePolicy::Preferred);
        }

        if(ui->wTableHeaderLayout)
        {
            ui->wTableHeaderLayout->invalidate();
        }
    }
}

QString TransfersWidget::getClearTooltip(TM_TAB tab)
{
    switch(tab)
    {
        case COMPLETED_TAB:
        {
            return tr("Clear all completed");
        }
        case SEARCH_TAB:
        {
            return tr("Clear all search results");
        }
        case TYPE_AUDIO_TAB:
        {
            return tr("Clear all audios");
        }
        case TYPE_VIDEO_TAB:
        {
            return tr("Clear all videos");
        }
        case TYPE_ARCHIVE_TAB:
        {
            return tr("Clear all archives");
        }
        case TYPE_DOCUMENT_TAB:
        {
            return tr("Clear all documents");
        }
        case TYPE_IMAGE_TAB:
        {
            return tr("Clear all images");
        }
        default:
        {
            return tr("Clear all transfers");
        }
    }
}

QString TransfersWidget::getCancelTooltip(TM_TAB tab)
{
    switch(tab)
    {
        case DOWNLOADS_TAB:
        {
            return tr("Cancel all downloads");
        }
        case UPLOADS_TAB:
        {
            return tr("Cancel all uploads");
        }
        case FAILED_TAB:
        {
            return tr("Cancel all failed");
        }
        case SEARCH_TAB:
        {
            return tr("Cancel all search results");
        }
        case TYPE_AUDIO_TAB:
        {
            return tr("Cancel all audios");
        }
        case TYPE_VIDEO_TAB:
        {
            return tr("Cancel all videos");
        }
        case TYPE_ARCHIVE_TAB:
        {
            return tr("Cancel all archives");
        }
        case TYPE_DOCUMENT_TAB:
        {
            return tr("Cancel all documents");
        }
        case TYPE_IMAGE_TAB:
        {
            return tr("Cancel all images");
        }
        default:
        {
            return tr("Cancel all transfers");
        }
    }
}

QString TransfersWidget::getCancelAndClearTooltip(TM_TAB tab)
{
    switch(tab)
    {
        case SEARCH_TAB:
        {
            return tr("Cancel and clear all search results");
        }
        case TYPE_AUDIO_TAB:
        {
            return tr("Cancel and clear all audios");
        }
        case TYPE_VIDEO_TAB:
        {
            return tr("Cancel and clear all videos");
        }
        case TYPE_ARCHIVE_TAB:
        {
            return tr("Cancel and clear all archives");
        }
        case TYPE_DOCUMENT_TAB:
        {
            return tr("Cancel and clear all documents");
        }
        case TYPE_IMAGE_TAB:
        {
            return tr("Cancel and clear all images");
        }
        default:
        {
            return tr("Cancel and clear all transfers");
        }
    }
}

QString TransfersWidget::getResumeTooltip(TM_TAB tab)
{
    switch(tab)
    {
        case DOWNLOADS_TAB:
        {
            return tr("Resume all downloads");
        }
        case UPLOADS_TAB:
        {
            return tr("Resume all uploads");
        }
        case SEARCH_TAB:
        {
            return tr("Resume all search results");
        }
        case TYPE_AUDIO_TAB:
        {
            return tr("Resume all audios");
        }
        case TYPE_VIDEO_TAB:
        {
            return tr("Resume all videos");
        }
        case TYPE_ARCHIVE_TAB:
        {
            return tr("Resume all archives");
        }
        case TYPE_DOCUMENT_TAB:
        {
            return tr("Resume all documents");
        }
        case TYPE_IMAGE_TAB:
        {
            return tr("Resume all images");
        }
        default:
        {
            return tr("Resume all transfers");
        }
    }
}

QString TransfersWidget::getPauseTooltip(TM_TAB tab)
{
    switch(tab)
    {
        case DOWNLOADS_TAB:
        {
            return tr("Pause all downloads");
        }
        case UPLOADS_TAB:
        {
            return tr("Pause all uploads");
        }
        case SEARCH_TAB:
        {
            return tr("Pause all search results");
        }
        case TYPE_AUDIO_TAB:
        {
            return tr("Pause all audios");
        }
        case TYPE_VIDEO_TAB:
        {
            return tr("Pause all videos");
        }
        case TYPE_ARCHIVE_TAB:
        {
            return tr("Pause all archives");
        }
        case TYPE_DOCUMENT_TAB:
        {
            return tr("Pause all documents");
        }
        case TYPE_IMAGE_TAB:
        {
            return tr("Pause all images");
        }
        default:
        {
            return tr("Pause all transfers");
        }
    }
}
