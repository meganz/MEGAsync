#include "MegaInterface.h"

#include "ContextMenuData.h"
#include "Utilities.h"

// clang-format off
LPCWSTR MegaInterface::MEGA_PIPE    = L"\\\\.\\pipe\\MEGAprivacyMEGAsync";

WCHAR MegaInterface::OP_PATH_STATE              = L'P'; //Path state
WCHAR MegaInterface::OP_INIT                    = L'I'; //Init operation
WCHAR MegaInterface::OP_END                     = L'E'; //End operation
WCHAR MegaInterface::OP_UPLOAD                  = L'F'; //File-Folder upload
WCHAR MegaInterface::OP_LINK                    = L'L'; //paste Link
WCHAR MegaInterface::OP_SHARE                   = L'S'; //Share folder
WCHAR MegaInterface::OP_SEND                    = L'C'; //Copy to user
WCHAR MegaInterface::OP_STRING                  = L'T'; //Get Translated String
WCHAR MegaInterface::OP_VIEW                    = L'V'; //View on MEGA
WCHAR MegaInterface::OP_VERSIONS                = L'R'; //View pRevious versions
WCHAR MegaInterface::OP_HASVERSIONS             = L'H'; //Has previous versions?
WCHAR MegaInterface::OP_SYNCBACKUP              = L'K'; // Sync
WCHAR MegaInterface::OP_REMOVE_FROM_LEFT_PANE   = L'J'; // Remove a sync from left pane (navigation pane on shell)

// clang-format on

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
    delete[] request;
    if (success) return readed;
    return 0;
}

bool MegaInterface::isMEGASyncOpen()
{
    return WaitNamedPipe(MegaInterface::MEGA_PIPE, 0) != 0;
}

MegaInterface::FileState MegaInterface::getPathState(PCWSTR filePath, bool overlayIcons)
{
    WCHAR chReadBuf[2];
    int requestLen = lstrlen(filePath) + 3;
    LPWSTR request = new WCHAR[requestLen];
    StringCchPrintfW(request, requestLen, L"%s|%d", filePath, overlayIcons);

    int cbRead = sendRequest(MegaInterface::OP_PATH_STATE, request, chReadBuf, sizeof(chReadBuf));
    delete[] request;
    if (cbRead > sizeof(WCHAR))
    {
        return ((MegaInterface::FileState)(chReadBuf[0] - L'0'));
    }
    return MegaInterface::FILE_ERROR;
}

bool MegaInterface::removeFromLeftPane(PCWSTR path)
{
    WCHAR chReadBuf[2];
    int cbRead =
        sendRequest(MegaInterface::OP_REMOVE_FROM_LEFT_PANE, path, chReadBuf, sizeof(chReadBuf));
    if (cbRead > sizeof(WCHAR))
    {
        return true;
    }
    return false;
}

std::unique_ptr<WCHAR[]> MegaInterface::getString(StringID stringID)
{
    WCHAR request[64];
    StringCchPrintfW(request, sizeof(request), L"%d", stringID);

    std::unique_ptr<WCHAR[]> chReadBuf(new WCHAR[128]);
    int cbRead =
        sendRequest(MegaInterface::OP_STRING, request, chReadBuf.get(), 128 * sizeof(WCHAR));
    // L"9" is the default value, so no string has been set
    if (cbRead > sizeof(WCHAR) && wcscmp(chReadBuf.get(), L"9") != 0)
    {
        return chReadBuf;
    }

    return nullptr;
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

bool MegaInterface::hasVersions(PCWSTR path)
{
    WCHAR chReadBuf[2];
    int cbRead = sendRequest(MegaInterface::OP_HASVERSIONS, path, chReadBuf, sizeof(chReadBuf));
    if (cbRead > sizeof(WCHAR) && (chReadBuf[0] - L'0'))
    {
        return true;
    }
    return false;
}

bool MegaInterface::viewOnMEGA(PCWSTR path)
{
    WCHAR chReadBuf[2];
    int cbRead = sendRequest(MegaInterface::OP_VIEW, path, chReadBuf, sizeof(chReadBuf));
    if (cbRead > sizeof(WCHAR))
    {
        return true;
    }
    return false;
}

bool MegaInterface::viewVersions(PCWSTR path)
{
    WCHAR chReadBuf[2];
    int cbRead = sendRequest(MegaInterface::OP_VERSIONS, path, chReadBuf, sizeof(chReadBuf));
    if (cbRead > sizeof(WCHAR))
    {
        return true;
    }
    return false;
}

bool MegaInterface::sync(const std::vector<std::wstring>& paths, SyncType type)
{
    std::wstring result;

    // Append all the requested paths, separated by "|"
    for (const auto& path: paths)
    {
        result += path + L"|";
    }

    // Append the type: Sync or Backup
    auto intSize(sizeof(int));
    std::unique_ptr<WCHAR[]> typeChar(new WCHAR[intSize]);
    swprintf_s(typeChar.get(), intSize, L"%d", type);
    result += typeChar.get();

    // Allocate the final WCHAR buffer with the perfect size
    std::unique_ptr<WCHAR[]> request(new WCHAR[result.size() + 1]);
    wcscpy_s(request.get(), result.size() + 1, result.c_str());

    WCHAR chReadBuf[2];
    int cbRead =
        sendRequest(MegaInterface::OP_SYNCBACKUP, request.get(), chReadBuf, sizeof(chReadBuf));

    return (cbRead > sizeof(WCHAR)) ? true : false;
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
