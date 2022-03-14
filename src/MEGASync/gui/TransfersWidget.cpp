#include "TransfersWidget.h"
#include "ui_TransfersWidget.h"
#include "MegaApplication.h"
#include "QMegaMessageBox.h"

#include <QTimer>
#include <QtConcurrent/QtConcurrent>

TransfersWidget::TransfersWidget(QWidget* parent) :
    QWidget (parent),
    ui (new Ui::TransfersWidget),
    tDelegate (nullptr),
    mClearMode(false),
    app (qobject_cast<MegaApplication*>(qApp)),
    mHeaderNameState (HS_SORT_PRIORITY),
    mHeaderSizeState (HS_SORT_PRIORITY)
{
    ui->setupUi(this);

    //Keep size when hidden
    auto sizePolicy = ui->tCancelClearVisible->sizePolicy();
    if(!sizePolicy.retainSizeWhenHidden())
    {
        sizePolicy.setRetainSizeWhenHidden(true);
        ui->tCancelClearVisible->setSizePolicy(sizePolicy);
    }

    model = app->getTransfersModel();

}
void TransfersWidget::setupTransfers()
{
    mProxyModel = new TransfersManagerSortFilterProxyModel(this);
    mProxyModel->setSourceModel(app->getTransfersModel());
    mProxyModel->sort(static_cast<int>(SortCriterion::PRIORITY), Qt::DescendingOrder);

    connect(mProxyModel, &TransfersManagerSortFilterProxyModel::modelAboutToBeChanged, this, &TransfersWidget::onModelAboutToBeChanged);
    connect(mProxyModel, &TransfersManagerSortFilterProxyModel::modelChanged, this, &TransfersWidget::onModelChanged);
    connect(mProxyModel, &TransfersManagerSortFilterProxyModel::cancelableTransfersChanged, this, &TransfersWidget::onCheckCancelButtonVisibility);
    connect(mProxyModel, &TransfersManagerSortFilterProxyModel::activeTransfersChanged, this, &TransfersWidget::onActiveTransferCounterChanged);
    connect(mProxyModel, &TransfersManagerSortFilterProxyModel::pausedTransfersChanged, this, &TransfersWidget::onPausedTransferCounterChanged);
    connect(mProxyModel, &TransfersManagerSortFilterProxyModel::transferPauseResume, this, &TransfersWidget::onPauseResumeButtonCheckedOnDelegate);
    connect(app->getTransfersModel(), &TransfersModel::transfersAboutToBeCanceled, this, &TransfersWidget::onModelAboutToBeChanged);
    connect(app->getTransfersModel(), &TransfersModel::transfersCanceled, this, &TransfersWidget::onModelChanged);

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

    onPauseStateChanged(model->areAllPaused());

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

void TransfersWidget::on_pHeaderName_clicked()
{
    Qt::SortOrder order (Qt::AscendingOrder);
    SortCriterion sortBy (
               SortCriterion::NAME);

    mHeaderNameState = static_cast<HeaderState>((mHeaderNameState + 1) % HS_NB_STATES);

    switch (mHeaderNameState)
    {
        case HS_SORT_ASCENDING:
        {
            order = Qt::AscendingOrder;
            break;
        }
        case HS_SORT_DESCENDING:
        {
            order = Qt::DescendingOrder;
            break;
        }
        case HS_SORT_PRIORITY:
        {
            order = Qt::DescendingOrder;
            sortBy = SortCriterion::PRIORITY;
            break;
        }
        case HS_NB_STATES: //this never should happen
        {
            break;
        }
    }

    if (mHeaderSizeState != HS_SORT_PRIORITY)
    {
        setHeaderState(ui->pHeaderSize, HS_SORT_PRIORITY);
        mHeaderSizeState = HS_SORT_PRIORITY;
    }

    mProxyModel->sort(static_cast<int>(sortBy), order);
    setHeaderState(ui->pHeaderName, mHeaderNameState);
}

void TransfersWidget::on_pHeaderSize_clicked()
{
    Qt::SortOrder order (Qt::AscendingOrder);
    SortCriterion sortBy (
                SortCriterion::TOTAL_SIZE);

    mHeaderSizeState = static_cast<HeaderState>((mHeaderSizeState + 1) % HS_NB_STATES);

    switch (mHeaderSizeState)
    {
        case HS_SORT_ASCENDING:
        {
            order = Qt::AscendingOrder;
            break;
        }
        case HS_SORT_DESCENDING:
        {
            order = Qt::DescendingOrder;
            break;
        }
        case HS_SORT_PRIORITY:
        {
            order = Qt::DescendingOrder;
            sortBy = SortCriterion::PRIORITY;
            break;
        }
        case HS_NB_STATES: //this never should happen
        {
            break;
        }
    }

    if (mHeaderNameState != HS_SORT_PRIORITY)
    {
        setHeaderState(ui->pHeaderName, HS_SORT_PRIORITY);
        mHeaderNameState = HS_SORT_PRIORITY;
    }

    mProxyModel->sort(static_cast<int>(sortBy), order);
    setHeaderState(ui->pHeaderSize, mHeaderSizeState);
}

void TransfersWidget::on_tCancelClearVisible_clicked()
{
    QPointer<TransfersWidget> dialog = QPointer<TransfersWidget>(this);

    if (QMegaMessageBox::warning(nullptr, QString::fromUtf8("MEGAsync"),
                             tr("Are you sure you want to cancel or clear the following transfers?"),
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
            != QMessageBox::Yes
            || !dialog)
    {
        return;
    }

    emit cancelClearVisibleRows();
}

void TransfersWidget::cancelClearAll()
{
    QPointer<TransfersWidget> dialog = QPointer<TransfersWidget>(this);

    if (QMegaMessageBox::warning(nullptr, QString::fromUtf8("MEGAsync"),
                             tr("Are you sure you want to cancel and clear all transfers?"),
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
            != QMessageBox::Yes
            || !dialog)
    {
        return;
    }

    emit cancelAndClearAllRows();
}

void TransfersWidget::onTransferAdded()
{
    ui->sWidget->setCurrentWidget(ui->pTransfers);
    ui->tvTransfers->scrollToTop();
}

void TransfersWidget::onShowCompleted(bool showCompleted)
{
    if (showCompleted)
    {
        ui->lHeaderTime->setText(tr("Time"));
        ui->tCancelClearVisible->setToolTip(tr("Clear All Visible"));
        ui->lHeaderSpeed->setText(tr("Avg. speed"));
    }
    else
    {
        ui->lHeaderTime->setText(tr("Time left"));
        ui->tCancelClearVisible->setToolTip(tr("Cancel/Clear All Visible"));
        ui->lHeaderSpeed->setText(tr("Speed"));
    }

    mClearMode = showCompleted;
}

void TransfersWidget::onPauseStateChanged(bool pauseState)
{
    ui->tPauseResumeVisible->setToolTip(pauseState ?
                                        tr("Resume visible transfers")
                                      : tr("Pause visible transfers"));
    ui->tPauseResumeVisible->blockSignals(true);
    ui->tPauseResumeVisible->setChecked(pauseState);
    ui->tPauseResumeVisible->blockSignals(false);
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

void TransfersWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}

void TransfersWidget::onModelAboutToBeChanged()
{
    mLoadingScene.setLoadingScene(true);

    emit disableTransferManager(true);
}

void TransfersWidget::onModelChanged()
{
    auto isAnyPaused = mProxyModel->isAnyPaused();
    onPauseStateChanged(isAnyPaused);
    mLoadingScene.setLoadingScene(false);

    emit disableTransferManager(false);

    auto isAnyActive = mProxyModel->isAnyActive();
    onActiveTransferCounterChanged(isAnyActive);
}


void TransfersWidget::onPauseResumeButtonCheckedOnDelegate(bool pause)
{
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
            auto pausedTransfers = mProxyModel->getPausedTransfers() -1;
            onPauseStateChanged(pausedTransfers > 0);
        }
    }
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

void TransfersWidget::on_tPauseResumeVisible_toggled(bool state)
{
    onPauseStateChanged(state);

    emit pauseResumeVisibleRows(state);
}

void TransfersWidget::setHeaderState(QPushButton* header, HeaderState state)
{
    QIcon icon;
    switch (state)
    {
        case HS_SORT_DESCENDING:
        {
            icon = Utilities::getCachedPixmap(QLatin1Literal(":/images/sort_descending.png"));
            break;
        }
        case HS_SORT_ASCENDING:
        {
            icon = Utilities::getCachedPixmap(QLatin1Literal(":/images/sort_ascending.png"));
            break;
        }
        case HS_SORT_PRIORITY:
        default:
        {
            icon = QIcon();
            break;
        }
    }
    header->setIcon(icon);
}
