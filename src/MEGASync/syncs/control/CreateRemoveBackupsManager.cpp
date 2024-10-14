#include "CreateRemoveBackupsManager.h"

#include "Backups.h"
#include "BackupsController.h"
#include "DialogOpener.h"
#include "QmlDialogWrapper.h"
#include "RemoveBackupDialog.h"
#include "SyncSettings.h"

const CreateRemoveBackupsManager *  CreateRemoveBackupsManager::addBackup(bool comesFromSettings)
{
    auto backupManager(new CreateRemoveBackupsManager());
    backupManager->performAddBackup(comesFromSettings);
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
    return DialogOpener::findDialog<QmlDialogWrapper<Backups>>() != nullptr;
}

void CreateRemoveBackupsManager::performAddBackup(bool comesFromSettings)
{
    auto overQuotaDialog = MegaSyncApp->showSyncOverquotaDialog();
    auto addBackupLambda = [overQuotaDialog, comesFromSettings, this]() {
        if (!overQuotaDialog || overQuotaDialog->result() == QDialog::Rejected)
        {
            QPointer<QmlDialogWrapper<Backups>> backupsDialog;
            if (auto dialog = DialogOpener::findDialog<QmlDialogWrapper<Backups>>())
            {
                backupsDialog = dialog->getDialog();
            }
            else
            {
                backupsDialog = new QmlDialogWrapper<Backups>();
            }
            DialogOpener::showDialog(backupsDialog, [this]() {
                deleteLater();
            });
            backupsDialog->wrapper()->setComesFromSettings(comesFromSettings);
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
