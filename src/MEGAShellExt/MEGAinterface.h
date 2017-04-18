#ifndef MEGAINTERFACE_H
#define MEGAINTERFACE_H

#include <windows.h>
#include <strsafe.h>

class MegaInterface
{

public:
    typedef enum {
           FILE_SYNCED = 0,
           FILE_PENDING = 1,
           FILE_SYNCING = 2,
           FILE_NOTFOUND = 9
    } FileState;

    typedef enum {
           TYPE_FILE = 0,
           TYPE_FOLDER = 1,
           TYPE_UNKNOWN = 2
    } FileType;

    typedef enum {
           STRING_UPLOAD = 0,
           STRING_GETLINK = 1,
           STRING_SHARE = 2,
           STRING_SEND = 3,
           STRING_REMOVE_FROM_LEFT_PANE = 4
    } StringID;

    static FileState getPathState(PCWSTR filePath, bool overlayIcons = true);
    static LPWSTR getString(StringID stringID, int numFiles, int numFolders);
    static bool upload(PCWSTR path);
    static bool send(PCWSTR path);
    static bool pasteLink(PCWSTR path);
    static bool shareFolder(PCWSTR path);
    static bool startRequest();
    static bool endRequest();

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
    static int sendRequest(WCHAR type, PCWSTR content, PCWSTR response, int responseLen);
    MegaInterface() {}
};

#endif // MEGAINTERFACE_H
