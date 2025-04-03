#ifndef CREATEREMOVESYNCFROMUIMANAGER_H
#define CREATEREMOVESYNCFROMUIMANAGER_H

#include "megaapi.h"
#include "SyncInfo.h"

#include <memory>

class SyncSettings;

class CreateRemoveSyncsManager
{
public:
    CreateRemoveSyncsManager() = delete;
    ~CreateRemoveSyncsManager() = delete;

    static void addSync(SyncInfo::SyncOrigin origin,
                        mega::MegaHandle handle = mega::INVALID_HANDLE);

    static bool removeSync(mega::MegaHandle handle, QWidget* parent);
    static bool removeSync(std::shared_ptr<SyncSettings> syncSettings, QWidget* parent);

private:
    static void performAddSync(SyncInfo::SyncOrigin origin,
                               mega::MegaHandle handle = mega::INVALID_HANDLE);
    static bool performRemoveSync(mega::MegaHandle remoteHandle, QWidget* parent);
    static bool performRemoveSync(std::shared_ptr<SyncSettings> syncSettings, QWidget* parent);
    static void showSyncDialog(SyncInfo::SyncOrigin origin, QString remoteFolder);
};

#endif // CREATEREMOVESYNCFROMUIMANAGER_H
