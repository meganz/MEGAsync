
#include "TransfersSummaryQuickWidget.h"

#include <QQuickItem>

TransfersSummaryQuickWidget::TransfersSummaryQuickWidget(QWidget* parent):
    MegaQuickWidget(parent)
{
    setSource(QString::fromUtf8("qrc:/quick_widget/TransfersSummaryItem.qml"));

    if (auto rootObject = this->rootObject())
    {
        connect(rootObject,
                SIGNAL(transferManagerClicked()),
                this,
                SIGNAL(transferManagerClicked()));
        connect(rootObject, SIGNAL(pauseResumeClicked()), this, SIGNAL(pauseResumeClicked()));
    }
}

void TransfersSummaryQuickWidget::setTransfersCount(uint completedTransfers, uint totalTransfers)
{
    if (auto rootObject = this->rootObject())
    {
        rootObject->setProperty("completedTransfers", completedTransfers);
        rootObject->setProperty("totalTransfers", totalTransfers);
    }
}

void TransfersSummaryQuickWidget::setTopTransferDirection(bool upload)
{
    if (auto rootObject = this->rootObject())
    {
        rootObject->setProperty("isTopTransferUpload", upload);
    }
}

void TransfersSummaryQuickWidget::setPaused(bool paused)
{
    if (auto rootObject = this->rootObject())
    {
        rootObject->setProperty("paused", paused);
    }
}

void TransfersSummaryQuickWidget::setPauseEnabled(bool pauseEnabled)
{
    if (auto rootObject = this->rootObject())
    {
        rootObject->setProperty("pauseEnabled", pauseEnabled);
    }
}

void TransfersSummaryQuickWidget::setOngoingTransfers(int count)
{
    if (auto rootObject = this->rootObject())
    {
        rootObject->setProperty("ongoingTransfers", count);
    }
}
