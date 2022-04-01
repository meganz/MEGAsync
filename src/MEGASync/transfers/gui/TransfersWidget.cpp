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
    app (qobject_cast<MegaApplication*>(qApp))
{
    ui->setupUi(this);

    ui->statusColumn->setSortOrder(Qt::DescendingOrder);

    connect(ui->nameColumn, &TransferWidgetHeaderItem::toggled, this, &TransfersWidget::onHeaderItemClicked);
    connect(ui->sizeColumn, &TransferWidgetHeaderItem::toggled, this, &TransfersWidget::onHeaderItemClicked);
    connect(ui->speedColumn, &TransferWidgetHeaderItem::toggled, this, &TransfersWidget::onHeaderItemClicked);
    connect(ui->statusColumn, &TransferWidgetHeaderItem::toggled, this, &TransfersWidget::onHeaderItemClicked);
    connect(ui->timeColumn, &TransferWidgetHeaderItem::toggled, this, &TransfersWidget::onHeaderItemClicked);


    //Keep size when hidden
    auto sizePolicy = ui->tCancelClearVisible->sizePolicy();
    if(!sizePolicy.retainSizeWhenHidden())
    {
        sizePolicy.setRetainSizeWhenHidden(true);
        ui->tCancelClearVisible->setSizePolicy(sizePolicy);
        ui->tPauseResumeVisible->setSizePolicy(sizePolicy);
    }

    model = app->getTransfersModel();

    //Align header pause/cancel buttons to view pause/cancel button
    connect(ui->tvTransfers, &MegaTransferView::verticalScrollBarVisibilityChanged, this, &TransfersWidget::onVerticalScrollBarVisibilityChanged);
}
void TransfersWidget::setupTransfers()
{
    mProxyModel = new TransfersManagerSortFilterProxyModel(this);
    mProxyModel->setSourceModel(app->getTransfersModel());
    mProxyModel->sort(static_cast<int>(SortCriterion::PRIORITY), Qt::DescendingOrder);

    mProxyModel->setDynamicSortFilter(true);

    connect(mProxyModel, &TransfersManagerSortFilterProxyModel::modelAboutToBeChanged, this, &TransfersWidget::onModelAboutToBeChanged);
    connect(mProxyModel, &TransfersManagerSortFilterProxyModel::modelChanged, this, &TransfersWidget::onModelChanged);
    connect(mProxyModel, &TransfersManagerSortFilterProxyModel::cancelableTransfersChanged, this, &TransfersWidget::onCheckCancelButtonVisibility);
    connect(mProxyModel, &TransfersManagerSortFilterProxyModel::activeTransfersChanged, this, &TransfersWidget::onActiveTransferCounterChanged);
    connect(mProxyModel, &TransfersManagerSortFilterProxyModel::pausedTransfersChanged, this, &TransfersWidget::onPausedTransferCounterChanged);
    connect(mProxyModel, &TransfersManagerSortFilterProxyModel::transferPauseResume, this, &TransfersWidget::onPauseResumeButtonCheckedOnDelegate);
    connect(mProxyModel, &TransfersManagerSortFilterProxyModel::transferCancelClear, this, &TransfersWidget::onCancelClearButtonPressedOnDelegate);
    connect(mProxyModel, &TransfersManagerSortFilterProxyModel::transferRetry, this, &TransfersWidget::onRetryButtonPressedOnDelegate);
    connect(app->getTransfersModel(), &TransfersModel::blockUi, this, &TransfersWidget::onUiBlocked);
    connect(app->getTransfersModel(), &TransfersModel::unblockUi, this, &TransfersWidget::onUiUnblocked);

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
    if (!model)
    {
        return;
    }

    tDelegate = new MegaTransferDelegate(mProxyModel, ui->tvTransfers);
    ui->tvTransfers->setup(this);
    mDelegateHoverManager.setView(ui->tvTransfers);
    ui->tvTransfers->setItemDelegate(tDelegate);

    onPauseStateChanged(mProxyModel->isAnyPaused());

    ui->tvTransfers->setModel(mProxyModel);

    ui->tvTransfers->setDragEnabled(true);
    ui->tvTransfers->viewport()->setAcceptDrops(true);
    ui->tvTransfers->setDropIndicatorShown(true);
    ui->tvTransfers->setDragDropMode(QAbstractItemView::InternalMove);

    mLoadingScene.setView(ui->tvTransfers);
}

void TransfersWidget::pausedTransfers(bool paused)
{
    ui->tPauseResumeVisible->setChecked(paused);
    ui->sWidget->setCurrentWidget(ui->pTransfers);
}

void TransfersWidget::disableGetLink(bool disable)
{
    ui->tvTransfers->disableGetLink(disable);
}

TransfersModel* TransfersWidget::getModel()
{
    return app->getTransfersModel();
}

void TransfersWidget::onHeaderItemClicked(int sortBy, Qt::SortOrder order)
{
    mProxyModel->sort(sortBy, order);
}

