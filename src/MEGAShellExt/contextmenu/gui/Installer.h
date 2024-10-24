#pragma once
#include "framework.h"

bool IsWindows11Installation();

HRESULT RegisterSparsePackage();
HRESULT UnregisterSparsePackage();

HRESULT InstallSparsePackage();
HRESULT UninstallSparsePackage();

void EnsureRegistrationOnCurrentUser();

STDAPI CleanupDll();
