#pragma once

#include "InfoDialog.h"
#include "Preferences.h"
#include "notificator.h"
#include "MegaTime.h"
#include <memory>
#include "UpgradeDialog.h"
#include <QObject>

class BandwidthOverquota: public QObject
{
    Q_OBJECT
public:
    BandwidthOverquota(mega::MegaApi* megaApi, Preferences *preferences,
                       Notificator *notificator);
    BandwidthOverquota(mega::MegaApi* megaApi, Preferences *preferences,
                       Notificator *notificator, std::shared_ptr<Time> time);

    void setFullState(std::chrono::milliseconds waitTime);
    void setOkState();
    bool isStateOk() const;
    bool isStateFull() const;
    bool isStateWarning() const;
    void setUsages(long long usedBytes, long long totalBytes);
    void refreshOverquotaDialogDetails();
    void setOverquotaDialogPricing(mega::MegaPricing *pricing);
    void closeDialogs();
    void checkStateAndAlerts();
    bool checkImportLinksAlertDismissed();
    bool checkDownloadAlertDismissed();
    bool checkStreamingAlertDismissed();

private:
    mega::MegaApi* megaApi;
    mega::MegaPricing* pricing;
    Preferences* preferences;
    Notificator *notificator;
    std::shared_ptr<Time> time;
    UpgradeDialog* upgradeDialog;
    bool upgradeDialogEventEnabled;

    void sendNotification(const QString& title);
    void sendOverBandwidthWarningNotification();
    void sendOverBandwithFullNotification();
    void checkExecuteDialog();
    void checkExecuteNotification();
    void checkExecuteUiMessage();
    void checkExecuteWarningOsNotification();
    void checkExecuteWarningUiMessage();
    void checkState();
    void checkExecuteAlerts();

public slots:
    void upgradeDialogFinished(int result);
    void onDismissUiMessageFullOverquota();
    void onDismissUiMessageAlmostOverquota();

signals:
    void almostOverquotaUiMessageEnabled();
    void overquotaUiMessageEnabled();
    void sendState(Preferences::OverquotaState state);
};
