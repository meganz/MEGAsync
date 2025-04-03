#include "CreateRemoveBackupsManager.h"

#include "BackupCandidatesComponent.h"
#include "BackupsController.h"
#include "DialogOpener.h"
#include "QmlDialogWrapper.h"
#include "RemoveBackupDialog.h"
#include "SyncSettings.h"

void CreateRemoveBackupsManager::addBackup(bool comesFromSettings)
{
    CreateRemoveBackupsManager::performAddBackup(comesFromSettings);
}

void CreateRemoveBackupsManager::removeBackup(std::shared_ptr<SyncSettings> backup, QWidget* parent)
{
    CreateRemoveBackupsManager::performRemoveBackup(backup, parent);
}

bool CreateRemoveBackupsManager::isBackupsDialogOpen()
{
    return DialogOpener::findDialog<QmlDialogWrapper<BackupCandidatesComponent>>() != nullptr;
}

void CreateRemoveBackupsManager::performAddBackup(bool comesFromSettings)
{
    auto overQuotaDialog = MegaSyncApp->createSyncOverquotaDialog();

    if (overQuotaDialog)
    {
        DialogOpener::showDialog(overQuotaDialog,
                                 [overQuotaDialog, comesFromSettings]()
                                 {
                                     if (overQuotaDialog->result() == QDialog::Rejected)
                                     {
                                         showBackupDialog(comesFromSettings);
                                     }
                                 });
    }
    else
    {
        showBackupDialog(comesFromSettings);
    }
}

void CreateRemoveBackupsManager::performRemoveBackup(std::shared_ptr<SyncSettings> backup,
                                                     QWidget* parent)
{
    QPointer<RemoveBackupDialog> dialog = new RemoveBackupDialog(backup, parent);

    DialogOpener::showDialog(dialog,
                             [dialog]()
                             {
                                 if (dialog->result() == QDialog::Accepted)
                                 {
                                     MegaSyncApp->getStatsEventHandler()->sendTrackedEvent(
                                         AppStatsEvents::EventType::CONFIRM_REMOVE_BACKUP);

                                     BackupsController::instance().removeSync(
                                         dialog->backupToRemove(),
                                         dialog->targetFolder());
                                 }
                             });
}

void CreateRemoveBackupsManager::showBackupDialog(bool comesFromSettings)
{
    QPointer<QmlDialogWrapper<BackupCandidatesComponent>> backupsDialog =
        new QmlDialogWrapper<BackupCandidatesComponent>();
    backupsDialog->wrapper()->setComesFromSettings(comesFromSettings);

    DialogOpener::showDialog(backupsDialog);
}
