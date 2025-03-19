#include "FatalEventHandler.h"

#include "DialogOpener.h"
#include "MessageDialogOpener.h"
#include "QmlManager.h"
#include "TextDecorator.h"
#include "Utilities.h"

#include <QQmlEngine>
#include <QString>
#include <QVariant>

const QMessageBox::StandardButton FatalEventHandler::DEFAULT_ACTION_BUTTON = QMessageBox::Ok;
const QMessageBox::StandardButton FatalEventHandler::SECONDARY_ACTION_BUTTON = QMessageBox::Help;
const QString FatalEventHandler::SCHEME_CONTACT_SUPPORT_URL = QLatin1String("support");
const QString FatalEventHandler::CONTACT_SUPPORT_URL =
    SCHEME_CONTACT_SUPPORT_URL + QLatin1String("://open-bug-report-dialog");

std::shared_ptr<FatalEventHandler> FatalEventHandler::instance()
{
    static std::shared_ptr<FatalEventHandler> fatalEventHandler(new FatalEventHandler());
    return fatalEventHandler;
}

QString FatalEventHandler::getErrorTitle() const
{
    switch (getErrorCode())
    {
        case FatalErrorCode::ERR_UNHANDLED:
        // Fallthrough
        case FatalErrorCode::ERR_UNKNOWN:
        {
            return QCoreApplication::translate("MegaError", "An unknown error has occurred");
        }
        case FatalErrorCode::ERR_FAILURE_UNSERIALIZE_NODE:
        {
            return QCoreApplication::translate("MegaError", "A critical error has been detected");
        }
        case FatalErrorCode::ERR_DB_FULL:
        {
            return QCoreApplication::translate("MegaError", "Your local storage is full");
        }
        case FatalErrorCode::ERR_DB_IO_FAILURE:
        {
            return QCoreApplication::translate("MegaError", "Error reading app system files");
        }
        case FatalErrorCode::ERR_NO_JSCD:
        {
            return QCoreApplication::translate("MegaError", "Error with sync configuration files");
        }
        case FatalErrorCode::ERR_REGENERATE_JSCD:
        {
            return QCoreApplication::translate("MegaError", "Sync configuration error");
        }
        case FatalErrorCode::ERR_DB_INDEX_OVERFLOW:
        {
            return QCoreApplication::translate("MegaError", "An error has been detected");
        }
        case FatalErrorCode::ERR_NO_ERROR:
        {
            return {};
        }
    }
    return {};
}

QString FatalEventHandler::getErrorReason() const
{
    switch (getErrorCode())
    {
        case FatalErrorCode::ERR_UNHANDLED:
        // Fallthrough
        case FatalErrorCode::ERR_UNKNOWN:
        {
            return QCoreApplication::translate(
                "MegaError",
                "An error is causing the communication with MEGA to fail. Your syncs and backups "
                "are unable to update, and there may be further issues if you continue using this "
                "app without restarting. We strongly recommend immediately restarting the app to "
                "resolve this problem.");
        }
        case FatalErrorCode::ERR_FAILURE_UNSERIALIZE_NODE:
        {
            return QCoreApplication::translate(
                "MegaError",
                "A serious issue has been detected in the MEGA software or the connection between "
                "this device and MEGA. Reinstall the app from [A]mega.io/desktop[/A] or contact "
                "support for further assistance.");
        }
        case FatalErrorCode::ERR_DB_FULL:
        {
            return QCoreApplication::translate("MegaError",
                                               "You need to make more space available in "
                                               "your local storage to be able to run MEGA.");
        }
        case FatalErrorCode::ERR_DB_IO_FAILURE:
        {
            return QCoreApplication::translate(
                "MegaError",
                "Critical system files which are required by this app are unable to be reached. "
                "This may be the permissions of the folder the system files are in. You can also "
                "try restarting the app to see if this resolves the issue. If the folder "
                "permissions have been checked and the app restarted, please [A]contact "
                "support[/A].");
        }
        case FatalErrorCode::ERR_NO_JSCD:
        {
            return QCoreApplication::translate(
                "MegaError",
                "The app has detected an error in your sync configuration data. You need to log "
                "out of MEGA to resolve this issue. If the problem persists after logging back in, "
                "report the issue to our Support team.");
        }
        case FatalErrorCode::ERR_DB_INDEX_OVERFLOW:
        {
            return QCoreApplication::translate(
                "MegaError",
                "The app has detected an error and needs to reload. If you experience this issue "
                "more than once, contact our Support team.");
        }
        case FatalErrorCode::ERR_REGENERATE_JSCD:
        {
            return QCoreApplication::translate(
                "MegaError",
                "Your sync and backup settings were corrupted and have been reset. If you had any, "
                "please set them up again.");
        }
        case FatalErrorCode::ERR_NO_ERROR:
        {
            return {};
        }
    }
    return {};
}

