#ifndef CREATEREMOVESYNCFROMUIMANAGER_H
#define CREATEREMOVESYNCFROMUIMANAGER_H

#include "megaapi.h"

#include <QObject>

class CreateRemoveSyncsManager : public QObject
{
    Q_OBJECT

public:
    CreateRemoveSyncsManager() = default;
    ~CreateRemoveSyncsManager() = default;

    static const CreateRemoveSyncsManager* const addSync(mega::MegaHandle handle = mega::INVALID_HANDLE, bool comesFromSettings = false);
    static const CreateRemoveSyncsManager* const removeSync(mega::MegaHandle handle, QWidget* parent);

private:
    void performAddSync(mega::MegaHandle handle = mega::INVALID_HANDLE, bool comesFromSettings = false);
    void performRemoveSync(mega::MegaHandle remoteHandle, QWidget* parent);
};

#endif // CREATEREMOVESYNCFROMUIMANAGER_H
