#include "wintoastlib.h"
#include "Preferences/Preferences.h"
#include <VersionHelpers.h>
#include <memory>
#include <assert.h>
#include <QMessageBox>
#include <QPushButton>
#include <QCoreApplication>

#pragma comment(lib,"shlwapi")
#pragma comment(lib,"user32")

#ifdef NDEBUG
    #define DEBUG_MSG(str) do { } while ( false )
 #else
    #define DEBUG_MSG(str) do { std::wcout << str << std::endl; } while( false )
#endif

// Thanks: https://stackoverflow.com/a/36545162/4297146

typedef LONG NTSTATUS, *PNTSTATUS;

#define STATUS_SUCCESS (0x00000000)

typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

RTL_OSVERSIONINFOW GetRealOSVersion() {
	HMODULE hMod = ::GetModuleHandleW(L"ntdll.dll");
	if (hMod) {
		RtlGetVersionPtr fxPtr = (RtlGetVersionPtr)::GetProcAddress(hMod, "RtlGetVersion");
		if (fxPtr != nullptr) {
			RTL_OSVERSIONINFOW rovi = { 0 };
			rovi.dwOSVersionInfoSize = sizeof(rovi);
			if (STATUS_SUCCESS == fxPtr(&rovi)) {
				return rovi;
			}
		}
	}
	RTL_OSVERSIONINFOW rovi = { 0 };
	return rovi;
}

// Quickstart: Handling toast activations from Win32 apps in Windows 10
// https://blogs.msdn.microsoft.com/tiles_and_toasts/2015/10/16/quickstart-handling-toast-activations-from-win32-apps-in-windows-10/

using namespace WinToastLib;
namespace DllImporter {

    // Function load a function from library
    template <typename Function>
	HRESULT loadFunctionFromLibrary(HINSTANCE library, LPCSTR name, Function &func) {
		if (!library) {
			return E_INVALIDARG;
		}
        func = reinterpret_cast<Function>(GetProcAddress(library, name));
        return (func != nullptr) ? S_OK : E_FAIL;
    }

    typedef HRESULT(FAR STDAPICALLTYPE *f_SetCurrentProcessExplicitAppUserModelID)(__in PCWSTR AppID);
    typedef HRESULT(FAR STDAPICALLTYPE *f_PropVariantToString)(_In_ REFPROPVARIANT propvar, _Out_writes_(cch) PWSTR psz, _In_ UINT cch);
    typedef HRESULT(FAR STDAPICALLTYPE *f_RoGetActivationFactory)(_In_ HSTRING activatableClassId, _In_ REFIID iid, _COM_Outptr_ void ** factory);
    typedef HRESULT(FAR STDAPICALLTYPE *f_WindowsCreateStringReference)(_In_reads_opt_(length + 1) PCWSTR sourceString, UINT32 length, _Out_ HSTRING_HEADER * hstringHeader, _Outptr_result_maybenull_ _Result_nullonfailure_ HSTRING * string);
    typedef PCWSTR(FAR STDAPICALLTYPE *f_WindowsGetStringRawBuffer)(_In_ HSTRING string, _Out_ UINT32 *length);
    typedef HRESULT(FAR STDAPICALLTYPE *f_WindowsDeleteString)(_In_opt_ HSTRING string);

    static f_SetCurrentProcessExplicitAppUserModelID    SetCurrentProcessExplicitAppUserModelID;
    static f_PropVariantToString                        PropVariantToString;
    static f_RoGetActivationFactory                     RoGetActivationFactory;
    static f_WindowsCreateStringReference               WindowsCreateStringReference;
    static f_WindowsGetStringRawBuffer                  WindowsGetStringRawBuffer;
    static f_WindowsDeleteString                        WindowsDeleteString;


    template<class T>
    _Check_return_ __inline HRESULT _1_GetActivationFactory(_In_ HSTRING activatableClassId, _COM_Outptr_ T** factory) {
        return RoGetActivationFactory(activatableClassId, IID_INS_ARGS(factory));
    }

    template<typename T>
#if NTDDI_VERSION < NTDDI_VISTA
    inline HRESULT Wrap_GetActivationFactory(_In_ HSTRING activatableClassId, _Inout_ T** factory) throw() {
        return _1_GetActivationFactory(activatableClassId, factory);
    }
#else
    inline HRESULT Wrap_GetActivationFactory(_In_ HSTRING activatableClassId, _Inout_ Details::ComPtrRef<T> factory) throw() {
        return _1_GetActivationFactory(activatableClassId, factory.ReleaseAndGetAddressOf());
    }
#endif

    inline HRESULT initialize() {
        HINSTANCE LibShell32 = LoadLibraryW(L"SHELL32.DLL");
        HRESULT hr = loadFunctionFromLibrary(LibShell32, "SetCurrentProcessExplicitAppUserModelID", SetCurrentProcessExplicitAppUserModelID);
        if (SUCCEEDED(hr)) {
            HINSTANCE LibPropSys = LoadLibraryW(L"PROPSYS.DLL");
            hr = loadFunctionFromLibrary(LibPropSys, "PropVariantToString", PropVariantToString);
            if (SUCCEEDED(hr)) {
                HINSTANCE LibComBase = LoadLibraryW(L"COMBASE.DLL");
                const bool succeded = SUCCEEDED(loadFunctionFromLibrary(LibComBase, "RoGetActivationFactory", RoGetActivationFactory))
										&& SUCCEEDED(loadFunctionFromLibrary(LibComBase, "WindowsCreateStringReference", WindowsCreateStringReference))
										&& SUCCEEDED(loadFunctionFromLibrary(LibComBase, "WindowsGetStringRawBuffer", WindowsGetStringRawBuffer))
										&& SUCCEEDED(loadFunctionFromLibrary(LibComBase, "WindowsDeleteString", WindowsDeleteString));
				return succeded ? S_OK : E_FAIL;
            }
        }
        return hr;
    }
}

