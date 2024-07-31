#ifndef CREATEREMOVEBACKUPSMANAGERQML_H
#define CREATEREMOVEBACKUPSMANAGERQML_H

#include "megaapi.h"

#include <QObject>
#include <QPointer>

class SyncSettings;
class CreateRemoveBackupsManager;

class CreateRemoveBackupsManagerQML
{
    Q_GADGET

public:
    explicit CreateRemoveBackupsManagerQML() = default;
    ~CreateRemoveBackupsManagerQML() = default;

    Q_INVOKABLE void addBackup(bool comesFromSettings);
    Q_INVOKABLE void removeBackup(std::shared_ptr<SyncSettings> backup, QWidget* parent);
};

#endif // CREATEREMOVEBACKUPSMANAGERQML_H
