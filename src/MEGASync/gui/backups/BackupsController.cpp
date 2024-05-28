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
    if(backupsInfoList.empty())
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
        mBackupController->addBackup(mBackupsToDoList.first().first,
                                     mBackupsToDoList.first().second);
    }
    else if(mBackupsToDoList.size() == 0)
    {
        emit backupsCreationFinished(mBackupsProcessedWithError == 0);
    }
}

QString BackupsController::getErrorString(int errorCode, int syncErrorCode)
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