QString FatalEventHandler::getErrorReasonUrl() const
{
    switch (getErrorCode())
    {
        case FatalErrorCode::ERR_FAILURE_UNSERIALIZE_NODE:
        {
            return Utilities::DESKTOP_APP_URL;
        }
        case FatalErrorCode::ERR_DB_IO_FAILURE:
        {
            // Open bug report dialog, do not send to support page
            return CONTACT_SUPPORT_URL;
        }
        case FatalErrorCode::ERR_UNKNOWN:
        // Fallthrough
        case FatalErrorCode::ERR_DB_FULL:
        // Fallthrough
        case FatalErrorCode::ERR_UNHANDLED:
        // Fallthrough
        case FatalErrorCode::ERR_NO_JSCD:
        // Fallthrough
        case FatalErrorCode::ERR_NO_ERROR:
        // Fallthrough
        case FatalErrorCode::ERR_DB_INDEX_OVERFLOW:
        // Fallthrough
        case FatalErrorCode::ERR_REGENERATE_JSCD:
        {
            return {};
        }
    }
    return {};
}

void FatalEventHandler::processEvent(std::unique_ptr<mega::MegaEvent> event, MegaSyncLogger* logger)
{
    mLogger = logger;

    mSdkErrorCode = event->getNumber();
    auto sdkErrorCode = QVariant::fromValue(mSdkErrorCode);

    // Make sure we can handle this error type, otherwise process as "Unhandled"
    auto canConvert = sdkErrorCode.canConvert(qMetaTypeId<FatalEventHandler::FatalErrorCode>());
    mErrorCode = canConvert ? sdkErrorCode.value<FatalEventHandler::FatalErrorCode>() :
                              FatalErrorCode::ERR_UNHANDLED;
    auto isValid =
        QMetaEnum::fromType<FatalEventHandler::FatalErrorCode>().valueToKey(mErrorCode) != nullptr;

    if (!isValid)
    {
        mErrorCode = FatalErrorCode::ERR_UNHANDLED;
    }
    const QString sdkErrorReason = QString::fromUtf8(event->getText());

    mega::MegaApi::log(
        mega::MegaApi::LOG_LEVEL_FATAL,
        QString::fromUtf8("Fatal error %1 (%2): %3")
            .arg(QString::number(mSdkErrorCode), getErrorCodeString(), sdkErrorReason)
            .toUtf8()
            .constData());

    // Explicitly do nothing more for MegaEvent::REASON_ERROR_NO_ERROR
    if (mErrorCode == FatalErrorCode::ERR_NO_ERROR)
    {
        clear();
    }
    else if (mErrorCode == FatalErrorCode::ERR_REGENERATE_JSCD)
    {
        // Put app in Fatal Error state after fetchnodes. Further action taken in
        // onAppStateChanged(), once the app is in Fatal Error state
        emit requestAppState(AppState::FATAL_ERROR_PENDING_FETCHNODES);
    }
    else
    {
        // Put app in Fatal Error state. Further action taken in onAppStateChanged(), once the app
        // is in Fatal Error state
        emit requestAppState(AppState::FATAL_ERROR);
    }
}

void FatalEventHandler::showFatalErrorBugReportDialog()
{
    mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO, "Fatal error event: action: contact support");

    if (mFatalErrorReportDialog == nullptr)
    {
        // Prepare report dialog
        mFatalErrorReportDialog = new BugReportDialog(nullptr, *mLogger);

        const QString reportTitle =
            QString::fromLatin1("%1 (%2)").arg(getErrorTitle(), getErrorCodeString());
        mFatalErrorReportDialog->setReportObject(reportTitle);

        const QString reportText = QString::fromLatin1("FATAL ERROR CODE: %1 (%2)\n")
                                       .arg(QString::number(mSdkErrorCode), getErrorCodeString());
        mFatalErrorReportDialog->setReportText(reportText);

        if (mRespawnWarningDialog)
        {
            connect(mFatalErrorReportDialog,
                    &BugReportDialog::finished,
                    this,
                    &FatalEventHandler::showFatalErrorMessage,
                    Qt::UniqueConnection);
        }
    }

    DialogOpener::showDialog(mFatalErrorReportDialog);
}

