#include "MegaInterface.h"

LPCWSTR MegaInterface::MEGA_PIPE    = L"\\\\.\\pipe\\MEGAprivacyMEGAsync";
WCHAR MegaInterface::OP_PATH_STATE  = L'P'; //Path state
WCHAR MegaInterface::OP_INIT        = L'I'; //Init operation
WCHAR MegaInterface::OP_END         = L'E'; //End operation
WCHAR MegaInterface::OP_UPLOAD      = L'F'; //File-Folder upload
WCHAR MegaInterface::OP_LINK        = L'L'; //paste Link
WCHAR MegaInterface::OP_SHARE       = L'S'; //Share folder
WCHAR MegaInterface::OP_SEND        = L'C'; //Copy to user

int MegaInterface::sendRequest(WCHAR type, PCWSTR filePath, PCWSTR response, int responseLen)
{
    BOOL success;
    DWORD readed = 0;
    int requestLen = lstrlen(filePath)+3;
    LPWSTR request = new wchar_t[requestLen];
    StringCchPrintfW(request, requestLen, L"%c:%s", type, filePath);
    success = CallNamedPipeW(
       MegaInterface::MEGA_PIPE,			// pipe name
       (LPVOID)request,                     // message to server
       requestLen * sizeof(WCHAR),                          // message length
       (LPVOID)response,					// buffer to receive reply
       responseLen,                         // size of read buffer
       &readed,								// number of bytes read
       NMPWAIT_NOWAIT);						// waits
    delete request;
    if (success) return readed;
    return 0;
}

MegaInterface::FileState MegaInterface::getPathState(PCWSTR filePath)
{
    WCHAR chReadBuf[2];
    int cbRead = sendRequest(MegaInterface::OP_PATH_STATE, filePath, chReadBuf, sizeof(chReadBuf));
    if(cbRead>sizeof(WCHAR))
        return ((MegaInterface::FileState)(chReadBuf[0]-L'0'));
    return MegaInterface::FILE_NOTFOUND;
}

bool MegaInterface::upload(PCWSTR path)
{
    WCHAR chReadBuf[2];
    int cbRead = sendRequest(MegaInterface::OP_UPLOAD, path, chReadBuf, sizeof(chReadBuf));
    if(cbRead>sizeof(WCHAR))
        return true;
    return false;
}

bool MegaInterface::send(PCWSTR path)
{
    WCHAR chReadBuf[2];
    int cbRead = sendRequest(MegaInterface::OP_SEND, path, chReadBuf, sizeof(chReadBuf));
    if(cbRead>sizeof(WCHAR))
        return true;
    return false;
}

bool MegaInterface::pasteLink(PCWSTR path)
{
    WCHAR chReadBuf[2];
    int cbRead = sendRequest(MegaInterface::OP_LINK, path, chReadBuf, sizeof(chReadBuf));
    if(cbRead>sizeof(WCHAR))
        return true;
    return false;
}

bool MegaInterface::shareFolder(PCWSTR path)
{
    WCHAR chReadBuf[2];
    int cbRead = sendRequest(MegaInterface::OP_SHARE, path, chReadBuf, sizeof(chReadBuf));
    if(cbRead>sizeof(WCHAR))
        return true;
    return false;
}

bool MegaInterface::startRequest()
{
    WCHAR chReadBuf[2];
    if(sendRequest(MegaInterface::OP_INIT, L"", chReadBuf, sizeof(chReadBuf)))
        return true;
    return false;
}

bool MegaInterface::endRequest()
{
    WCHAR chReadBuf[2];
    if(sendRequest(MegaInterface::OP_END, L"", chReadBuf, sizeof(chReadBuf)))
        return true;
    return false;
}
