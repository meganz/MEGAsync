#ifndef CREATEREMOVEBACKUPSMANAGER_H
#define CREATEREMOVEBACKUPSMANAGER_H

#include "megaapi.h"

#include <QObject>
#include <QPointer>

class SyncSettings;

class CreateRemoveBackupsManager : public QObject
{
    Q_OBJECT

public:
    CreateRemoveBackupsManager() = default;
    ~CreateRemoveBackupsManager() = default;

    static const CreateRemoveBackupsManager*  addBackup(bool comesFromSettings);
    static const CreateRemoveBackupsManager*  removeBackup(std::shared_ptr<SyncSettings> backup, QWidget* parent);

    bool isBackupsDialogOpen() const;

private:
    void performAddBackup(bool comesFromSettings);
    void performRemoveBackup(std::shared_ptr<SyncSettings> backup, QWidget* parent);
};

#endif // CREATEREMOVEBACKUPSMANAGER_H
