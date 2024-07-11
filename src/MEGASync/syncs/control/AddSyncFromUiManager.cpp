#include "AddSyncFromUiManager.h"

#include <GuiUtilities.h>
#include <DialogOpener.h>
#include <syncs/gui/Twoways/BindFolderDialog.h>
#include <syncs/control/SyncSettings.h>
#include <syncs/gui/Twoways/RemoveSyncConfirmationDialog.h>

const AddSyncFromUiManager* const AddSyncFromUiManager::addSync_static(mega::MegaHandle handle, bool disableUi)
{
    auto syncManager(new AddSyncFromUiManager());
    syncManager->addSync(handle, disableUi);
    return syncManager;
}

const AddSyncFromUiManager* const AddSyncFromUiManager::removeSync_static(mega::MegaHandle handle, QWidget* parent)
{
    auto syncManager(new AddSyncFromUiManager());
    syncManager->removeSync(handle, parent);
    return syncManager;
}

void AddSyncFromUiManager::addSync(mega::MegaHandle handle, bool disableUi)
{
    performAddSync(handle, disableUi);
}

void AddSyncFromUiManager::removeSync(mega::MegaHandle handle, QWidget* parent)
{
    performRemoveSync(handle, parent);
}

void AddSyncFromUiManager::performAddSync(mega::MegaHandle handle, bool disableUi)
{
    auto overQuotaDialog = MegaSyncApp->showSyncOverquotaDialog();
    auto addSyncLambda = [overQuotaDialog, handle, disableUi, this]()
    {
        if(!overQuotaDialog || overQuotaDialog->result() == QDialog::Rejected)
        {
            QPointer<BindFolderDialog> addSyncDialog = new BindFolderDialog(MegaSyncApp);

            if (handle != mega::INVALID_HANDLE)
            {
                addSyncDialog->setMegaFolder(handle, disableUi);
            }

            DialogOpener::showDialog(addSyncDialog, this, &AddSyncFromUiManager::onAddSyncDialogFinished);
        }
    };

    if(overQuotaDialog)
    {
        DialogOpener::showDialog(overQuotaDialog,addSyncLambda);
    }
    else
    {
        addSyncLambda();
    }
}

void AddSyncFromUiManager::onAddSyncDialogFinished(QPointer<BindFolderDialog> dialog)
{
    if (dialog->result() != QDialog::Accepted)
    {
        deleteLater();
        return;
    }

    QString localFolderPath = QDir::toNativeSeparators(QDir(dialog->getLocalFolder()).canonicalPath());
    mega::MegaHandle handle = dialog->getMegaFolder();
    QString syncName = dialog->getSyncName();

    mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO, QString::fromLatin1("Adding sync %1 from addSync: ").arg(localFolderPath).toUtf8().constData());

    auto syncController = new SyncController(this);
    SyncController::connect(syncController, &SyncController::syncAddStatus, this, [this, handle, syncController](const int errorCode, const int,
                                                                                       const QString name)
        {
            if (errorCode == mega::MegaError::API_OK)
            {
                emit syncAdded(handle, name);
            }

            syncController->deleteLater();
            deleteLater();
        });

    emit syncAddingStarted();
    syncController->addSync(localFolderPath, handle, syncName, mega::MegaSync::TYPE_TWOWAY);
}


void AddSyncFromUiManager::performRemoveSync(mega::MegaHandle remoteHandle, QWidget* parent)
{
    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(remoteHandle));
    if(node)
    {
        std::unique_ptr<mega::MegaSync> sync(MegaSyncApp->getMegaApi()->getSyncByNode(node.get()));
        auto syncSettings(SyncInfo::instance()->getSyncSettingByTag(sync->getBackupId()));
        if(syncSettings)
        {
            QPointer<RemoveSyncConfirmationDialog> dialog = new RemoveSyncConfirmationDialog(parent);

            DialogOpener::showDialog<RemoveSyncConfirmationDialog>(dialog,
                [dialog, syncSettings, remoteHandle, this]()
                {
                    if(dialog->result() == QDialog::Accepted)
                    {
                        auto syncController = new SyncController(this);
                        syncController->removeSync(syncSettings, remoteHandle);

                        SyncController::connect(syncController,
                            &SyncController::syncRemoveStatus,
                            this,
                            [this, syncController](const int)
                            {
                                syncController->deleteLater();
                                deleteLater();
                            });
                    }
                    else
                    {
                        deleteLater();
                        return;
                    }
                });

            //Dialog correctly shown
            return;
        }
    }

    QMegaMessageBox::MessageBoxInfo msgInfo;
    msgInfo.title = QMegaMessageBox::errorTitle();
    msgInfo.text = QCoreApplication::translate("MegaSyncError", "Sync removal failed. Sync not found");
    msgInfo.buttons = QMessageBox::Ok;
    QMegaMessageBox::information(msgInfo);

    deleteLater();
}
