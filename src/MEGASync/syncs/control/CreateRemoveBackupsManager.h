#ifndef CREATEREMOVEBACKUPSMANAGER_H
#define CREATEREMOVEBACKUPSMANAGER_H

#include "RemoveBackup.h"
#include "SyncInfo.h"

#include <memory>

class SyncSettings;

class CreateRemoveBackupsManager
{
public:
    CreateRemoveBackupsManager() = delete;
    ~CreateRemoveBackupsManager() = delete;

    static void addBackup(SyncInfo::SyncOrigin origin,
                          const QStringList& localFolders = QStringList());
    static void removeBackup(std::shared_ptr<SyncSettings> backup, QWidget* parent);
    static bool isBackupsDialogOpen();

private:
    static void showBackupDialog(SyncInfo::SyncOrigin origin, const QStringList& localFolders);

    static void showBackupDialog(bool comesFromSettings, const QStringList& localFolders);

    static inline RemoveBackup* mRemoveBackupHandler = nullptr;
};

#endif // CREATEREMOVEBACKUPSMANAGER_H
