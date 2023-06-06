#include "BackupSettingsUI.h"
#include "ui_BackupSettingsUI.h"

#include "syncs/gui/Backups/BackupTableView.h"
#include "syncs/model/BackupItemModel.h"

#include "Utilities.h"
#include "UserAttributesRequests/MyBackupsHandle.h"
#include "DialogOpener.h"
#include "RemoveBackupDialog.h"
#include "QMegaMessageBox.h"

BackupSettingsUI::BackupSettingsUI(QWidget *parent) :
    SyncSettingsUIBase(parent)
{
    auto myBackupsHandle = UserAttributes::MyBackupsHandle::requestMyBackupsHandle();
    connect(myBackupsHandle.get(), &UserAttributes::MyBackupsHandle::attributeReady,
            this, &BackupSettingsUI::onMyBackupsFolderHandleSet);
    onMyBackupsFolderHandleSet(myBackupsHandle->getMyBackupsHandle());

    setType(mega::MegaSync::SyncType::TYPE_BACKUP);
    setTable<BackupTableView, BackupItemModel>();
    mTable->setColumnHidden(SyncItemModel::Column_DOWNLOADS, true);

    connect(mSyncController, &SyncController::backupMoveOrRemoveRemoteFolderError, this, [this](std::shared_ptr<mega::MegaError> err)
    {
        onSavingSyncsCompleted(SAVING_FINISHED);
        QMegaMessageBox::warning(nullptr, tr("Error moving or removing remote backup folder"),
                                 tr("Failed to move or remove the remote backup folder. Reason: %1")
                                 .arg(QCoreApplication::translate("MegaError", err->getErrorString())));

    });

//    ui->bOpenBackupFolder->setEnabled(false);
//    ui->gSyncs->setUsePermissions(false);
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
    QString backupsDirPath = UserAttributes::MyBackupsHandle::getMyBackupsLocalizedPath();
    //ui->lBackupFolder->setText(backupsDirPath);

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

void BackupSettingsUI::on_bOpenBackupFolder_clicked()
{
    auto myBackupsHandle = UserAttributes::MyBackupsHandle::requestMyBackupsHandle();
    Utilities::openInMega(myBackupsHandle->getMyBackupsHandle());
}

void BackupSettingsUI::onMyBackupsFolderHandleSet(mega::MegaHandle h)
{
//    ui->lBackupFolder->setText(UserAttributes::MyBackupsHandle::getMyBackupsLocalizedPath());

//    if (h == mega::INVALID_HANDLE)
//    {
//        ui->bOpenBackupFolder->setEnabled(false);
//    }
//    else
//    {
//        ui->bOpenBackupFolder->setEnabled(true);
//    }
}
