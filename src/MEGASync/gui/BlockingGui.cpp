#include "BlockingGui.h"

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

    QString styles = QString::fromLatin1(getControlStyles());
    blockingWidget->setStyleSheet(styles);
    confirmWidget->setStyleSheet(styles);
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
    emit cancelTransfers();
}

void BlockingUi::onCancelDismissed()
{
    container->setCurrentWidget(blockingWidget);
    blockingWidget->show();
}

const char* BlockingUi::getControlStyles()
{
    const char* styles = "*[role=\"title\"] {font-size: 18px; font:bold;}"
                         "*[role=\"details\"] {font-size: 12px;}"
                         "QPushButton {font-size: 12px; padding: 10px}"
                         "QPushButton#pProceed {background-color: #00BFA5;}";
    return styles;
}
