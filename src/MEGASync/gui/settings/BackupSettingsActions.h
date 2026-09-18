#ifndef BACKUP_SETTINGS_ACTIONS_H
#define BACKUP_SETTINGS_ACTIONS_H

#include "megaapi.h"
#include "SyncSettingsActionsBase.h"

#include <memory>

namespace UserAttributes
{
class MyBackupsHandle;
}

// Backups tab actions (from BackupSettingsQuickWidget, SNC-6795).
class BackupSettingsActions: public SyncSettingsActionsBase
{
    Q_OBJECT

    Q_PROPERTY(
        QString backupFolderPath READ getBackupFolderPath NOTIFY backupFolderPathChanged FINAL)
    Q_PROPERTY(bool backupFolderAvailable READ isBackupFolderAvailable NOTIFY
                   backupFolderAvailableChanged FINAL)

public:
    explicit BackupSettingsActions(QWidget* hostWidget, QObject* parent = nullptr);

    void addItem() const override;
    void remove(int index) const override;

    Q_INVOKABLE void openBackupFolder() const;

    QString getBackupFolderPath() const;
    bool isBackupFolderAvailable() const;

signals:
    void backupFolderPathChanged();
    void backupFolderAvailableChanged();

private slots:
    void onMyBackupsFolderHandleSet(mega::MegaHandle handle);

private:
    std::shared_ptr<UserAttributes::MyBackupsHandle> mMyBackupsHandleRequest;
    QString mBackupFolderPath;
    bool mBackupFolderAvailable;
};

#endif // BACKUP_SETTINGS_ACTIONS_H
