#include "CreateRemoveSyncsManager.h"

#include "DialogOpener.h"
#include "QmlDialogWrapper.h"
#include "QmlDialog.h"
#include "RemoveSyncConfirmationDialog.h"
#include "SyncController.h"
#include "SyncsComponent.h"
#include "SyncsQmlDialog.h"
#include "SyncSettings.h"

#include <QPointer>
#include <QTimer>

namespace
{
void surfaceSyncsDialog(QPointer<QmlDialogWrapper<SyncsComponent>> dialog)
{
    if (!dialog)
    {
        return;
    }

    dialog->show();
    dialog->activateWindow();
    dialog->raise();

    if (auto syncsWindow = dynamic_cast<SyncsQmlDialog*>(dialog->windowHandle()))
    {
        if (syncsWindow->width() <= 1 || syncsWindow->height() <= 1)
        {
            syncsWindow->resize(syncsWindow->minimumWidth(), syncsWindow->minimumHeight());
        }

        syncsWindow->showNormal();
        syncsWindow->raise();
    }
}

void queueSyncsDialogSurface(QPointer<QmlDialogWrapper<SyncsComponent>> dialog)
{
    QTimer::singleShot(
        0,
        [dialog]()
        {
            surfaceSyncsDialog(dialog);
        });
}
}

void CreateRemoveSyncsManager::addSync(SyncInfo::SyncOrigin origin,
                                       mega::MegaHandle handle,
                                       const QString& localPath)
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
                                 [overQuotaDialog, origin, remoteFolder, localPath]()
                                 {
                                     if (overQuotaDialog->result() == QDialog::Rejected)
                                     {
                                         showSyncDialog(origin, remoteFolder, localPath);
                                     }
                                 });
    }
    else
    {
        showSyncDialog(origin, remoteFolder, localPath);
    }
}

bool CreateRemoveSyncsManager::removeSync(mega::MegaHandle handle, QWidget* parent)
{
    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(handle));
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

    return removeSync(syncSettings, parent);
}

bool CreateRemoveSyncsManager::removeSync(std::shared_ptr<SyncSettings> syncSettings,
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

void CreateRemoveSyncsManager::showSyncDialog(SyncInfo::SyncOrigin origin,
                                              QString remoteFolder,
                                              QString localFolder)
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

    if (!localFolder.isEmpty())
    {
        syncsDialog->wrapper()->setLocalFolder(localFolder);
    }

    if (origin == SyncInfo::SyncOrigin::SETTINGS_ORIGIN)
    {
        DialogOpener::showDialogWithoutWindowModality(syncsDialog);
    }
    else
    {
        DialogOpener::showDialog(syncsDialog);
    }
    queueSyncsDialogSurface(syncsDialog);
}
