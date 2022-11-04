#include "BlockingStageProgressController.h"

BlockingStageProgressController::BlockingStageProgressController()
{
    connect(&uiUpdatingTimer, &QTimer::timeout, this, [this]()
    {
        FolderTransferUpdateEvent dataCopy = data;
        emit updateUi(dataCopy);
    });
}

void BlockingStageProgressController::update(const FolderTransferUpdateEvent &event)
{
    data = event;

    if (!uiUpdatingTimer.isActive())
    {
        uiUpdatingTimer.start(updateThresholdInMs);
    }
}

void BlockingStageProgressController::stopUiUpdating()
{
    uiUpdatingTimer.stop();
}
