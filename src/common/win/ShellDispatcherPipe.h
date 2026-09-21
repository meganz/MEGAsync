#ifndef SHELL_DISPATCHER_PIPE_H
#define SHELL_DISPATCHER_PIPE_H

// Single source of truth for the name of the Windows shell-integration named
// pipe. MEGAsync (the server, in WinShellDispatcherTask) and the Explorer shell
// extension (the client, in MegaInterface) both derive the name from here so
// they always agree on it.
//
// This header is a WIRE CONTRACT, not a utility library: it holds exactly what
// the two binaries must compute identically (which fields, which API produces
// each one, how they are joined) and nothing else. Read README.md in this
// directory before adding anything to it.
//
// The name is scoped to the current Windows user (account SID) and logon
// session id. On a shared or multi-session host (RDS/Citrix) that keeps one
// user from occupying the name another user's MEGAsync serves, and routes each
// session's Explorer to that same session's MEGAsync. See SNC-6808.
//
// Both processes run as the same user in the same session, so each one reads
// its OWN process token and session id and independently produces the identical
// string. No data is exchanged between them to establish the name.
//
// The name is NOT a secret and is NOT a security boundary on its own: SIDs are
// public (LookupAccountName) and session ids are enumerable, and creating any
// name under \\.\pipe\ needs no privilege, so another local user CAN pre-create
// this name. What actually protects the channel is (a) the server claiming the
// name with FILE_FLAG_FIRST_PIPE_INSTANCE and a current-user-only DACL, so a
// squatted name is a hard, logged failure instead of a silent handover, and
// (b) the client verifying the server process's user SID before sending
// anything. Accepted residual: a squatter can still deny service - the victim's
// shell integration goes dark (fail-closed, logged) but nothing leaks and
// nothing can be spoofed.
//
// Header-only and static so both build targets (which share no link library)
// can use it without a common object file.

// clang-format off
#include <windows.h> // must come before <sddl.h>, which relies on its types
#include <sddl.h>
// clang-format on

#include <memory>
#include <string>
#include <vector>

class ShellDispatcherPipe
{
public:
    // The fixed, unscoped stem kept from earlier versions. Never used on its own
    // any more - it is only the prefix of the per-user name built by name().
    static const wchar_t* baseName()
    {
        return L"MEGAprivacyMEGAsync";
    }

    // The SID (string form, e.g. "S-1-5-21-...-1001") of the user owning
    // `process`, which must be open for PROCESS_QUERY_(LIMITED_)INFORMATION, or
    // an empty string on failure. ConvertSidToStringSidW yields a canonical
    // representation, so independent callers always format the same SID
    // identically - which is what lets the client compare the server's owner to
    // its own (SNC-6808).
    static std::wstring processUserSid(HANDLE process)
    {
        HANDLE rawToken = nullptr;
        if (!OpenProcessToken(process, TOKEN_QUERY, &rawToken))
        {
            return std::wstring();
        }

        // RAII from here on: the handle and the string SID are released on every
        // path, including a thrown std::bad_alloc from the allocations below.
        const HandlePtr token(rawToken);

        // First call learns the required buffer size.
        DWORD length = 0;
        if (GetTokenInformation(token.get(), TokenUser, nullptr, 0, &length) ||
            GetLastError() != ERROR_INSUFFICIENT_BUFFER || length == 0)
        {
            return std::wstring();
        }

        std::vector<BYTE> buffer(length);
        if (!GetTokenInformation(token.get(), TokenUser, buffer.data(), length, &length))
        {
            return std::wstring();
        }

        const TOKEN_USER* tokenUser = reinterpret_cast<const TOKEN_USER*>(buffer.data());

        LPWSTR rawSidText = nullptr;
        if (!ConvertSidToStringSidW(tokenUser->User.Sid, &rawSidText))
        {
            return std::wstring();
        }

        const std::unique_ptr<WCHAR, LocalFreer> sidText(rawSidText);
        return std::wstring(sidText.get());
    }

    // The current process user's SID in string form, or empty on failure.
    static std::wstring currentUserSid()
    {
        return processUserSid(GetCurrentProcess());
    }

    // The Windows session id of the current process, via out-param. Returns
    // false on failure.
    static bool currentSessionId(DWORD& sessionId)
    {
        return ProcessIdToSessionId(GetCurrentProcessId(), &sessionId) != FALSE;
    }

    // Composes the full pipe name from an already-resolved identity, e.g.
    // "\\.\pipe\MEGAprivacyMEGAsync_S-1-5-21-...-1001_1".
    //
    // Pure (no OS calls), so the naming format and the fail-closed rule below
    // can be unit tested directly. Returns an empty string when userSid is
    // empty - callers MUST treat that as "no pipe available" and never fall
    // back to any shared or unscoped name, which is the very exposure this
    // avoids.
    static std::wstring composeName(const std::wstring& userSid, DWORD sessionId)
    {
        if (userSid.empty())
        {
            return std::wstring();
        }

        std::wstring pipeName(L"\\\\.\\pipe\\");
        pipeName += baseName();
        pipeName += L'_';
        pipeName += userSid;
        pipeName += L'_';
        pipeName += std::to_wstring(sessionId);
        return pipeName;
    }

    // The full pipe name for the current user and session. Returns an empty
    // string if the user identity or session could not be determined; see the
    // fail-closed note on composeName().
    static std::wstring name()
    {
        const std::wstring sid = currentUserSid();
        if (sid.empty())
        {
            return std::wstring();
        }

        DWORD sessionId = 0;
        if (!currentSessionId(sessionId))
        {
            return std::wstring();
        }

        return composeName(sid, sessionId);
    }

    // Legacy unscoped name (pre-SNC-6808), served only transitionally for old
    // shell-ext DLLs. Not a boundary: serve it with the same hardening as name()
    // (first-instance + current-user DACL). New code must use name().
    static std::wstring legacyName()
    {
        std::wstring pipeName(L"\\\\.\\pipe\\");
        pipeName += baseName();
        return pipeName;
    }

private:
    // RAII deleters used only inside this header. Kept private on purpose: the
    // contract must not double as a shared utility library. The server and the
    // client each define the (trivial) deleters they need on their own side.
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

    struct LocalFreer
    {
        void operator()(void* memory) const
        {
            if (memory)
            {
                LocalFree(memory);
            }
        }
    };

    using HandlePtr = std::unique_ptr<void, HandleCloser>;
};

#endif // SHELL_DISPATCHER_PIPE_H
