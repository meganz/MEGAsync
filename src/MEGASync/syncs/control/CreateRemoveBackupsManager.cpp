#include "CreateRemoveBackupsManager.h"

#include "BackupCandidatesComponent.h"
#include "BackupsController.h"
#include "DialogOpener.h"
#include "QmlDialogWrapper.h"
#include "RemoveBackupDialog.h"
#include "SyncSettings.h"

void CreateRemoveBackupsManager::addBackup(bool comesFromSettings, const QStringList& localFolders)
{
    auto overQuotaDialog = MegaSyncApp->createOverquotaDialogIfNeeded();

    if (overQuotaDialog)
    {
        DialogOpener::showDialog(overQuotaDialog,
                                 [overQuotaDialog, comesFromSettings, localFolders]()
                                 {
                                     if (overQuotaDialog->result() == QDialog::Rejected)
                                     {
                                         showBackupDialog(comesFromSettings, localFolders);
                                     }
                                 });
    }
    else
    {
        showBackupDialog(comesFromSettings, localFolders);
    }
}

void CreateRemoveBackupsManager::removeBackup(std::shared_ptr<SyncSettings> backup, QWidget* parent)
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

bool CreateRemoveBackupsManager::isBackupsDialogOpen()
{
    return DialogOpener::findDialog<QmlDialogWrapper<BackupCandidatesComponent>>() != nullptr;
}

void CreateRemoveBackupsManager::showBackupDialog(bool comesFromSettings,
                                                  const QStringList& localFolders)
{
    QPointer<QmlDialogWrapper<BackupCandidatesComponent>> backupsDialog;
    if (auto dialog = DialogOpener::findDialog<QmlDialogWrapper<BackupCandidatesComponent>>())
    {
        backupsDialog = dialog->getDialog();
    }
    else
    {
        backupsDialog = new QmlDialogWrapper<BackupCandidatesComponent>();
    }
    backupsDialog->wrapper()->setComesFromSettings(comesFromSettings);
    if (!localFolders.empty())
    {
        backupsDialog->wrapper()->insertFolders(localFolders);
    }

    DialogOpener::showDialog(backupsDialog);
}
