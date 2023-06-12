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
        QMegaMessageBox::warning(nullptr, tr("Error moving or removing remote backup folder"),
                                 tr("Failed to move or remove the remote backup folder. Reason: %1")
                                 .arg(QCoreApplication::translate("MegaError", err->getErrorString())));

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
    return QString::fromUtf8(":/images/settings-backups-warn.png");
}

QString BackupSettingsUI::getFinishIconString()
{
    return QString::fromUtf8(":/images/settings-backup.png");
}

QString BackupSettingsUI::typeString()
{
    return tr("backup");
}
