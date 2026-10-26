#include "BackupSettingsActions.h"

#include "BackupsController.h"
#include "BackupSettingsModel.h"
#include "CreateRemoveBackupsManager.h"
#include "MegaApplication.h"
#include "MyBackupsHandle.h"
#include "StatsEventHandler.h"
#include "Utilities.h"

BackupSettingsActions::BackupSettingsActions(QWidget* hostWidget, QObject* parent):
    SyncSettingsActionsBase(new BackupSettingsModel(),
                            BackupsController::instance(),
                            hostWidget,
                            parent),
    mMyBackupsHandleRequest(UserAttributes::MyBackupsHandle::requestMyBackupsHandle()),
    mBackupFolderPath(UserAttributes::MyBackupsHandle::getMyBackupsLocalizedPath()),
    mBackupFolderAvailable(false)
{
    connect(mMyBackupsHandleRequest.get(),
            &UserAttributes::MyBackupsHandle::attributeReady,
            this,
            &BackupSettingsActions::onMyBackupsFolderHandleSet);
    onMyBackupsFolderHandleSet(mMyBackupsHandleRequest->getMyBackupsHandle());
}

void BackupSettingsActions::addItem() const
{
    CreateRemoveBackupsManager::addBackup(SyncInfo::SyncOrigin::SETTINGS_ORIGIN,
                                          QStringList(),
                                          hostWindow());
}

void BackupSettingsActions::remove(int index) const
{
    CreateRemoveBackupsManager::removeBackup(model()->getSyncSetting(index), hostWindow());
}

void BackupSettingsActions::openBackupFolder() const
{
    if (!mBackupFolderAvailable)
    {
        return;
    }

    Utilities::openInMega(mMyBackupsHandleRequest->getMyBackupsHandle());
    MegaSyncApp->getStatsEventHandler()->sendTrackedEvent(
        AppStatsEvents::EventType::SETTINGS_VIEW_BACKUP_CLICKED);
}

QString BackupSettingsActions::getBackupFolderPath() const
{
    return mBackupFolderPath;
}

bool BackupSettingsActions::isBackupFolderAvailable() const
{
    return mBackupFolderAvailable;
}

void BackupSettingsActions::onMyBackupsFolderHandleSet(mega::MegaHandle handle)
{
    const auto newPath = UserAttributes::MyBackupsHandle::getMyBackupsLocalizedPath();
    const auto newAvailable = handle != mega::INVALID_HANDLE;

    if (mBackupFolderPath != newPath)
    {
        mBackupFolderPath = newPath;
        emit backupFolderPathChanged();
    }

    if (mBackupFolderAvailable != newAvailable)
    {
        mBackupFolderAvailable = newAvailable;
        emit backupFolderAvailableChanged();
    }
}
