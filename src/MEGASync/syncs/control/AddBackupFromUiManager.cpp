#include "AddBackupFromUiManager.h"

#include "qml/QmlDialogManager.h"
#include "DialogOpener.h"
#include "backups/BackupsController.h"
#include "syncs/gui/Backups/RemoveBackupDialog.h"
#include "syncs/control/SyncSettings.h"

const AddBackupFromUiManager* AddBackupFromUiManager::addBackup_static(bool fromOnboarding)
{
    auto backupManager(new AddBackupFromUiManager());
    backupManager->performAddBackup(fromOnboarding);
    return backupManager;
}

const AddBackupFromUiManager* AddBackupFromUiManager::removeBackup_static(std::shared_ptr<SyncSettings> backup, QWidget* parent)
{
    auto backupManager(new AddBackupFromUiManager());
    backupManager->performRemoveBackup(backup, parent);
    return backupManager;
}

void AddBackupFromUiManager::addBackup(bool fromOnboarding)
{
    performAddBackup(fromOnboarding);
}

void AddBackupFromUiManager::removeBackup(std::shared_ptr<SyncSettings> backup, QWidget* parent)
{
    performRemoveBackup(backup, parent);
}

void AddBackupFromUiManager::performAddBackup(bool fromOnboarding)
{
    auto overQuotaDialog = MegaSyncApp->showSyncOverquotaDialog();
    auto addBackupLambda = [overQuotaDialog, fromOnboarding, this]()
    {
        if(!overQuotaDialog || overQuotaDialog->result() == QDialog::Rejected)
        {
           QmlDialogManager::instance()->openBackupsDialog(!fromOnboarding);
        }
    };

    if(overQuotaDialog)
    {
        DialogOpener::showDialog(overQuotaDialog,addBackupLambda);
    }
    else
    {
        addBackupLambda();
    }
}

void AddBackupFromUiManager::performRemoveBackup(std::shared_ptr<SyncSettings> backup, QWidget* parent)
{
    QPointer<RemoveBackupDialog> dialog = new RemoveBackupDialog(backup, parent);

    DialogOpener::showDialog(dialog,
        [this, dialog]()
        {
            if(dialog->result() == QDialog::Accepted)
            {
                connect(&BackupsController::instance(),
                    &BackupsController::syncRemoveStatus,
                    this,
                    [this](
                        const int)
                    {
                        deleteLater();
                    });

                BackupsController::instance().removeSync(dialog->backupToRemove(), dialog->targetFolder());
            }
            else
            {
                deleteLater();
            }
        });
}
