#ifndef CREATEREMOVEBACKUPSMANAGERQML_H
#define CREATEREMOVEBACKUPSMANAGERQML_H

#include "megaapi.h"

#include <QObject>
#include <QPointer>

class SyncSettings;
class CreateRemoveBackupsManager;

class CreateRemoveBackupsManagerQML
{
public:
    CreateRemoveBackupsManagerQML() = default;
    ~CreateRemoveBackupsManagerQML() = default;

    const CreateRemoveBackupsManager* const addBackup(bool comesFromSettings);
    const CreateRemoveBackupsManager* const removeBackup(std::shared_ptr<SyncSettings> backup,
                                                         QWidget* parent);
};

#endif // CREATEREMOVEBACKUPSMANAGERQML_H
