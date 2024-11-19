#ifndef INSTALLER_H
#define INSTALLER_H

#include "framework.h"

bool IsWindows11Installation();

HRESULT RegisterSparsePackage();
HRESULT UnregisterSparsePackage();

HRESULT InstallSparsePackage();
HRESULT UninstallSparsePackage();

void EnsureRegistrationOnCurrentUser();

#endif
