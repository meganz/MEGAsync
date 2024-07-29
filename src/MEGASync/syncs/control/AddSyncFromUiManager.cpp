#include "AddSyncFromUiManager.h"

#include <GuiUtilities.h>
#include <DialogOpener.h>
#include <syncs/control/SyncSettings.h>
#include <syncs/gui/Twoways/RemoveSyncConfirmationDialog.h>
#include <QmlDialogWrapper.h>
#include <syncs/SyncsComponent.h>

const AddSyncFromUiManager* const AddSyncFromUiManager::addSync_static(mega::MegaHandle handle, bool disableUi, bool comesFromSettings)
{
    auto syncManager(new AddSyncFromUiManager());
    syncManager->addSync(handle, disableUi, comesFromSettings);
    return syncManager;
}

const AddSyncFromUiManager* const AddSyncFromUiManager::removeSync_static(mega::MegaHandle handle, QWidget* parent)
{
    auto syncManager(new AddSyncFromUiManager());
    syncManager->removeSync(handle, parent);
    return syncManager;
}

void AddSyncFromUiManager::addSync(mega::MegaHandle handle, bool disableUi, bool comesFromSettings)
{
    performAddSync(handle, disableUi, comesFromSettings);
}

void AddSyncFromUiManager::removeSync(mega::MegaHandle handle, QWidget* parent)
{
    performRemoveSync(handle, parent);
}

void AddSyncFromUiManager::performAddSync(mega::MegaHandle handle, bool disableUi, bool comesFromSettings)
{
    QString remoteFolder;

    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(handle));
    if(node)
    {
        remoteFolder = QString::fromUtf8(MegaSyncApp->getMegaApi()->getNodePath(node.get()));
    }

    auto overQuotaDialog = MegaSyncApp->showSyncOverquotaDialog();
    auto addSyncLambda = [overQuotaDialog, handle, disableUi, comesFromSettings, remoteFolder, this]()
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
            syncsDialog->wrapper()->setComesFromSettings(comesFromSettings);
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
    bool invalidSync(true);

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
                                emit syncRemoved();
                                deleteLater();
                            });
                    }
                    else
                    {
                        deleteLater();
                    }
                });

            //Node and sync settings found, removing has started asynchronously
            invalidSync = false;
        }
    }

    if(invalidSync)
    {
        //Even if the node is invalid or the sync has not been found, that means that the sync is no longer valid, so we
        //remove it from the UI without warning the user...
        emit syncRemoved();
        deleteLater();
    }
}
