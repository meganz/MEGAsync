#include "BackupSettingsQuickWidget.h"

#include "BackupsController.h"
#include "BackupSettingsModel.h"
#include "CreateRemoveBackupsManager.h"
#include "MegaApplication.h"
#include "MyBackupsHandle.h"
#include "QmlManager.h"
#include "StatsEventHandler.h"
#include "Utilities.h"

#include <QFileInfo>

BackupSettingsQuickWidget::BackupSettingsQuickWidget(QWidget* parent):
    SettingsQuickWidgetBase(new BackupSettingsModel(), BackupsController::instance(), parent),
    mMyBackupsHandleRequest(UserAttributes::MyBackupsHandle::requestMyBackupsHandle()),
    mBackupFolderPath(UserAttributes::MyBackupsHandle::getMyBackupsLocalizedPath()),
    mBackupFolderAvailable(false)
{
    qmlRegisterType<BackupSettingsModel>("BackupSettingsModel", 1, 0, "BackupSettingsModel");

    QmlManager::instance()->setRootContextProperty(QStringLiteral("backupSettingsModel"), model());
    QmlManager::instance()->setRootContextProperty(QStringLiteral("backupSettingsAccess"), this);

    connect(mMyBackupsHandleRequest.get(),
            &UserAttributes::MyBackupsHandle::attributeReady,
            this,
            &BackupSettingsQuickWidget::onMyBackupsFolderHandleSet);
    onMyBackupsFolderHandleSet(mMyBackupsHandleRequest->getMyBackupsHandle());

    setSource(QString::fromUtf8("qrc:/settings/BackupSettings.qml"));
}

void BackupSettingsQuickWidget::addItem() const
{
    CreateRemoveBackupsManager::addBackup(SyncInfo::SyncOrigin::SETTINGS_ORIGIN,
                                          QStringList(),
                                          this->window());
}

void BackupSettingsQuickWidget::remove(int index) const
{
    CreateRemoveBackupsManager::removeBackup(model()->getSyncSetting(index), this->window());
}

void BackupSettingsQuickWidget::openBackupFolder() const
{
    if (!mBackupFolderAvailable)
    {
        return;
    }

    Utilities::openInMega(mMyBackupsHandleRequest->getMyBackupsHandle());
    MegaSyncApp->getStatsEventHandler()->sendTrackedEvent(
        AppStatsEvents::EventType::SETTINGS_VIEW_BACKUP_CLICKED);
}

QString BackupSettingsQuickWidget::getBackupFolderPath() const
{
    return mBackupFolderPath;
}

bool BackupSettingsQuickWidget::isBackupFolderAvailable() const
{
    return mBackupFolderAvailable;
}

void BackupSettingsQuickWidget::onMyBackupsFolderHandleSet(mega::MegaHandle handle)
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
