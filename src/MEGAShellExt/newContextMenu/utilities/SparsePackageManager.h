#ifndef SPARSEPACKAGE_H
#define SPARSEPACKAGE_H

#include "framework.h"

class SparsePackageManager
{
public:
    static void findSparsePackage();
    static HRESULT RegisterSparsePackage();
    static HRESULT UnregisterSparsePackage();

    enum MODIFY_TYPE
    {
        INSTALL,
        UNINSTALL
    };

    static HRESULT modifySparsePackage(MODIFY_TYPE type);

    static void EnsureRegistrationOnCurrentUser();

private:
    SparsePackageManager() = default;
    static winrt::Windows::ApplicationModel::Package mSparsePackage;

    static void EnsureRegistrationOnCurrentUserWorker();
    static void ReRegisterSparsePackage();
};

#endif
