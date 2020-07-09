#include "mega/types.h"
#include "TransferQuota.h"
#include "platform/Platform.h"
#include "OverQuotaDialog.h"

TransferQuota::TransferQuota(mega::MegaApi* megaApi,
                                       Preferences *preferences,
                                       Notificator *notificator)
    :mMegaApi{megaApi},
      mPricing(nullptr),
      mPreferences{preferences},
      mNotificator{notificator},
      mUpgradeDialog{nullptr},
      mQuotaState{QuotaState::OK}
{
}

void TransferQuota::setOverQuota(std::chrono::milliseconds waitTime)
{
    mWaitTimeUntil = std::chrono::system_clock::now()+waitTime;
    mQuotaState = QuotaState::FULL;
    emit sendState(QuotaState::FULL);
    checkExecuteAlerts();
}

bool TransferQuota::isOverQuota()
{    
    auto isOverQuota{mQuotaState == QuotaState::FULL};
    if(isOverQuota)
    {
        const auto waitTimeIsOver{std::chrono::system_clock::now() >= mWaitTimeUntil};
        if(waitTimeIsOver)
        {
            isOverQuota = false;
            setQuotaOk();            
        }
    }
    return isOverQuota;
}

bool TransferQuota::isQuotaWarning() const
{
    return mQuotaState == QuotaState::WARNING;
}

void TransferQuota::setQuotaOk()
{
    if(mQuotaState != QuotaState::OK)
    {
        mWaitTimeUntil = std::chrono::system_clock::time_point();
        mQuotaState = QuotaState::OK;
        mPreferences->clearTemporalBandwidth();
        emit sendState(QuotaState::OK);
        emit waitTimeIsOver();

        if (mUpgradeDialog)
        {
            mUpgradeDialog->close();
        }
    }
}

void TransferQuota::checkExecuteDialog()
{
    const auto disabledUntil{mPreferences->getTransferOverQuotaDialogLastExecution()+Preferences::OVER_QUOTA_DIALOG_DISABLE_DURATION};
    const auto dialogExecutionEnabled{std::chrono::system_clock::now() >= disabledUntil};
    if(dialogExecutionEnabled)
    {
        mPreferences->setTransferOverQuotaDialogLastExecution(std::chrono::system_clock::now());
        mMegaApi->sendEvent(EVENT_ID_TRANSFER_OVER_QUOTA_DIALOG, EVENT_MESSAGE_TRANSFER_OVER_QUOTA_DIALOG);
        if (!mUpgradeDialog)
        {
            mUpgradeDialog = new UpgradeDialog(mMegaApi, mPricing);
            QObject::connect(mUpgradeDialog, &UpgradeDialog::finished, this, &TransferQuota::upgradeDialogFinished);
            Platform::activateBackgroundWindow(mUpgradeDialog);
            mUpgradeDialog->show();
        }
        else if (!mUpgradeDialog->isVisible())
        {
            mUpgradeDialog->activateWindow();
            mUpgradeDialog->raise();
        }

        const auto endWaitTimeSinceEpoch{mWaitTimeUntil.time_since_epoch()};
        const auto endWaitTimeSinceEpochSeconds{std::chrono::duration_cast<std::chrono::seconds>(endWaitTimeSinceEpoch).count()};
        mUpgradeDialog->setTimestamp(endWaitTimeSinceEpochSeconds);
        mUpgradeDialog->refreshAccountDetails();
    }
}

void TransferQuota::checkExecuteNotification()
{
    const auto disabledUntil{mPreferences->getTransferOverQuotaOsNotificationLastExecution()+Preferences::OVER_QUOTA_OS_NOTIFICATION_DISABLE_DURATION};
    const auto notificationExecutionEnabled{std::chrono::system_clock::now() >= disabledUntil};
    if (notificationExecutionEnabled)
    {
        mPreferences->setTransferOverQuotaOsNotificationLastExecution(std::chrono::system_clock::now());
        mMegaApi->sendEvent(EVENT_ID_TRANSFER_OVER_QUOTA_OS_NOTIFICATION, EVENT_MESSAGE_TRANSFER_OVER_QUOTA_OS_NOTIFICATION);
        sendOverQuotaOsNotification();
    }
}

