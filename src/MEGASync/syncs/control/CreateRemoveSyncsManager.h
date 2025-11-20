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
                        mega::MegaHandle handle = mega::INVALID_HANDLE,
                        const QString& localPath = QString());

    static bool removeSync(mega::MegaHandle handle, QWidget* parent);
    static bool removeSync(std::shared_ptr<SyncSettings> syncSettings, QWidget* parent);

private:
    static void showSyncDialog(SyncInfo::SyncOrigin origin,
                               QString remoteFolder,
                               QString localFolder);
};

#endif // CREATEREMOVESYNCFROMUIMANAGER_H
