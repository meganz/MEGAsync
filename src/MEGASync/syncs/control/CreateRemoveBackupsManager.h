#ifndef CREATEREMOVEBACKUPSMANAGER_H
#define CREATEREMOVEBACKUPSMANAGER_H

#include <QWidget>

#include <memory>

class SyncSettings;

class CreateRemoveBackupsManager
{
public:
    CreateRemoveBackupsManager() = delete;
    ~CreateRemoveBackupsManager() = delete;

    static void addBackup(bool comesFromSettings);
    static void removeBackup(std::shared_ptr<SyncSettings> backup, QWidget* parent);
    static bool isBackupsDialogOpen();

private:
    static void performAddBackup(bool comesFromSettings);
    static void performRemoveBackup(std::shared_ptr<SyncSettings> backup, QWidget* parent);
    static void showBackupDialog(bool comesFromSettings);
};

#endif // CREATEREMOVEBACKUPSMANAGER_H
