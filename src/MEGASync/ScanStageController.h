#ifndef SCANSTAGECONTROLLER_H
#define SCANSTAGECONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QPointer>

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

    InfoDialog* mInfoDialog = nullptr;
    QPointer<TransferManager> mTransferManager;
    QTimer mScanStageTimer;
    bool mIsInScanningState = false;
    bool mIsInScanningStateInMinimumTime = false;
    const int mDelayToShowDialogInMs = 800;
    const int mMinimumDialogDisplayTimeInMs = 1200;
};

#endif // SCANSTAGECONTROLLER_H
