#ifndef CREATEREMOVESYNCSMANAGERQML_H
#define CREATEREMOVESYNCSMANAGERQML_H

#include "megaapi.h"

#include <QObject>
#include <QPointer>

class SyncSettings;
class CreateRemoveSyncsManager;

class CreateRemoveSyncsManagerQML
{
public:
    CreateRemoveSyncsManagerQML() = default;
    ~CreateRemoveSyncsManagerQML() = default;

    const CreateRemoveSyncsManager* const addBackup(bool comesFromSettings);
    const CreateRemoveSyncsManager* const removeBackup(mega::MegaHandle handle, QWidget* parent);
};

#endif // CREATEREMOVESYNCSMANAGERQML_H
