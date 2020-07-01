#include "mega/types.h"
#include "TransferQuota.h"
#include "platform/Platform.h"
#include "OverQuotaDialog.h"

TransferQuota::TransferQuota(mega::MegaApi* megaApi,
                                       Preferences *preferences,
                                       Notificator *notificator)
    :megaApi{megaApi},
      pricing(nullptr),
      preferences{preferences},
      notificator{notificator},
      upgradeDialog{nullptr},
      upgradeDialogEventEnabled{true}
{
}

void TransferQuota::setOverQuota(std::chrono::milliseconds waitTime)
{
    preferences->setTransferOverQuotaWaitUntil(std::chrono::system_clock::now()+waitTime);
    preferences->setTransferQuotaState(Preferences::QuotaState::FULL);
    emit sendState(Preferences::QuotaState::FULL);
    checkExecuteAlerts();
}

bool TransferQuota::isOverQuota()
{    
    auto isOverQuota{preferences->getTransferQuotaState() == Preferences::QuotaState::FULL};
    if(isOverQuota)
    {
        const auto waitTimeIsOver{std::chrono::system_clock::now() >= preferences->getTransferOverQuotaWaitUntil()};
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
    return preferences->getTransferQuotaState() == Preferences::QuotaState::WARNING;
}

void TransferQuota::setQuotaOk()
{
    preferences->setTransferOverQuotaWaitUntil(std::chrono::system_clock::time_point());
    preferences->setTransferQuotaState(Preferences::QuotaState::OK);
    preferences->clearTemporalBandwidth(); // TODO: why this?
    emit sendState(Preferences::QuotaState::OK);

    if (upgradeDialog)
    {
        upgradeDialog->close();
    }
}

void TransferQuota::checkExecuteDialog()
{
    const auto executeDialog{std::chrono::system_clock::now() >= preferences->getTransferOverQuotaDialogDisabledUntil()};
    if(executeDialog)
    {
        preferences->setTransferOverQuotaDialogDisabledUntil(std::chrono::system_clock::now()+Preferences::overquotaDialogDisableDuration);
        if (!upgradeDialog)
        {
            upgradeDialog = new UpgradeDialog(megaApi, pricing);
            QObject::connect(upgradeDialog, &UpgradeDialog::finished, this, &TransferQuota::upgradeDialogFinished);
            Platform::activateBackgroundWindow(upgradeDialog);
            upgradeDialog->show();

            if (upgradeDialogEventEnabled)
            {
                megaApi->sendEvent(EVENT_ID_TRANSFER_OVER_QUOTA_DIALOG, EVENT_MESSAGE_TRANSFER_OVER_QUOTA_DIALOG);
                upgradeDialogEventEnabled = false;
            }
        }
        else if (!upgradeDialog->isVisible())
        {
            upgradeDialog->activateWindow();
            upgradeDialog->raise();
        }

        const auto endWaitTimeSinceEpoch{preferences->getTransferOverQuotaWaitUntil().time_since_epoch()};
        const auto endWaitTimeSinceEpochSeconds{std::chrono::duration_cast<std::chrono::seconds>(endWaitTimeSinceEpoch).count()};
        upgradeDialog->setTimestamp(endWaitTimeSinceEpochSeconds);
        upgradeDialog->refreshAccountDetails();
    }
}

void TransferQuota::checkExecuteNotification()
{
    const auto executeNotification{std::chrono::system_clock::now() >= preferences->getTransferOverQuotaOsNotificationDisabledUntil()};
    if (executeNotification)
    {
        preferences->setTransferOverQuotaOsNotificationDisabledUntil(std::chrono::system_clock::now()+Preferences::overquotaNotificationDisableDuration);
        megaApi->sendEvent(EVENT_ID_TRANSFER_OVER_QUOTA_OS_NOTIFICATION, EVENT_MESSAGE_TRANSFER_OVER_QUOTA_OS_NOTIFICATION);
        sendOverQuotaOsNotification();
    }
}

void TransferQuota::checkExecuteUiMessage()
{
    const auto executeUiAlert{std::chrono::system_clock::now() >= preferences->getTransferOverQuotaUiAlertDisabledUntil()};
    if (executeUiAlert)
    {
        megaApi->sendEvent(EVENT_ID_TRANSFER_OVER_QUOTA_UI_ALERT, EVENT_MESSAGE_TRANSFER_OVER_QUOTA_UI_ALERTST_OVER_QUOTA_UI_ALERT);
        emit overQuotaUiMessage();
    }
}

void TransferQuota::checkExecuteWarningOsNotification()
{
    const auto executeNotification{std::chrono::system_clock::now() >= preferences->getTransferAlmostOverQuotaOsNotificationDisabledUntil()};
    if (executeNotification)
    {
        preferences->setTransferAlmostOverQuotaOsNotificationDisabledUntil(std::chrono::system_clock::now()+Preferences::almostOverquotaOsNotificationDisableDuration);
        megaApi->sendEvent(EVENT_ID_TRANSFER_ALMOST_OVER_QUOTA_OS_NOTIFICATION


, EVENT_MESSAGE_TRANSFER_ALMOST_OVER_QUOTA_OS_NOTIFICATION);
        sendQuotaWarningOsNotification();
    }
}

void TransferQuota::checkExecuteWarningUiMessage()
{
    const auto executeUiWarningAlert{std::chrono::system_clock::now() >= preferences->getTransferAlmostOverQuotaUiAlertDisableUntil()};
    if (executeUiWarningAlert)
    {
        megaApi->sendEvent(EVENT_ID_TRANSFER_ALMOST_QUOTA_UI_ALERT, EVENT_MESSAGE_TRANSFER_ALMOST_QUOTA_UI_ALERT);
        emit almostOverQuotaUiMessage();
    }
}

void TransferQuota::checkExecuteAlerts()
{
    const auto megaApp{static_cast<MegaApplication*>(qApp)};
    const auto allowAlerts{megaApp->isInfoDialogVisible() || upgradeDialog || Platform::isUserActive()};
    const auto bandwidthAlertsEnabled{!megaApp->finished() && preferences->logged() && allowAlerts};
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
        preferences->setTransferQuotaState(Preferences::QuotaState::WARNING);
        emit sendState(Preferences::QuotaState::WARNING);
        checkExecuteAlerts();
    }
}

