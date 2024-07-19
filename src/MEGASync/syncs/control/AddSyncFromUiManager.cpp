#include "AddSyncFromUiManager.h"

#include <GuiUtilities.h>
#include <DialogOpener.h>
#include <syncs/gui/Twoways/BindFolderDialog.h>
#include <syncs/control/SyncSettings.h>
#include <syncs/gui/Twoways/RemoveSyncConfirmationDialog.h>
#include <QmlDialogWrapper.h>
#include <syncs/SyncsComponent.h>

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
    QString remoteFolder;

    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(handle));
    if(node)
    {
        remoteFolder = QString::fromUtf8(MegaSyncApp->getMegaApi()->getNodePath(node.get()));
    }

    auto overQuotaDialog = MegaSyncApp->showSyncOverquotaDialog();
    auto addSyncLambda = [overQuotaDialog, handle, disableUi, remoteFolder, this]()
    {
        if(!overQuotaDialog || overQuotaDialog->result() == QDialog::Rejected)
        {
            QPointer<QmlDialogWrapper<SyncsComponent>> syncsDialog;
            if(auto dialog = DialogOpener::findDialog<QmlDialogWrapper<SyncsComponent>>())
            {
                syncsDialog = dialog->getDialog();
            }
            else
            {
                syncsDialog = new QmlDialogWrapper<SyncsComponent>();
            }
            syncsDialog->wrapper()->setComesFromSettings(false);
            syncsDialog->wrapper()->setRemoteFolder(remoteFolder);
            syncsDialog->wrapper()->setRemoteFolderDisabled(disableUi);
            DialogOpener::showDialog(syncsDialog);
        }
    };

    if(overQuotaDialog)
    {
        DialogOpener::showDialog(overQuotaDialog, addSyncLambda);
    }
    else
    {
        addSyncLambda();
    }
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
                                emit syncRemoved();
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
