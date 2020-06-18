#include "BandwidthOverquota.h"
#include "mega/types.h"
#include "platform/Platform.h"
#include "OverquotaFullDialog.h"

BandwidthOverquota::BandwidthOverquota(mega::MegaApi* megaApi, Preferences *preferences, Notificator *notificator)
    :BandwidthOverquota{megaApi, preferences, notificator, mega::make_unique<Time>()}
{
}

BandwidthOverquota::BandwidthOverquota(mega::MegaApi* megaApi,
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

void BandwidthOverquota::setFullState(std::chrono::milliseconds waitTime)
{
    preferences->setOverBandwidthWaitUntil(time->now()+waitTime);
    preferences->setBandwidthOverquotaState(Preferences::OverquotaState::full);
    emit sendState(Preferences::OverquotaState::full);
    checkExecuteAlerts();
}

void BandwidthOverquota::checkState()
{
    if(isStateFull())
    {
        const auto waitTimeIsOver{time->now() >= preferences->getOverBandwidthWaitUntil()};
        if(waitTimeIsOver)
        {
            setOkState();
        }
    }
}

bool BandwidthOverquota::isStateOk() const
{
    return preferences->getBandwidthOverquotaState() == Preferences::OverquotaState::ok;
}

bool BandwidthOverquota::isStateFull() const
{
    return preferences->getBandwidthOverquotaState() == Preferences::OverquotaState::full;
}

bool BandwidthOverquota::isStateWarning() const
{
    return preferences->getBandwidthOverquotaState() == Preferences::OverquotaState::warning;
}

void BandwidthOverquota::setOkState()
{
    preferences->setOverBandwidthWaitUntil(std::chrono::system_clock::time_point());
    preferences->setBandwidthOverquotaState(Preferences::OverquotaState::ok);
    preferences->clearTemporalBandwidth(); // TODO: why this?
    emit sendState(Preferences::OverquotaState::ok);

    if (upgradeDialog)
    {
        upgradeDialog->close();
    }
}

void BandwidthOverquota::checkExecuteDialog()
{
    const auto executeDialog{time->now() >= preferences->getOverBandwidthDialogDisabledUntil()};
    if(executeDialog)
    {
        preferences->setOverBandwidthDialogDisabledUntil(time->now()+Preferences::overquotaDialogDisableDuration);
        if (!upgradeDialog)
        {
            upgradeDialog = new UpgradeDialog(megaApi, pricing);
            QObject::connect(upgradeDialog, &UpgradeDialog::finished, this, &BandwidthOverquota::upgradeDialogFinished);
            Platform::activateBackgroundWindow(upgradeDialog);
            upgradeDialog->show();

            if (upgradeDialogEventEnabled)
            {
                megaApi->sendEvent(eventIdTransferQuotaDialogExecuted, eventMessageTransferQuotaDialogExecuted);
                upgradeDialogEventEnabled = false;
            }
        }
        else if (!upgradeDialog->isVisible())
        {
            upgradeDialog->activateWindow();
            upgradeDialog->raise();
        }

        const auto endWaitTimeSinceEpoch{preferences->getOverBandwidthWaitUntil().time_since_epoch()};
        const auto endWaitTimeSinceEpochSeconds{std::chrono::duration_cast<std::chrono::seconds>(endWaitTimeSinceEpoch).count()};
        upgradeDialog->setTimestamp(endWaitTimeSinceEpochSeconds);
        upgradeDialog->refreshAccountDetails();
    }
}

void BandwidthOverquota::checkExecuteNotification()
{
    const auto executeNotification{time->now() >= preferences->getOverBandwidthNotificationDisabledUntil()};
    if (executeNotification)
    {
        preferences->setOverBandwidthNotificationDisabledUntil(time->now()+Preferences::overquotaNotificationDisableDuration);
        megaApi->sendEvent(eventIdTransferQuotaOsNotificationExecuted, eventMessageTransferQuotaOsNotificationExecuted);
        sendOverBandwithFullNotification();
    }
}

void BandwidthOverquota::checkExecuteUiMessage()
{
    const auto executeUiAlert{time->now() >= preferences->getOverBandwidthUiMessageDisabledUntil()};
    if (executeUiAlert)
    {
        megaApi->sendEvent(eventIdTransferQuotaUiDialogExecuted, eventMessageTransferQuotaUiDialogExecuted);
        emit overquotaUiMessageEnabled();
    }
}

void BandwidthOverquota::checkExecuteWarningOsNotification()
{
    const auto executeNotification{time->now() >= preferences->getAlmostOverBandwidthNotificationDisabledUntil()};
    if (executeNotification)
    {
        preferences->setAlmostOverBandwidthNotificationDisabledUntil(time->now()+Preferences::almostOverquotaOsNotificationDisableDuration);
        megaApi->sendEvent(eventIdTransferAlmostQuotaNotificationExecuted, eventMessageTransferAlmostQuotaNotificationExecuted);
        sendOverBandwidthWarningNotification();
    }
}

void BandwidthOverquota::checkExecuteWarningUiMessage()
{
    const auto executeUiWarningAlert{time->now() >= preferences->getAlmostOverBandwidthUiMessageDisableUntil()};
    if (executeUiWarningAlert)
    {
        megaApi->sendEvent(eventIdTransferAlmostQuotaUiDialogExecuted, eventMessageTransferAlmostQuotaUiDialogExecuted);
        emit almostOverquotaUiMessageEnabled();
    }
}

void BandwidthOverquota::checkExecuteAlerts()
{
    const auto megaApp{static_cast<MegaApplication*>(qApp)};
    const auto allowAlerts{megaApp->isInfoDialogVisible() || upgradeDialog || Platform::isUserActive()};
    const auto bandwidthAlertsEnabled{!megaApp->finished() && preferences->logged() && allowAlerts};
    if (bandwidthAlertsEnabled)
    {
        if(isStateFull())
        {
            checkExecuteDialog();
            checkExecuteNotification();
            checkExecuteUiMessage();
        }
        else if(isStateWarning())
        {
            checkExecuteWarningOsNotification();
            checkExecuteWarningUiMessage();
        }
    }
}

void BandwidthOverquota::setUsages(long long usedBytes, long long totalBytes)
{
    const auto usagePercent{std::floor(100.0 * usedBytes / totalBytes)};
    if(usagePercent >= warningPercent)
    {
        preferences->setBandwidthOverquotaState(Preferences::OverquotaState::warning);
        emit sendState(Preferences::OverquotaState::warning);
        checkExecuteAlerts();
    }
}

void BandwidthOverquota::refreshOverquotaDialogDetails()
{
    if(upgradeDialog)
    {
        upgradeDialog->refreshAccountDetails();
    }
}

void BandwidthOverquota::setOverquotaDialogPricing(mega::MegaPricing *pricing)
{
    this->pricing = pricing;
    if(upgradeDialog)
    {
        upgradeDialog->setPricing(pricing);
    }
}

void BandwidthOverquota::closeDialogs()
{
    if(upgradeDialog)
    {
        delete upgradeDialog;
        upgradeDialog = nullptr;
    }
}

void BandwidthOverquota::checkStateAndAlerts()
{
    checkState();
    checkExecuteAlerts();
}

bool BandwidthOverquota::checkImportLinksAlertDismissed()
{
    auto dismissed{true};
    const auto dialogEnabled{time->now() >= preferences->getBandwidthFullImportLinksDialogDisabledUntil()};
    if(isStateFull() && dialogEnabled)
    {
        preferences->setBandwidthFullImportLinksDialogDisabledUntil(time->now()+overquotaDialogDisableTime);
        const auto bandwidthFullDialog{OverquotaFullDialog::createDialog(OverquotaFullDialogType::bandwidthFullImportLink)};
        dismissed = bandwidthFullDialog->exec() == QDialog::Accepted;
    }
    return dismissed;
}

bool BandwidthOverquota::checkDownloadAlertDismissed()
{
    auto dismissed{true};
    const auto dialogEnabled{time->now() >= preferences->getBandwidthFullDownloadsDialogDisabledUntil()};
    if(isStateFull() && dialogEnabled)
    {
        preferences->setBandwidthFullDownloadsDialogDisabledUntil(time->now()+overquotaDialogDisableTime);
        const auto bandwidthFullDialog{OverquotaFullDialog::createDialog(OverquotaFullDialogType::bandwidthFullDownlads)};
        dismissed = bandwidthFullDialog->exec() == QDialog::Accepted;
    }
    return dismissed;
}

bool BandwidthOverquota::checkStreamingAlertDismissed()
{
    auto dismissed{true};
    const auto dialogEnabled{time->now() >= preferences->getBandwidthFullStreamDialogDisabledUntil()};
    if(isStateFull() && dialogEnabled)
    {
        preferences->setBandwidthFullStreamDialogDisabledUntil(time->now()+overquotaDialogDisableTime);
        const auto bandwidthFullDialog{OverquotaFullDialog::createDialog(OverquotaFullDialogType::bandwidthFullStream)};
        dismissed = bandwidthFullDialog->exec() == QDialog::Accepted;
    }
    return dismissed;
}

void BandwidthOverquota::sendNotification(const QString &title)
{
    MegaNotification *notification = new MegaNotification();
    notification->setTitle(title);
    notification->setText(tr("Upgrade now to a PRO account."));
    notification->setActions(QStringList() << tr("Get PRO"));
    const auto megaApp{static_cast<MegaApplication*>(qApp)};
    connect(notification, &MegaNotification::activated, megaApp, &MegaApplication::redirectToUpgrade);
    notificator->notify(notification);
}

void BandwidthOverquota::sendOverBandwidthWarningNotification()
{
    const auto title{tr("Limited available transfer quota.")};
    sendNotification(title);
}

void BandwidthOverquota::sendOverBandwithFullNotification()
{
    const auto title{tr("Depleted transfer quota.")};
    sendNotification(title);
}

void BandwidthOverquota::upgradeDialogFinished(int)
{
    if (!static_cast<MegaApplication*>(qApp)->finished() && upgradeDialog)
    {
        upgradeDialog->deleteLater();
        upgradeDialog = nullptr;
    }
}

void BandwidthOverquota::onDismissUiMessageFullOverquota()
{
    preferences->setOverBandwidthUiMessageDisabledUntil(time->now()+Preferences::overquotaUiMessageDisableDuration);
}

void BandwidthOverquota::onDismissUiMessageAlmostOverquota()
{
    preferences->setAlmostOverBandwidthUiMessageDisabledUntil(time->now()+Preferences::almostOverquotaUiMessageDisableDuration);
}
