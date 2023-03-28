#pragma once

#include "Preferences.h"
#include "DesktopNotifications.h"
#include <memory>
#include "UpgradeDialog.h"
#include "UpgradeOverStorage.h"
#include "OverQuotaDialog.h"

#include <QObject>

// Events messages strings
constexpr char EVENT_MESSAGE_TRANSFER_OVER_QUOTA_DIALOG[]{"Transfer over quota dialog shown"};
constexpr char EVENT_MESSAGE_TRANSFER_OVER_QUOTA_OS_NOTIFICATION[]{"Transfer over quota os notification shown"};
constexpr char EVENT_MESSAGE_TRANSFER_OVER_QUOTA_UI_ALERTST_OVER_QUOTA_UI_ALERT[]{"Transfer over quota ui message shown"};
constexpr char EVENT_MESSAGE_TRANSFER_ALMOST_QUOTA_UI_ALERT[]{"Transfer almost over quota ui message shown"};
constexpr char EVENT_MESSAGE_TRANSFER_ALMOST_OVER_QUOTA_OS_NOTIFICATION[]{"Transfer almost over quota os notification shown"};

// % for almost over quota
constexpr int ALMOST_OVER_QUOTA_PER_CENT{90};
constexpr int FULL_QUOTA_PER_CENT{100};

enum class QuotaState
{
    OK        = 0,
    WARNING   = 1,
    FULL      = 2, // Account could reach full usage of quota but not enter in overquota situation yet.
    OVERQUOTA = 3,
};

class TransferQuota: public QObject
{
    Q_OBJECT
public:
    TransferQuota(std::shared_ptr<DesktopNotifications> desktopNotifications);
    void setOverQuota(std::chrono::milliseconds waitTime);
    void updateQuotaState();
    bool isOverQuota();
    bool isQuotaWarning();
    bool isQuotaFull();
    void setOverQuotaDialogPricing(std::shared_ptr<mega::MegaPricing> pricing, std::shared_ptr<mega::MegaCurrency> currency);
    void closeDialog();
    void checkQuotaAndAlerts();
    void checkImportLinksAlertDismissed(std::function<void(int)> func);
    void checkDownloadAlertDismissed(std::function<void(int)> func);
    void checkStreamingAlertDismissed(std::function<void (int)> func);
    QTime getTransferQuotaDeadline();
    void reset();

private:
    mega::MegaApi* mMegaApi;
    std::shared_ptr<mega::MegaPricing> mPricing;
    std::shared_ptr<mega::MegaCurrency> mCurrency;
    std::shared_ptr<Preferences> mPreferences;
    std::shared_ptr<DesktopNotifications> mOsNotifications;
    QPointer<UpgradeDialog> mUpgradeDialog;
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
    void checkAlertDismissed(OverQuotaDialogType type, std::function<void (int)> func);

public slots:
    void onTransferOverquotaVisibilityChange(bool messageShown);
    void onAlmostTransferOverquotaVisibilityChange(bool messageShown);

signals:
    // emitted when checking almost OQ and the respective UI message
    // needs to be present (hasn't been discarded by the user for a while)
    void almostOverQuotaMessageNeedsToBeShown();
    // emitted when checking OQ and the respective UI message
    // needs to be present (hasn't been discarded by the user for a while)
    void overQuotaMessageNeedsToBeShown();
    void overQuotaDialogFinished(int result);
    void sendState(QuotaState state);
    void waitTimeIsOver();
};
