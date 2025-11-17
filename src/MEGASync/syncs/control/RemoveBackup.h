#ifndef REMOVE_BACKUP_H
#define REMOVE_BACKUP_H

#include "megaapi.h"

#include <QObject>
#include <QPointer>

#include <memory>

class SyncSettings;
class RemoveBackupDialog;

class RemoveBackup: public QObject
{
    Q_OBJECT

public:
    RemoveBackup() = default;
    ~RemoveBackup() = default;
    void removeBackup(std::shared_ptr<SyncSettings> backup, QWidget* parent);

private:
    void onConfirmRemove(mega::MegaHandle targetFolder);
    void backupMoveOrRemoveRemoteFolderError(std::shared_ptr<mega::MegaError> error);
    bool checkBackupFolderExistOnTargetFolder(mega::MegaHandle targetFolder);

    std::shared_ptr<SyncSettings> mBackupToRemove;
    mega::MegaHandle mFolderToMoveBackupData;
    QWidget* mParent;
    QPointer<RemoveBackupDialog> mRemoveBackupDialog;
};

#endif
