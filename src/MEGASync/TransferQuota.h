#pragma once

#include "InfoDialog.h"
#include "Preferences.h"
#include "notificator.h"
#include "TimeWrapper.h"
#include <memory>
#include "UpgradeDialog.h"
#include <QObject>

// Events
constexpr auto eventIdTransferOverQuotaDialog{98526};
constexpr auto eventMessageTransferOverQuotaDialog{"Transfer over quota dialog shown"};
constexpr auto eventIdTransferOverQuotaOsNotification{98527};
constexpr auto eventMessageTransferOverQuotaOsNotification{"Transfer over quota os notification shown"};
constexpr auto eventIdTransferOverQuotaUiAlert{98528};
constexpr auto eventMessageTransferOverQuotaUiAlert{"Transfer over quota ui message shown"};
constexpr auto eventIdTransferAlmostOverQuotaUiAlert{98529};
constexpr auto eventMessageTransferAlmostQuotaUiAlert{"Transfer almost over quota ui message shown"};
constexpr auto eventIdTransferAlmostOverQuotaOsNotification{98531};
constexpr auto eventMessageTransferAlmostOverQuotaOsNotification{"Transfer almost over quota os notification shown"};
constexpr auto almostOverquotaPercent{90};

class TransferQuota: public QObject
{
    Q_OBJECT
public:
    TransferQuota(mega::MegaApi* megaApi, Preferences *preferences,
                       Notificator *notificator);
    TransferQuota(mega::MegaApi* megaApi, Preferences *preferences,
                       Notificator *notificator, std::shared_ptr<Time> time);

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
    std::shared_ptr<Time> time;
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
