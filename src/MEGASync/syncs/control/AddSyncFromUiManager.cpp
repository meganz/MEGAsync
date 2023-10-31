#include "AddSyncFromUiManager.h"

#include <GuiUtilities.h>
#include <DialogOpener.h>
#include <syncs/gui/Twoways/BindFolderDialog.h>

void AddSyncFromUiManager::addSync(mega::MegaHandle handle, bool disableUi)
{
    auto overQuotaDialog = MegaSyncApp->showSyncOverquotaDialog();
    auto addSyncLambda = [overQuotaDialog, handle, disableUi, this]()
    {
        if(!overQuotaDialog || overQuotaDialog->result() == QDialog::Rejected)
        {
            mAddSyncDialog = new BindFolderDialog(MegaSyncApp);

            if (handle != mega::INVALID_HANDLE)
            {
                mAddSyncDialog->setMegaFolder(handle, disableUi);
            }

            DialogOpener::showDialog(mAddSyncDialog, this, &AddSyncFromUiManager::onAddSyncDialogFinished);
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
        return;
    }

    QString localFolderPath = QDir::toNativeSeparators(QDir(dialog->getLocalFolder()).canonicalPath());
    mega::MegaHandle handle = dialog->getMegaFolder();
    QString syncName = dialog->getSyncName();

    mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO, QString::fromLatin1("Adding sync %1 from addSync: ").arg(localFolderPath).toUtf8().constData());

    std::unique_ptr<SyncController> syncController(new SyncController());
    GuiUtilities::connectAddSyncDefaultHandler(syncController.get(), Preferences::instance()->accountType());
    SyncController::connect(syncController.get(), &SyncController::syncAddStatus, this, [this, handle](const int errorCode, const int,
                                                                                                       const QString, QString localPath)
                            {
                                if (errorCode == mega::MegaError::API_OK)
                                {
                                    emit syncAdded(handle, localPath);
                                }

                                deleteLater();
                            });

    syncController->addSync(localFolderPath, handle, syncName, mega::MegaSync::TYPE_TWOWAY);
}
