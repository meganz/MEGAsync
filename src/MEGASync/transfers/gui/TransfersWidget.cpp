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

    mCheckPauseResumeButtonTimer.setInterval(100);
    mCheckCancelClearButtonTimer.setInterval(100);

    connect(&mCheckPauseResumeButtonTimer, &QTimer::timeout, this, &TransfersWidget::onCheckPauseResumeButton);
    connect(&mCheckCancelClearButtonTimer, &QTimer::timeout, this, &TransfersWidget::onCheckCancelClearButton);

    auto leftPaneButtons = ui->wTableHeader->findChildren<QAbstractButton*>();
    foreach(auto& button, leftPaneButtons)
    {
        mButtonIconManager.addButton(button);
    }

    mTooltipNameByTab[ALL_TRANSFERS_TAB] = tr("all transfers");
    mTooltipNameByTab[DOWNLOADS_TAB]     = tr("all downloads");
    mTooltipNameByTab[UPLOADS_TAB]       = tr("all uploads");
    mTooltipNameByTab[COMPLETED_TAB]     = tr("all completed");
    mTooltipNameByTab[FAILED_TAB]        = tr("all failed");
    mTooltipNameByTab[SEARCH_TAB]        = tr("all search results");
    mTooltipNameByTab[TYPE_OTHER_TAB]    = tr("all transfers");
    mTooltipNameByTab[TYPE_AUDIO_TAB]    = tr("all audios");
    mTooltipNameByTab[TYPE_VIDEO_TAB]    = tr("all videos");
    mTooltipNameByTab[TYPE_ARCHIVE_TAB]  = tr("all archives");
    mTooltipNameByTab[TYPE_DOCUMENT_TAB] = tr("all documents");
    mTooltipNameByTab[TYPE_IMAGE_TAB]    = tr("all images");
}
void TransfersWidget::setupTransfers()
{
    mProxyModel = new TransfersManagerSortFilterProxyModel(ui->tvTransfers);
    mProxyModel->setSourceModel(app->getTransfersModel());
    mProxyModel->sort(static_cast<int>(SortCriterion::PRIORITY), Qt::DescendingOrder);

    mProxyModel->setDynamicSortFilter(true);

    connect(mProxyModel, &TransfersManagerSortFilterProxyModel::modelAboutToBeChanged, this, &TransfersWidget::onUiBlocked);
    connect(mProxyModel, &TransfersManagerSortFilterProxyModel::modelChanged, this, &TransfersWidget::onModelChanged);
    connect(mProxyModel, &TransfersManagerSortFilterProxyModel::pauseResumeTransfer, this, &TransfersWidget::onPauseResumeTransfer);
    connect(mProxyModel, &TransfersManagerSortFilterProxyModel::transferCancelClear, this, &TransfersWidget::onCancelClearButtonPressedOnDelegate);
    connect(mProxyModel, &TransfersManagerSortFilterProxyModel::transferRetry, this, &TransfersWidget::onRetryButtonPressedOnDelegate);
    connect(app->getTransfersModel(), &TransfersModel::blockUi, this, &TransfersWidget::onUiBlocked);
    connect(app->getTransfersModel(), &TransfersModel::unblockUi, this, &TransfersWidget::onUiUnblocked);
    connect(app->getTransfersModel(), &TransfersModel::unblockUiAndFilter, this, &TransfersWidget::onUiUnblockedAndFilter);

    configureTransferView();
}

TransfersWidget::~TransfersWidget()
{
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
    ui->tvTransfers->setModel(mProxyModel);
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
        ui->tvTransfers->onCancelAndClearVisibleTransfers();
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
    ui->tPauseResumeVisible->setVisible(mProxyModel->isAnyActive() || mProxyModel->rowCount() == 0);

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

    ui->tPauseResumeVisible->setToolTip(state ?
                                            mHeaderInfo.resumeTooltip
                                          : mHeaderInfo.pauseTooltip);

    //Use to repaint and update the transfers state
    ui->tvTransfers->update();
}

