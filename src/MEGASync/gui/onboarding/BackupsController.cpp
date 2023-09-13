#include "BackupsController.h"

#include "MegaApplication.h"
#include "TextDecorator.h"
#include "QMegaMessageBox.h"
#include "Utilities.h"

BackupsController::BackupsController(QObject *parent)
    : QObject(parent)
    , mBackupController(new SyncController())
{
    connect(mBackupController, &SyncController::syncAddStatus,
            this, &BackupsController::onBackupAddRequestStatus);
}

QSet<QString> BackupsController::getRemoteFolders() const
{
    return SyncInfo::getRemoteBackupFolderNames();
}

void BackupsController::addBackups(const BackupInfoList& backupsInfoList)
{
    if(backupsInfoList.size() <= 0)
    {
        emit backupsCreationFinished(true);
        return;
    }

    mBackupsProcessedWithError = 0;
    mBackupsToDoSize = backupsInfoList.size();
    mBackupsToDoList = backupsInfoList;
    mBackupController->addBackup(mBackupsToDoList.first().first,
                                 mBackupsToDoList.first().second);
}

bool BackupsController::existsName(const QString& name) const
{
    bool found = false;
    auto it = mBackupsToDoList.constBegin();
    while(!found && it != mBackupsToDoList.constEnd())
    {
        found = (it->first == name);
        it++;
    }
    return found;
}

void BackupsController::onBackupAddRequestStatus(int errorCode,
                                                 const QString& errorMsg,
                                                 const QString& name)
{
    Q_UNUSED(errorMsg)

    if(!existsName(name))
    {
        return;
    }

    if(errorCode == mega::MegaError::API_OK)
    {
        emit backupFinished(mBackupsToDoList.first().first, true);
    }
    else
    {
        QString message = MegaSyncApp->getMegaApi()->isBusinessAccount()
                            && !MegaSyncApp->getMegaApi()->isBusinessAccountActive()
                          ? QCoreApplication::translate("MegaSyncError",
                                mega::MegaSync::getMegaSyncErrorCode(mega::MegaSync::ACCOUNT_EXPIRED))
                          : errorMsg;
        emit backupFinished(mBackupsToDoList.first().first, false, message);
        mBackupsProcessedWithError++;
    }

    mBackupsToDoList.removeFirst();
    if(mBackupsToDoList.size() > 0)
    {
        mBackupController->addBackup(mBackupsToDoList.first().first,
                                     mBackupsToDoList.first().second);
    }
    else if(mBackupsToDoList.size() == 0)
    {
        emit backupsCreationFinished(mBackupsProcessedWithError == 0);
    }
}