void TransferQuota::checkExecuteUiMessage()
{
    const auto disabledUntil{mPreferences->getTransferOverQuotaUiAlertLastExecution()+Preferences::OVER_QUOTA_UI_ALERT_DISABLE_DURATION};
    const auto uiAlertExecutionEnabled{std::chrono::system_clock::now() >= disabledUntil};
    if (uiAlertExecutionEnabled)
    {
        mMegaApi->sendEvent(EVENT_ID_TRANSFER_OVER_QUOTA_UI_ALERT, EVENT_MESSAGE_TRANSFER_OVER_QUOTA_UI_ALERTST_OVER_QUOTA_UI_ALERT);
        emit overQuotaUiMessage();
    }
}

void TransferQuota::checkExecuteWarningOsNotification()
{
    const auto disabledUntil{mPreferences->getTransferAlmostOverQuotaOsNotificationLastExecution()+Preferences::ALMOST_OVER_QUOTA_OS_NOTIFICATION_DISABLE_DURATION};
    const auto notificationExecutionEnabled{std::chrono::system_clock::now() >= disabledUntil};
    if (notificationExecutionEnabled)
    {
        mPreferences->setTransferAlmostOverQuotaOsNotificationLastExecution(std::chrono::system_clock::now());
        mMegaApi->sendEvent(EVENT_ID_TRANSFER_ALMOST_OVER_QUOTA_OS_NOTIFICATION,
                           EVENT_MESSAGE_TRANSFER_ALMOST_OVER_QUOTA_OS_NOTIFICATION);
        sendQuotaWarningOsNotification();
    }
}

void TransferQuota::checkExecuteWarningUiMessage()
{
    const auto disabledUntil{mPreferences->getTransferAlmostOverQuotaUiAlertLastExecution()+Preferences::OVER_QUOTA_UI_ALERT_DISABLE_DURATION};
    const auto executeUiWarningAlert{std::chrono::system_clock::now() >= disabledUntil};
    if (executeUiWarningAlert)
    {
        mMegaApi->sendEvent(EVENT_ID_TRANSFER_ALMOST_QUOTA_UI_ALERT, EVENT_MESSAGE_TRANSFER_ALMOST_QUOTA_UI_ALERT);
        emit almostOverQuotaUiMessage();
    }
}

void TransferQuota::checkExecuteAlerts()
{
    const auto megaApp{static_cast<MegaApplication*>(qApp)};
    const auto allowAlerts{megaApp->isInfoDialogVisible() || mUpgradeDialog || Platform::isUserActive()};
    const auto userLogged{mPreferences && mPreferences->logged()};
    const auto bandwidthAlertsEnabled{!megaApp->finished() && userLogged && allowAlerts};
    if (bandwidthAlertsEnabled)
    {
        if(isOverQuota())
        {
            checkExecuteDialog();
            checkExecuteNotification();
            checkExecuteUiMessage();
        }
        else if(isQuotaWarning())
        {
            checkExecuteWarningOsNotification();
            checkExecuteWarningUiMessage();
        }
    }
}

void TransferQuota::setUserProUsages(long long usedBytes, long long totalBytes)
{
    const auto usagePercent{std::floor(100.0 * usedBytes / totalBytes)};
    if(usagePercent >= ALMOST_OVER_QUOTA_PER_CENT)
    {
        mQuotaState = QuotaState::WARNING;
        emit sendState(QuotaState::WARNING);
        checkExecuteAlerts();
    }
}

void TransferQuota::refreshOverQuotaDialogDetails()
{
    if(mUpgradeDialog)
    {
        mUpgradeDialog->refreshAccountDetails();
    }
}

void TransferQuota::setOverQuotaDialogPricing(mega::MegaPricing *pricing)
{
    mPricing = pricing;
    if(mUpgradeDialog)
    {
        mUpgradeDialog->setPricing(pricing);
    }
}

void TransferQuota::closeDialogs()
{
    if(mUpgradeDialog)
    {
        delete mUpgradeDialog;
        mUpgradeDialog = nullptr;
    }
}

