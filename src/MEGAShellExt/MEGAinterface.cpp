#include "MegaInterface.h"

LPCWSTR MegaInterface::MEGA_PIPE    = L"\\\\.\\pipe\\MEGAprivacyMEGAsync";
WCHAR MegaInterface::OP_PATH_STATE  = L'P'; //Path state
WCHAR MegaInterface::OP_INIT        = L'I'; //Init operation
WCHAR MegaInterface::OP_END         = L'E'; //End operation
WCHAR MegaInterface::OP_UPLOAD      = L'F'; //File-Folder upload
WCHAR MegaInterface::OP_LINK        = L'L'; //paste Link
WCHAR MegaInterface::OP_SHARE       = L'S'; //Share folder
WCHAR MegaInterface::OP_SEND        = L'C'; //Copy to user
WCHAR MegaInterface::OP_STRING      = L'T'; //Get Translated String

int MegaInterface::sendRequest(WCHAR type, PCWSTR content, PCWSTR response, int responseLen)
{
    BOOL success;
    DWORD readed = 0;
    int requestLen = lstrlen(content)+3;
    LPWSTR request = new WCHAR[requestLen];
    StringCchPrintfW(request, requestLen, L"%c:%s", type, content);
    success = CallNamedPipeW(
       MegaInterface::MEGA_PIPE,            // pipe name
       (LPVOID)request,                     // message to server
       requestLen * sizeof(WCHAR),          // message length
       (LPVOID)response,                    // buffer to receive reply
       responseLen,                         // size of read buffer
       &readed,                             // number of bytes read
       NMPWAIT_NOWAIT);                     // waits
    delete request;
    if (success) return readed;
    return 0;
}

MegaInterface::FileState MegaInterface::getPathState(PCWSTR filePath, bool overlayIcons)
{
    WCHAR chReadBuf[2];
    int requestLen = lstrlen(filePath) + 3;
    LPWSTR request = new WCHAR[requestLen];
    StringCchPrintfW(request, requestLen, L"%s|%d", filePath, overlayIcons);

    int cbRead = sendRequest(MegaInterface::OP_PATH_STATE, request, chReadBuf, sizeof(chReadBuf));
    if (cbRead > sizeof(WCHAR))
    {
        return ((MegaInterface::FileState)(chReadBuf[0] - L'0'));
    }
    return MegaInterface::FILE_NOTFOUND;
}

LPWSTR MegaInterface::getString(StringID stringID, int numFiles, int numFolders)
{
    WCHAR request[64];
    StringCchPrintfW(request, sizeof(request), L"%d:%d:%d", stringID, numFiles, numFolders);

    LPWSTR chReadBuf = new WCHAR[128];
    int cbRead = sendRequest(MegaInterface::OP_STRING, request, chReadBuf, 128*sizeof(WCHAR));
    if (cbRead > sizeof(WCHAR))
    {
        return chReadBuf;
    }

    delete chReadBuf;
    return NULL;
}

bool MegaInterface::upload(PCWSTR path)
{
    WCHAR chReadBuf[2];
    int cbRead = sendRequest(MegaInterface::OP_UPLOAD, path, chReadBuf, sizeof(chReadBuf));
    if (cbRead > sizeof(WCHAR))
    {
        return true;
    }
    return false;
}

bool MegaInterface::send(PCWSTR path)
{
    WCHAR chReadBuf[2];
    int cbRead = sendRequest(MegaInterface::OP_SEND, path, chReadBuf, sizeof(chReadBuf));
    if (cbRead > sizeof(WCHAR))
    {
        return true;
    }
    return false;
}

bool MegaInterface::pasteLink(PCWSTR path)
{
    WCHAR chReadBuf[2];
    int cbRead = sendRequest(MegaInterface::OP_LINK, path, chReadBuf, sizeof(chReadBuf));
    if (cbRead > sizeof(WCHAR))
    {
        return true;
    }
    return false;
}

bool MegaInterface::shareFolder(PCWSTR path)
{
    WCHAR chReadBuf[2];
    int cbRead = sendRequest(MegaInterface::OP_SHARE, path, chReadBuf, sizeof(chReadBuf));
    if (cbRead > sizeof(WCHAR))
    {
        return true;
    }
    return false;
}

bool MegaInterface::startRequest()
{
    WCHAR chReadBuf[2];
    if (sendRequest(MegaInterface::OP_INIT, L"", chReadBuf, sizeof(chReadBuf)))
    {
        return true;
    }
    return false;
}

bool MegaInterface::endRequest()
{
    WCHAR chReadBuf[2];
    if (sendRequest(MegaInterface::OP_END, L"", chReadBuf, sizeof(chReadBuf)))
    {
        return true;
    }
    return false;
}