void TransferQuota::refreshOverQuotaDialogDetails()
{
    if(upgradeDialog)
    {
        upgradeDialog->refreshAccountDetails();
    }
}

void TransferQuota::setOverQuotaDialogPricing(mega::MegaPricing *pricing)
{
    this->pricing = pricing;
    if(upgradeDialog)
    {
        upgradeDialog->setPricing(pricing);
    }
}

void TransferQuota::closeDialogs()
{
    if(upgradeDialog)
    {
        delete upgradeDialog;
        upgradeDialog = nullptr;
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
    const auto dialogEnabled{std::chrono::system_clock::now() >= preferences->getTransferOverQuotaImportLinksDialogDisabledUntil()};
    if(isOverQuota() && dialogEnabled)
    {
        preferences->setTransferOverQuotaImportLinksDialogDisabledUntil(std::chrono::system_clock::now()+OVER_QUOTA_DIALOGS_DISABLE_TIME);
        const auto bandwidthFullDialog{OverQuotaDialog::createDialog(OverQuotaDialogType::BANDWIDTH_IMPORT_LINK)};
        dismissed = (bandwidthFullDialog->exec() == QDialog::Rejected);
    }
    return dismissed;
}

bool TransferQuota::checkDownloadAlertDismissed()
{
    auto dismissed{true};
    const auto dialogEnabled{std::chrono::system_clock::now() >= preferences->getTransferOverQuotaDownloadsDialogDisabledUntil()};
    if(isOverQuota() && dialogEnabled)
    {
        preferences->setTransferOverQuotaDownloadsDialogDisabledUntil(std::chrono::system_clock::now()+OVER_QUOTA_DIALOGS_DISABLE_TIME);
        const auto bandwidthFullDialog{OverQuotaDialog::createDialog(OverQuotaDialogType::BANDWIDTH_DOWNLOAD)};
        dismissed = (bandwidthFullDialog->exec() == QDialog::Rejected);
    }
    return dismissed;
}

bool TransferQuota::checkStreamingAlertDismissed()
{
    auto dismissed{true};
    const auto dialogEnabled{std::chrono::system_clock::now() >= preferences->getTransferOverQuotaStreamDialogDisabledUntil()};
    if(isOverQuota() && dialogEnabled)
    {
        preferences->setTransferOverQuotaStreamDialogDisabledUntil(std::chrono::system_clock::now()+OVER_QUOTA_DIALOGS_DISABLE_TIME);
        const auto bandwidthFullDialog{OverQuotaDialog::createDialog(OverQuotaDialogType::BANDWIDTH_STREAM)};
        dismissed = (bandwidthFullDialog->exec() == QDialog::Rejected);
    }
    return dismissed;
}

void TransferQuota::sendNotification(const QString &title)
{
    MegaNotification *notification = new MegaNotification();
    notification->setTitle(title);
    notification->setText(tr("Upgrade now to a PRO account."));
    notification->setActions(QStringList() << tr("Get PRO"));
    const auto megaApp{static_cast<MegaApplication*>(qApp)};
    connect(notification, &MegaNotification::activated, megaApp, &MegaApplication::redirectToUpgrade);
    notificator->notify(notification);
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
    if (!static_cast<MegaApplication*>(qApp)->finished() && upgradeDialog)
    {
        upgradeDialog->deleteLater();
        upgradeDialog = nullptr;
    }
}

void TransferQuota::onDismissOverQuotaUiAlert()
{
    preferences->setTransferOverQuotaUiAlertDisabledUntil(std::chrono::system_clock::now()+Preferences::overquotaUiMessageDisableDuration);
}

void TransferQuota::onDismissAlmostOverQuotaUiMessage()
{
    preferences->setTransferAlmostOverQuotaUiAlertDisabledUntil(std::chrono::system_clock::now()+Preferences::almostOverquotaUiMessageDisableDuration);
}
