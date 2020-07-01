#pragma once

#include "InfoDialog.h"
#include "Preferences.h"
#include "notificator.h"
#include <memory>
#include "UpgradeDialog.h"
#include <QObject>

// Events
constexpr auto EVENT_ID_TRANSFER_OVER_QUOTA_DIALOG{98526};
constexpr auto EVENT_MESSAGE_TRANSFER_OVER_QUOTA_DIALOG{"Transfer over quota dialog shown"};
constexpr auto EVENT_ID_TRANSFER_OVER_QUOTA_OS_NOTIFICATION{98527};
constexpr auto EVENT_MESSAGE_TRANSFER_OVER_QUOTA_OS_NOTIFICATION{"Transfer over quota os notification shown"};
constexpr auto EVENT_ID_TRANSFER_OVER_QUOTA_UI_ALERT{98528};
constexpr auto EVENT_MESSAGE_TRANSFER_OVER_QUOTA_UI_ALERTST_OVER_QUOTA_UI_ALERT{"Transfer over quota ui message shown"};
constexpr auto EVENT_ID_TRANSFER_ALMOST_QUOTA_UI_ALERT{98529};
constexpr auto EVENT_MESSAGE_TRANSFER_ALMOST_QUOTA_UI_ALERT{"Transfer almost over quota ui message shown"};
constexpr auto EVENT_ID_TRANSFER_ALMOST_OVER_QUOTA_OS_NOTIFICATION{98531};
constexpr auto EVENT_MESSAGE_TRANSFER_ALMOST_OVER_QUOTA_OS_NOTIFICATION{"Transfer almost over quota os notification shown"};
constexpr auto ALMOST_OVER_QUOTA_PER_CENT{90};
constexpr auto OVER_QUOTA_DIALOGS_DISABLE_TIME{std::chrono::hours{12}};

class TransferQuota: public QObject
{
    Q_OBJECT
public:
    TransferQuota(mega::MegaApi* megaApi, Preferences *preferences,
                       Notificator *notificator);

    void setOverQuota(std::chrono::milliseconds waitTime);
    void setQuotaOk();
    bool isOverQuota();
    bool isQuotaWarning() const;
    void setUserProUsages(long long usedBytes, long long totalBytes);
    void refreshOverQuotaDialogDetails();
    void setOverQuotaDialogPricing(mega::MegaPricing *pricing);
    void closeDialogs();
    void checkQuotaAndAlerts();
    bool checkImportLinksAlertDismissed();
    bool checkDownloadAlertDismissed();
    bool checkStreamingAlertDismissed();

private:
    mega::MegaApi* megaApi;
    mega::MegaPricing* pricing;
    Preferences* preferences;
    Notificator *notificator;
    UpgradeDialog* upgradeDialog;
    bool upgradeDialogEventEnabled;

    void sendNotification(const QString& title);
    void sendQuotaWarningOsNotification();
    void sendOverQuotaOsNotification();
    void checkExecuteDialog();
    void checkExecuteNotification();
    void checkExecuteUiMessage();
    void checkExecuteWarningOsNotification();
    void checkExecuteWarningUiMessage();
    void checkExecuteAlerts();

public slots:
    void upgradeDialogFinished(int result);
    void onDismissOverQuotaUiAlert();
    void onDismissAlmostOverQuotaUiMessage();

signals:
    void almostOverQuotaUiMessage();
    void overQuotaUiMessage();
    void sendState(Preferences::QuotaState state);
};
