#include "BackupSettingsUI.h"

#include "syncs/gui/Backups/BackupTableView.h"
#include "syncs/model/BackupItemModel.h"

#include "Utilities.h"
#include "DialogOpener.h"
#include "RemoveBackupDialog.h"
#include "QMegaMessageBox.h"

#include "ui_SyncSettingsUIBase.h"

BackupSettingsUI::BackupSettingsUI(QWidget *parent) :
    SyncSettingsUIBase(parent)
{
    setTitle(tr("Backups"));
    setTable<BackupTableView, BackupItemModel>();

    connect(mSyncController, &SyncController::backupMoveOrRemoveRemoteFolderError, this, [this](std::shared_ptr<mega::MegaError> err)
    {
        onSavingSyncsCompleted(SAVING_FINISHED);
        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.title = tr("Error moving or removing remote backup folder");
        msgInfo.text =                                  tr("Failed to move or remove the remote backup folder. Reason: %1")
                .arg(QCoreApplication::translate("MegaError", err->getErrorString()));
        QMegaMessageBox::warning(msgInfo);

    });

    mElements.initElements(this);
    ui->gSyncs->setUsePermissions(false);
}

BackupSettingsUI::~BackupSettingsUI()
{
}

void BackupSettingsUI::addButtonClicked(mega::MegaHandle)
{
    QPointer<AddBackupDialog> addBackup = new AddBackupDialog(this);
    DialogOpener::showDialog(addBackup,[this, addBackup]()
    {
        if(addBackup->result() == QDialog::Accepted)
        {
            mSyncController->addBackup(addBackup->getSelectedFolder(), addBackup->getBackupName());
            syncsStateInformation(SyncStateInformation::SAVING);
        }
    });
}

void BackupSettingsUI::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        mElements.updateUI();
    }

    SyncSettingsUIBase::changeEvent(event);
}

void BackupSettingsUI::removeSync(std::shared_ptr<SyncSettings> backup)
{
    QPointer<RemoveBackupDialog> dialog = new RemoveBackupDialog(backup, this);

    DialogOpener::showDialog(dialog,[this, dialog]()
    {
        if(dialog->result() == QDialog::Accepted)
        {
            syncsStateInformation(SyncStateInformation::SAVING);
            mSyncController->removeSync(dialog->backupToRemove(), dialog->targetFolder());
        }
    });
}

QString BackupSettingsUI::getFinishWarningIconString()
{
#ifdef Q_OS_MACOS
    return QString::fromUtf8("settings-backups-error");
#else
    return QString::fromUtf8(":/images/settings-backups-warn.png");
#endif
}

QString BackupSettingsUI::getFinishIconString()
{
#ifdef Q_OS_MACOS
    return QString::fromUtf8("settings-backup");
#else
    return QString::fromUtf8(":/images/settings-backup.png");
#endif
}

QString BackupSettingsUI::getOperationFailTitle()
{
    return tr("Sync operation failed");
}

QString BackupSettingsUI::getOperationFailText(std::shared_ptr<SyncSettings> sync)
{
    return tr("Operation on sync '%1' failed. Reason: %2")
        .arg(sync->name(),
             QCoreApplication::translate("MegaSyncError", mega::MegaSync::getMegaSyncErrorCode(sync->getError())));
}

QString BackupSettingsUI::getErrorAddingTitle()
{
    return tr("Error adding sync");
}

QString BackupSettingsUI::getErrorRemovingTitle()
{
    return tr("Error removing backup");
}

QString BackupSettingsUI::getErrorRemovingText(std::shared_ptr<mega::MegaError> err)
{
    return tr("Your sync can't be removed. Reason: %1")
        .arg(QCoreApplication::translate("MegaError", err->getErrorString()));
}

QString BackupSettingsUI::disableString()
{
    return tr("Some folders haven't been backed up. For more information, hover over the red icon.");
}
