#ifndef BlockingStageProgressController_H
#define BlockingStageProgressController_H

#include "FolderTransferEvents.h"

#include <QObject>
#include <QTimer>
#include <mutex>

class BlockingStageProgressController : public QObject
{
    Q_OBJECT

public:
    BlockingStageProgressController();

    void update(const FolderTransferUpdateEvent &event);
    void stopUiUpdating();

signals:
    void updateUi(const FolderTransferUpdateEvent& event);

private:
    FolderTransferUpdateEvent data;
    std::mutex dataMutex;
    QTimer uiUpdatingTimer;
    const unsigned int updateThresholdInMs = 100;
};

#endif // BlockingStageProgressController_H
