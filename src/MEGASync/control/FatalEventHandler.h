#ifndef FATALEVENTHANDLER_H
#define FATALEVENTHANDLER_H

#include "AppState.h"
#include "BugReportDialog.h"
#include "megaapi.h"
#include "MegaSyncLogger.h"

#include <QMessageBox>
#include <QObject>

#include <memory>

class FatalEventHandler: public QObject
{
    Q_OBJECT

public:
    // Mapping of SDK fatal error codes to app fatal error codes
    enum FatalErrorCode
    {
        ERR_UNKNOWN = mega::MegaEvent::REASON_ERROR_UNKNOWN,
        ERR_NO_ERROR = mega::MegaEvent::REASON_ERROR_NO_ERROR,
        ERR_FAILURE_UNSERIALIZE_NODE = mega::MegaEvent::REASON_ERROR_FAILURE_UNSERIALIZE_NODE,
        ERR_DB_IO_FAILURE = mega::MegaEvent::REASON_ERROR_DB_IO_FAILURE,
        ERR_DB_FULL = mega::MegaEvent::REASON_ERROR_DB_FULL,
        ERR_DB_INDEX_OVERFLOW = mega::MegaEvent::REASON_ERROR_DB_INDEX_OVERFLOW,
        ERR_NO_JSCD = mega::MegaEvent::REASON_ERROR_NO_JSCD,
        ERR_REGENERATE_JSCD = mega::MegaEvent::REASON_ERROR_REGENERATE_JSCD,
        ERR_DB_CORRUPT = mega::MegaEvent::REASON_ERROR_DB_CORRUPT,
        // In case there is a new event the app does not support yet:
        // It will be processed as ERR_UNKNOWN (logs and report will show sdk code though)
        // Make sure this value is not used by any mega::MegaEvent::REASON_ERROR_* !!
        ERR_UNHANDLED = 42,
    };
    Q_ENUM(FatalErrorCode)

    // Corrective actions
    enum FatalErrorCorrectiveAction
    {
        NO_ACTION,
        CONTACT_SUPPORT,
        LOGOUT,
        RESTART_APP,
        CHECK_PERMISSIONS,
        RELOAD,
        FORCE_ONBOARDING,
        DISMISS_WARNING,
    };
    Q_ENUM(FatalErrorCorrectiveAction)

    static std::shared_ptr<FatalEventHandler> instance();

    // Returns the error code, as processed by the app
    FatalErrorCode getErrorCode() const;
    // Returns the title of the error description
    Q_INVOKABLE QString getErrorTitle() const;
    // Returns the text body of the error decription
    Q_INVOKABLE QString getErrorReason() const;
    // Returns the url of an optional link in the error description
    Q_INVOKABLE QString getErrorReasonUrl() const;

    // This method takes action in reaction to a revceived SDK event <event>, <logger> is passed to
    // the Bug Report Dialog
    void processEvent(std::unique_ptr<mega::MegaEvent> event, MegaSyncLogger* logger);

    // Labels to be used on buttons shown to the user
    Q_INVOKABLE QString getDefaultActionLabel() const;
    Q_INVOKABLE QString getSecondaryActionLabel() const;

    // Icon to be used on the promary action button
    Q_INVOKABLE QString getDefaultActionIcon() const;

    // Methods to trigger corrective actions
    Q_INVOKABLE void triggerDefaultAction();
    Q_INVOKABLE void triggerSecondaryAction();

signals:
    void requestAppState(AppState::AppStates newAppState);
    void requestExitApp(bool force);
    void requestRebootApp(bool update);
    void requestUnlink(bool keepLogs);
    void requestForceOnBoarding();

private:
    explicit FatalEventHandler();

    // Returns the name, in the app error code enum, corresponding to the current event error
    QString getErrorCodeString() const;

    // Returns the actions suggested to fix the issue
    FatalErrorCorrectiveAction getDefaultAction() const;
    FatalErrorCorrectiveAction getSecondaryAction() const;

    QString getActionLabel(FatalErrorCorrectiveAction action) const;
    QString getActionIcon(FatalErrorCorrectiveAction action) const;

    void triggerAction(FatalErrorCorrectiveAction action);

    // Corrective actions
    void showFatalErrorBugReportDialog();
    void restartOnFatalError();
    void logoutOnFatalError();
    void reloadOnFatalError();
    void openAppDataFolder();
    void forceOnboarding();
    void dismissWarning();

    void clear();
    bool useContactSupportUrlHandler() const;
    bool closeAllAllowed() const;

    static const QMessageBox::StandardButton DEFAULT_ACTION_BUTTON;
    static const QMessageBox::StandardButton SECONDARY_ACTION_BUTTON;
    static const QString SCHEME_CONTACT_SUPPORT_URL;
    static const QString CONTACT_SUPPORT_URL;

    int64_t mSdkErrorCode;
    FatalErrorCode mErrorCode;
    QPointer<BugReportDialog> mFatalErrorReportDialog;
    bool mRespawnWarningDialog;

    // The life cycle of the object that will be affected here is not the responsability of this
    // class. It will be passed to mFatalErrorReportDialog at creation.
    MegaSyncLogger* mLogger;

private slots:
    void showFatalErrorMessage();
    void onAppStateChanged(AppState::AppStates oldAppState, AppState::AppStates newAppState);
    void handleContactSupport(const QUrl& url);
};

#endif // FATALEVENTHANDLER_H
