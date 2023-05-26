#include "BackupsController.h"

BackupsController::BackupsController(QObject *parent)
    : QObject(parent)
    , mBackupController(new SyncController())
{
    mRemoteBackups = SyncInfo::getRemoteBackupFolderNames();

    connect(mBackupController, &SyncController::syncAddStatus, this, &BackupsController::onBackupAddRequestStatus);
}

QSet<QString> BackupsController::getRemoteFolders() const
{
    return mRemoteBackups;
}

void BackupsController::addBackups(const BackupInfoList& backupsInfoList)
{
    if(backupsInfoList.size() <= 0)
    {
        return;
    }

    mBackupsProcessed = 0;
    mBackupsToDoSize = backupsInfoList.size();
    mBackupsToDoList = backupsInfoList;
    mBackupController->addBackup(mBackupsToDoList.first().first, mBackupsToDoList.first().second);
}

void BackupsController::onBackupAddRequestStatus(int errorCode,
                                          const QString& errorMsg,
                                          const QString& name)
{
    bool found = false;
    auto it = mBackupsToDoList.constBegin();
    QString backupName(QString::fromUtf8(""));
    while(!found && it != mBackupsToDoList.constEnd())
    {
        if((found = (it->first == name)))
        {
            backupName = it->second;
        }
        it++;
    }

    if(!found)
    {
        return;
    }

    mBackupsProcessed++;

    if(errorCode == 0)
    {
        mBackupsToDoList.removeFirst();
        if(mBackupsToDoList.size() > 0)
        {
            mBackupController->addBackup(mBackupsToDoList.first().first, mBackupsToDoList.first().second);
        }
    }

    if(mBackupsToDoList.size() == 0 || mBackupsProcessed == mBackupsToDoSize) {
        emit backupsCreationFinished();
    }
}
