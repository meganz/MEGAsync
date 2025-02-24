#include "DesktopManager.h"

#include "Laf.h"
#include "megaapi.h"

#include <QString>
#include <winrt/Windows.ApplicationModel.Activation.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/windows.management.deployment.h>
#include <winrt/windows.management.h>
#include <winrt/Windows.UI.Shell.h>
#include <winrt/windows.ui.startscreen.h>

using namespace std::string_literals;

namespace winrt
{
using namespace Windows::ApplicationModel;
using namespace Windows::UI::Shell;
}

namespace
{
const std::wstring lafFeature = L"com.microsoft.windows.taskbar.pin";
const std::wstring lafAttestationFirstPart = L" has registered their use of ";
const std::wstring lafAttestationSecondPart = L" with Microsoft and agrees to the terms of use.";
const std::wstring publisherId = L"x4jagz123a1p0";
}

bool DesktopManager::IsDesktopAppPinningSupported()
{
    try
    {
        return !!winrt::try_get_activation_factory<
            winrt::TaskbarManager,
            winrt::ITaskbarManagerDesktopAppSupportStatics>();
    }
    catch (const winrt::hresult_error& error)
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR,
                           QString::fromWCharArray(error.message().c_str()).toStdString().c_str());
    }

    return false;
}

bool DesktopManager::TryUnlockPinningLaf()
{
    try
    {
        auto attestation =
            publisherId + lafAttestationFirstPart + lafFeature + lafAttestationSecondPart;

        auto limitedAccessFeatureRequestResult =
            winrt::LimitedAccessFeatures::TryUnlockFeature(lafFeature.data(),
                                                           LAF_TOKEN.data(),
                                                           attestation.data());

        auto unlockStatus = limitedAccessFeatureRequestResult.Status();
        return ((unlockStatus == winrt::LimitedAccessFeatureStatus::Available) ||
                (unlockStatus == winrt::LimitedAccessFeatureStatus::AvailableWithoutToken));
    }
    catch (const winrt::hresult_error& error)
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR,
                           QString::fromWCharArray(error.message().c_str()).toStdString().c_str());
    }

    return false;
}

bool DesktopManager::requestPinToTaskBar()
{
    try
    {
        if (IsDesktopAppPinningSupported() && TryUnlockPinningLaf())
        {
            auto taskbarManager = winrt::Windows::UI::Shell::TaskbarManager::GetDefault();

            if (taskbarManager.IsSupported() && taskbarManager.IsPinningAllowed() &&
                !taskbarManager.IsCurrentAppPinnedAsync().get())
            {
                taskbarManager.RequestPinCurrentAppAsync();
                return true;
            }
        }
    }
    catch (const winrt::hresult_error& error)
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR,
                           QString::fromWCharArray(error.message().c_str()).toStdString().c_str());
    }
    catch (...)
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, "unknown error.");
    }

    return false;
}
