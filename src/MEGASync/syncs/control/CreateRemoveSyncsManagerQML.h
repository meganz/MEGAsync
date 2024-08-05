#ifndef CREATEREMOVESYNCSMANAGERQML_H
#define CREATEREMOVESYNCSMANAGERQML_H

#include "megaapi.h"

#include <QObject>
#include <QPointer>

class SyncSettings;
class CreateRemoveSyncsManager;

class CreateRemoveSyncsManagerQML
{
    Q_GADGET

public:
    explicit CreateRemoveSyncsManagerQML() = default;
    ~CreateRemoveSyncsManagerQML() = default;

    Q_INVOKABLE void addBackup(bool comesFromSettings);
    Q_INVOKABLE void removeBackup(mega::MegaHandle handle, QWidget* parent);
};

#endif // CREATEREMOVESYNCSMANAGERQML_H
