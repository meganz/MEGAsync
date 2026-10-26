#include "MEGAinterface.h"

#include "ContextMenuData.h"
#include "ShellDispatcherPipe.h"
#include "Utilities.h"

#include <memory>
#include <string>

// clang-format off
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

namespace
{

// Closes a Win32 HANDLE. Local to the extension on purpose: the only thing it
// shares with MEGAsync is the pipe name contract (ShellDispatcherPipe.h), not
// utility code.
struct HandleCloser
{
    void operator()(HANDLE handle) const
    {
        if (handle && handle != INVALID_HANDLE_VALUE)
        {
            CloseHandle(handle);
        }
    }
};

using HandlePtr = std::unique_ptr<void, HandleCloser>;

// The pipe name and this process's own SID are process-constant; Explorer asks
// for the state of every item it paints, so avoid re-querying the token each
// time. Magic statics: initialized once, thread-safe. If resolution fails at
// first use the empty result is cached too - fail-closed for the process
// lifetime, same outcome as failing on every call.
const std::wstring& cachedPipeName()
{
    static const std::wstring pipeName = ShellDispatcherPipe::name();
    return pipeName;
}

const std::wstring& cachedOwnSid()
{
    static const std::wstring sid = ShellDispatcherPipe::currentUserSid();
    return sid;
}

// Confirms the process serving `pipe` runs as the current user, i.e. it is this
// user's own MEGAsync and not another local user squatting the name (SNC-6808).
// CallNamedPipeW gave no handle to inspect; that is why sendRequest now opens
// the pipe explicitly and calls this before sending anything.
bool serverIsCurrentUser(HANDLE pipe)
{
    ULONG serverPid = 0;
    if (!GetNamedPipeServerProcessId(pipe, &serverPid))
    {
        return false;
    }

    const HandlePtr server(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, serverPid));
    if (!server)
    {
        return false;
    }

    const std::wstring serverSid = ShellDispatcherPipe::processUserSid(server.get());
    const std::wstring& ownSid = cachedOwnSid();
    return !serverSid.empty() && serverSid == ownSid;
}

} // namespace

int MegaInterface::sendRequest(WCHAR type, PCWSTR content, PCWSTR response, int responseLen)
{
    const std::wstring& pipeName = cachedPipeName();
    if (pipeName.empty())
    {
        return 0;
    }

    const int requestLen = lstrlen(content) + 3;
    std::unique_ptr<WCHAR[]> request(new WCHAR[requestLen]);
    StringCchPrintfW(request.get(), requestLen, L"%c:%s", type, content);

    // Open the pipe explicitly instead of CallNamedPipeW, so the server can be
    // verified before anything is sent. An unavailable pipe (missing or all
    // instances busy) fails immediately here, matching the old NMPWAIT_NOWAIT.
    const HandlePtr pipe(CreateFileW(pipeName.c_str(),
                                     GENERIC_READ | GENERIC_WRITE,
                                     0,
                                     nullptr,
                                     OPEN_EXISTING,
                                     // Cap the server's impersonation of this client at
                                     // identification: a rogue server that somehow got past the
                                     // SID check below could otherwise impersonate Explorer's
                                     // token at impersonation level.
                                     SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION,
                                     nullptr));
    if (pipe.get() == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    // Only talk to our own MEGAsync.
    if (!serverIsCurrentUser(pipe.get()))
    {
        return 0;
    }

    // The server pipe is message mode; switch this handle to match so one read
    // returns one whole reply.
    DWORD mode = PIPE_READMODE_MESSAGE;
    if (!SetNamedPipeHandleState(pipe.get(), &mode, nullptr, nullptr))
    {
        return 0;
    }

    DWORD written = 0;
    if (!WriteFile(pipe.get(), request.get(), requestLen * sizeof(WCHAR), &written, nullptr))
    {
        return 0;
    }

    DWORD readed = 0;
    if (!ReadFile(pipe.get(), (LPVOID)response, responseLen, &readed, nullptr))
    {
        return 0;
    }

    return static_cast<int>(readed);
}

bool MegaInterface::isMEGASyncOpen()
{
    const std::wstring& pipeName = cachedPipeName();
    if (pipeName.empty())
    {
        return false;
    }
    return WaitNamedPipeW(pipeName.c_str(), 0) != 0;
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
