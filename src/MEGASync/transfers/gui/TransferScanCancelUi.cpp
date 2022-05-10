#include "TransferScanCancelUi.h"

TransferScanCancelUi::TransferScanCancelUi(QStackedWidget* _container)
    : mContainer(_container)
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

void TransferScanCancelUi::hide()
{
    mContainer->setCurrentWidget(mLastSelectedWidget);
}

void TransferScanCancelUi::update()
{
    if (mContainer->currentWidget() == mBlockingWidget)
        mBlockingWidget->updateAnimation();
}

bool TransferScanCancelUi::isActive()
{
    return (mContainer->currentWidget() == mBlockingWidget) ||
           (mContainer->currentWidget() == mConfirmWidget);
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
    const char* styles = "*[role=\"title\"] {font-size: 18px; font:bold;}"
                         "*[role=\"details\"] {font-size: 12px;}"
                         "QPushButton {font-size: 12px; padding: 10px}"
                         "QPushButton#pProceed {background-color: #00BFA5;}";
    return styles;
}