void TransferQuota::checkQuotaAndAlerts()
{
    isOverQuota();
    checkExecuteAlerts();
}

bool TransferQuota::checkImportLinksAlertDismissed()
{
    auto dismissed{true};
    const auto disabledUntil{mPreferences->getTransferOverQuotaImportLinksDialogLastExecution() + Preferences::OVER_QUOTA_ACTION_DIALOGS_DISABLE_TIME};
    const auto dialogEnabled{std::chrono::system_clock::now() >= disabledUntil};
    if(isOverQuota() && dialogEnabled)
    {
        mPreferences->setTransferOverQuotaImportLinksDialogLastExecution(std::chrono::system_clock::now());
        const auto bandwidthFullDialog{OverQuotaDialog::createDialog(OverQuotaDialogType::BANDWIDTH_IMPORT_LINK)};
        dismissed = (bandwidthFullDialog->exec() == QDialog::Rejected);
    }
    return dismissed;
}

bool TransferQuota::checkDownloadAlertDismissed()
{
    auto dismissed{true};
    const auto disabledUntil{mPreferences->getTransferOverQuotaDownloadsDialogLastExecution() + Preferences::OVER_QUOTA_ACTION_DIALOGS_DISABLE_TIME};
    const auto dialogEnabled{std::chrono::system_clock::now() >= disabledUntil};
    if(isOverQuota() && dialogEnabled)
    {
        mPreferences->setTransferOverQuotaDownloadsDialogLastExecution(std::chrono::system_clock::now());
        const auto bandwidthFullDialog{OverQuotaDialog::createDialog(OverQuotaDialogType::BANDWIDTH_DOWNLOAD)};
        dismissed = (bandwidthFullDialog->exec() == QDialog::Rejected);
    }
    return dismissed;
}

bool TransferQuota::checkStreamingAlertDismissed()
{
    auto dismissed{true};
    const auto disabledUntil{mPreferences->getTransferOverQuotaStreamDialogLastExecution() + Preferences::OVER_QUOTA_ACTION_DIALOGS_DISABLE_TIME};
    const auto dialogEnabled{std::chrono::system_clock::now() >= disabledUntil};
    if(isOverQuota() && dialogEnabled)
    {
        mPreferences->setTransferOverQuotaStreamDialogLastExecution(std::chrono::system_clock::now());
        const auto bandwidthFullDialog{OverQuotaDialog::createDialog(OverQuotaDialogType::BANDWIDTH_STREAM)};
        dismissed = (bandwidthFullDialog->exec() == QDialog::Rejected);
    }
    return dismissed;
}

void TransferQuota::reset()
{
    mQuotaState = QuotaState::OK;
    mWaitTimeUntil = std::chrono::system_clock::time_point();
}

void TransferQuota::sendNotification(const QString &title)
{
    MegaNotification *notification = new MegaNotification();
    notification->setTitle(title);
    notification->setText(tr("Upgrade now to a PRO account."));
    notification->setActions(QStringList() << tr("Get PRO"));
    const auto megaApp{static_cast<MegaApplication*>(qApp)};
    connect(notification, &MegaNotification::activated, megaApp, &MegaApplication::redirectToUpgrade);
    mNotificator->notify(notification);
}

void TransferQuota::sendQuotaWarningOsNotification()
{
    const auto title{tr("Limited available transfer quota.")};
    sendNotification(title);
}

void TransferQuota::sendOverQuotaOsNotification()
{
    const auto title{tr("Depleted transfer quota.")};
    sendNotification(title);
}

void TransferQuota::upgradeDialogFinished(int)
{
    if (!static_cast<MegaApplication*>(qApp)->finished() && mUpgradeDialog)
    {
        mUpgradeDialog->deleteLater();
        mUpgradeDialog = nullptr;
    }
}

void TransferQuota::onDismissOverQuotaUiAlert()
{
    mPreferences->setTransferOverQuotaUiAlertLastExecution(std::chrono::system_clock::now());
}

void TransferQuota::onDismissAlmostOverQuotaUiMessage()
{
    mPreferences->setTransferAlmostOverQuotaUiAlertLastExecution(std::chrono::system_clock::now());
}
