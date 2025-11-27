#include "CreateRemoveBackupsManager.h"

#include "BackupCandidatesComponent.h"
#include "DialogOpener.h"
#include "QmlDialogWrapper.h"
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
    if (mRemoveBackupHandler == nullptr)
    {
        mRemoveBackupHandler = new RemoveBackup();
    }

    mRemoveBackupHandler->removeBackup(backup, parent);
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
