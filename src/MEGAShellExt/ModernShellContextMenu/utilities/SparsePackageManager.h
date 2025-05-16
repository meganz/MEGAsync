#ifndef SPARSEPACKAGE_H
#define SPARSEPACKAGE_H

#include "framework.h"
#include "pplawait.h"

using namespace winrt::Windows::Management::Deployment;

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
    static concurrency::task<HRESULT> addPackage(const winrt::Windows::Foundation::Uri& packageUri,
                                                 const AddPackageOptions& options);
};

// Exported function called from MEGA Desktop App
// in order to install the sparse package after auto-updating the app
STDAPI installSparsePackage();

#endif
