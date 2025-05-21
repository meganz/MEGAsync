#ifndef CREATEREMOVESYNCFROMUIMANAGER_H
#define CREATEREMOVESYNCFROMUIMANAGER_H

#include "megaapi.h"
#include "SyncInfo.h"

#include <QObject>

#include <memory>

class SyncSettings;

class CreateRemoveSyncsManager: public QObject
{
    Q_OBJECT

public:
    CreateRemoveSyncsManager() = default;
    ~CreateRemoveSyncsManager() = default;

    static const CreateRemoveSyncsManager* addSync(SyncInfo::SyncOrigin origin,
                                                   mega::MegaHandle handle = mega::INVALID_HANDLE,
                                                   const QString& localPath = QString());

    static bool removeSync(mega::MegaHandle handle, QWidget* parent);
    static bool removeSync(std::shared_ptr<SyncSettings> syncSettings, QWidget* parent);

private:
    void performAddSync(SyncInfo::SyncOrigin origin,
                        mega::MegaHandle handle = mega::INVALID_HANDLE,
                        const QString& localPath = QString());
    bool performRemoveSync(mega::MegaHandle remoteHandle, QWidget* parent);
    bool performRemoveSync(std::shared_ptr<SyncSettings> syncSettings, QWidget* parent);
};

#endif // CREATEREMOVESYNCFROMUIMANAGER_H
