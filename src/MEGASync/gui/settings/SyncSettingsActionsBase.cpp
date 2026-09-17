#include "SyncSettingsActionsBase.h"

#include "DialogOpener.h"
#include "MegaApplication.h"
#include "MessageDialogOpener.h"
#include "QmlDialogWrapper.h"
#include "SyncController.h"
#include "SyncExclusions.h"
#include "SyncSettingsModelBase.h"
#include "Utilities.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QWidget>

SyncSettingsActionsBase::SyncSettingsActionsBase(SyncSettingsModelBase* model,
                                                 SyncController& controller,
                                                 QWidget* hostWidget,
                                                 QObject* parent):
    QObject(parent),
    mModel(model),
    mController(controller),
    mHostWidget(hostWidget)
{
    // The model is created by the subclass without a parent (it cannot take
    // `this` in the initializer list); own it from here on.
    mModel->setParent(this);
}

SyncSettingsModelBase* SyncSettingsActionsBase::model() const
{
    return mModel;
}

QWidget* SyncSettingsActionsBase::hostWindow() const
{
    return mHostWidget ? mHostWidget->window() : nullptr;
}

QWidget* SyncSettingsActionsBase::hostParentWidget() const
{
    return mHostWidget ? mHostWidget->parentWidget() : nullptr;
}

void SyncSettingsActionsBase::setContextMenuOpen(bool open)
{
    emit contextMenuOpenChanged(open);
}

void SyncSettingsActionsBase::requestCloseContextMenu()
{
    emit closeContextMenu();
}

void SyncSettingsActionsBase::exploreLocal(const QString& localFolder) const
{
    Utilities::openUrl(QUrl::fromLocalFile(localFolder));
}

void SyncSettingsActionsBase::openInMega(int index) const
{
    Utilities::openInMega(mModel->getSyncSetting(index)->getMegaHandle());
}

void SyncSettingsActionsBase::pause(int index) const
{
    mController.setSyncToSuspend(mModel->getSyncSetting(index));
}

void SyncSettingsActionsBase::resume(int index) const
{
    mController.setSyncToRun(mModel->getSyncSetting(index));
}

void SyncSettingsActionsBase::openExclusionsDialog(int index) const
{
    const auto& sync = mModel->getSyncSetting(index);
    QFileInfo syncDir(sync->getLocalFolder());
    if (syncDir.exists())
    {
        QPointer<QmlDialogWrapper<SyncExclusions>> exclusions =
            new QmlDialogWrapper<SyncExclusions>(hostWindow(), sync->getLocalFolder(), sync);

        DialogOpener::showDialog(exclusions);
    }
    else
    {
        MessageDialogInfo msgInfo;
        msgInfo.parent = hostParentWidget();
        // Translation context kept from SettingsQuickWidgetBase (pre SNC-6795).
        msgInfo.descriptionText =
            QCoreApplication::translate("SettingsQuickWidgetBase", "Error opening megaignore file");
        MessageDialogOpener::warning(msgInfo);
    }
}

void SyncSettingsActionsBase::rescan(int index) const
{
    const auto& sync = mModel->getSyncSetting(index);
    MegaSyncApp->getMegaApi()->rescanSync(sync->backupId(), true);
}

void SyncSettingsActionsBase::reboot(int index) const
{
    const auto& sync = mModel->getSyncSetting(index);
    mController.resetSync(sync, mega::MegaSync::SyncRunningState::RUNSTATE_DISABLED);
}

void SyncSettingsActionsBase::removeNonConfirmation(int index) const
{
    const auto& sync = mModel->getSyncSetting(index);
    mController.removeSync(sync);
}

void SyncSettingsActionsBase::onGetMoreQuotaClicked() const
{
    Utilities::upgradeClicked();
}

void SyncSettingsActionsBase::sortModelByName(bool ascending)
{
    mModel->sortByName(ascending);
}

void SyncSettingsActionsBase::sortModelByStatus(bool ascending)
{
    mModel->sortByStatus(ascending);
}
