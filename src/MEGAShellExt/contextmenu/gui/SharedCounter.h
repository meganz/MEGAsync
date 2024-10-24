#pragma once
#include "framework.h"

class SharedCounter
{
public:
    SharedCounter();
    ~SharedCounter();

    int GetValue() const;

private:
    HANDLE hFileMapping;
    HANDLE hMutex;
    int* pCounter;
    int localValue;
};
