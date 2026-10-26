// The shell dispatcher pipe name is the one thing MEGAsync and the Explorer
// shell extension must compute identically without talking to each other
// (src/common/win/README.md). These tests pin the exact wire format, so any
// change to it is a deliberate, visible contract change rather than a silent
// drift that only shows up as Explorer integration going dark.
#include "ShellDispatcherPipe.h"
#include <catch.hpp>

#include <string>

TEST_CASE("ShellDispatcherPipe::composeName() pins the wire format")
{
    // \\.\pipe\<stem>_<user SID>_<session id>
    CHECK(ShellDispatcherPipe::composeName(L"S-1-5-21-1-2-3-1001", 1) ==
          std::wstring(L"\\\\.\\pipe\\MEGAprivacyMEGAsync_S-1-5-21-1-2-3-1001_1"));
    CHECK(ShellDispatcherPipe::composeName(L"S-1-5-21-1-2-3-1001", 12) ==
          std::wstring(L"\\\\.\\pipe\\MEGAprivacyMEGAsync_S-1-5-21-1-2-3-1001_12"));
}

TEST_CASE("ShellDispatcherPipe::composeName() fails closed without a user SID")
{
    // Never fall back to a shared, unscoped name.
    CHECK(ShellDispatcherPipe::composeName(L"", 1).empty());
}

TEST_CASE("ShellDispatcherPipe::baseName() keeps the historical stem")
{
    CHECK(std::wstring(ShellDispatcherPipe::baseName()) == L"MEGAprivacyMEGAsync");
}

TEST_CASE("ShellDispatcherPipe::name() agrees with the pure composer")
{
    const std::wstring sid = ShellDispatcherPipe::currentUserSid();
    REQUIRE_FALSE(sid.empty());

    DWORD sessionId = 0;
    REQUIRE(ShellDispatcherPipe::currentSessionId(sessionId));

    CHECK(ShellDispatcherPipe::name() == ShellDispatcherPipe::composeName(sid, sessionId));
}
