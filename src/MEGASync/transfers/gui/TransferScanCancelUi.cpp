#include "TransferScanCancelUi.h"

TransferScanCancelUi::TransferScanCancelUi(QStackedWidget* _container, QWidget *_finishedWidget)
    : mContainer(_container), mFinishedWidget(_finishedWidget)
{
    mBlockingWidget = new ScanningWidget(mContainer);
    mConfirmWidget = new CancelConfirmWidget(mContainer);
    mContainer->addWidget(mBlockingWidget);
    mContainer->addWidget(mConfirmWidget);

    connect(mBlockingWidget, &ScanningWidget::cancel, this, &TransferScanCancelUi::onCancelClicked);
    connect(mConfirmWidget, &CancelConfirmWidget::proceed, this, &TransferScanCancelUi::cancelTransfers);
    connect(mConfirmWidget,
            &CancelConfirmWidget::dismiss,
            this,
            &TransferScanCancelUi::onCancelDismissed);
}

void TransferScanCancelUi::show()
{
    mLastSelectedWidget = mContainer->currentWidget();
    mContainer->setCurrentWidget(mBlockingWidget);
    mBlockingWidget->show();
}

void TransferScanCancelUi::hide(bool fromCancellation)
{
    QWidget* widget = fromCancellation ? mLastSelectedWidget : mFinishedWidget;
    mContainer->setCurrentWidget(widget);
}

void TransferScanCancelUi::disableCancelling()
{
    mBlockingWidget->disableCancelButton();
    if (mContainer->currentWidget() != mBlockingWidget)
    {
        mContainer->setCurrentWidget(mBlockingWidget);
    }
}

void TransferScanCancelUi::setInCancellingStage()
{
    mConfirmWidget->setInCancellingStage();
    if (mContainer->currentWidget() != mConfirmWidget)
    {
        mContainer->setCurrentWidget(mConfirmWidget);
    }
}

void TransferScanCancelUi::update()
{
    if (mContainer->currentWidget() == mBlockingWidget)
    {
        mBlockingWidget->updateAnimation();
    }
}

bool TransferScanCancelUi::isActive()
{
    return (mContainer->currentWidget() == mBlockingWidget) ||
           (mContainer->currentWidget() == mConfirmWidget);
}

void TransferScanCancelUi::onFolderTransferUpdate(const FolderTransferUpdateEvent & event)
{
    if (mContainer->currentWidget() == mBlockingWidget)
    {
        mBlockingWidget->onReceiveStatusUpdate(event);
    }
}

void TransferScanCancelUi::onCancelClicked()
{
    mContainer->setCurrentWidget(mConfirmWidget);
    mConfirmWidget->show();
    mBlockingWidget->hide();
}

void TransferScanCancelUi::onCancelDismissed()
{
    mContainer->setCurrentWidget(mBlockingWidget);

    //If the dismiss button has been pressed it´s because the cancel button was visible, so it should be again visible
    mBlockingWidget->show();
}