void FatalEventHandler::restartOnFatalError()
{
    mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO, "Fatal error event: action: restart app");
    emit requestRebootApp(false);
}

void FatalEventHandler::logoutOnFatalError()
{
    mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO, "Fatal error event: action: log out");
    emit requestUnlink(true);
}

void FatalEventHandler::reloadOnFatalError()
{
    mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO, "Fatal error event: action: reload");
    emit requestAppState(AppState::RELOADING);
}

void FatalEventHandler::openAppDataFolder()
{
    mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO, "Fatal error event: action: open app folder");
    Utilities::openAppDataPath();
}

void FatalEventHandler::forceOnboarding()
{
    mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO,
                       "Fatal error event: action: force onboarding");
    emit requestForceOnBoarding();
    emit requestAppState(AppState::NOMINAL);
}

void FatalEventHandler::dismissWarning()
{
    mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO, "Fatal error event: action: dismiss warning");
    emit requestAppState(AppState::NOMINAL);
}

QString FatalEventHandler::getDefaultActionLabel() const
{
    return getActionLabel(getDefaultAction());
}

QString FatalEventHandler::getSecondaryActionLabel() const
{
    return getActionLabel(getSecondaryAction());
}

QString FatalEventHandler::getDefaultActionIcon() const
{
    return getActionIcon(getDefaultAction());
}

void FatalEventHandler::triggerDefaultAction()
{
    triggerAction(getDefaultAction());
}

void FatalEventHandler::triggerSecondaryAction()
{
    triggerAction(getSecondaryAction());
}

FatalEventHandler::FatalErrorCode FatalEventHandler::getErrorCode() const
{
    return mErrorCode;
}

QString FatalEventHandler::getErrorCodeString() const
{
    return QVariant::fromValue<FatalEventHandler::FatalErrorCode>(getErrorCode()).toString();
}

FatalEventHandler::FatalErrorCorrectiveAction FatalEventHandler::getDefaultAction() const
{
    switch (getErrorCode())
    {
        case FatalErrorCode::ERR_FAILURE_UNSERIALIZE_NODE:
        {
            return FatalErrorCorrectiveAction::CONTACT_SUPPORT;
        }
        case FatalErrorCode::ERR_UNKNOWN:
        // Fallthrough
        case FatalErrorCode::ERR_UNHANDLED:
        // Fallthrough
        case FatalErrorCode::ERR_DB_FULL:
        {
            return FatalErrorCorrectiveAction::RESTART_APP;
        }
        case FatalErrorCode::ERR_DB_IO_FAILURE:
        {
            return FatalErrorCorrectiveAction::CHECK_PERMISSIONS;
        }
        case FatalErrorCode::ERR_NO_JSCD:
        {
            return FatalErrorCorrectiveAction::LOGOUT;
        }
        case FatalErrorCode::ERR_DB_INDEX_OVERFLOW:
        {
            return FatalErrorCorrectiveAction::RELOAD;
        }
        case FatalErrorCode::ERR_REGENERATE_JSCD:
        {
            return FatalErrorCorrectiveAction::FORCE_ONBOARDING;
        }
        case FatalErrorCode::ERR_NO_ERROR:
        {
            return FatalErrorCorrectiveAction::NO_ACTION;
        }
    }
    return FatalErrorCorrectiveAction::NO_ACTION;
}

FatalEventHandler::FatalErrorCorrectiveAction FatalEventHandler::getSecondaryAction() const
{
    switch (getErrorCode())
    {
        case FatalErrorCode::ERR_UNKNOWN:
        // Fallthrough
        case FatalErrorCode::ERR_UNHANDLED:
        // Fallthrough
        case FatalErrorCode::ERR_NO_JSCD:
        // Fallthrough
        case FatalErrorCode::ERR_DB_INDEX_OVERFLOW:
        {
            return FatalErrorCorrectiveAction::CONTACT_SUPPORT;
        }
        case FatalErrorCode::ERR_DB_IO_FAILURE:
        {
            return FatalErrorCorrectiveAction::RESTART_APP;
        }
        case FatalErrorCode::ERR_REGENERATE_JSCD:
        {
            return FatalErrorCorrectiveAction::DISMISS_WARNING;
        }
        case FatalErrorCode::ERR_DB_FULL:
        // Fallthrough
        case FatalErrorCode::ERR_NO_ERROR:
        // Fallthrough
        case FatalErrorCode::ERR_FAILURE_UNSERIALIZE_NODE:
        {
            return FatalErrorCorrectiveAction::NO_ACTION;
        }
    }
    return FatalErrorCorrectiveAction::NO_ACTION;
}

