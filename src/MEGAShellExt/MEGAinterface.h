// clang-format off
#ifndef MEGAINTERFACE_H
#define MEGAINTERFACE_H

#include <windows.h>
#include <strsafe.h>
#include <ostream>
#include <vector>

class MegaInterface
{
public:
    MegaInterface() = delete;

    typedef enum {
           FILE_SYNCED = 0,
           FILE_PENDING = 1,
           FILE_SYNCING = 2,
           FILE_IGNORED = 3,
           FILE_PAUSED = 4, // used for paused & suspendend syncs.
           FILE_NOTFOUND_NON_SYNCABLE = 8,
           FILE_NOTFOUND_SYNCABLE = 9,
           FILE_ERROR = 10
    } FileState;

    typedef enum {
           TYPE_FILE = 0,
           TYPE_FOLDER = 1,
           TYPE_NOTFOUND = 2,
           TYPE_UNKNOWN = 3
    } FileType;

    typedef enum {
           STRING_UPLOAD = 0,
           STRING_GETLINK = 1,
           STRING_SHARE = 2,
           STRING_SEND = 3,
           STRING_REMOVE_FROM_LEFT_PANE = 4,
           STRING_VIEW_ON_MEGA = 5,
           STRING_VIEW_VERSIONS = 6,
           STRING_SYNC = 7,
           STRING_BACKUP = 8
    } StringID;

    typedef enum {
        TYPE_TWOWAY = 3, // Two-way sync
        TYPE_BACKUP
    } SyncType;

    static FileState getPathState(PCWSTR filePath, bool overlayIcons = true);
    static std::unique_ptr<WCHAR[]> getString(StringID stringID);
    static bool upload(PCWSTR path);
    static bool send(PCWSTR path);
    static bool pasteLink(PCWSTR path);
    static bool shareFolder(PCWSTR path);
    static bool hasVersions(PCWSTR path);
    static bool viewOnMEGA(PCWSTR path);
    static bool viewVersions(PCWSTR path);
    static bool sync(const std::vector<std::wstring>& paths, SyncType type);
    static bool removeFromLeftPane(PCWSTR path);
    static bool startRequest();
    static bool endRequest();
    static bool isMEGASyncOpen();

private:
    static LPCWSTR MEGA_PIPE;
    static WCHAR OP_PATH_STATE;
    static WCHAR OP_INIT;
    static WCHAR OP_END;
    static WCHAR OP_UPLOAD;
    static WCHAR OP_LINK;
    static WCHAR OP_SHARE;
    static WCHAR OP_SEND;
    static WCHAR OP_STRING;
    static WCHAR OP_VIEW;
    static WCHAR OP_VERSIONS;
    static WCHAR OP_HASVERSIONS;
    static WCHAR OP_SYNCBACKUP;
    static WCHAR OP_REMOVE_FROM_LEFT_PANE;
    static int sendRequest(WCHAR type, PCWSTR content, PCWSTR response, int responseLen);
};

#endif // MEGAINTERFACE_H
