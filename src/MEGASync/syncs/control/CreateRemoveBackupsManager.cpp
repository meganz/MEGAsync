#include "CreateRemoveBackupsManager.h"

#include "BackupCandidatesComponent.h"
#include "BackupsController.h"
#include "DialogOpener.h"
#include "QmlDialogWrapper.h"
#include "RemoveBackupDialog.h"
#include "SyncSettings.h"

const CreateRemoveBackupsManager*
    CreateRemoveBackupsManager::addBackup(bool comesFromSettings, const QStringList& localFolders)
{
    auto backupManager(new CreateRemoveBackupsManager());
    backupManager->performAddBackup(localFolders, comesFromSettings);
    return backupManager;
}

const CreateRemoveBackupsManager *CreateRemoveBackupsManager::removeBackup(std::shared_ptr<SyncSettings> backup, QWidget* parent)
{
    auto backupManager(new CreateRemoveBackupsManager());
    backupManager->performRemoveBackup(backup, parent);
    return backupManager;
}

bool CreateRemoveBackupsManager::isBackupsDialogOpen() const
{
    return DialogOpener::findDialog<QmlDialogWrapper<BackupCandidatesComponent>>() != nullptr;
}

void CreateRemoveBackupsManager::performAddBackup(const QStringList& localFolders,
                                                  bool comesFromSettings)
{
    auto overQuotaDialog = MegaSyncApp->showSyncOverquotaDialog();
    auto addBackupLambda = [overQuotaDialog, comesFromSettings, localFolders, this]()
    {
        if (!overQuotaDialog || overQuotaDialog->result() == QDialog::Rejected)
        {
            QPointer<QmlDialogWrapper<BackupCandidatesComponent>> backupsDialog;
            if (auto dialog =
                    DialogOpener::findDialog<QmlDialogWrapper<BackupCandidatesComponent>>())
            {
                backupsDialog = dialog->getDialog();
            }
            else
            {
                backupsDialog = new QmlDialogWrapper<BackupCandidatesComponent>();
            }
            DialogOpener::showDialog(backupsDialog, [this]() {
                deleteLater();
            });
            backupsDialog->wrapper()->setComesFromSettings(comesFromSettings);
            backupsDialog->wrapper()->insertFolders(localFolders);
        }
        else
        {
            deleteLater();
        }
    };

    if (overQuotaDialog)
    {
        DialogOpener::showDialog(overQuotaDialog, addBackupLambda);
    }
    else
    {
        addBackupLambda();
    }
}

void CreateRemoveBackupsManager::performRemoveBackup(std::shared_ptr<SyncSettings> backup,
                                                     QWidget* parent)
{
    QPointer<RemoveBackupDialog> dialog = new RemoveBackupDialog(backup, parent);

    DialogOpener::showDialog(dialog, [this, dialog]() {
        if (dialog->result() == QDialog::Accepted)
        {
            MegaSyncApp->getStatsEventHandler()->sendTrackedEvent(
                AppStatsEvents::EventType::CONFIRM_REMOVE_BACKUP);
            connect(&BackupsController::instance(),
                    &BackupsController::syncRemoveStatus,
                    this,
                    [this](const int) {
                        deleteLater();
                    });

            BackupsController::instance().removeSync(dialog->backupToRemove(),
                                                     dialog->targetFolder());
        }
        else
        {
            deleteLater();
        }
    });
}
