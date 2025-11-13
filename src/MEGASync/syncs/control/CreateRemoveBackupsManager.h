#ifndef CREATEREMOVEBACKUPSMANAGER_H
#define CREATEREMOVEBACKUPSMANAGER_H

#include "RemoveBackup.h"

#include <QObject>
#include <QPointer>
#include <QWidget>

#include <memory>

class SyncSettings;

class CreateRemoveBackupsManager
{
public:
    CreateRemoveBackupsManager() = delete;
    ~CreateRemoveBackupsManager() = delete;

    static void addBackup(bool comesFromSettings, const QStringList& localFolders = QStringList());
    static void removeBackup(std::shared_ptr<SyncSettings> backup, QWidget* parent);
    static bool isBackupsDialogOpen();

private:
    static void showBackupDialog(bool comesFromSettings, const QStringList& localFolders);

    static RemoveBackup mRemoveBackupHandler;
};

#endif // CREATEREMOVEBACKUPSMANAGER_H
