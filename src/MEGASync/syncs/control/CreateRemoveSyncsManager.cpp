#include "CreateRemoveSyncsManager.h"

#include "DialogOpener.h"
#include "QmlDialogWrapper.h"
#include "RemoveSyncConfirmationDialog.h"
#include "SyncController.h"
#include "SyncsComponent.h"
#include "SyncSettings.h"

const CreateRemoveSyncsManager* CreateRemoveSyncsManager::addSync(SyncInfo::SyncOrigin origin,
                                                                  mega::MegaHandle handle)
{
    auto syncManager(new CreateRemoveSyncsManager());
    syncManager->performAddSync(origin, handle);
    return syncManager;
}

bool CreateRemoveSyncsManager::removeSync(mega::MegaHandle handle, QWidget* parent)
{
    auto syncManager(new CreateRemoveSyncsManager());
    return syncManager->performRemoveSync(handle, parent);
}

bool CreateRemoveSyncsManager::removeSync(std::shared_ptr<SyncSettings> syncSettings,
                                          QWidget* parent)
{
    auto syncManager(new CreateRemoveSyncsManager());
    return syncManager->performRemoveSync(syncSettings, parent);
}

void CreateRemoveSyncsManager::performAddSync(SyncInfo::SyncOrigin origin, mega::MegaHandle handle)
{
    QString remoteFolder;

    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(handle));
    if (node)
    {
        remoteFolder = QString::fromUtf8(MegaSyncApp->getMegaApi()->getNodePath(node.get()));
    }

    auto overQuotaDialog = MegaSyncApp->showSyncOverquotaDialog();
    auto addSyncLambda = [overQuotaDialog, origin, remoteFolder, this]()
    {
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
            syncsDialog->wrapper()->setSyncOrigin(origin);
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

bool CreateRemoveSyncsManager::performRemoveSync(mega::MegaHandle remoteHandle, QWidget* parent)
{
    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(remoteHandle));
    if (!node)
    {
        deleteLater();
        return false;
    }
    std::unique_ptr<mega::MegaSync> sync(MegaSyncApp->getMegaApi()->getSyncByNode(node.get()));
    if (!sync)
    {
        deleteLater();
        return false;
    }
    auto syncSettings(SyncInfo::instance()->getSyncSettingByTag(sync->getBackupId()));
    if (!syncSettings)
    {
        deleteLater();
        return false;
    }

    return performRemoveSync(syncSettings, parent);
}

bool CreateRemoveSyncsManager::performRemoveSync(std::shared_ptr<SyncSettings> syncSettings,
                                                 QWidget* parent)
{
    if (syncSettings)
    {
        QPointer<RemoveSyncConfirmationDialog> dialog = new RemoveSyncConfirmationDialog(parent);

        DialogOpener::showDialog<RemoveSyncConfirmationDialog>(
            dialog,
            [dialog, syncSettings, this]()
            {
                if (dialog->result() == QDialog::Accepted)
                {
                    SyncController::instance().removeSync(syncSettings);
                    connect(&SyncController::instance(),
                            &SyncController::syncRemoveStatus,
                            this,
                            [this](const int)
                            {
                                deleteLater();
                            });
                }
                else
                {
                    deleteLater();
                }
            });

        return true;
    }
    else
    {
        deleteLater();
        return false;
    }
}
