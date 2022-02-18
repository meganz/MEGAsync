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
    mHeaderSizeState (HS_SORT_PRIORITY),
    mThreadPool(ThreadPoolSingleton::getInstance()),
    mProxyActivityCloseTimer (new QTimer(this)),
    mProxyActivityLaunchTimer (new QTimer(this)),
    mProxyActivityMessage (new QMessageBox(this)),
    mModelIsChanging(false)
{
    ui->setupUi(this);

    model2 = app->getTransfersModel();

    mProxyActivityMessage->setAttribute(Qt::WA_DeleteOnClose, false);

    mProxyActivityLaunchTimer->setSingleShot(true);
    connect(mProxyActivityLaunchTimer, &QTimer::timeout, this, &TransfersWidget::onProxyActivityLaunchTimeout);

    mProxyActivityCloseTimer->setSingleShot(true);
    connect(mProxyActivityCloseTimer, &QTimer::timeout, this, &TransfersWidget::onProxyActivityCloseTimeout);

}
void TransfersWidget::setupTransfers()
{
    mProxyModel = new TransfersSortFilterProxyModel(this);
    mProxyModel->setSourceModel(app->getTransfersModel());
    mProxyModel->sort(TransfersSortFilterProxyModel::SortCriterion::PRIORITY, Qt::DescendingOrder);

    connect(mProxyModel, &TransfersSortFilterProxyModel::modelAboutToBeChanged, this, &TransfersWidget::onModelAboutToBeChanged);
    connect(mProxyModel, &TransfersSortFilterProxyModel::modelChanged, this, &TransfersWidget::onModelChanged);
    connect(mProxyModel, &TransfersSortFilterProxyModel::transferPauseResume, this, &TransfersWidget::onPauseResumeButtonCheckedOnDelegate);

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
    if (!model2)
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

QTransfersModel* TransfersWidget::getModel()
{
    return app->getTransfersModel();
}

void TransfersWidget::on_pHeaderName_clicked()
{
    Qt::SortOrder order (Qt::AscendingOrder);
    TransfersSortFilterProxyModel::SortCriterion sortBy (
                TransfersSortFilterProxyModel::SortCriterion::NAME);

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
            sortBy = TransfersSortFilterProxyModel::SortCriterion::PRIORITY;
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

    mProxyModel->sort(sortBy, order);
    setHeaderState(ui->pHeaderName, mHeaderNameState);
}

void TransfersWidget::on_pHeaderSize_clicked()
{
    Qt::SortOrder order (Qt::AscendingOrder);
    TransfersSortFilterProxyModel::SortCriterion sortBy (
                TransfersSortFilterProxyModel::SortCriterion::TOTAL_SIZE);

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
            sortBy = TransfersSortFilterProxyModel::SortCriterion::PRIORITY;
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

    mProxyModel->sort(sortBy, order);
    setHeaderState(ui->pHeaderSize, mHeaderSizeState);
}

void TransfersWidget::on_tCancelClearVisible_clicked()
{
    QPointer<TransfersWidget> dialog = QPointer<TransfersWidget>(this);

    if (QMegaMessageBox::warning(nullptr, QString::fromUtf8("MEGAsync"),
                             tr("Are you sure you want to %1 the following transfers?").arg(mClearMode ? tr("clear") : tr("cancel")),
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
                             tr("Are you sure you want to %1 all transfers?").arg(mClearMode ? tr("clear") : tr("cancel")),
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
            != QMessageBox::Yes
            || !dialog)
    {
        return;
    }

    emit cancelClearAllRows();
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
    ui->tPauseResumeVisible->setVisible(!showCompleted);
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

void TransfersWidget::filtersChanged(const TransferData::TransferTypes transferTypes,
                                     const TransferData::TransferStates transferStates,
                                     const TransferData::FileTypes fileTypes)
{
    textFilterChanged(QString());
    mProxyModel->setFilters(transferTypes, transferStates, fileTypes);
}

void TransfersWidget::transferFilterReset()
{
    mProxyModel->resetAllFilters();
}

int TransfersWidget::rowCount()
{
    return ui->tvTransfers->model()->rowCount();
}

void TransfersWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}

void TransfersWidget::onModelChanged()
{
    mModelIsChanging = false;

    auto allPaused = mProxyModel->isAnyPaused();
    onPauseStateChanged(allPaused);

    if(!mProxyActivityCloseTimer->isActive())
    {
        mProxyActivityMessage->close();
    }
}

void TransfersWidget::onModelAboutToBeChanged()
{
    mModelIsChanging = true;
    mProxyActivityLaunchTimer->start(std::chrono::milliseconds(PROXY_ACTIVITY_LAUNCH_TIMEOUT_MS));
}

void TransfersWidget::onProxyActivityCloseTimeout()
{
    if(!mModelIsChanging)
    {
        mProxyActivityMessage->close();
    }
}

void TransfersWidget::onProxyActivityLaunchTimeout()
{
    if(mModelIsChanging)
    {
        mProxyActivityCloseTimer->start(std::chrono::milliseconds(PROXY_ACTIVITY_CLOSE_TIMEOUT_MS));

        mProxyActivityMessage->setText(tr("Please, wait..."));
        mProxyActivityMessage->exec();
    }
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
