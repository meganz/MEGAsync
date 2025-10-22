#include "InfoDialogTransfersWidget.h"

#include "ui_InfoDialogTransfersWidget.h"
#include "MegaApplication.h"
#include "MegaTransferDelegate.h"
#include "InfoDialogTransfersProxyModel.h"

using namespace mega;

//WIDGET

InfoDialogTransfersWidget::InfoDialogTransfersWidget(QWidget *parent) :
    QWidget(parent),
    mUi(new Ui::InfoDialogTransfersWidget),
    mProxyModel(nullptr)
{
    mUi->setupUi(this);
}

void InfoDialogTransfersWidget::setupTransfers()
{
    mProxyModel = new InfoDialogTransfersProxyModel(mUi->tView);
    mProxyModel->setSourceModel(MegaSyncApp->getTransfersModel());
    mProxyModel->sort(0);
    mProxyModel->setDynamicSortFilter(true);

    connect(mProxyModel,
            &InfoDialogTransfersProxyModel::dataChanged,
            this,
            &InfoDialogTransfersWidget::onProxyDataChanged);

    configureTransferView();
}

InfoDialogTransfersWidget::~InfoDialogTransfersWidget()
{
    delete mUi;
    delete mProxyModel;
}

void InfoDialogTransfersWidget::showEvent(QShowEvent*)
{
}

void InfoDialogTransfersWidget::onUiBlocked()
{
    mUi->tView->loadingView().toggleLoadingScene(true);
}

void InfoDialogTransfersWidget::onUiUnblocked()
{
    mUi->tView->loadingView().toggleLoadingScene(false);
}

void InfoDialogTransfersWidget::configureTransferView()
{
    if (!mProxyModel)
    {
        return;
    }

    auto tDelegate = new MegaTransferDelegate(mProxyModel, mUi->tView);
    mUi->tView->setItemDelegate(tDelegate);
    mUi->tView->header()->close();
    mUi->tView->setSelectionMode(QAbstractItemView::NoSelection);
    mUi->tView->setDragEnabled(false);
    mUi->tView->viewport()->setAcceptDrops(false);
    mUi->tView->setDropIndicatorShown(false);
    mUi->tView->setDragDropMode(QAbstractItemView::InternalMove);
    mUi->tView->setModel(mProxyModel);
    mUi->tView->setFocusPolicy(Qt::NoFocus);

    connect(MegaSyncApp->getTransfersModel(), &TransfersModel::blockUi, this, &InfoDialogTransfersWidget::onUiBlocked);
    connect(MegaSyncApp->getTransfersModel(), &TransfersModel::unblockUi, this, &InfoDialogTransfersWidget::onUiUnblocked);
    connect(MegaSyncApp->getTransfersModel(), &TransfersModel::unblockUiAndFilter, this, &InfoDialogTransfersWidget::onUiUnblocked);

    mViewHoverManager.setView(mUi->tView);
}

std::optional<TransferData::TransferTypes> InfoDialogTransfersWidget::getTopTransferType()
{
    if (!mProxyModel)
        return std::nullopt;

    QModelIndex firstProxyIndex = mProxyModel->index(0, 0);
    if (!firstProxyIndex.isValid())
        return std::nullopt;

    auto transferData(qvariant_cast<TransferItem>(firstProxyIndex.data()).getTransferData());
    return transferData->mType;
}

void InfoDialogTransfersWidget::onProxyDataChanged(const QModelIndex& topLeft,
                                                   const QModelIndex& bottomRight,
                                                   const QVector<int>& roles)
{
    if (topLeft.row() != 0)
    {
        return;
    }
    auto topTransferType = getTopTransferType();
    if (topTransferType)
    {
        emit topTransferTypeChanged(topTransferType.value());
    }
}
