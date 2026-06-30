#ifndef SYNCERRORS_H
#define SYNCERRORS_H

#include "megaapi.h"

#include <QObject>

namespace SyncErrors
{
Q_NAMESPACE

/**
 * @brief Subset of mega::MegaSync::Error exposed to QML.
 *
 * Only the codes the sync/backup error UI actually branches on are mirrored
 * here. Each value is defined directly from the SDK enum, so the two can
 * never drift: renaming or removing a code in the SDK becomes a build error
 * at the definition below instead of a silent UI mis-mapping.
 */
enum Code
{
    LOCAL_PATH_TEMPORARY_UNAVAILABLE =
        static_cast<int>(mega::MegaSync::LOCAL_PATH_TEMPORARY_UNAVAILABLE),
    LOCAL_PATH_UNAVAILABLE = static_cast<int>(mega::MegaSync::LOCAL_PATH_UNAVAILABLE),
    REMOTE_NODE_NOT_FOUND = static_cast<int>(mega::MegaSync::REMOTE_NODE_NOT_FOUND),
    STORAGE_OVERQUOTA = static_cast<int>(mega::MegaSync::STORAGE_OVERQUOTA),
    ACCOUNT_EXPIRED = static_cast<int>(mega::MegaSync::ACCOUNT_EXPIRED),
    FOREIGN_TARGET_OVERSTORAGE = static_cast<int>(mega::MegaSync::FOREIGN_TARGET_OVERSTORAGE),
    LOCAL_FILESYSTEM_MISMATCH = static_cast<int>(mega::MegaSync::LOCAL_FILESYSTEM_MISMATCH),
    REMOTE_NODE_MOVED_TO_RUBBISH = static_cast<int>(mega::MegaSync::REMOTE_NODE_MOVED_TO_RUBBISH),
    REMOTE_NODE_INSIDE_RUBBISH = static_cast<int>(mega::MegaSync::REMOTE_NODE_INSIDE_RUBBISH),
    LOGGED_OUT = static_cast<int>(mega::MegaSync::LOGGED_OUT),
    SYNC_CONFIG_WRITE_FAILURE = static_cast<int>(mega::MegaSync::SYNC_CONFIG_WRITE_FAILURE),
    COULD_NOT_CREATE_IGNORE_FILE = static_cast<int>(mega::MegaSync::COULD_NOT_CREATE_IGNORE_FILE),
    SYNC_CONFIG_READ_FAILURE = static_cast<int>(mega::MegaSync::SYNC_CONFIG_READ_FAILURE),
    UNKNOWN_DRIVE_PATH = static_cast<int>(mega::MegaSync::UNKNOWN_DRIVE_PATH),
    NOTIFICATION_SYSTEM_UNAVAILABLE =
        static_cast<int>(mega::MegaSync::NOTIFICATION_SYSTEM_UNAVAILABLE),
    UNABLE_TO_ADD_WATCH = static_cast<int>(mega::MegaSync::UNABLE_TO_ADD_WATCH),
    UNABLE_TO_OPEN_DATABASE = static_cast<int>(mega::MegaSync::UNABLE_TO_OPEN_DATABASE),
    INSUFFICIENT_DISK_SPACE = static_cast<int>(mega::MegaSync::INSUFFICIENT_DISK_SPACE),
    FAILURE_ACCESSING_PERSISTENT_STORAGE =
        static_cast<int>(mega::MegaSync::FAILURE_ACCESSING_PERSISTENT_STORAGE),
    MISMATCH_OF_ROOT_FSID = static_cast<int>(mega::MegaSync::MISMATCH_OF_ROOT_FSID),
};
Q_ENUM_NS(Code)
};

#endif // SYNCERRORS_H
