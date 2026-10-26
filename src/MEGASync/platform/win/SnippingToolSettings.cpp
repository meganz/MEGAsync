#include "SnippingToolSettings.h"

#include <QWinEventNotifier>

#include <tlhelp32.h>
#include <vector>

namespace
{
const wchar_t* const LOCAL_STATE_KEY = L"LocalState";
const wchar_t* const AUTO_SAVE_FOLDER_VALUE = L"AutoSaveScreenshotsLocation";
const wchar_t* const TOOL_EXECUTABLE = L"SnippingTool.exe";

DWORD findToolProcessId()
{
    const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    DWORD processId = 0;
    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);
    for (BOOL found = Process32FirstW(snapshot, &entry); found && !processId;
         found = Process32NextW(snapshot, &entry))
    {
        if (_wcsicmp(entry.szExeFile, TOOL_EXECUTABLE) == 0)
        {
            processId = entry.th32ProcessID;
        }
    }

    CloseHandle(snapshot);
    return processId;
}

// The value is a UWP-typed blob (type 0x5F5E10C): UTF-16 text followed by a
// binary timestamp. Keep the leading printable run, which is the token.
QString extractToken(const std::vector<BYTE>& data)
{
    const wchar_t DELETE_CHAR(0x7F);

    const wchar_t* wp = reinterpret_cast<const wchar_t*>(data.data());
    const size_t wlen = data.size() / sizeof(wchar_t);
    QString token = QString::fromWCharArray(wp, static_cast<int>(wlen));
    int end = 0;
    while (end < token.size() && token.at(end) >= L' ' && token.at(end) < DELETE_CHAR)
    {
        end++;
    }

    token.truncate(end);

    return token.trimmed();
}
}

SnippingToolSettings::SnippingToolSettings(QObject* parent):
    QObject(parent)
{}

SnippingToolSettings::~SnippingToolSettings()
{
    close();
    delete mNotifier;
    if (mWrittenEvent)
    {
        CloseHandle(mWrittenEvent);
    }
    delete mToolExitNotifier;
    if (mToolProcess)
    {
        CloseHandle(mToolProcess);
    }
}

QString SnippingToolSettings::autoSaveFolderToken()
{
    QString token;
    if (!readToken(token))
    {
        // Not open yet, or the key went away under the open handle (the
        // settings were replaced). Either way, (re)open and ask again.
        close();
        if (!open() || !readToken(token))
        {
            close();
            token.clear();
        }
    }

    mToken = token;
    return token;
}

bool SnippingToolSettings::isWatching() const
{
    return mNotifier && mNotifier->isEnabled();
}

bool SnippingToolSettings::watchToolExit()
{
    if (mToolProcess)
    {
        return true; // already armed for the running instance
    }

    const DWORD processId = findToolProcessId();
    if (!processId)
    {
        return false;
    }

    mToolProcess = OpenProcess(SYNCHRONIZE, FALSE, processId);
    if (!mToolProcess)
    {
        return false;
    }

    if (!mToolExitNotifier)
    {
        mToolExitNotifier = new QWinEventNotifier();
        connect(mToolExitNotifier,
                &QWinEventNotifier::activated,
                this,
                &SnippingToolSettings::onToolExited);
    }
    mToolExitNotifier->setHandle(mToolProcess);
    mToolExitNotifier->setEnabled(true);
    return true;
}

void SnippingToolSettings::onToolExited()
{
    // An exited process stays signalled: disarm before anything else.
    mToolExitNotifier->setEnabled(false);
    CloseHandle(mToolProcess);
    mToolProcess = nullptr;

    emit toolExited();
}

std::wstring SnippingToolSettings::packageRoot()
{
    WCHAR localAppData[MAX_PATH] = {};
    GetEnvironmentVariableW(L"LOCALAPPDATA", localAppData, MAX_PATH);
    return std::wstring(localAppData) + L"\\Packages\\Microsoft.ScreenSketch_8wekyb3d8bbwe";
}

void SnippingToolSettings::onSettingsWritten()
{
    const QString previous = mToken;

    // Armed again before reading, so a write landing in between signals the
    // new notification instead of being lost.
    watch();

    if (autoSaveFolderToken() != previous)
    {
        emit autoSaveFolderTokenChanged();
    }
}

bool SnippingToolSettings::open()
{
    const std::wstring settingsDat = packageRoot() + L"\\Settings\\settings.dat";
    if (RegLoadAppKeyW(settingsDat.c_str(), &mHive, KEY_READ, 0, 0) != ERROR_SUCCESS)
    {
        mHive = nullptr;
        return false;
    }

    if (RegOpenKeyExW(mHive, LOCAL_STATE_KEY, 0, KEY_READ, &mLocalState) != ERROR_SUCCESS)
    {
        mLocalState = nullptr;
        return false;
    }

    // Reading still works unwatched; isWatching() tells the caller to poll.
    watch();
    return true;
}

void SnippingToolSettings::close()
{
    if (mNotifier)
    {
        mNotifier->setEnabled(false);
    }
    if (mLocalState)
    {
        RegCloseKey(mLocalState);
        mLocalState = nullptr;
    }
    if (mHive)
    {
        RegCloseKey(mHive);
        mHive = nullptr;
    }
}

bool SnippingToolSettings::watch()
{
    if (!mLocalState)
    {
        return false;
    }

    if (!mWrittenEvent)
    {
        mWrittenEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        if (!mWrittenEvent)
        {
            return false;
        }
        mNotifier = new QWinEventNotifier(mWrittenEvent);
        connect(mNotifier,
                &QWinEventNotifier::activated,
                this,
                &SnippingToolSettings::onSettingsWritten);
    }

    // A notification fires once; each one is armed anew.
    ResetEvent(mWrittenEvent);
    const bool armed = RegNotifyChangeKeyValue(mLocalState,
                                               FALSE,
                                               REG_NOTIFY_CHANGE_LAST_SET,
                                               mWrittenEvent,
                                               TRUE) == ERROR_SUCCESS;
    mNotifier->setEnabled(armed);
    return armed;
}

bool SnippingToolSettings::readToken(QString& token) const
{
    token.clear();
    if (!mLocalState)
    {
        return false;
    }

    DWORD length = 0;
    LONG result =
        RegQueryValueExW(mLocalState, AUTO_SAVE_FOLDER_VALUE, nullptr, nullptr, nullptr, &length);
    if (result == ERROR_FILE_NOT_FOUND)
    {
        // Never relocated, or moved back to the default: no token is a valid
        // answer, not a failure.
        return true;
    }
    if (result != ERROR_SUCCESS)
    {
        // ERROR_KEY_DELETED among others: the handle is stale.
        return false;
    }
    if (length == 0)
    {
        return true;
    }

    std::vector<BYTE> data(length);
    result = RegQueryValueExW(mLocalState,
                              AUTO_SAVE_FOLDER_VALUE,
                              nullptr,
                              nullptr,
                              data.data(),
                              &length);
    if (result != ERROR_SUCCESS)
    {
        return false;
    }
    data.resize(length);

    token = extractToken(data);
    return true;
}
