#include "TransfersWidget.h"
#include "ui_TransfersWidget.h"
#include "MegaApplication.h"

#include <QTimer>
#include <QtConcurrent/QtConcurrent>

const int TransfersWidget::PROXY_ACTIVITY_TIMEOUT_MS;

TransfersWidget::TransfersWidget(QWidget* parent) :
    QWidget (parent),
    ui (new Ui::TransfersWidget),
    tDelegate2 (nullptr),
    mIsPaused (false),
    app (qobject_cast<MegaApplication*>(qApp)),
    mHeaderNameState (HS_SORT_PRIORITY),
    mHeaderSizeState (HS_SORT_PRIORITY),
    mThreadPool(ThreadPoolSingleton::getInstance()),
    mProxyActivityTimer (new QTimer(this)),
    mProxyActivityMessage (new QMessageBox(this))
{
    ui->setupUi(this);

    model2 = app->getTransfersModel();
}
void TransfersWidget::setupTransfers()
{
    mProxyModel = new TransfersSortFilterProxyModel(this);
    mProxyModel->setSourceModel(app->getTransfersModel());
    mProxyModel->sort(TransfersSortFilterProxyModel::SortCriterion::PRIORITY, Qt::DescendingOrder);

    configureTransferView();
}

TransfersWidget::~TransfersWidget()
{
    delete ui;
    if (tDelegate2) delete tDelegate2;
    if (mProxyModel) delete mProxyModel;
}

void TransfersWidget::configureTransferView()
{
    if (!model2)
    {
        return;
    }

    tDelegate2 = new MegaTransferDelegate(mProxyModel, ui->tvTransfers);
    ui->tvTransfers->setup(this);
    QtConcurrent::run([=]
    {
        ui->tvTransfers->setModel(mProxyModel);
    });
    ui->tvTransfers->setItemDelegate(tDelegate2);
    onPauseStateChanged(model2->areAllPaused());

    mProxyActivityTimer->setSingleShot(true);
    connect(mProxyActivityTimer, &QTimer::timeout,
            mProxyActivityMessage, &QMessageBox::exec);

    connect(mProxyModel, &TransfersSortFilterProxyModel::modelAboutToBeSorted,
            this, [this]
    {
        mProxyActivityMessage->setText(tr("Sorting..."));
        mProxyActivityTimer->start(std::chrono::milliseconds(PROXY_ACTIVITY_TIMEOUT_MS));
    });

    connect(mProxyModel, &TransfersSortFilterProxyModel::modelSorted,
            this, [this]
    {
        mProxyActivityTimer->stop();
        mProxyActivityMessage->hide();
        ui->tvTransfers->update();
    });

    connect(mProxyModel, &TransfersSortFilterProxyModel::modelAboutToBeFiltered,
            this, [this]
    {
        mProxyActivityMessage->setText(tr("Filtering..."));
        mProxyActivityTimer->start(std::chrono::milliseconds(PROXY_ACTIVITY_TIMEOUT_MS));
    });

    connect(mProxyModel, &TransfersSortFilterProxyModel::modelFiltered,
            this, [this]
    {
        mProxyActivityTimer->stop();
        mProxyActivityMessage->hide();
    });

    ui->tvTransfers->setDragEnabled(true);
    ui->tvTransfers->viewport()->setAcceptDrops(true);
    ui->tvTransfers->setDropIndicatorShown(true);
    ui->tvTransfers->setDragDropMode(QAbstractItemView::InternalMove);
}

void TransfersWidget::pausedTransfers(bool paused)
{
    mIsPaused = paused;
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

void TransfersWidget::on_tPauseResumeAll_clicked()
{
    onPauseStateChanged(!mIsPaused);
    emit pauseResumeAllRows(mIsPaused);
}

void TransfersWidget::on_tCancelAll_clicked()
{
    emit cancelClearAllRows(true, true);
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
        ui->tCancelAll->setToolTip(tr("Clear All"));
        ui->lHeaderSpeed->setText(tr("Avg. speed"));
    }
    else
    {
        ui->lHeaderTime->setText(tr("Time left"));
        ui->tCancelAll->setToolTip(tr("Cancel or Clear All"));
        ui->lHeaderSpeed->setText(tr("Speed"));
    }

    ui->tPauseResumeAll->setVisible(!showCompleted);
}

void TransfersWidget::onPauseStateChanged(bool pauseState)
{
    ui->tPauseResumeAll->setIcon(pauseState ?
                                     QIcon(QString::fromUtf8(":/images/lists_resume_all_ico.png"))
                                   : QIcon(QString::fromUtf8(":/images/lists_pause_all_ico.png")));
    ui->tPauseResumeAll->setToolTip(pauseState ?
                                        tr("Resume visible transfers")
                                      : tr("Pause visible transfers"));
    mIsPaused = pauseState;
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
    mProxyModel->setFilters(transferTypes, transferStates, fileTypes);
}

void TransfersWidget::transferFilterReset(bool invalidate)
{
    mProxyModel->resetAllFilters(invalidate);
}

void TransfersWidget::transferFilterApply(bool invalidate)
{
    if (!mProxyModel->dynamicSortFilter())
    {
        mProxyModel->applyFilters(false);
        mProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
        mProxyModel->setDynamicSortFilter(true);
    }
    else
    {
        mProxyModel->resetNumberOfItems();
        mProxyModel->applyFilters(invalidate);
    }
    ui->tvTransfers->scrollToTop();
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
