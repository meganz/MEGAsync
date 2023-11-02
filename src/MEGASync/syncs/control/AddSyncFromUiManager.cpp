#include "AddSyncFromUiManager.h"

#include <GuiUtilities.h>
#include <DialogOpener.h>
#include <syncs/gui/Twoways/BindFolderDialog.h>
#include <syncs/control/SyncSettings.h>

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

void AddSyncFromUiManager::removeSync(mega::MegaHandle remoteHandle)
{
    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(remoteHandle));
    if(node)
    {
        std::unique_ptr<mega::MegaSync> sync(MegaSyncApp->getMegaApi()->getSyncByNode(node.get()));
        auto syncSettings(SyncInfo::instance()->getSyncSettingByTag(sync->getBackupId()));
        if(syncSettings)
        {
            QMegaMessageBox::MessageBoxInfo msgInfo;
            msgInfo.title = QMegaMessageBox::warningTitle();
            msgInfo.text = tr("Are you sure you want to remove %1 sync?").arg(syncSettings->name());
            msgInfo.buttons = QMessageBox::Ok | QMessageBox::Cancel;
            QMap<QMegaMessageBox::StandardButton, QString> buttonsText;
            buttonsText.insert(QMessageBox::Ok, tr("Sync"));
            msgInfo.buttonsText = buttonsText;
            msgInfo.finishFunc = [this, syncSettings, remoteHandle](QPointer<QMessageBox> msg)
            {
                if(msg->result() == QMessageBox::Ok)
                {
                    mSyncController = new SyncController(this);
                    mSyncController->removeSync(syncSettings, remoteHandle);
                }
                deleteLater();
            };

            QMegaMessageBox::information(msgInfo);
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

    mSyncController = new SyncController(this);
    GuiUtilities::connectAddSyncDefaultHandler(mSyncController, Preferences::instance()->accountType());
    SyncController::connect(mSyncController, &SyncController::syncAddStatus, this, [this, handle](const int errorCode, const int,
                                                                                                       const QString, QString localPath)
                            {
                                if (errorCode == mega::MegaError::API_OK)
                                {
                                    emit syncAdded(handle, localPath);
                                }

                                deleteLater();
                            });

    mSyncController->addSync(localFolderPath, handle, syncName, mega::MegaSync::TYPE_TWOWAY);
}
