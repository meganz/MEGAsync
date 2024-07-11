#include "AddBackupFromUiManager.h"

#include "qml/QmlDialogManager.h"
#include "DialogOpener.h"
#include "syncs/control/SyncController.h"
#include "syncs/gui/Backups/RemoveBackupDialog.h"
#include "syncs/control/SyncSettings.h"

const AddBackupFromUiManager* AddBackupFromUiManager::addBackup(bool fromOnboarding)
{
    auto backupManager(new AddBackupFromUiManager());
    backupManager->performAddBackup(fromOnboarding);
    return backupManager;
}

const AddBackupFromUiManager* AddBackupFromUiManager::removeBackup(std::shared_ptr<SyncSettings> backup, QWidget* parent)
{
    auto backupManager(new AddBackupFromUiManager());
    backupManager->performRemoveBackup(backup, parent);
    return backupManager;
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
                auto syncController = new SyncController();

                SyncController::connect(syncController,
                    &SyncController::syncRemoveStatus,
                    this,
                    [this, syncController](
                        const int)
                    {
                        syncController->deleteLater();
                        deleteLater();
                    });

                syncController->removeSync(dialog->backupToRemove(), dialog->targetFolder());
            }
            else
            {
                deleteLater();
            }
        });
}
