#pragma once
#include "framework.h"

enum CounterState
{
    NotSet,
    Set
};

class SharedState
{
public:
    SharedState();
    ~SharedState();

    CounterState GetState(const wstring caller) const;
    void SetState(const wstring caller, const CounterState state);

private:
    HANDLE hFileMapping = 0;
    HANDLE hMutex = 0;
    CounterState* pState = 0;
};
