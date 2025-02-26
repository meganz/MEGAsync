#include "SharedState.h"

SharedState::SharedState()
{
    // Create or open the shared memory mapped file
    hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE,
                                     NULL,
                                     PAGE_READWRITE,
                                     0,
                                     sizeof(int),
                                     L"Local\\BaseNppExplorerCommandHandlerSharedStateMemory");
    if (hFileMapping == NULL)
    {
        return;
    }

    // Create a mutex to synchronize access to the shared memory
    hMutex = CreateMutex(NULL, FALSE, L"Local\\BaseNppExplorerCommandHandlerSharedStateMutex");
    if (hMutex == NULL)
    {
        CloseHandle(hFileMapping);
        return;
    }

    // Map the shared memory into the current process's address space
    pState =
        (CounterState*)MapViewOfFile(hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(CounterState));
    if (pState == NULL)
    {
        MessageBox(NULL, L"Failed to map shared memory", L"SharedState", MB_OK | MB_ICONERROR);
        CloseHandle(hMutex);
        CloseHandle(hFileMapping);
        return;
    }
}

SharedState::~SharedState()
{
    // Unmap the shared memory from the current process's address space
    UnmapViewOfFile(pState);

    // Close the mutex and the shared memory mapped file
    CloseHandle(hMutex);
    CloseHandle(hFileMapping);
}

CounterState SharedState::GetState(const std::wstring caller) const
{
    WaitForSingleObject(hMutex, INFINITE);
    CounterState value = *pState;
    ReleaseMutex(hMutex);

    return value;
}

void SharedState::SetState(const std::wstring caller, const CounterState state)
{
    WaitForSingleObject(hMutex, INFINITE);
    *pState = state;
    ReleaseMutex(hMutex);
}
