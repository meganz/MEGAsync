#include "RemoveBackup.h"

#include "BackupsController.h"
#include "ChooseMoveBackupFolderErrorDialog.h"
#include "DialogOpener.h"
#include "MessageDialogOpener.h"
#include "RemoveBackupDialog.h"
#include "StatsEventHandler.h"
#include "SyncSettings.h"

void RemoveBackup::removeBackup(std::shared_ptr<SyncSettings> backup, QWidget* parent)
{
    mBackupToRemove = backup;
    mParent = parent;

    QPointer<RemoveBackupDialog> dialog = new RemoveBackupDialog(mParent);

    DialogOpener::showDialog(dialog,
                             [dialog]()
                             {
                                 dialog->deleteLater();
                             });

    connect(dialog, &RemoveBackupDialog::removeBackup, this, &RemoveBackup::onConfirmRemove);
}

void RemoveBackup::onConfirmRemove(mega::MegaHandle targetFolder)
{
    mFolderToMoveBackupData = targetFolder;

    MegaSyncApp->getStatsEventHandler()->sendTrackedEvent(
        AppStatsEvents::EventType::CONFIRM_REMOVE_BACKUP);

    connect(&BackupsController::instance(),
            &BackupsController::backupMoveOrRemoveRemoteFolderError,
            this,
            &RemoveBackup::backupMoveOrRemoveRemoteFolderError);

    BackupsController::instance().removeSync(mBackupToRemove, mFolderToMoveBackupData);
}

void RemoveBackup::backupMoveOrRemoveRemoteFolderError(std::shared_ptr<mega::MegaError> error)
{
    if (error->getErrorCode() != mega::MegaError::API_OK)
    {
        if (error->getErrorCode() == mega::MegaError::API_EEXIST)
        {
            auto title = tr("Error moving remote backup folder");
            auto description =
                tr("Reason: %1")
                    .arg(QCoreApplication::translate("MegaError", error->getErrorString()));

            // show new target folder selection
            QPointer<ChooseMoveBackupFolderErrorDialog> dialog =
                new ChooseMoveBackupFolderErrorDialog(mFolderToMoveBackupData, mParent);

            DialogOpener::showDialog(dialog,
                                     [dialog]()
                                     {
                                         dialog->deleteLater();
                                     });

            connect(dialog,
                    &ChooseMoveBackupFolderErrorDialog::moveBackup,
                    this,
                    &RemoveBackup::onConfirmNewTargetFolder);
        }
        else
        {
            MessageDialogInfo msgInfo;
            msgInfo.titleText = tr("Error moving or removing remote backup folder");
            msgInfo.descriptionText =
                tr("Failed to move or remove the remote backup folder. Reason: %1")
                    .arg(QCoreApplication::translate("MegaError", error->getErrorString()));
            MessageDialogOpener::warning(msgInfo);
        }
    }
}

void RemoveBackup::onConfirmNewTargetFolder(mega::MegaHandle targetFolder)
{
    mFolderToMoveBackupData = targetFolder;

    BackupsController::instance().moveOrDeleteRemovedBackupData(mBackupToRemove,
                                                                mFolderToMoveBackupData);
}
