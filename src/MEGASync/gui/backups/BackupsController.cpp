#include "BackupsController.h"

#include "MegaApplication.h"
#include "Utilities.h"

BackupsController::BackupsController(QObject* parent):
    SyncController(parent),
    mBackupsOrigin(SyncInfo::SyncOrigin::NONE)
{
    connect(this, &SyncController::syncAddStatus,
            this, &BackupsController::onBackupAddRequestStatus);
}

QSet<QString> BackupsController::getRemoteFolders() const
{
    return SyncInfo::getRemoteBackupFolderNames();
}

void BackupsController::addBackups(const BackupInfoList& backupsInfoList,
                                   SyncInfo::SyncOrigin origin)
{
    if(backupsInfoList.empty())
    {
        emit backupsCreationFinished(true);
        return;
    }
    mBackupsOrigin = origin;
    mBackupsProcessedWithError = 0;
    mBackupsToDoSize = backupsInfoList.size();
    mBackupsToDoList = backupsInfoList;
    const auto&[fullPath, backupName] = mBackupsToDoList.first();
    addBackup(fullPath, backupName, mBackupsOrigin);
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

bool BackupsController::hasBackupsWithErrors() const
{
    return mBackupsProcessedWithError > 0 && mBackupsProcessedWithError != mBackupsToDoSize;
}

void BackupsController::showErrorMessage() const
{
    auto completedItems(mBackupsToDoSize - mBackupsProcessedWithError);
    QString successItems(tr("%n folder was backed up", "", completedItems));
    QString message(tr("%1, but %n folder couldn’t be backed up.", "", mBackupsProcessedWithError)
                        .arg(successItems));
    MegaSyncApp->showErrorMessage(message, tr("Not all folders were backed up"));
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
        addBackup(fullPath, backupName, mBackupsOrigin);
    }
    else if(mBackupsToDoList.size() == 0)
    {
        mBackupsOrigin = SyncInfo::SyncOrigin::NONE;
        emit backupsCreationFinished(mBackupsProcessedWithError == 0);
        if (hasBackupsWithErrors())
        {
            showErrorMessage();
        }
    }
}

QString BackupsController::getErrorString(int errorCode, int syncErrorCode) const
{
    QString errorMsg;
    if(errorCode != mega::MegaError::API_OK)
    {
        if(MegaSyncApp->getMegaApi()->isBusinessAccount()
            && !MegaSyncApp->getMegaApi()->isBusinessAccountActive())
        {
            errorMsg = QCoreApplication::translate("MegaSyncError",
                                                   mega::MegaSync::getMegaSyncErrorCode(mega::MegaSync::ACCOUNT_EXPIRED));
        }
        else
        {
            errorMsg = SyncController::getErrorString(errorCode, syncErrorCode);
        }
    }
    return errorMsg;
}
