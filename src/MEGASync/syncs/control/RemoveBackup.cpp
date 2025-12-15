#include "RemoveBackup.h"

#include "BackupsController.h"
#include "DialogOpener.h"
#include "MessageDialogOpener.h"
#include "RemoveBackupDialog.h"
#include "StatsEventHandler.h"
#include "SyncSettings.h"

RemoveBackup::RemoveBackup(QObject* parent):
    QObject(parent)
{
    connect(&BackupsController::instance(),
            &BackupsController::backupMoveOrRemoveRemoteFolderError,
            this,
            &RemoveBackup::backupMoveOrRemoveRemoteFolderError);
}

void RemoveBackup::removeBackup(std::shared_ptr<SyncSettings> backup, QWidget* parent)
{
    mBackupToRemove = backup;
    mParent = parent;
    mRemoveBackupDialog = new RemoveBackupDialog(mParent);

    DialogOpener::showDialog(mRemoveBackupDialog,
                             [this]()
                             {
                                 mRemoveBackupDialog->deleteLater();
                             });

    connect(mRemoveBackupDialog,
            &RemoveBackupDialog::removeBackup,
            this,
            &RemoveBackup::onConfirmRemove);
}

void RemoveBackup::onConfirmRemove(mega::MegaHandle targetFolder)
{
    if (targetFolder != mega::INVALID_HANDLE && !checkTargetFolderExist(targetFolder))
    {
        auto error = tr("Destination folder doesn't exists. Choose another.");

        mRemoveBackupDialog->setTargetFolderErrorHint(error);
    }
    else if (targetFolder != mega::INVALID_HANDLE &&
             checkBackupFolderExistOnTargetFolder(targetFolder))
    {
        auto error = tr("Backup folder already exists on destination. Choose another.");

        mRemoveBackupDialog->setTargetFolderErrorHint(error);
    }
    else
    {
        mFolderToMoveBackupData = targetFolder;

        MegaSyncApp->getStatsEventHandler()->sendTrackedEvent(
            AppStatsEvents::EventType::CONFIRM_REMOVE_BACKUP);

        BackupsController::instance().removeSync(mBackupToRemove, mFolderToMoveBackupData);

        mRemoveBackupDialog->close();
    }
}

void RemoveBackup::backupMoveOrRemoveRemoteFolderError(std::shared_ptr<mega::MegaError> error)
{
    if (error->getErrorCode() != mega::MegaError::API_OK)
    {
        MessageDialogInfo msgInfo;
        msgInfo.titleText = tr("Error moving or removing remote backup folder");
        msgInfo.descriptionText =
            tr("Failed to move or remove the remote backup folder. Reason: %1")
                .arg(QCoreApplication::translate("MegaError", error->getErrorString()));
        MessageDialogOpener::warning(msgInfo);
    }
}

bool RemoveBackup::checkBackupFolderExistOnTargetFolder(mega::MegaHandle targetFolder)
{
    QString backupName = mBackupToRemove->name();
    auto targetNode =
        std::unique_ptr<mega::MegaNode>(MegaSyncApp->getMegaApi()->getNodeByHandle(targetFolder));

    auto targetFolderName =
        QString::fromUtf8(MegaSyncApp->getMegaApi()->getNodePath(targetNode.get()));

    std::unique_ptr<mega::MegaNodeList> folderTargetChildNodes(
        MegaSyncApp->getMegaApi()->getChildren(targetNode.get()));

    bool found = false;
    for (int index = 0; index < folderTargetChildNodes->size() && !found; ++index)
    {
        auto childNode = folderTargetChildNodes->get(index);
        QString childNodeName = QString::fromUtf8(
            MegaSyncApp->getMegaApi()->unescapeFsIncompatible(childNode->getName(), nullptr));

        if (!SyncController::instance().isSyncCaseSensitive(mBackupToRemove->getMegaHandle()))
        {
            childNodeName = childNodeName.toLower();
            backupName = backupName.toLower();
        }

        found = (backupName == childNodeName);
    }

    return found;
}

bool RemoveBackup::checkTargetFolderExist(mega::MegaHandle targetFolder)
{
    auto targetNode =
        std::unique_ptr<mega::MegaNode>(MegaSyncApp->getMegaApi()->getNodeByHandle(targetFolder));

    return targetNode && !targetNode->isRemoved() &&
           !MegaSyncApp->getMegaApi()->isInRubbish(targetNode.get());
}