class WinToastStringWrapper {
public:
    WinToastStringWrapper(_In_reads_(length) PCWSTR stringRef, _In_ UINT32 length) throw() {
        HRESULT hr = DllImporter::WindowsCreateStringReference(stringRef, length, &_header, &_hstring);
        if (!SUCCEEDED(hr)) {
            RaiseException(static_cast<DWORD>(STATUS_INVALID_PARAMETER), EXCEPTION_NONCONTINUABLE, 0, nullptr);
        }
    }
    WinToastStringWrapper(_In_ const std::wstring &stringRef) throw() {
        HRESULT hr = DllImporter::WindowsCreateStringReference(stringRef.c_str(), static_cast<UINT32>(stringRef.length()), &_header, &_hstring);
        if (FAILED(hr)) {
            RaiseException(static_cast<DWORD>(STATUS_INVALID_PARAMETER), EXCEPTION_NONCONTINUABLE, 0, nullptr);
        }
    }
    ~WinToastStringWrapper() {
        DllImporter::WindowsDeleteString(_hstring);
    }
    inline HSTRING Get() const throw() { return _hstring; }
private:
    HSTRING _hstring;
    HSTRING_HEADER _header;

};

class MyDateTime : public IReference<DateTime>
{
protected:
    DateTime _dateTime;

public:
    static INT64 Now() {
        FILETIME now;
        GetSystemTimeAsFileTime(&now);
        return ((((INT64)now.dwHighDateTime) << 32) | now.dwLowDateTime);
    }

    MyDateTime(DateTime dateTime) : _dateTime(dateTime) {}

    MyDateTime(INT64 millisecondsFromNow) {
        _dateTime.UniversalTime = Now() + millisecondsFromNow * 10000;
    }

    operator INT64() {
        return _dateTime.UniversalTime;
    }

