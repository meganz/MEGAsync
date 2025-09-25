#ifndef TRANSFERS_SUMMARY_QUICK_WIDGET_H
#define TRANSFERS_SUMMARY_QUICK_WIDGET_H

#include "MegaQuickWidget.h"

#include <QWidget>

class TransfersSummaryQuickWidget: public MegaQuickWidget
{
    Q_OBJECT

public:
    explicit TransfersSummaryQuickWidget(QWidget* parent = nullptr);

    void setTransfersCount(uint completedTransfers, uint totalTransfers);
    void setTopTransferDirection(bool upload);
    void setPaused(bool paused);
    void setPauseEnabled(bool pauseEnabled);
    void setOngoingTransfers(int count);

signals:
    void transferManagerClicked();
    void pauseResumeClicked();
};
#endif
