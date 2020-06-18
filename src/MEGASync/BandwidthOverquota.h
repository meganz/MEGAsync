#pragma once
#include "InfoDialog.h"
#include "Preferences.h"
#include "notificator.h"
#include "Time.h"
#include <memory>
#include "UpgradeDialog.h"
#include "megaapi.h"
#include <QObject>

// Events
constexpr auto eventIdTransferQuotaDialogExecuted{98526};
constexpr auto eventMessageTransferQuotaDialogExecuted{"Bandwidth overquota full dialog shown"};
constexpr auto eventIdTransferQuotaOsNotificationExecuted{98527};
constexpr auto eventMessageTransferQuotaOsNotificationExecuted{"Bandwidth overquota full os notification shown"};
constexpr auto eventIdTransferQuotaUiDialogExecuted{98528};
constexpr auto eventMessageTransferQuotaUiDialogExecuted{"Bandwidth overquota full ui message shown"};
constexpr auto eventIdTransferAlmostQuotaUiDialogExecuted{98529};
constexpr auto eventMessageTransferAlmostQuotaUiDialogExecuted{"Bandwidth almost overquota ui message shown"};
constexpr auto eventIdTransferAlmostQuotaNotificationExecuted{98531};
constexpr auto eventMessageTransferAlmostQuotaNotificationExecuted{"Bandwidth almost overquota os notification shown"};
constexpr auto warningPercent{90};

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
