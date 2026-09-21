# src/common/win

Wire contracts shared by the MEGAsync app and the Windows Explorer shell
extension. These are two independent binaries with no common link library;
what lives here is the one thing they must compute identically without
talking to each other.

Rules for anything in this directory:

- Header-only. No `.cpp`, nothing that links, nothing that needs a common
  object file.
- Win32 only. No Qt, no SDK, no C++ dependencies beyond the standard library.
  The shell extension is loaded into `explorer.exe` and must stay
  dependency-free.
- Contract only. A file belongs here when both binaries must produce the same
  bytes from it (a pipe name, a message layout, an on-disk format). Helpers
  that merely happen to be useful on both sides do not: duplicate those on
  each side instead.
- Any change to a contract is a protocol change. Keep the unit test that pins
  the format (see `ShellDispatcherPipeTests.cpp` in MEGAAutoTests) in step
  with it, and think about the upgrade path: the extension already loaded in
  Explorer keeps running the old build until Explorer restarts.

Current contents:

- `ShellDispatcherPipe.h`: per-user, per-session name of the shell dispatcher
  named pipe (SNC-6808).