void TransfersWidget::onCheckCancelClearButton()
{
    bool changeIcon(false);

    bool areAllTransfersCompleted(mProxyModel->areAllCompleted());

    auto buttonVisible(!mProxyModel->areAllSync() || areAllTransfersCompleted);
    if(mCancelClearInfo.visible != buttonVisible)
    {
        mCancelClearInfo.visible = buttonVisible;
        ui->tCancelClearVisible->setVisible(mCancelClearInfo.visible);
    }

    QString cancelBase;
    if (mCurrentTab == TransfersWidget::COMPLETED_TAB)
    {        
        if(!mCancelClearInfo.clearAction)
        {
            changeIcon = true;
        }

        mCancelClearInfo.clearAction = true;
        cancelBase = tr("Clear ");

    }
    else if ((mCurrentTab > TransfersWidget::TYPES_TAB_BASE && mCurrentTab < TransfersWidget::TYPES_LAST) || mCurrentTab == TransfersWidget::SEARCH_TAB)
    {        
        if(mCancelClearInfo.clearAction != areAllTransfersCompleted)
        {
            changeIcon = true;
        }

        mCancelClearInfo.clearAction = areAllTransfersCompleted;
        bool isAnyCompleted(mProxyModel->isAnyCompleted());

        if(areAllTransfersCompleted)
        {
            cancelBase = tr("Clear ");
        }
        else if(isAnyCompleted)
        {
            cancelBase = tr("Cancel and clear ");
        }
        else
        {
            cancelBase = tr("Cancel ");
        }
    }
    else
    {
        if(mCancelClearInfo.clearAction)
        {
            changeIcon = true;
        }

        mCancelClearInfo.clearAction = false;
        cancelBase = tr("Cancel ");
    }

    mCancelClearInfo.cancelClearTooltip = cancelBase + mTooltipNameByTab[mCurrentTab];
    ui->tCancelClearVisible->setToolTip(mCancelClearInfo.cancelClearTooltip);

    if(mCancelClearInfo.visible && changeIcon)
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
    updateTimersState();

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
        else if (mCurrentTab > TransfersWidget::TYPES_TAB_BASE && mCurrentTab < TransfersWidget::TYPES_LAST)
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

    mHeaderInfo.pauseTooltip = tr("Pause ") + mTooltipNameByTab[mCurrentTab];
    mHeaderInfo.resumeTooltip = tr("Resume ") + mTooltipNameByTab[mCurrentTab];

    ui->timeColumn->setTitle(mHeaderInfo.headerTime);
    ui->speedColumn->setTitle(mHeaderInfo.headerSpeed);

    ui->tPauseResumeVisible->setToolTip(ui->tPauseResumeVisible->isChecked() ?
                                           mHeaderInfo.resumeTooltip
                                          : mHeaderInfo.pauseTooltip);
}

void TransfersWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}

void TransfersWidget::onDialogHidden()
{
    //Do not check buttons while it is hidden
    mCheckCancelClearButtonTimer.stop();
    mCheckPauseResumeButtonTimer.stop();
}

void TransfersWidget::onDialogShown()
{
    //Check buttons while it is visible
    updateTimersState();
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
    mLoadingScene.setLoadingScene(true);

    if(!mScanningIsActive)
    {
        emit disableTransferManager(true);
    }
}

void TransfersWidget::onUiUnblocked()
{
    mLoadingScene.setLoadingScene(false);
    emit disableTransferManager(false);
}

void TransfersWidget::onUiUnblockedAndFilter()
{
    textFilterChanged(QString());
}

void TransfersWidget::onModelAboutToBeChanged()
{
    //Do not update any
    mCheckCancelClearButtonTimer.stop();
    mCheckPauseResumeButtonTimer.stop();

    onUiBlocked();
}

void TransfersWidget::onModelChanged()
{
    onUiUnblocked();

    updateHeaders();
}

void TransfersWidget::updateTimersState()
{
    //In Completed, pauseResume always hidden and cancelClear button always shows clear icon
    //In Failed, pauseResume always shows play button, and cancelClear  button always show cancel icon
    if (mCurrentTab == TransfersWidget::COMPLETED_TAB || mCurrentTab == TransfersWidget::FAILED_TAB)
    {
        mCheckCancelClearButtonTimer.stop();
        mCheckPauseResumeButtonTimer.stop();
    }
    else if ((mCurrentTab > TransfersWidget::TYPES_TAB_BASE && mCurrentTab < TransfersWidget::TYPES_LAST)
             || mCurrentTab == TransfersWidget::SEARCH_TAB)
    {
        mCheckCancelClearButtonTimer.start();
        mCheckPauseResumeButtonTimer.start();
    }
    //UPLOAD // DOWNLOAD // ALL_TRANSFERS
    //cancelClear button always shows cancel icon
    else
    {
        mCheckCancelClearButtonTimer.stop();
        mCheckPauseResumeButtonTimer.start();
    }

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

    auto action = ui->tvTransfers->getSelectedAction();

    QPointer<TransfersWidget> dialog = QPointer<TransfersWidget>(this);

    if (QMegaMessageBox::warning(this, QString::fromUtf8("MEGAsync"),
                             tr("%1 transfer(s)?", "", sourceSelectionIndexes.size()).arg(action),
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
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

    QPointer<TransfersWidget> dialog = QPointer<TransfersWidget>(this);

    if (QMegaMessageBox::warning(this, QString::fromUtf8("MEGAsync"),
                             tr("Retry transfer(s)?", "", sourceSelectionIndexes.size()),
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
            != QMessageBox::Yes
            || !dialog)
    {
        return;
    }

    getModel()->retryTransfers(sourceSelectionIndexes);
}

void TransfersWidget::on_tPauseResumeVisible_toggled(bool state)
{
    mCheckPauseResumeButtonTimer.stop();

    emit pauseResumeVisibleRows(state);

    updateTimersState();
}

void TransfersWidget::onVerticalScrollBarVisibilityChanged(bool state)
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