QString
    FatalEventHandler::getActionLabel(FatalEventHandler::FatalErrorCorrectiveAction action) const
{
    switch (action)
    {
        case FatalErrorCorrectiveAction::CONTACT_SUPPORT:
        {
            return QCoreApplication::translate("MegaError", "Contact support");
        }
        case FatalErrorCorrectiveAction::RESTART_APP:
        {
            return QCoreApplication::translate("MegaError", "Restart MEGA");
        }
        case FatalErrorCorrectiveAction::LOGOUT:
        {
            return QCoreApplication::translate("MegaError", "Log out");
        }
        case FatalErrorCorrectiveAction::CHECK_PERMISSIONS:
        {
            return QCoreApplication::translate("MegaError", "Check permissions");
        }
        case FatalErrorCorrectiveAction::RELOAD:
        {
            return QCoreApplication::translate("MegaError", "Reload");
        }
        case FatalErrorCorrectiveAction::FORCE_ONBOARDING:
        {
            return QCoreApplication::translate("MegaError", "Reconfigure");
        }
        case FatalErrorCorrectiveAction::DISMISS_WARNING:
        {
            return QCoreApplication::translate("MegaError", "Dismiss");
        }
        case FatalErrorCorrectiveAction::NO_ACTION:
        {
            // No associated user interaction
            return {};
        }
    }
    return {};
}

QString FatalEventHandler::getActionIcon(FatalEventHandler::FatalErrorCorrectiveAction action) const
{
    switch (action)
    {
        case FatalErrorCorrectiveAction::CONTACT_SUPPORT:
        {
            return QLatin1String("icons/headset.svg");
        }
        case FatalErrorCorrectiveAction::RESTART_APP:
        {
            return QLatin1String("icons/rotate_ccw.svg");
        }
        case FatalErrorCorrectiveAction::LOGOUT:
        {
            return QLatin1String("icons/log-out-01.svg");
        }
        case FatalErrorCorrectiveAction::CHECK_PERMISSIONS:
        {
            return QLatin1String("icons/file_edit.svg");
        }
        case FatalErrorCorrectiveAction::FORCE_ONBOARDING:
        // Fallthrough
        case FatalErrorCorrectiveAction::DISMISS_WARNING:
        // Fallthrough
        case FatalErrorCorrectiveAction::RELOAD:
        // Fallthrough
        case FatalErrorCorrectiveAction::NO_ACTION:
        {
            // No associated user interaction
            return {};
        }
    }
    return {};
}

void FatalEventHandler::triggerAction(FatalEventHandler::FatalErrorCorrectiveAction action)
{
    switch (action)
    {
        case FatalErrorCorrectiveAction::CONTACT_SUPPORT:
        {
            showFatalErrorBugReportDialog();
            break;
        }
        case FatalErrorCorrectiveAction::RESTART_APP:
        {
            restartOnFatalError();
            break;
        }
        case FatalErrorCorrectiveAction::LOGOUT:
        {
            logoutOnFatalError();
            break;
        }
        case FatalErrorCorrectiveAction::CHECK_PERMISSIONS:
        {
            openAppDataFolder();
            break;
        }
        case FatalErrorCorrectiveAction::RELOAD:
        {
            reloadOnFatalError();
            break;
        }
        case FatalErrorCorrectiveAction::FORCE_ONBOARDING:
        {
            forceOnboarding();
            break;
        }
        case FatalErrorCorrectiveAction::DISMISS_WARNING:
        {
            dismissWarning();
            break;
        }
        case FatalErrorCorrectiveAction::NO_ACTION:
        {
            // Do nothing
            break;
        }
    }
}

FatalEventHandler::FatalEventHandler():
    QObject{nullptr},
    mSdkErrorCode{mega::MegaEvent::REASON_ERROR_NO_ERROR},
    mErrorCode{FatalErrorCode::ERR_NO_ERROR},
    mFatalErrorReportDialog{nullptr},
    mLogger{nullptr}
{
    qmlRegisterUncreatableType<FatalEventHandler>(
        "FatalEventHandler",
        1,
        0,
        "FatalEventHandler",
        QLatin1String("Warning FatalEventHandler: not allowed to be instantiated"));
    QmlManager::instance()->setRootContextProperty(QLatin1String("fatalEventHandlerAccess"), this);

    // Plug into AppStates
    connect(this,
            &FatalEventHandler::requestAppState,
            AppState::instance().get(),
            &AppState::setAppState);
    connect(AppState::instance().get(),
            &AppState::appStateChanged,
            this,
            &FatalEventHandler::onAppStateChanged);
}

