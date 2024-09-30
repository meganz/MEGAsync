#include "CreateRemoveSyncsManager.h"

#include "DialogOpener.h"
#include "QmlDialogWrapper.h"
#include "RemoveSyncConfirmationDialog.h"
#include "SyncController.h"
#include "SyncsComponent.h"
#include "SyncSettings.h"

const CreateRemoveSyncsManager* CreateRemoveSyncsManager::addSync(mega::MegaHandle handle,
                                                                        bool comesFromSettings)
{
    auto syncManager(new CreateRemoveSyncsManager());
    syncManager->performAddSync(handle, comesFromSettings);
    return syncManager;
}

const CreateRemoveSyncsManager *CreateRemoveSyncsManager::removeSync(mega::MegaHandle handle,
                                                                           QWidget* parent)
{
    auto syncManager(new CreateRemoveSyncsManager());
    syncManager->performRemoveSync(handle, parent);
    return syncManager;
}

void CreateRemoveSyncsManager::performAddSync(mega::MegaHandle handle, bool comesFromSettings)
{
    QString remoteFolder;

    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(handle));
    if (node)
    {
        remoteFolder = QString::fromUtf8(MegaSyncApp->getMegaApi()->getNodePath(node.get()));
    }

    auto overQuotaDialog = MegaSyncApp->showSyncOverquotaDialog();
    auto addSyncLambda = [overQuotaDialog, comesFromSettings, remoteFolder, this]() {
        if (!overQuotaDialog || overQuotaDialog->result() == QDialog::Rejected)
        {
            QPointer<QmlDialogWrapper<SyncsComponent>> syncsDialog;
            if (auto dialog = DialogOpener::findDialog<QmlDialogWrapper<SyncsComponent>>())
            {
                syncsDialog = dialog->getDialog();
            }
            else
            {
                syncsDialog = new QmlDialogWrapper<SyncsComponent>();
            }
            syncsDialog->wrapper()->setComesFromSettings(comesFromSettings);
            syncsDialog->wrapper()->setRemoteFolder(remoteFolder);
            DialogOpener::showDialog(syncsDialog, [this]() {
                deleteLater();
            });
        }
        else
        {
            deleteLater();
        }
    };

    if (overQuotaDialog)
    {
        DialogOpener::showDialog(overQuotaDialog, addSyncLambda);
    }
    else
    {
        addSyncLambda();
    }
}

void CreateRemoveSyncsManager::performRemoveSync(mega::MegaHandle remoteHandle, QWidget* parent)
{
    bool invalidSync(true);

    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(remoteHandle));
    if (node)
    {
        std::unique_ptr<mega::MegaSync> sync(MegaSyncApp->getMegaApi()->getSyncByNode(node.get()));
        auto syncSettings(SyncInfo::instance()->getSyncSettingByTag(sync->getBackupId()));
        if (syncSettings)
        {
            QPointer<RemoveSyncConfirmationDialog> dialog =
                new RemoveSyncConfirmationDialog(parent);

            DialogOpener::showDialog<RemoveSyncConfirmationDialog>(
                dialog,
                [dialog, syncSettings, remoteHandle, this]() {
                    if (dialog->result() == QDialog::Accepted)
                    {
                        SyncController::instance().removeSync(syncSettings, remoteHandle);
                        connect(&SyncController::instance(),
                                &SyncController::syncRemoveStatus,
                                this,
                                [this](const int) {
                                    deleteLater();
                                });
                    }
                    else
                    {
                        deleteLater();
                    }
                });

            // Node and sync settings found, removing has started asynchronously
            invalidSync = false;
        }
    }

    if (invalidSync)
    {
        deleteLater();
    }
}
