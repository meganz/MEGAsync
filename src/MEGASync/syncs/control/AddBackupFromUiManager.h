#ifndef ADDBACKUPFROMUIMANAGER_H
#define ADDBACKUPFROMUIMANAGER_H

#include <megaapi.h>

#include <QObject>
#include <QPointer>

class BindFolderDialog;
class SyncController;
class SyncSettings;
class RemoveBackupDialog;

class AddBackupFromUiManager : public QObject
{
    Q_OBJECT

public:
    AddBackupFromUiManager() = default;
    ~AddBackupFromUiManager() = default;

    static const AddBackupFromUiManager* addBackup_static(bool fromOnboarding);
    static const AddBackupFromUiManager* removeBackup_static(std::shared_ptr<SyncSettings> backup, QWidget* parent);

    void addBackup(bool fromOnboarding);
    void removeBackup(std::shared_ptr<SyncSettings> backup, QWidget* parent);

signals:
    void backupAdded(mega::MegaHandle remote, const QString& localPath);
    void backupAddingStarted();

private:
    void performAddBackup(bool fromOnboarding);
    void performRemoveBackup(std::shared_ptr<SyncSettings> backup, QWidget* parent);
};

#endif // ADDBACKUPFROMUIMANAGER_H
