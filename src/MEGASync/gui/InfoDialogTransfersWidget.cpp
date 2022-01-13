#include "InfoDialogTransfersWidget.h"
#include "ui_InfoDialogTransfersWidget.h"
#include "MegaApplication.h"
#include "MegaTransferDelegate2.h"

using namespace mega;

//SORT FILTER PROXY MODEL
InfoDialogCurrentTransfersProxyModel::InfoDialogCurrentTransfersProxyModel(QObject *parent) : TransfersSortFilterProxyModel(parent)
{
}

InfoDialogCurrentTransfersProxyModel::~InfoDialogCurrentTransfersProxyModel()
{

}

bool InfoDialogCurrentTransfersProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    QMutexLocker lock (mActivityMutex);
    bool lessThan (false);
    const auto leftItem (qvariant_cast<TransferItem2>(left.data()).getTransferData());
    const auto rightItem (qvariant_cast<TransferItem2>(right.data()).getTransferData());

    int leftItemState = leftItem->mState;
    int rightItemState = rightItem->mState;

    if(leftItemState != rightItemState)
    {
       lessThan = leftItemState < rightItemState;
    }
    else
    {
       lessThan = leftItem->mPriority < rightItem->mPriority;
    }

    return lessThan;
}

bool InfoDialogCurrentTransfersProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QMutexLocker lock (mActivityMutex);
    bool accept (false);

    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    if(index.isValid())
    {
       const auto d (qvariant_cast<TransferItem2>(index.data()).getTransferData());

       accept = (d->mState & TransferData::TransferState::TRANSFER_COMPLETED
                 || d->mState & TransferData::TransferState::TRANSFER_COMPLETING
                 || d->mState & TransferData::TransferState::TRANSFER_ACTIVE);
    }

    return accept;
}

//WIDGET

InfoDialogTransfersWidget::InfoDialogTransfersWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InfoDialogTransfersWidget)
{
    ui->setupUi(this);
    this->model = NULL;
    tDelegate = NULL;
    app = (MegaApplication *)qApp;
}

void InfoDialogTransfersWidget::setupTransfers()
{
    model = new InfoDialogCurrentTransfersProxyModel(this);
    model->setSourceModel(app->getTransfersModel());
    model->sort(0);
    model->setDynamicSortFilter(true);

    configureTransferView();
}

InfoDialogTransfersWidget::~InfoDialogTransfersWidget()
{
    delete ui;
    delete model;
    delete tDelegate;
}

void InfoDialogTransfersWidget::configureTransferView()
{
    if (!model)
    {
        return;
    }

    tDelegate = new MegaTransferDelegate(model, ui->tView);
    ui->tView->setup(QTransfersModel::TYPE_CUSTOM_TRANSFERS);
    ui->tView->setItemDelegate((QAbstractItemDelegate *)tDelegate);
    ui->tView->header()->close();
    ui->tView->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tView->setDragEnabled(false);
    ui->tView->viewport()->setAcceptDrops(false);
    ui->tView->setDropIndicatorShown(false);
    ui->tView->setDragDropMode(QAbstractItemView::InternalMove);
    ui->tView->setModel(model);
    ui->tView->setFocusPolicy(Qt::NoFocus);
    ui->tView->disableContextMenus(true);
}
