#include "CreateRemoveBackupsManager.h"

#include "BackupCandidatesComponent.h"
#include "DialogOpener.h"
#include "QmlDialogWrapper.h"
#include "SyncSettings.h"

void CreateRemoveBackupsManager::addBackup(SyncInfo::SyncOrigin origin,
                                           const QStringList& localFolders)
{
    auto overQuotaDialog = MegaSyncApp->createOverquotaDialogIfNeeded();

    if (overQuotaDialog)
    {
        DialogOpener::showDialog(overQuotaDialog,
                                 [overQuotaDialog, origin, localFolders]()
                                 {
                                     if (overQuotaDialog->result() == QDialog::Rejected)
                                     {
                                         showBackupDialog(origin, localFolders);
                                     }
                                 });
    }
    else
    {
        showBackupDialog(origin, localFolders);
    }
}

void CreateRemoveBackupsManager::removeBackup(std::shared_ptr<SyncSettings> backup, QWidget* parent)
{
    if (mRemoveBackupHandler == nullptr)
    {
        mRemoveBackupHandler = new RemoveBackup(qApp);
    }

    mRemoveBackupHandler->removeBackup(backup, parent);
}

bool CreateRemoveBackupsManager::isBackupsDialogOpen()
{
    return DialogOpener::findDialog<QmlDialogWrapper<BackupCandidatesComponent>>() != nullptr;
}

void CreateRemoveBackupsManager::showBackupDialog(SyncInfo::SyncOrigin origin,
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
    backupsDialog->wrapper()->setOrigin(origin);
    if (!localFolders.empty())
    {
        backupsDialog->wrapper()->insertFolders(localFolders);
    }

    DialogOpener::showDialog(backupsDialog);
}
