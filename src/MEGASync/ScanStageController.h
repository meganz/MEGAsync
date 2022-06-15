#ifndef SCANSTAGECONTROLLER_H
#define SCANSTAGECONTROLLER_H

#include <QObject>
#include <QTimer>

class InfoDialog;
class TransferManager;

class ScanStageController : public QObject
{
    Q_OBJECT
public:
    explicit ScanStageController(QObject *parent = nullptr);

    void updateReference(InfoDialog* _infoDialog);
    void updateReference(TransferManager* _transferManager);

    void startDelayedScanStage();
    void stopDelayedScanStage();

signals:
    void enableTransferActions(bool enable);

private:
    void startScanStage();

    void setUiInScanStage();
    void setUiInNormalStage();
    void setUiInDisabledScanStage();

    void onMinimumDisplayTimeElapsed();

    InfoDialog* infoDialog = nullptr;
    TransferManager* transferManager = nullptr;
    QTimer scanStageTimer;
    bool isInScanningState = false;
    bool isInScanningStateInMinimumTime = false;
    const int delayToShowDialogInMs = 800;
    const int minimumDialogDisplayTimeInMs = 1200;
};

#endif // SCANSTAGECONTROLLER_H
