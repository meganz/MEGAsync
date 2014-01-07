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

    static FileState getPathState(PCWSTR filePath);
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
    static int sendRequest(WCHAR type, PCWSTR content, PCWSTR response, int responseLen);
    MegaInterface() {}
};

#endif // MEGAINTERFACE_H
