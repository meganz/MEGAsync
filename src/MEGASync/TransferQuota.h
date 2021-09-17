#pragma once

#include "Preferences.h"
#include "DesktopNotifications.h"
#include <memory>
#include "UpgradeDialog.h"
#include "UpgradeOverStorage.h"
#include <QObject>

// Events messages strings
constexpr char EVENT_MESSAGE_TRANSFER_OVER_QUOTA_DIALOG[]{"Transfer over quota dialog shown"};
constexpr char EVENT_MESSAGE_TRANSFER_OVER_QUOTA_OS_NOTIFICATION[]{"Transfer over quota os notification shown"};
constexpr char EVENT_MESSAGE_TRANSFER_OVER_QUOTA_UI_ALERTST_OVER_QUOTA_UI_ALERT[]{"Transfer over quota ui message shown"};
constexpr char EVENT_MESSAGE_TRANSFER_ALMOST_QUOTA_UI_ALERT[]{"Transfer almost over quota ui message shown"};
constexpr char EVENT_MESSAGE_TRANSFER_ALMOST_OVER_QUOTA_OS_NOTIFICATION[]{"Transfer almost over quota os notification shown"};

// % for almost over quota
constexpr int ALMOST_OVER_QUOTA_PER_CENT{90};

enum class QuotaState
{
    OK = 0,
    WARNING,
    FULL
};

class TransferQuota: public QObject
{
    Q_OBJECT
public:
    TransferQuota(mega::MegaApi* megaApi, Preferences *preferences, std::shared_ptr<DesktopNotifications> desktopNotifications);
    void setOverQuota(std::chrono::milliseconds waitTime);
    void setQuotaOk();
    bool isOverQuota();
    bool isQuotaWarning() const;
    void setUserProUsages(long long usedBytes, long long totalBytes);
    void refreshOverQuotaDialogDetails();
    void setOverQuotaDialogPricing(std::shared_ptr<mega::MegaPricing> pricing, std::shared_ptr<mega::MegaCurrency> currency);
    void closeDialogs();
    void checkQuotaAndAlerts();
    bool checkImportLinksAlertDismissed();
    bool checkDownloadAlertDismissed();
    bool checkStreamingAlertDismissed();
    void reset();

private:
    mega::MegaApi* mMegaApi;
    std::shared_ptr<mega::MegaPricing> mPricing;
    std::shared_ptr<mega::MegaCurrency> mCurrency;
    Preferences* mPreferences;
    std::shared_ptr<DesktopNotifications> mOsNotifications;
    UpgradeDialog* mUpgradeDialog;
    QuotaState mQuotaState;
    std::chrono::system_clock::time_point mWaitTimeUntil;
    bool overQuotaAlertVisible;
    bool almostQuotaAlertVisible;

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
    void onTransferOverquotaVisibilityChange(bool messageShown);
    void onAlmostTransferOverquotaVisibilityChange(bool messageShown);

signals:
    // emitted when checking almost OQ and the respective UI message
    // needs to be present (hasn't been discarded by the user for a while)
    void almostOverQuotaMessageNeedsToBeShown();
    // emitted when checking OQ and the respective UI message
    // needs to be present (hasn't been discarded by the user for a while)
    void overQuotaMessageNeedsToBeShown();
    void sendState(QuotaState state);
    void waitTimeIsOver();
};