    HRESULT STDMETHODCALLTYPE get_Value(DateTime *dateTime) {
        *dateTime = _dateTime;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(const IID& riid, void** ppvObject) {
        if (!ppvObject) {
            return E_POINTER;
        }
        if (riid == __uuidof(IUnknown) || riid == __uuidof(IReference<DateTime>)) {
            *ppvObject = static_cast<IUnknown*>(static_cast<IReference<DateTime>*>(this));
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE Release() {
        return 1;
    }

    ULONG STDMETHODCALLTYPE AddRef() {
        return 2;
    }

    HRESULT STDMETHODCALLTYPE GetIids(ULONG*, IID**) {
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE GetRuntimeClassName(HSTRING*) {
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE GetTrustLevel(TrustLevel*) {
        return E_NOTIMPL;
    }
};

namespace Util {
    enum UserScope
    {
        CURRENT_USER,
        ALL_USERS
    };

    inline HRESULT defaultExecutablePath(_In_ WCHAR* path, _In_ DWORD nSize = MAX_PATH) {
        DWORD written = GetModuleFileNameExW(GetCurrentProcess(), nullptr, path, nSize);
        DEBUG_MSG("Default executable path: " << path);
        return (written > 0) ? S_OK : E_FAIL;
    }


    std::wstring getShellLinkPath(const std::wstring& appname, UserScope scope)
    {
        std::wstring link;
        wchar_t programsPath[MAX_PATH];
        auto csidl = scope == CURRENT_USER ? CSIDL_PROGRAMS : CSIDL_COMMON_PROGRAMS;

        if (SUCCEEDED(SHGetSpecialFolderPathW(nullptr, programsPath, csidl, FALSE)))
        {
            link = programsPath + std::wstring(L"\\") + appname + L"\\" + appname + DEFAULT_LINK_FORMAT;
        }

        return link;
    }

    std::wstring findShellLink(const std::wstring& appname)
    {
        // Get "Start Menu/Programs" path for current user
        std::wstring link = getShellLinkPath(appname, CURRENT_USER);

        if (!PathFileExistsW(link.c_str()))
        {
            // Get "Start Menu/Programs" path for all users
            link = getShellLinkPath(appname, ALL_USERS);

            if (!PathFileExistsW(link.c_str()))  link.clear();
        }

        return link;
    }

    inline PCWSTR AsString(ComPtr<IXmlDocument> &xmlDocument) {
        HSTRING xml;
        ComPtr<IXmlNodeSerializer> ser;
        HRESULT hr = xmlDocument.As<IXmlNodeSerializer>(&ser);
        hr = ser->GetXml(&xml);
        if (SUCCEEDED(hr))
            return DllImporter::WindowsGetStringRawBuffer(xml, NULL);
        return NULL;
    }

    inline PCWSTR AsString(HSTRING hstring) {
		return DllImporter::WindowsGetStringRawBuffer(hstring, NULL);
    }

    inline HRESULT setNodeStringValue(const std::wstring& string, IXmlNode *node, IXmlDocument *xml) {
        ComPtr<IXmlText> textNode;
        HRESULT hr = xml->CreateTextNode( WinToastStringWrapper(string).Get(), &textNode);
        if (SUCCEEDED(hr)) {
            ComPtr<IXmlNode> stringNode;
            hr = textNode.As(&stringNode);
            if (SUCCEEDED(hr)) {
                ComPtr<IXmlNode> appendedChild;
                hr = node->AppendChild(stringNode.Get(), &appendedChild);
            }
        }
        return hr;
    }

    inline HRESULT setEventHandlers(_In_ IToastNotification* notification, _In_ std::shared_ptr<IWinToastHandler> eventHandler, _In_ INT64 expirationTime,
                                    EventRegistrationToken &activatedToken, EventRegistrationToken &dismissedToken, EventRegistrationToken &failedToken) {
        HRESULT hr = notification->add_Activated(
                #if NTDDI_VERSION < NTDDI_VISTA
                    MegaCompatToastCallbackWrapper<IInspectable, IInspectable>(
                #else
                    Callback < Implements < RuntimeClassFlags<ClassicCom>,
                    ITypedEventHandler<ToastNotification*, IInspectable* >> >(
                #endif
                    [eventHandler](IToastNotification*, IInspectable* inspectable)
                {
                    IToastActivatedEventArgs *activatedEventArgs;
                    HRESULT hr = inspectable->QueryInterface(&activatedEventArgs);
                    if (SUCCEEDED(hr)) {
                        HSTRING argumentsHandle;
                        hr = activatedEventArgs->get_Arguments(&argumentsHandle);
                        if (SUCCEEDED(hr)) {
                            PCWSTR arguments = Util::AsString(argumentsHandle);
                            if (arguments && *arguments) {
                                eventHandler->toastActivated((int)wcstol(arguments, NULL, 10));
                                return S_OK;
                            }
                        }
                    }
                    eventHandler->toastActivated();
                    return S_OK;
                }).Get(), &activatedToken);

        if (SUCCEEDED(hr)) {
            hr = notification->add_Dismissed(
                #if NTDDI_VERSION < NTDDI_VISTA
                     MegaCompatToastCallbackWrapper<ToastDismissedEventArgs, IToastDismissedEventArgs>(
                #else
                     Callback < Implements < RuntimeClassFlags<ClassicCom>,
                     ITypedEventHandler<ToastNotification*, ToastDismissedEventArgs* >> >(
                #endif
                     [eventHandler, expirationTime](IToastNotification*, IToastDismissedEventArgs* e)
                 {
                     ToastDismissalReason reason;
                     if (SUCCEEDED(e->get_Reason(&reason)))
                     {
                         if (reason == ToastDismissalReason_UserCanceled && expirationTime && MyDateTime::Now() >= expirationTime)
                            reason = ToastDismissalReason_TimedOut;
                         eventHandler->toastDismissed(static_cast<IWinToastHandler::WinToastDismissalReason>(reason));
                     }
                     return S_OK;
                 }).Get(), &dismissedToken);
            if (SUCCEEDED(hr)) {
                hr = notification->add_Failed(
                #if NTDDI_VERSION < NTDDI_VISTA
                    MegaCompatToastCallbackWrapper<ToastFailedEventArgs, IToastFailedEventArgs>(
                #else
                    Callback < Implements < RuntimeClassFlags<ClassicCom>,
                    ITypedEventHandler<ToastNotification*, ToastFailedEventArgs* >> >(
                #endif
                    [eventHandler](IToastNotification*, IToastFailedEventArgs*)
                {
                    eventHandler->toastFailed();
                    return S_OK;
                }).Get(), &failedToken);
            }
            else
            {
                notification->remove_Dismissed(dismissedToken);
            }
        }
        else
        {
            notification->remove_Activated(activatedToken);
        }
        return hr;
    }

    inline HRESULT addAttribute(_In_ IXmlDocument *xml, const std::wstring &name, IXmlNamedNodeMap *attributeMap) {
        ComPtr<ABI::Windows::Data::Xml::Dom::IXmlAttribute> srcAttribute;
        HRESULT hr = xml->CreateAttribute(WinToastStringWrapper(name).Get(), &srcAttribute);
        if (SUCCEEDED(hr)) {
            ComPtr<IXmlNode> node;
            hr = srcAttribute.As(&node);
            if (SUCCEEDED(hr)) {
                ComPtr<IXmlNode> pNode;
                hr = attributeMap->SetNamedItem(node.Get(), &pNode);
            }
        }
        return hr;
    }

    inline HRESULT createElement(_In_ IXmlDocument *xml, _In_ const std::wstring& root_node, _In_ const std::wstring& element_name, _In_ const std::vector<std::wstring>& attribute_names) {
        ComPtr<IXmlNodeList> rootList;
        HRESULT hr = xml->GetElementsByTagName(WinToastStringWrapper(root_node).Get(), &rootList);
        if (SUCCEEDED(hr)) {
            ComPtr<IXmlNode> root;
            hr = rootList->Item(0, &root);
            if (SUCCEEDED(hr)) {
                ComPtr<ABI::Windows::Data::Xml::Dom::IXmlElement> audioElement;
                hr = xml->CreateElement(WinToastStringWrapper(element_name).Get(), &audioElement);
                if (SUCCEEDED(hr)) {
                    ComPtr<IXmlNode> audioNodeTmp;
                    hr = audioElement.As(&audioNodeTmp);
                    if (SUCCEEDED(hr)) {
                        ComPtr<IXmlNode> audioNode;
                        hr = root->AppendChild(audioNodeTmp.Get(), &audioNode);
                        if (SUCCEEDED(hr)) {
                            ComPtr<IXmlNamedNodeMap> attributes;
                            hr = audioNode->get_Attributes(&attributes);
                            if (SUCCEEDED(hr)) {
                                for (auto it : attribute_names) {
                                    hr = addAttribute(xml, it, attributes.Get());
                                }
                            }
                        }
                    }
                }
            }
        }
        return hr;
    }
}

WinToast* WinToast::instance() {
    static WinToast instance;
    return &instance;
}

WinToast::WinToast() :
    _isInitialized(false),
    _hasCoInitialized(false)
{
	if (!isCompatible()) {
		DEBUG_MSG(L"Warning: Your system is not compatible with this library ");
	}
}

WinToast::~WinToast() {
    if (_hasCoInitialized) {
        CoUninitialize();
    }
}

void WinToast::setAppName(_In_ const std::wstring& appName) {
    _appName = appName;
}


void WinToast::setAppUserModelId(_In_ const std::wstring& aumi) {
    _aumi = aumi;
    DEBUG_MSG(L"Default App User Model Id: " << _aumi.c_str());
}

bool WinToast::isCompatible() {
	DllImporter::initialize();
	return !((DllImporter::SetCurrentProcessExplicitAppUserModelID == nullptr)
		|| (DllImporter::PropVariantToString == nullptr)
		|| (DllImporter::RoGetActivationFactory == nullptr)
		|| (DllImporter::WindowsCreateStringReference == nullptr)
		|| (DllImporter::WindowsDeleteString == nullptr));
}

bool WinToastLib::WinToast::supportModernFeatures() {
	RTL_OSVERSIONINFOW tmp = GetRealOSVersion();
	return tmp.dwMajorVersion > 6;

}
std::wstring WinToast::configureAUMI(_In_ const std::wstring &companyName,
                                               _In_ const std::wstring &productName,
                                               _In_ const std::wstring &subProduct,
                                               _In_ const std::wstring &versionInformation)
{
    std::wstring aumi = companyName;
    aumi += L"." + productName;
    if (subProduct.length() > 0) {
        aumi += L"." + subProduct;
        if (versionInformation.length() > 0) {
            aumi += L"." + versionInformation;
        }
    }

    if (aumi.length() > SCHAR_MAX) {
        DEBUG_MSG("Error: max size allowed for AUMI: 128 characters.");
    }
    return aumi;
}

bool getCreateLinkUserPreference(const std::wstring &appName)
{
    // Load user preference
    auto p = Preferences::instance();
    if (p->neverCreateLink())
        return false;

    // Create user dialog
    const QString& title = QString::fromWCharArray(appName.c_str());
    QMessageBox msgbox(QMessageBox::Question, title,
        QCoreApplication::translate("WinToastLib", "%1 did not find a valid link in Start Menu. "
            "Not having a link may prevent the correct functioning of desktop notifications.\n\n"
            "Do you want to create one?").arg(title));
    auto yesButton = msgbox.addButton(QCoreApplication::translate("WinToastLib", "Yes (recommended)"), QMessageBox::YesRole);
    msgbox.addButton(QCoreApplication::translate("WinToastLib", "No"), QMessageBox::NoRole);
    auto neverButton = msgbox.addButton(QCoreApplication::translate("WinToastLib", "No (never ask again)"), QMessageBox::NoRole);

    msgbox.exec();
    auto clickedButton = msgbox.clickedButton();

    // Evaluate user option
    if (clickedButton == neverButton)
        p->setNeverCreateLink(true);

    return clickedButton == yesButton;
}

enum WinToast::ShortcutResult WinToast::createShortcut() {
    if (_aumi.empty() || _appName.empty()) {
        DEBUG_MSG(L"Error: App User Model Id or Appname is empty!");
        return SHORTCUT_MISSING_PARAMETERS;
    }

    if (!isCompatible()) {
        DEBUG_MSG(L"Your OS is not compatible with this library! =(");
        return SHORTCUT_INCOMPATIBLE_OS;
    }

    if (!_hasCoInitialized) {
        HRESULT initHr = CoInitializeEx(NULL, COINIT::COINIT_MULTITHREADED);
        if (initHr != RPC_E_CHANGED_MODE) {
            if (FAILED(initHr) && initHr != S_FALSE) {
                DEBUG_MSG(L"Error on COM library initialization!");
                return SHORTCUT_COM_INIT_FAILURE;
            }
            else {
                _hasCoInitialized = true;
            }
        }
    }

    bool wasChanged;
    HRESULT hr = validateShellLinkHelper(wasChanged);
    if (SUCCEEDED(hr))
        return wasChanged ? SHORTCUT_WAS_CHANGED : SHORTCUT_UNCHANGED;

    // If shortcut was not found, get user preference for creating one
    bool create = getCreateLinkUserPreference(_appName);
    if (!create)
        return SHORTCUT_CREATE_SKIPPED;

    hr = createShellLinkHelper();
    if (SUCCEEDED(hr))
        return SHORTCUT_WAS_CREATED;
    return SHORTCUT_CREATE_FAILED;
}

bool WinToast::initialize() {
    _isInitialized = false;

    if (createShortcut() < 0 && !IsWindows10OrGreater()) // for Win10 allow fallback notifications when toasts don't work
        return false;

    if (FAILED(DllImporter::SetCurrentProcessExplicitAppUserModelID(_aumi.c_str()))) {
        DEBUG_MSG(L"Error while attaching the AUMI to the current proccess =(");
        return false;
    }

    _isInitialized = true;
    return _isInitialized;
}

HRESULT	WinToast::validateShellLinkHelper(_Out_ bool& wasChanged) {
    const std::wstring& path = Util::findShellLink(_appName);
    if (path.empty())
    {
#ifndef NDEBUG
        const std::wstring& defaultPath = Util::getShellLinkPath(_appName, Util::CURRENT_USER);
        DEBUG_MSG("Error, shell link not found. Try to create a new one in: " << defaultPath.c_str());
#endif
        return E_FAIL;
    }

    // Let's load the file as shell link to validate.
    // - Create a shell link
    // - Create a persistant file
    // - Load the path as data for the persistant file
    // - Read the property AUMI and validate with the current
    // - Review if AUMI is equal.
    ComPtr<IShellLink> shellLink;
    HRESULT hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&shellLink));
    if (SUCCEEDED(hr)) {
        ComPtr<IPersistFile> persistFile;
        hr = shellLink.As(&persistFile);
        if (SUCCEEDED(hr)) {
            hr = persistFile->Load(path.c_str(), STGM_READWRITE);
            if (SUCCEEDED(hr)) {
                ComPtr<IPropertyStore> propertyStore;
                hr = shellLink.As(&propertyStore);
                if (SUCCEEDED(hr)) {
                    PROPVARIANT appIdPropVar;
                    hr = propertyStore->GetValue(PKEY_AppUserModel_ID, &appIdPropVar);
                    if (SUCCEEDED(hr)) {
                        WCHAR AUMI[MAX_PATH];
                        hr = DllImporter::PropVariantToString(appIdPropVar, AUMI, MAX_PATH);
                        wasChanged = false;
                        if (FAILED(hr) || _aumi != AUMI) {
                            // AUMI Changed for the same app, let's update the current value! =)
                            wasChanged = true;
                            PropVariantClear(&appIdPropVar);
                            hr = InitPropVariantFromString(_aumi.c_str(), &appIdPropVar);
                            if (SUCCEEDED(hr)) {
                                hr = propertyStore->SetValue(PKEY_AppUserModel_ID, appIdPropVar);
                                if (SUCCEEDED(hr)) {
                                    hr = propertyStore->Commit();
                                    if (SUCCEEDED(hr) && SUCCEEDED(persistFile->IsDirty())) {
                                        hr = persistFile->Save(path.c_str(), TRUE);
                                    }
                                }
                            }
                        }
                        PropVariantClear(&appIdPropVar);
                    }
                }
            }
        }
    }
    return hr;
}



HRESULT	WinToast::createShellLinkHelper() {
	WCHAR   exePath[MAX_PATH]{L'\0'};
    const std::wstring& slPath = getShellLinkPath(_appName, Util::CURRENT_USER);
    Util::defaultExecutablePath(exePath);
    ComPtr<IShellLinkW> shellLink;
    HRESULT hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&shellLink));
    if (SUCCEEDED(hr)) {
        hr = shellLink->SetPath(exePath);
        if (SUCCEEDED(hr)) {
            hr = shellLink->SetArguments(L"");
            if (SUCCEEDED(hr)) {
                hr = shellLink->SetWorkingDirectory(exePath);
                if (SUCCEEDED(hr)) {
                    ComPtr<IPropertyStore> propertyStore;
                    hr = shellLink.As(&propertyStore);
                    if (SUCCEEDED(hr)) {
                        PROPVARIANT appIdPropVar;
                        hr = InitPropVariantFromString(_aumi.c_str(), &appIdPropVar);
                        if (SUCCEEDED(hr)) {
                            hr = propertyStore->SetValue(PKEY_AppUserModel_ID, appIdPropVar);
                            if (SUCCEEDED(hr)) {
                                hr = propertyStore->Commit();
                                if (SUCCEEDED(hr)) {
                                    ComPtr<IPersistFile> persistFile;
                                    hr = shellLink.As(&persistFile);
                                    if (SUCCEEDED(hr)) {
                                        hr = persistFile->Save(slPath.c_str(), TRUE);

                                        // attempt to create the full path if some dir was missing, then try again
                                        if (hr == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND)) {
                                            std::wstring linkDir(slPath);
                                            auto pos = linkDir.find_last_of('\\');
                                            if (pos != std::wstring::npos) {
                                                linkDir.erase(pos);
                                            }
                                            int err = SHCreateDirectoryEx(nullptr, linkDir.c_str(), nullptr);
                                            if (err == ERROR_SUCCESS) {
                                                hr = persistFile->Save(slPath.c_str(), TRUE);
                                            }
                                        }
                                    }
                                }
                            }
                            PropVariantClear(&appIdPropVar);
                        }
                    }
                }
            }
        }
    }
    return hr;
}

INT64 WinToast::showToast(_In_ const WinToastTemplate& toast, _In_  std::shared_ptr<IWinToastHandler> handler)  {
    INT64 id = -1;
    if (!isInitialized()) {
        DEBUG_MSG("Error when launching the toast. WinToast is not initialized =(");
        return id;
    }
    if (!handler) {
        DEBUG_MSG("Error when launching the toast. handler cannot be null.");
        return id;
    }

    ComPtr<IToastNotificationManagerStatics> notificationManager;
    HRESULT hr = DllImporter::Wrap_GetActivationFactory(WinToastStringWrapper(RuntimeClass_Windows_UI_Notifications_ToastNotificationManager).Get(), &notificationManager);
    if (SUCCEEDED(hr)) {
        ComPtr<IToastNotifier> notifier;
        hr = notificationManager->CreateToastNotifierWithId(WinToastStringWrapper(_aumi).Get(), &notifier);
        if (SUCCEEDED(hr)) {
            ComPtr<IToastNotificationFactory> notificationFactory;
            hr = DllImporter::Wrap_GetActivationFactory(WinToastStringWrapper(RuntimeClass_Windows_UI_Notifications_ToastNotification).Get(), &notificationFactory);
            if (SUCCEEDED(hr)) {
				ComPtr<IXmlDocument> xmlDocument;
				HRESULT hr = notificationManager->GetTemplateContent(ToastTemplateType(toast.type()), &xmlDocument);
                if (SUCCEEDED(hr)) {
                    const int fieldsCount = toast.textFieldsCount();
                    for (int i = 0; i < fieldsCount && SUCCEEDED(hr); i++) {
                        hr = setTextFieldHelper(xmlDocument.Get(), toast.textField(WinToastTemplate::TextField(i)), i);
                    }

                    // Modern feature are supported Windows > Windows 10
                    if (SUCCEEDED(hr) && supportModernFeatures()) {

                        // Note that we do this *after* using toast.textFieldsCount() to
                        // iterate/fill the template's text fields, since we're adding yet another text field.
                        if (SUCCEEDED(hr)
                            && !toast.attributionText().empty()) {
                            hr = setAttributionTextFieldHelper(xmlDocument.Get(), toast.attributionText());
                        }

                        const int actionsCount = toast.actionsCount();
                        WCHAR buf[12];
                        for (int i = 0; i < actionsCount && SUCCEEDED(hr); i++) {
                            _snwprintf_s(buf, sizeof(buf) / sizeof(*buf), _TRUNCATE, L"%d", i);
                            hr = addActionHelper(xmlDocument.Get(), toast.actionLabel(i), buf);
                        }

                        if (SUCCEEDED(hr)) {
                            hr = (toast.audioPath().empty() && toast.audioOption() == WinToastTemplate::Default)
                                ? hr : setAudioFieldHelper(xmlDocument.Get(), toast.audioPath(), toast.audioOption());
                        }
                    } else {
                        DEBUG_MSG("Modern features (Actions/Sounds/Attributes) not supported in this os version");
                    }

                    if (SUCCEEDED(hr)) {
                        hr = toast.hasImage() ? setImageFieldHelper(xmlDocument.Get(), toast.imagePath()) : hr;
                        if (SUCCEEDED(hr)) {
                            ComPtr<IToastNotification> notification;
                            hr = notificationFactory->CreateToastNotification(xmlDocument.Get(), &notification);
                            if (SUCCEEDED(hr)) {
                                INT64 expiration = 0, relativeExpiration = toast.expiration();
                                if (relativeExpiration > 0) {
                                    MyDateTime expirationDateTime(relativeExpiration);
                                    expiration = expirationDateTime;
                                    hr = notification->put_ExpirationTime(&expirationDateTime);
                                }

                                if (SUCCEEDED(hr)) {
                                    GUID guid;
                                    hr = CoCreateGuid(&guid);
                                    if (SUCCEEDED(hr)) {
                                        id = guid.Data1;
                                        _buffer[id] = notification;
                                        hr = Util::setEventHandlers(notification.Get(), handler, expiration,
                                                                    _activationTokens[id], _dismissedTokens[id], _failedTokens[id]);
                                        if (SUCCEEDED(hr)) {
                                            DEBUG_MSG("xml: " << Util::AsString(xmlDocument));
                                            hr = notifier->Show(notification.Get());
                                        }
                                        else
                                        {
                                            _activationTokens.erase(id);
                                            _dismissedTokens.erase(id);
                                            _failedTokens.erase(id);
                                            _buffer.erase(id);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return FAILED(hr) ? -1 : id;
}

ComPtr<IToastNotifier> WinToast::notifier(_In_ bool* succeded) const  {
	ComPtr<IToastNotificationManagerStatics> notificationManager;
	ComPtr<IToastNotifier> notifier;
	HRESULT hr = DllImporter::Wrap_GetActivationFactory(WinToastStringWrapper(RuntimeClass_Windows_UI_Notifications_ToastNotificationManager).Get(), &notificationManager);
	if (SUCCEEDED(hr)) {
		hr = notificationManager->CreateToastNotifierWithId(WinToastStringWrapper(_aumi).Get(), &notifier);
	}
	*succeded = SUCCEEDED(hr);
	return notifier;
}


bool WinToast::hideToast(_In_ INT64 id) {
    if (!isInitialized()) {
        DEBUG_MSG("Error when hiding the toast. WinToast is not initialized.");
        return false;
    }
    const bool find = _buffer.find(id) != _buffer.end();
	if (find) {
		bool succeded = false;
        _buffer[id].Get()->remove_Activated(_activationTokens[id]);
        _buffer[id].Get()->remove_Dismissed(_dismissedTokens[id]);
        _buffer[id].Get()->remove_Failed(_failedTokens[id]);
		ComPtr<IToastNotifier> notify = notifier(&succeded);
		if (succeded) {
			notify->Hide(_buffer[id].Get());
		}
        _activationTokens.erase(id);
        _dismissedTokens.erase(id);
        _failedTokens.erase(id);
		_buffer.erase(id);
	}
    return find;
}

void WinToast::clear() {
	bool succeded = false;
	ComPtr<IToastNotifier> notify = notifier(&succeded);
	if (succeded) {
		auto end = _buffer.end();
		for (auto it = _buffer.begin(); it != end; ++it) {
			notify->Hide(it->second.Get());
		}
	}
    _buffer.clear();
}

//
// Available as of Windows 10 Anniversary Update
// Ref: https://docs.microsoft.com/en-us/windows/uwp/design/shell/tiles-and-notifications/adaptive-interactive-toasts
//
// NOTE: This will add a new text field, so be aware when iterating over
//       the toast's text fields or getting a count of them.
//
HRESULT WinToast::setAttributionTextFieldHelper(_In_ IXmlDocument *xml, _In_ const std::wstring& text) {
    Util::createElement(xml, L"binding", L"text", { L"placement" });
    ComPtr<IXmlNodeList> nodeList;
    HRESULT hr = xml->GetElementsByTagName(WinToastStringWrapper(L"text").Get(), &nodeList);
    if (SUCCEEDED(hr)) {
        UINT32 nodeListLength;
        hr = nodeList->get_Length(&nodeListLength);
        if (SUCCEEDED(hr)) {
            for (UINT32 i = 0; i < nodeListLength; i++) {
                ComPtr<IXmlNode> textNode;
                hr = nodeList->Item(i, &textNode);
                if (SUCCEEDED(hr)) {
                    ComPtr<IXmlNamedNodeMap> attributes;
                    hr = textNode->get_Attributes(&attributes);
                    if (SUCCEEDED(hr)) {
                        ComPtr<IXmlNode> editedNode;
                        if (SUCCEEDED(hr)) {
                            hr = attributes->GetNamedItem(WinToastStringWrapper(L"placement").Get(), &editedNode);
                            if (FAILED(hr) || !editedNode) {
                                continue;
                            }
                            hr = Util::setNodeStringValue(L"attribution", editedNode.Get(), xml);
                            if (SUCCEEDED(hr)) {
                                return setTextFieldHelper(xml, text, i);
                            }
                        }
                    }
                }
            }
        }
    }
    return hr;
}

HRESULT WinToast::setTextFieldHelper(_In_ IXmlDocument *xml, _In_ const std::wstring& text, _In_ int pos) {
    ComPtr<IXmlNodeList> nodeList;
    HRESULT hr = xml->GetElementsByTagName(WinToastStringWrapper(L"text").Get(), &nodeList);
    if (SUCCEEDED(hr)) {
        ComPtr<IXmlNode> node;
        hr = nodeList->Item(pos, &node);
        if (SUCCEEDED(hr)) {
            hr = Util::setNodeStringValue(text, node.Get(), xml);
        }
    }
    return hr;
}


HRESULT WinToast::setImageFieldHelper(_In_ IXmlDocument *xml, _In_ const std::wstring& path)  {
    wchar_t imagePath[MAX_PATH] = L"file:///";
    HRESULT hr = StringCchCatW(imagePath, MAX_PATH, path.c_str());
    if (SUCCEEDED(hr)) {
        ComPtr<IXmlNodeList> nodeList;
        HRESULT hr = xml->GetElementsByTagName(WinToastStringWrapper(L"image").Get(), &nodeList);
        if (SUCCEEDED(hr)) {
            ComPtr<IXmlNode> node;
            hr = nodeList->Item(0, &node);
            if (SUCCEEDED(hr))  {
                ComPtr<IXmlNamedNodeMap> attributes;
                hr = node->get_Attributes(&attributes);
                if (SUCCEEDED(hr)) {
                    ComPtr<IXmlNode> editedNode;
                    hr = attributes->GetNamedItem(WinToastStringWrapper(L"src").Get(), &editedNode);
                    if (SUCCEEDED(hr)) {
                        Util::setNodeStringValue(imagePath, editedNode.Get(), xml);
                    }
                }
            }
        }
    }
    return hr;
}

HRESULT WinToast::setAudioFieldHelper(_In_ IXmlDocument *xml, _In_ const std::wstring& path, _In_opt_ WinToastTemplate::AudioOption option) {
    std::vector<std::wstring> attrs;
    if (!path.empty()) attrs.push_back(L"src");
    if (option == WinToastTemplate::AudioOption::Loop) attrs.push_back(L"loop");
    if (option == WinToastTemplate::AudioOption::Silent) attrs.push_back(L"silent");
    Util::createElement(xml, L"toast", L"audio", attrs);

    ComPtr<IXmlNodeList> nodeList;
    HRESULT hr = xml->GetElementsByTagName(WinToastStringWrapper(L"audio").Get(), &nodeList);
    if (SUCCEEDED(hr)) {
        ComPtr<IXmlNode> node;
        hr = nodeList->Item(0, &node);
        if (SUCCEEDED(hr)) {
            ComPtr<IXmlNamedNodeMap> attributes;
            hr = node->get_Attributes(&attributes);
            if (SUCCEEDED(hr)) {
                ComPtr<IXmlNode> editedNode;
                if (!path.empty()) {
                    if (SUCCEEDED(hr)) {
                        hr = attributes->GetNamedItem(WinToastStringWrapper(L"src").Get(), &editedNode);
                        if (SUCCEEDED(hr)) {
                            Util::setNodeStringValue(path, editedNode.Get(), xml);
                        }
                    }
                }
                //
                // These options are mutually exclusive
                //
                switch (option) {
                case WinToastTemplate::AudioOption::Loop:
                    hr = attributes->GetNamedItem(WinToastStringWrapper(L"loop").Get(), &editedNode);
                    if (SUCCEEDED(hr)) {
                        Util::setNodeStringValue(L"true", editedNode.Get(), xml);
                    }
                    break;
                case WinToastTemplate::AudioOption::Silent:
                    hr = attributes->GetNamedItem(WinToastStringWrapper(L"silent").Get(), &editedNode);
                    if (SUCCEEDED(hr)) {
                        Util::setNodeStringValue(L"true", editedNode.Get(), xml);
                    }
                default:
                    break;
                }
            }
        }
    }
    return hr;
}

HRESULT WinToast::addActionHelper(_In_ IXmlDocument *xml, _In_ const std::wstring& content, _In_ const std::wstring& arguments) {
	ComPtr<IXmlNodeList> nodeList;
	HRESULT hr = xml->GetElementsByTagName(WinToastStringWrapper(L"actions").Get(), &nodeList);
    if (SUCCEEDED(hr)) {
        UINT32 length;
        hr = nodeList->get_Length(&length);
        if (SUCCEEDED(hr)) {
            ComPtr<IXmlNode> actionsNode;
            if (length > 0) {
                hr = nodeList->Item(0, &actionsNode);
            } else {
                hr = xml->GetElementsByTagName(WinToastStringWrapper(L"toast").Get(), &nodeList);
                if (SUCCEEDED(hr)) {
                    hr = nodeList->get_Length(&length);
                    if (SUCCEEDED(hr)) {
                        ComPtr<IXmlNode> toastNode;
                        hr = nodeList->Item(0, &toastNode);
                        if (SUCCEEDED(hr)) {
                            ComPtr<IXmlElement> toastElement;
                            hr = toastNode.As(&toastElement);
                            if (SUCCEEDED(hr))
                                        hr = toastElement->SetAttribute(WinToastStringWrapper(L"template").Get(), WinToastStringWrapper(L"ToastGeneric").Get());
                            if (SUCCEEDED(hr))
                                        hr = toastElement->SetAttribute(WinToastStringWrapper(L"duration").Get(), WinToastStringWrapper(L"long").Get());
                            if (SUCCEEDED(hr)) {
                                ComPtr<IXmlElement> actionsElement;
                                hr = xml->CreateElement(WinToastStringWrapper(L"actions").Get(), &actionsElement);
                                if (SUCCEEDED(hr)) {
                                    hr = actionsElement.As(&actionsNode);
                                    if (SUCCEEDED(hr)) {
                                        ComPtr<IXmlNode> appendedChild;
                                        hr = toastNode->AppendChild(actionsNode.Get(), &appendedChild);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if (SUCCEEDED(hr)) {
                ComPtr<IXmlElement> actionElement;
                hr = xml->CreateElement(WinToastStringWrapper(L"action").Get(), &actionElement);
                if (SUCCEEDED(hr))
                    hr = actionElement->SetAttribute(WinToastStringWrapper(L"content").Get(), WinToastStringWrapper(content).Get());
                if (SUCCEEDED(hr))
                    hr = actionElement->SetAttribute(WinToastStringWrapper(L"arguments").Get(), WinToastStringWrapper(arguments).Get());
                if (SUCCEEDED(hr)) {
                    ComPtr<IXmlNode> actionNode;
                    hr = actionElement.As(&actionNode);
                    if (SUCCEEDED(hr)) {
                        ComPtr<IXmlNode> appendedChild;
                        hr = actionsNode->AppendChild(actionNode.Get(), &appendedChild);
                    }
                }
            }
        }
    }
    return hr;
}

WinToastTemplate::WinToastTemplate(_In_ WinToastTemplateType type) : _type(type) {
    static const std::size_t TextFieldsCount[] = { 1, 2, 2, 3, 1, 2, 2, 3};
    _textFields = std::vector<std::wstring>(TextFieldsCount[_type], L"");
}

WinToastTemplate::~WinToastTemplate() {
    _textFields.clear();
}

void WinToastTemplate::setTextField(_In_ const std::wstring& txt, _In_ WinToastTemplate::TextField pos) {
    _textFields[pos] = txt;
}

void WinToastTemplate::setImagePath(_In_ const std::wstring& imgPath) {
    _imagePath = imgPath;
}

void WinToastTemplate::setAudioPath(_In_ const std::wstring& audioPath) {
    _audioPath = audioPath;
}

void WinToastTemplate::setAudioOption(_In_ const WinToastTemplate::AudioOption & audioOption) {
    _audioOption = audioOption;
}

void WinToastTemplate::setAttributionText(_In_ const std::wstring& attributionText) {
    _attributionText = attributionText;
}

void WinToastTemplate::addAction(_In_ const std::wstring & label)
{
	_actions.push_back(label);
}
