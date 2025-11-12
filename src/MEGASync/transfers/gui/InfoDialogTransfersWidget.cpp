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

    connect(mProxyModel,
            &InfoDialogTransfersProxyModel::dataChanged,
            this,
            &InfoDialogTransfersWidget::onProxyModelModified);

    connect(mProxyModel,
            &InfoDialogTransfersProxyModel::rowsInserted,
            this,
            &InfoDialogTransfersWidget::onProxyModelModified);

    connect(mProxyModel,
            &InfoDialogTransfersProxyModel::rowsRemoved,
            this,
            &InfoDialogTransfersWidget::onProxyModelModified);

    mProxyModel->setSourceModel(MegaSyncApp->getTransfersModel());
    mProxyModel->sort(0);
    mProxyModel->setDynamicSortFilter(true);

    configureTransferView();
}

InfoDialogTransfersWidget::~InfoDialogTransfersWidget()
{
    delete mUi;
    delete mProxyModel;
}

void InfoDialogTransfersWidget::showEvent(QShowEvent* event)
{
    onProxyModelModified();

    QWidget::showEvent(event);
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

void InfoDialogTransfersWidget::onProxyModelModified()
{
    if (isVisible())
    {
        auto topTransferType = getTopTransferType();
        if (topTransferType)
        {
            emit topTransferTypeChanged(topTransferType.value());
        }
    }
}
