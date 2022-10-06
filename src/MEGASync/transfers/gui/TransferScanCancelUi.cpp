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
    connect(mConfirmWidget, &CancelConfirmWidget::dismiss, this, &TransferScanCancelUi::onCancelDismissed);

    QString styles = QString::fromLatin1(getControlStyles());
    mBlockingWidget->setStyleSheet(styles);
    mConfirmWidget->setStyleSheet(styles);
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
    mBlockingWidget->show();
}

const char* TransferScanCancelUi::getControlStyles()
{
    const char* styles =    "*{ font-family: Lato; }"
                            "*[role=\"title\"] { "
                            "   font-size: 18px; font-weight:500;"
                            "   color: #333333;"
                            "}"
                            "*[role=\"details\"] {font-size: 14px; color: #666666;}"
                            "QPushButton { "
                            "   font-size: 16px; font-weight:400;"
                            "   padding-top : 11px; padding-bottom : 12px; padding-left : 18px; padding-right : 18px;"
                            "   background-color : #FCFCFC;"
                            "   border-style: solid; border-width: 1px; border-color: #d7d6d5; border-radius: 3px;"
                            "   color: #333333;"
                            "}"
                            "QPushButton:pressed { background-color : rgb(238, 238, 236); }"
                            "QPushButton:disabled { background-color : rgb(218, 218, 216); }"
                            "QPushButton#pProceed { background-color: #00BFA5; color: #FFFFFF; border-color: #00AC94}"
                            "QPushButton#pProceed:pressed { background-color: rgb(0, 179, 155);}"
                            "QPushButton#pProceed:disabled { background-color: rgb(70, 159, 135);}";
    return styles;
}
