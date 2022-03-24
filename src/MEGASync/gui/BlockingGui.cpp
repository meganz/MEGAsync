#include "BlockingGui.h"

BlockingUi::BlockingUi(QStackedWidget* _container)
    : mContainer(_container)
{
    mBlockingWidget = new ScanningWidget();
    mConfirmWidget = new CancelConfirmWidget();
    mContainer->addWidget(mBlockingWidget);
    mContainer->addWidget(mConfirmWidget);

    connect(mBlockingWidget, &ScanningWidget::cancel, this, &BlockingUi::onCancelClicked);
    connect(mConfirmWidget, &CancelConfirmWidget::proceed, this, &BlockingUi::onCancelConfirmed);
    connect(mConfirmWidget, &CancelConfirmWidget::dismiss, this, &BlockingUi::onCancelDismissed);

    QString styles = QString::fromLatin1(getControlStyles());
    mBlockingWidget->setStyleSheet(styles);
    mConfirmWidget->setStyleSheet(styles);
}

BlockingUi::~BlockingUi()
{
    delete mBlockingWidget;
    delete mConfirmWidget;
}

void BlockingUi::show()
{
    mLastSelectedWidget = mContainer->currentWidget();
    mContainer->setCurrentWidget(mBlockingWidget);
    mBlockingWidget->show();
}

void BlockingUi::hide()
{
    mContainer->setCurrentWidget(mLastSelectedWidget);
}

bool BlockingUi::isActive()
{
    return (mContainer->currentWidget() == mBlockingWidget) ||
           (mContainer->currentWidget() == mConfirmWidget);
}

void BlockingUi::onCancelClicked()
{
    mContainer->setCurrentWidget(mConfirmWidget);
    mConfirmWidget->show();
    mBlockingWidget->hide();
}

void BlockingUi::onCancelConfirmed()
{
    emit cancelTransfers();
}

void BlockingUi::onCancelDismissed()
{
    mContainer->setCurrentWidget(mBlockingWidget);
    mBlockingWidget->show();
}

const char* BlockingUi::getControlStyles()
{
    const char* styles = "*[role=\"title\"] {font-size: 18px; font:bold;}"
                         "*[role=\"details\"] {font-size: 12px;}"
                         "QPushButton {font-size: 12px; padding: 10px}"
                         "QPushButton#pProceed {background-color: #00BFA5;}";
    return styles;
}
