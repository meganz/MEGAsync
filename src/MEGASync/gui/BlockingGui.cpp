#include "blockingui.h"

BlockingUi::BlockingUi(QStackedWidget* _container)
    : container(_container)
{
    blockingWidget = new ScanningWidget();
    confirmWidget = new CancelConfirmWidget();
    container->addWidget(blockingWidget);
    container->addWidget(confirmWidget);

    connect(blockingWidget, SIGNAL(cancel()), this, SLOT(onCancelClicked()));
    connect(confirmWidget, SIGNAL(proceed()), this, SLOT(onCancelConfirmed()));
    connect(confirmWidget, SIGNAL(dismiss()), this, SLOT(onCancelDismissed()));
}

BlockingUi::~BlockingUi()
{
    delete blockingWidget;
    delete confirmWidget;
}

void BlockingUi::show()
{
    lastSelectedWidget = container->currentWidget();
    container->setCurrentWidget(blockingWidget);
    blockingWidget->show();
}

void BlockingUi::hide()
{
    container->setCurrentWidget(lastSelectedWidget);
}

void BlockingUi::onCancelClicked()
{
    container->setCurrentWidget(confirmWidget);
    blockingWidget->hide();
}

void BlockingUi::onCancelConfirmed()
{
    container->setCurrentWidget(lastSelectedWidget);
    emit cancelTransfers();
}

void BlockingUi::onCancelDismissed()
{
    container->setCurrentWidget(blockingWidget);
    blockingWidget->show();
}
