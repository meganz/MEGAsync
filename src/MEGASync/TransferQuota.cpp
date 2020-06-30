#include "mega/types.h"
#include "TransferQuota.h"
#include "platform/Platform.h"
#include "OverQuotaDialog.h"

TransferQuota::TransferQuota(mega::MegaApi* megaApi, Preferences *preferences, Notificator *notificator)
    :TransferQuota{megaApi, preferences, notificator, ::mega::make_unique<Time>()}
{
}

TransferQuota::TransferQuota(mega::MegaApi* megaApi,
                                       Preferences *preferences,
                                       Notificator *notificator,
                                       std::shared_ptr<Time> time)
    :megaApi{megaApi},
      pricing(nullptr),
      preferences{preferences},
      notificator{notificator},
      time{time},
      upgradeDialog{nullptr},
      upgradeDialogEventEnabled{true}
{
}

void TransferQuota::setOverQuota(std::chrono::milliseconds waitTime)
{
    preferences->setTransferOverQuotaWaitUntil(time->now()+waitTime);
    preferences->setTransferQuotaState(Preferences::QuotaState::full);
    emit sendState(Preferences::QuotaState::full);
    checkExecuteAlerts();
}

bool TransferQuota::isOverQuota()
{    
    auto isOverQuota{preferences->getTransferQuotaState() == Preferences::QuotaState::full};
    if(isOverQuota)
    {
        const auto waitTimeIsOver{time->now() >= preferences->getTransferOverQuotaWaitUntil()};
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
    return preferences->getTransferQuotaState() == Preferences::QuotaState::warning;
}

void TransferQuota::setQuotaOk()
{
    preferences->setTransferOverQuotaWaitUntil(std::chrono::system_clock::time_point());
    preferences->setTransferQuotaState(Preferences::QuotaState::ok);
    preferences->clearTemporalBandwidth(); // TODO: why this?
    emit sendState(Preferences::QuotaState::ok);

    if (upgradeDialog)
    {
        upgradeDialog->close();
    }
}

void TransferQuota::checkExecuteDialog()
{
    const auto executeDialog{time->now() >= preferences->getTransferOverQuotaDialogDisabledUntil()};
    if(executeDialog)
    {
        preferences->setTransferOverQuotaDialogDisabledUntil(time->now()+Preferences::overquotaDialogDisableDuration);
        if (!upgradeDialog)
        {
            upgradeDialog = new UpgradeDialog(megaApi, pricing);
            QObject::connect(upgradeDialog, &UpgradeDialog::finished, this, &TransferQuota::upgradeDialogFinished);
            Platform::activateBackgroundWindow(upgradeDialog);
            upgradeDialog->show();

            if (upgradeDialogEventEnabled)
            {
                megaApi->sendEvent(eventIdTransferOverQuotaDialog, eventMessageTransferOverQuotaDialog);
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
    const auto executeNotification{time->now() >= preferences->getTransferOverQuotaOsNotificationDisabledUntil()};
    if (executeNotification)
    {
        preferences->setTransferOverQuotaOsNotificationDisabledUntil(time->now()+Preferences::overquotaNotificationDisableDuration);
        megaApi->sendEvent(eventIdTransferOverQuotaOsNotification, eventMessageTransferOverQuotaOsNotification);
        sendOverQuotaOsNotification();
    }
}

void TransferQuota::checkExecuteUiMessage()
{
    const auto executeUiAlert{time->now() >= preferences->getTransferOverQuotaUiAlertDisabledUntil()};
    if (executeUiAlert)
    {
        megaApi->sendEvent(eventIdTransferOverQuotaUiAlert, eventMessageTransferOverQuotaUiAlert);
        emit overQuotaUiMessage();
    }
}

void TransferQuota::checkExecuteWarningOsNotification()
{
    const auto executeNotification{time->now() >= preferences->getTransferAlmostOverQuotaOsNotificationDisabledUntil()};
    if (executeNotification)
    {
        preferences->setTransferAlmostOverQuotaOsNotificationDisabledUntil(time->now()+Preferences::almostOverquotaOsNotificationDisableDuration);
        megaApi->sendEvent(eventIdTransferAlmostOverQuotaOsNotification, eventMessageTransferAlmostOverQuotaOsNotification);
        sendQuotaWarningOsNotification();
    }
}

void TransferQuota::checkExecuteWarningUiMessage()
{
    const auto executeUiWarningAlert{time->now() >= preferences->getTransferAlmostOverQuotaUiAlertDisableUntil()};
    if (executeUiWarningAlert)
    {
        megaApi->sendEvent(eventIdTransferAlmostOverQuotaUiAlert, eventMessageTransferAlmostQuotaUiAlert);
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
    if(usagePercent >= almostOverquotaPercent)
    {
        preferences->setTransferQuotaState(Preferences::QuotaState::warning);
        emit sendState(Preferences::QuotaState::warning);
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
    const auto dialogEnabled{time->now() >= preferences->getTransferOverQuotaImportLinksDialogDisabledUntil()};
    if(isOverQuota() && dialogEnabled)
    {
        preferences->setTransferOverQuotaImportLinksDialogDisabledUntil(time->now()+overquotaDialogDisableTime);
        const auto bandwidthFullDialog{OverQuotaDialog::createDialog(OverquotaFullDialogType::bandwidthFullImportLink)};
        dismissed = bandwidthFullDialog->exec() == QDialog::Accepted;
    }
    return dismissed;
}

bool TransferQuota::checkDownloadAlertDismissed()
{
    auto dismissed{true};
    const auto dialogEnabled{time->now() >= preferences->getTransferOverQuotaDownloadsDialogDisabledUntil()};
    if(isOverQuota() && dialogEnabled)
    {
        preferences->setTransferOverQuotaDownloadsDialogDisabledUntil(time->now()+overquotaDialogDisableTime);
        const auto bandwidthFullDialog{OverQuotaDialog::createDialog(OverquotaFullDialogType::bandwidthFullDownloads)};
        dismissed = bandwidthFullDialog->exec() == QDialog::Accepted;
    }
    return dismissed;
}

bool TransferQuota::checkStreamingAlertDismissed()
{
    auto dismissed{true};
    const auto dialogEnabled{time->now() >= preferences->getTransferOverQuotaStreamDialogDisabledUntil()};
    if(isOverQuota() && dialogEnabled)
    {
        preferences->setTransferOverQuotaStreamDialogDisabledUntil(time->now()+overquotaDialogDisableTime);
        const auto bandwidthFullDialog{OverQuotaDialog::createDialog(OverquotaFullDialogType::bandwidthFullStream)};
        dismissed = bandwidthFullDialog->exec() == QDialog::Accepted;
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
    preferences->setTransferOverQuotaUiAlertDisabledUntil(time->now()+Preferences::overquotaUiMessageDisableDuration);
}

void TransferQuota::onDismissAlmostOverQuotaUiMessage()
{
    preferences->setTransferAlmostOverQuotaUiAlertDisabledUntil(time->now()+Preferences::almostOverquotaUiMessageDisableDuration);
}