void TransfersWidget::on_pHeaderSize_clicked()
{
    Qt::SortOrder order (Qt::AscendingOrder);
    SortCriterion sortBy (
                SortCriterion::TOTAL_SIZE);

    mProxyModel->sort(static_cast<int>(sortBy), order);
}

void TransfersWidget::on_tCancelClearVisible_clicked()
{
    emit cancelClearVisibleRows();
}

void TransfersWidget::onPauseStateChanged(bool pauseState)
{
    ui->tPauseResumeVisible->setToolTip(pauseState ?
                                        tr("Resume visible transfers")
                                      : tr("Pause visible transfers"));
    ui->tPauseResumeVisible->blockSignals(true);
    ui->tPauseResumeVisible->setChecked(pauseState);
    ui->tPauseResumeVisible->blockSignals(false);

    //Use to repaint and update the transfers state
    ui->tvTransfers->update();
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

void TransfersWidget::updateHeaderItems(const QString &headerTime, const QString &cancelClearTooltip, const QString &headerSpeed)
{
    ui->timeColumn->setTitle(headerTime);
    ui->tCancelClearVisible->setToolTip(cancelClearTooltip);
    ui->speedColumn->setTitle(headerSpeed);
}

void TransfersWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}

void TransfersWidget::onUiBlocked()
{
    mLoadingScene.setLoadingScene(true);

    emit disableTransferManager(true);
}

void TransfersWidget::onUiUnblocked()
{
    mLoadingScene.setLoadingScene(false);

    emit disableTransferManager(false);
}

void TransfersWidget::onModelAboutToBeChanged()
{
    onUiBlocked();
}

void TransfersWidget::onModelChanged()
{
    auto isAnyPaused = mProxyModel->isAnyPaused();
    onPauseStateChanged(isAnyPaused);

    auto isAnyActive = mProxyModel->isAnyActive();
    onActiveTransferCounterChanged(isAnyActive);

    onUiUnblocked();
}

void TransfersWidget::onPauseResumeButtonCheckedOnDelegate(bool pause)
{
    auto selection = ui->tvTransfers->selectionModel()->selection();
    auto sourceSelection= mProxyModel->mapSelectionToSource(selection);
    auto sourceSelectionIndexes = sourceSelection.indexes();

    foreach(auto& selectedSourceIndex, sourceSelectionIndexes)
    {
        getModel()->pauseResumeTransferByIndex(selectedSourceIndex,
                                               pause);
    }

    auto rows = mProxyModel->rowCount();

    if(rows == 1)
    {
        onPauseStateChanged(pause);
    }
    else
    {
        if(pause)
        {
            onPauseStateChanged(true);
        }
        else
        {
            //Reduce by one as the resume transfer is still unpaused
            //auto pausedTransfers = mProxyModel->getPausedTransfers() -1;
            onPauseStateChanged(mProxyModel->isAnyPaused());
        }
    }
}

void TransfersWidget::onCancelClearButtonPressedOnDelegate()
{
    auto selection = ui->tvTransfers->selectionModel()->selection();
    auto sourceSelection= mProxyModel->mapSelectionToSource(selection);
    auto sourceSelectionIndexes = sourceSelection.indexes();

    QPointer<TransfersWidget> dialog = QPointer<TransfersWidget>(this);

    if (QMegaMessageBox::warning(this, QString::fromUtf8("MEGAsync"),
                             tr("Are you sure you want to cancel or clear the following transfer(s)?", "", sourceSelectionIndexes.size()),
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
            != QMessageBox::Yes
            || !dialog)
    {
        return;
    }

    getModel()->cancelTransfers(sourceSelectionIndexes, this);
}

void TransfersWidget::onRetryButtonPressedOnDelegate()
{
    auto selection = ui->tvTransfers->selectionModel()->selection();
    auto sourceSelection= mProxyModel->mapSelectionToSource(selection);
    auto sourceSelectionIndexes = sourceSelection.indexes();

    QPointer<TransfersWidget> dialog = QPointer<TransfersWidget>(this);

    if (QMegaMessageBox::warning(this, QString::fromUtf8("MEGAsync"),
                             tr("Are you sure you want to retry the following transfer(s)?", "", sourceSelectionIndexes.size()),
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
            != QMessageBox::Yes
            || !dialog)
    {
        return;
    }

    getModel()->retryTransfers(sourceSelectionIndexes);
}

void TransfersWidget::onCheckCancelButtonVisibility(bool state)
{
    ui->tCancelClearVisible->setVisible(state);
}

void TransfersWidget::onActiveTransferCounterChanged(bool state)
{
    ui->tPauseResumeVisible->setVisible(state);
}

void TransfersWidget::onPausedTransferCounterChanged(bool state)
{
    onPauseStateChanged(state);
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

void TransfersWidget::on_tPauseResumeVisible_toggled(bool state)
{
    onPauseStateChanged(state);

    emit pauseResumeVisibleRows(state);
}
