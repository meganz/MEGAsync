#ifndef CREATEREMOVESYNCFROMUIMANAGER_H
#define CREATEREMOVESYNCFROMUIMANAGER_H

#include "megaapi.h"

#include <QObject>

#include <memory>

class SyncSettings;

class CreateRemoveSyncsManager: public QObject
{
    Q_OBJECT

public:
    CreateRemoveSyncsManager() = default;
    ~CreateRemoveSyncsManager() = default;

    static const CreateRemoveSyncsManager* addSync(mega::MegaHandle handle = mega::INVALID_HANDLE,
                                                   bool comesFromSettings = false);
    static bool removeSync(mega::MegaHandle handle, QWidget* parent);
    static bool removeSync(std::shared_ptr<SyncSettings> syncSettings, QWidget* parent);

private:
    void performAddSync(mega::MegaHandle handle = mega::INVALID_HANDLE,
                        bool comesFromSettings = false);
    bool performRemoveSync(mega::MegaHandle remoteHandle, QWidget* parent);
    bool performRemoveSync(std::shared_ptr<SyncSettings> syncSettings, QWidget* parent);
};

#endif // CREATEREMOVESYNCFROMUIMANAGER_H
