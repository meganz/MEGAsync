#ifndef FATALEVENTHANDLER_H
#define FATALEVENTHANDLER_H

#include "AppState.h"
#include "BugReportDialog.h"
#include "MegaSyncLogger.h"

#include <QObject>

#include <memory>

class FatalEventHandler: public QObject
{
    Q_OBJECT

public:
    enum FatalErrorCode
    {
        ERR_UNKNOWN = mega::MegaEvent::REASON_ERROR_UNKNOWN,
        ERR_NO_ERROR = mega::MegaEvent::REASON_ERROR_NO_ERROR,
        ERR_FAILURE_UNSERIALIZE_NODE = mega::MegaEvent::REASON_ERROR_FAILURE_UNSERIALIZE_NODE,
        ERR_DB_IO_FAILURE = mega::MegaEvent::REASON_ERROR_DB_IO_FAILURE,
        ERR_DB_FULL = mega::MegaEvent::REASON_ERROR_DB_FULL,
        ERR_DB_INDEX_OVERFLOW = mega::MegaEvent::REASON_ERROR_DB_INDEX_OVERFLOW,
        ERR_NO_JSCD = mega::MegaEvent::REASON_ERROR_NO_JSCD,
        // In case there is a new event the app does not support yet:
        // It will be processed as ERR_UNKNOWN (logs and report will show sdk code though)
        // Make sure this value is not used by any mega::MegaEvent::REASON_ERROR_* !!
        ERR_UNHANDLED = 42,
    };
    Q_ENUM(FatalErrorCode)

    static std::shared_ptr<FatalEventHandler> instance();

    Q_INVOKABLE QString getErrorTitle() const;
    Q_INVOKABLE QString getErrorReason() const;
    Q_INVOKABLE QString getErrorReasonUrl() const;

    void setLogger(MegaSyncLogger* logger);

    void processFatalErrorEvent(std::unique_ptr<mega::MegaEvent> event);
    Q_INVOKABLE void showFatalErrorBugReportDialog(bool respawnWarningDialog = false);
    Q_INVOKABLE void restartOnFatalError();

signals:
    void requestAppState(AppState::AppStates newAppState);
    void requestExitApp(bool force);
    void requestRebootApp(bool update);
    void requestUnlink(bool keepLogs);

private:
    explicit FatalEventHandler();

    FatalErrorCode getErrorCode() const;
    QString getErrorCodeString() const;
    QString getErrorString() const;

    void handleUserAction(int choice);
    void clear();

    std::unique_ptr<mega::MegaEvent> mEvent;
    FatalErrorCode mErrorCode;
    QPointer<BugReportDialog> mFatalErrorReportDialog;
    MegaSyncLogger* mLogger; // The life cycle of the object that will be affected here is not the
                             // responsability of this class. It will be passed
                             // to mFatalErrorReportDialog at creation.

private slots:
    void showFatalErrorMessage();
    void onAppStateChanged(AppState::AppStates newAppState);
};

#endif // FATALEVENTHANDLER_H
