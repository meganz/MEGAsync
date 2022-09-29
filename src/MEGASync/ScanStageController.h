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
    void stopDelayedScanStage(bool fromCancellation);

    bool isInScanningState() const;

signals:
    void enableTransferActions(bool enable);

private:
    void startScanStage();

    void setUiInScanStage();
    void setUiInNormalStage();
    void setUiInDisabledScanStage();

    void onMinimumDisplayTimeElapsed();

    QPointer<InfoDialog> mInfoDialog;
    QPointer<TransferManager> mTransferManager;
    QTimer mScanStageTimer;
    bool mIsInScanningState = false;
    bool mIsInScanningStateInMinimumTime = false;
    bool mLastScanCancelled = false;
    const int mDelayToShowDialogInMs = 800;
    const int mMinimumDialogDisplayTimeInMs = 1200;
};

#endif // SCANSTAGECONTROLLER_H