void FatalEventHandler::clear()
{
    if (useContactSupportUrlHandler())
    {
        QDesktopServices::unsetUrlHandler(SCHEME_CONTACT_SUPPORT_URL);
    }
    mFatalErrorReportDialog.clear();
    mSdkErrorCode = mega::MegaEvent::REASON_ERROR_NO_ERROR;
    mErrorCode = FatalErrorCode::ERR_NO_ERROR;
    mLogger = nullptr; // Do not delete mLogger, it belongs to another object!
}

bool FatalEventHandler::useContactSupportUrlHandler() const
{
    return getErrorReasonUrl().startsWith(SCHEME_CONTACT_SUPPORT_URL);
}

void FatalEventHandler::showFatalErrorMessage()
{
    mRespawnWarningDialog = true;
    MessageDialogInfo msgInfo;
    msgInfo.textFormat = Qt::RichText;
    msgInfo.dialogTitle = MessageDialogOpener::fatalErrorTitle();
    msgInfo.imageUrl = QLatin1String(":images/alert-triangle-small.png");

    msgInfo.titleText = QLatin1String("<b>%1</b>").arg(getErrorTitle());
    msgInfo.descriptionText = getErrorReason();

    auto url(getErrorReasonUrl());
    if (!url.isEmpty())
    {
        const Text::Link link(url);
        link.process(msgInfo.descriptionText);
    }

    // Close button
    msgInfo.buttons = QMessageBox::Close;
    msgInfo.hideCloseButton = true;

    // Primary action button
    if (getDefaultAction() != FatalErrorCorrectiveAction::NO_ACTION)
    {
        msgInfo.buttons |= DEFAULT_ACTION_BUTTON;
        msgInfo.buttonsText.insert(DEFAULT_ACTION_BUTTON, getDefaultActionLabel());
        msgInfo.buttonsIcons.insert(DEFAULT_ACTION_BUTTON, QUrl());
        msgInfo.defaultButton = DEFAULT_ACTION_BUTTON;
    }

    // Eventual secondary action button
    if (getSecondaryAction() != FatalErrorCorrectiveAction::NO_ACTION)
    {
        msgInfo.buttons |= SECONDARY_ACTION_BUTTON;
        msgInfo.buttonsText.insert(SECONDARY_ACTION_BUTTON, getSecondaryActionLabel());
        msgInfo.buttonsIcons.insert(SECONDARY_ACTION_BUTTON, QUrl());
    }

    // User choice handling
    msgInfo.finishFunc = [this](QPointer<MessageDialogResult> msg)
    {
        auto choice = msg->result();
        if (choice == DEFAULT_ACTION_BUTTON)
        {
            triggerDefaultAction();
        }
        else if (choice == SECONDARY_ACTION_BUTTON)
        {
            triggerSecondaryAction();
        }
        else
        {
            // Close
            mRespawnWarningDialog = false;
        }
    };

    MessageDialogOpener::warning(msgInfo);
}

void FatalEventHandler::onAppStateChanged(AppState::AppStates oldAppState,
                                          AppState::AppStates newAppState)
{
    // Take further action once the app is in Fatal Error state
    if (newAppState == AppState::FATAL_ERROR)
    {
        if (useContactSupportUrlHandler())
        {
            QDesktopServices::setUrlHandler(SCHEME_CONTACT_SUPPORT_URL,
                                            this,
                                            "handleContactSupport");
        }
        // Show a warning dialog to the user
        showFatalErrorMessage();
    }
    else if (oldAppState == AppState::FATAL_ERROR)
    {
        clear();
    }
}

void FatalEventHandler::handleContactSupport(const QUrl& url)
{
    if (url == QUrl(CONTACT_SUPPORT_URL))
    {
        // Check if the warning dialog is displayed, and close it if it is, and set to respawn.
        auto warningDialog = DialogOpener::findDialog<QMessageBox>();
        if (mRespawnWarningDialog && warningDialog)
        {
            warningDialog->close();
            mRespawnWarningDialog = true;
        }
        triggerAction(FatalEventHandler::CONTACT_SUPPORT);
    }
}
