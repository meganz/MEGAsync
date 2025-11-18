#include "CreateRemoveSyncsManager.h"

#include "DialogOpener.h"
#include "QmlDialogWrapper.h"
#include "RemoveSyncConfirmationDialog.h"
#include "SyncController.h"
#include "SyncsComponent.h"
#include "SyncSettings.h"

void CreateRemoveSyncsManager::addSync(SyncInfo::SyncOrigin origin, mega::MegaHandle handle)
{
    CreateRemoveSyncsManager::performAddSync(origin, handle);
}

bool CreateRemoveSyncsManager::removeSync(mega::MegaHandle handle, QWidget* parent)
{
    return CreateRemoveSyncsManager::performRemoveSync(handle, parent);
}

bool CreateRemoveSyncsManager::removeSync(std::shared_ptr<SyncSettings> syncSettings,
                                          QWidget* parent)
{
    return CreateRemoveSyncsManager::performRemoveSync(syncSettings, parent);
}

void CreateRemoveSyncsManager::performAddSync(SyncInfo::SyncOrigin origin, mega::MegaHandle handle)
{
    QString remoteFolder;

    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(handle));
    if (node)
    {
        remoteFolder = QString::fromUtf8(MegaSyncApp->getMegaApi()->getNodePath(node.get()));
    }

    auto overQuotaDialog = MegaSyncApp->createOverquotaDialogIfNeeded();
    if (overQuotaDialog)
    {
        DialogOpener::showDialog(overQuotaDialog,
                                 [overQuotaDialog, origin, remoteFolder]()
                                 {
                                     if (overQuotaDialog->result() == QDialog::Rejected)
                                     {
                                         showSyncDialog(origin, remoteFolder);
                                     }
                                 });
    }
    else
    {
        showSyncDialog(origin, remoteFolder);
    }
}

bool CreateRemoveSyncsManager::performRemoveSync(mega::MegaHandle remoteHandle, QWidget* parent)
{
    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(remoteHandle));
    if (!node)
    {
        return false;
    }

    std::unique_ptr<mega::MegaSync> sync(MegaSyncApp->getMegaApi()->getSyncByNode(node.get()));
    if (!sync)
    {
        return false;
    }

    auto syncSettings(SyncInfo::instance()->getSyncSettingByTag(sync->getBackupId()));
    if (!syncSettings)
    {
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
            [dialog, syncSettings]()
            {
                if (dialog->result() == QDialog::Accepted)
                {
                    SyncController::instance().removeSync(syncSettings);
                }
            });

        return true;
    }

    return false;
}

void CreateRemoveSyncsManager::showSyncDialog(SyncInfo::SyncOrigin origin, QString remoteFolder)
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

    DialogOpener::showDialog(syncsDialog);
}
