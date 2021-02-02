#ifndef WINTOASTLIB_H
#define WINTOASTLIB_H
#include <Windows.h>
#include <sdkddkver.h>
#include <WinUser.h>
#include <ShObjIdl.h>
#include <windows.ui.notifications.h>
#include <strsafe.h>
#include <Psapi.h>
#include <ShlObj.h>
#include <roapi.h>
#include <propvarutil.h>
#include <functiondiscoverykeys.h>
#include <iostream>
#include <winstring.h>
#include <string.h>
#include <vector>
#include <map>
#include <memory>

#if NTDDI_VERSION < NTDDI_VISTA
#include "WintoastCompat.h"
#else
#include <wrl/implements.h>
#include <wrl/event.h>
using namespace Microsoft::WRL;
#endif

using namespace ABI::Windows::Data::Xml::Dom;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::UI::Notifications;
using namespace Windows::Foundation;

#define DEFAULT_LINK_FORMAT			L".lnk"
namespace WinToastLib {

    class IWinToastHandler {
    public:
        enum WinToastDismissalReason {
            UserCanceled = ToastDismissalReason::ToastDismissalReason_UserCanceled,
            ApplicationHidden = ToastDismissalReason::ToastDismissalReason_ApplicationHidden,
            TimedOut = ToastDismissalReason::ToastDismissalReason_TimedOut
        };
        virtual void toastActivated() = 0;
        virtual void toastActivated(int actionIndex) = 0;
        virtual void toastDismissed(WinToastDismissalReason state) = 0;
        virtual void toastFailed() = 0;
        virtual ~IWinToastHandler() {}
    };

    class WinToastTemplate {
    public:
        enum AudioOption { Default = 0, Silent = 1, Loop = 2 };
        enum TextField { FirstLine = 0, SecondLine, ThirdLine };
        enum WinToastTemplateType {
            ImageAndText01 = ToastTemplateType::ToastTemplateType_ToastImageAndText01,
            ImageAndText02 = ToastTemplateType::ToastTemplateType_ToastImageAndText02,
            ImageAndText03 = ToastTemplateType::ToastTemplateType_ToastImageAndText03,
            ImageAndText04 = ToastTemplateType::ToastTemplateType_ToastImageAndText04,
            Text01 = ToastTemplateType::ToastTemplateType_ToastText01,
            Text02 = ToastTemplateType::ToastTemplateType_ToastText02,
            Text03 = ToastTemplateType::ToastTemplateType_ToastText03,
            Text04 = ToastTemplateType::ToastTemplateType_ToastText04,
            WinToastTemplateTypeCount
        };

        WinToastTemplate(_In_ WinToastTemplateType type = ImageAndText02);
        ~WinToastTemplate();

        void                                        setTextField(_In_ const std::wstring& txt, _In_ TextField pos);
        void                                        setImagePath(_In_ const std::wstring& imgPath);
        void                                        setAudioPath(_In_ const std::wstring& audioPath);
        void                                        setAudioOption(_In_ const WinToastTemplate::AudioOption& audioOption);
        void                                        setAttributionText(_In_ const std::wstring & attributionText);
        void                                        addAction(_In_ const std::wstring& label);
        inline void                                 setExpiration(_In_ INT64 millisecondsFromNow) { _expiration = millisecondsFromNow; }
        inline int                                  textFieldsCount() const { return static_cast<int>(_textFields.size()); }
        inline int                                  actionsCount() const { return static_cast<int>(_actions.size()); }
        inline bool                                 hasImage() const { return _type < Text01; }
        inline std::vector<std::wstring>            textFields() const { return _textFields; }
        inline std::wstring                         textField(_In_ TextField pos) const { return _textFields[pos]; }
        inline std::wstring                         actionLabel(_In_ int pos) const { return _actions[pos]; }
        inline std::wstring                         imagePath() const { return _imagePath; }
        inline std::wstring                         audioPath() const { return _audioPath; }
        inline std::wstring                         attributionText() const { return _attributionText; }
        inline INT64                                expiration() const { return _expiration; }
        inline WinToastTemplateType                 type() const { return _type; }
        inline WinToastTemplate::AudioOption        audioOption() const { return _audioOption; }

    private:
        std::vector<std::wstring>			_textFields;
        std::wstring                        _imagePath;
        std::wstring                        _audioPath;
        std::vector<std::wstring>           _actions;
        INT64                               _expiration;
        WinToastTemplateType                _type;
        WinToastTemplate::AudioOption       _audioOption = WinToastTemplate::AudioOption::Default;
        std::wstring                        _attributionText;
    };

    class WinToast {
    public:
        WinToast(void);
        virtual ~WinToast();
        static WinToast* instance();
        static bool             isCompatible();
		static bool				supportModernFeatures();
		static std::wstring     configureAUMI(_In_ const std::wstring& companyName,
                                                    _In_ const std::wstring& productName,
                                                    _In_ const std::wstring& subProduct = std::wstring(),
                                                    _In_ const std::wstring& versionInformation = std::wstring()
                                                    );
        virtual bool            initialize();
        virtual bool            isInitialized() const { return _isInitialized; }
        virtual INT64           showToast(_In_ const WinToastTemplate& toast, _In_ std::shared_ptr<IWinToastHandler> handler);
        virtual bool            hideToast(_In_ INT64 id);
        virtual void            clear();
        inline std::wstring     appName() const { return _appName; }
        inline std::wstring     appUserModelId() const { return _aumi; }
        void                    setAppUserModelId(_In_ const std::wstring& appName);
        void                    setAppName(_In_ const std::wstring& appName);

        enum ShortcutResult {
            SHORTCUT_UNCHANGED = 0,
            SHORTCUT_WAS_CHANGED = 1,
            SHORTCUT_WAS_CREATED = 2,

            SHORTCUT_MISSING_PARAMETERS = -1,
            SHORTCUT_INCOMPATIBLE_OS = -2,
            SHORTCUT_COM_INIT_FAILURE = -3,
            SHORTCUT_CREATE_FAILED = -4,
            SHORTCUT_CREATE_SKIPPED = -5,
        };
        virtual enum ShortcutResult createShortcut();
    protected:
        bool											_isInitialized;
        bool                                            _hasCoInitialized;
        std::wstring                                    _appName;
        std::wstring                                    _aumi;
        std::map<INT64, ComPtr<IToastNotification>>     _buffer;   
        std::map<INT64, EventRegistrationToken>         _activationTokens;
        std::map<INT64, EventRegistrationToken>         _dismissedTokens;
        std::map<INT64, EventRegistrationToken>         _failedTokens;

        HRESULT     validateShellLinkHelper(_Out_ bool& wasChanged);
        HRESULT		createShellLinkHelper();
        HRESULT		setImageFieldHelper(_In_ IXmlDocument *xml, _In_ const std::wstring& path);
        HRESULT     setAudioFieldHelper(_In_ IXmlDocument *xml, _In_ const std::wstring& path, _In_opt_ WinToastTemplate::AudioOption option = WinToastTemplate::AudioOption::Default);
        HRESULT     setTextFieldHelper(_In_ IXmlDocument *xml, _In_ const std::wstring& text, _In_ int pos);
        HRESULT     setAttributionTextFieldHelper(_In_ IXmlDocument *xml, _In_ const std::wstring& text);
        HRESULT     addActionHelper(_In_ IXmlDocument *xml, _In_ const std::wstring& action, _In_ const std::wstring& arguments);
		ComPtr<IToastNotifier> notifier(_In_ bool* succeded) const;
    };
}
#endif // WINTOASTLIB_H
