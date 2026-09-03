#include "BackupsController.h"

#include "MegaApplication.h"
#include "Utilities.h"

#include <memory>

BackupsController::BackupsController(QObject* parent):
    SyncController(parent)
{
    connect(this, &SyncController::syncAddStatus,
            this, &BackupsController::onBackupAddRequestStatus);
}

QSet<QString> BackupsController::getRemoteFolders() const
{
    return SyncInfo::getRemoteBackupFolderNames();
}

void BackupsController::addBackups(const BackupInfoList& backupsInfoList)
{
    if(backupsInfoList.empty())
    {
        emit backupsCreationFinished(true);
        return;
    }
    mBackupsProcessedWithError = 0;
    mBackupsToDoSize = static_cast<int>(backupsInfoList.size());
    mBackupsToDoList = backupsInfoList;
    const auto&[fullPath, backupName] = mBackupsToDoList.first();
    addBackup(fullPath, backupName);
}

bool BackupsController::existsName(const QString& name) const
{
    auto foundIt = std::find_if(mBackupsToDoList.constBegin(),
                                mBackupsToDoList.constEnd(),
                                [&name](const auto& backupToDo)
                                {
                                    return (backupToDo.first == name);
                                });

    return foundIt != mBackupsToDoList.constEnd();
}

void BackupsController::onBackupAddRequestStatus(int errorCode, int syncErrorCode, QString name)
{
    if(!existsName(name))
    {
        return;
    }

    emit backupFinished(mBackupsToDoList.first().first, errorCode, syncErrorCode);
    if(errorCode != mega::MegaError::API_OK)
    {
        mBackupsProcessedWithError++;
    }

    mBackupsToDoList.removeFirst();
    if(mBackupsToDoList.size() > 0)
    {
        const auto&[fullPath, backupName] = mBackupsToDoList.first();
        addBackup(fullPath, backupName);
    }
    else if(mBackupsToDoList.size() == 0)
    {
        emit backupsCreationFinished(mBackupsProcessedWithError == 0);
    }
}

QString BackupsController::getErrorString(int errorCode, int syncErrorCode) const
{
    QString errorMsg;
    if (errorCode != mega::MegaError::API_OK && MegaSyncApp->getMegaApi()->isBusinessAccount() &&
        !MegaSyncApp->getMegaApi()->isBusinessAccountActive())
    {
        syncErrorCode = mega::MegaSync::ACCOUNT_EXPIRED;
    }

    if (syncErrorCode != mega::MegaSync::NO_SYNC_ERROR)
    {
        std::unique_ptr<const char[]> syncErrorText(
            mega::MegaSync::getMegaSyncErrorCode(syncErrorCode));
        errorMsg = QCoreApplication::translate("MegaSyncError", syncErrorText.get());
    }
    else if (errorCode != mega::MegaError::API_OK)
    {
        errorMsg = getSyncAPIErrorMsg(errorCode);
        if (errorMsg.isEmpty())
        {
            errorMsg = QCoreApplication::translate("MegaError",
                                                   mega::MegaError::getErrorString(errorCode));
        }
    }

    return errorMsg;
}

QString BackupsController::getSyncAPIErrorMsg(int megaError) const
{
    switch (megaError)
    {
        case mega::MegaError::API_EARGS:
            return QCoreApplication::translate(
                "SyncController",
                "Unable to create backup as selected folder is not valid. Try again.");
            break;
        case mega::MegaError::API_EACCESS:
            return QCoreApplication::translate(
                "SyncController",
                "Unable to create backup. Try again and if issue continues, contact "
                "[A]Support[/A].");
            break;
        case mega::MegaError::API_EINCOMPLETE:
            return QCoreApplication::translate(
                "SyncController",
                "Unable to create backup as the device you're backing up from doesn't "
                "have a name. "
                "Give your device a name and then try again. If issue continues, contact "
                "[A]Support[/A].");
        case mega::MegaError::API_EINTERNAL:
        // Fallthrough
        case mega::MegaError::API_ENOENT:
        // Fallthrough
        case mega::MegaError::API_EEXIST:
            return QCoreApplication::translate(
                "SyncController",
                "Unable to create backup. For further information, contact [A]Support[/A].");
        default:
            break;
    }
    return {};
}
