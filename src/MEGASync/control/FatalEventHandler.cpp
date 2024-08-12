#include "FatalEventHandler.h"

#include "DialogOpener.h"
#include "QMegaMessageBox.h"
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
    QString errorTitle;
    switch (getErrorCode())
    {
        case FatalErrorCode::ERR_UNHANDLED:
        // Fallthrough
        case FatalErrorCode::ERR_UNKNOWN:
        {
            errorTitle = QCoreApplication::translate("MegaError", "An unknown error has occurred");
            break;
        }
        case FatalErrorCode::ERR_FAILURE_UNSERIALIZE_NODE:
        {
            errorTitle =
                QCoreApplication::translate("MegaError", "A critical error has been detected");
            break;
        }
        case FatalErrorCode::ERR_DB_FULL:
        {
            errorTitle = QCoreApplication::translate("MegaError", "Your local storage is full");
            break;
        }
        case FatalErrorCode::ERR_DB_IO_FAILURE:
        {
            errorTitle = QCoreApplication::translate("MegaError", "Error reading app system files");
            break;
        }
        case FatalErrorCode::ERR_NO_JSCD:
        {
            errorTitle =
                QCoreApplication::translate("MegaError", "Error with sync configuration files");
            break;
        }
        case FatalErrorCode::ERR_DB_INDEX_OVERFLOW:
        {
            errorTitle = QCoreApplication::translate("MegaError", "An error has been detected");
            break;
        }
        case FatalErrorCode::ERR_NO_ERROR:
        {
            // Do nothing
            break;
        }
    }
    return errorTitle;
}

QString FatalEventHandler::getErrorReason() const
{
    QString errorReason;
    switch (getErrorCode())
    {
        case FatalErrorCode::ERR_UNHANDLED:
        // Fallthrough
        case FatalErrorCode::ERR_UNKNOWN:
        {
            errorReason = QCoreApplication::translate(
                "MegaError",
                "An error is causing the communication with MEGA to fail. Your syncs and backups "
                "are unable to update, and there may be further issues if you continue using this "
                "app without restarting. We strongly recommend immediately restarting the app to "
                "resolve this problem.");
            break;
        }
        case FatalErrorCode::ERR_FAILURE_UNSERIALIZE_NODE:
        {
            errorReason = QCoreApplication::translate(
                "MegaError",
                "A serious issue has been detected in the MEGA software or the connection between "
                "this device and MEGA. Reinstall the app from [A]mega.io/desktop[/A] or contact "
                "support for further assistance.");
            break;
        }
        case FatalErrorCode::ERR_DB_FULL:
        {
            errorReason = QCoreApplication::translate("MegaError",
                                                      "You need to make more space available in "
                                                      "your local storage to be able to run MEGA.");
            break;
        }
        case FatalErrorCode::ERR_DB_IO_FAILURE:
        {
            errorReason = QCoreApplication::translate(
                "MegaError",
                "Critical system files which are required by this app are unable to be reached. "
                "This may be the permissions of the folder the system files are in. You can also "
                "try restarting the app to see if this resolves the issue. If the folder "
                "permissions have been checked and the app restarted, please [A]contact "
                "support[/A].");
            break;
        }
        case FatalErrorCode::ERR_NO_JSCD:
        {
            errorReason = QCoreApplication::translate(
                "MegaError",
                "The app has detected an error in your sync configuration data. You need to log "
                "out of MEGA to resolve this issue. If the problem persists after logging back in, "
                "report the issue to our Support team.");
            break;
        }
        case FatalErrorCode::ERR_DB_INDEX_OVERFLOW:
        {
            errorReason = QCoreApplication::translate(
                "MegaError",
                "The app has detected an error and needs to reload. If you experience this issue "
                "more than once, contact our Support team.");
            break;
        }
        case FatalErrorCode::ERR_NO_ERROR:
        {
            // Do nothing
            break;
        }
    }
    return errorReason;
}

QString FatalEventHandler::getErrorReasonUrl() const
{
    QString errorReasonUrl;
    switch (getErrorCode())
    {
        case FatalErrorCode::ERR_FAILURE_UNSERIALIZE_NODE:
        {
            errorReasonUrl = Utilities::DESKTOP_APP_URL;
            break;
        }
        case FatalErrorCode::ERR_DB_IO_FAILURE:
        {
            // Open bug report dialog, do not send to support page
            errorReasonUrl = CONTACT_SUPPORT_URL;
            break;
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
        {
            // Do nothing
            break;
        }
    }
    return errorReasonUrl;
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

    // Put app in Fatal Error state. Further action taken in onAppStateChanged(), once the app is in
    // Fatal Error state
    emit requestAppState(AppState::FATAL_ERROR);
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
    FatalEventHandler::FatalErrorCorrectiveAction action = FatalErrorCorrectiveAction::NO_ACTION;
    switch (getErrorCode())
    {
        case FatalErrorCode::ERR_FAILURE_UNSERIALIZE_NODE:
        {
            action = FatalErrorCorrectiveAction::CONTACT_SUPPORT;
            break;
        }
        case FatalErrorCode::ERR_UNKNOWN:
        // Fallthrough
        case FatalErrorCode::ERR_UNHANDLED:
        // Fallthrough
        case FatalErrorCode::ERR_DB_FULL:
        {
            action = FatalErrorCorrectiveAction::RESTART_APP;
            break;
        }
        case FatalErrorCode::ERR_DB_IO_FAILURE:
        {
            action = FatalErrorCorrectiveAction::CHECK_PERMISSIONS;
            break;
        }
        case FatalErrorCode::ERR_NO_JSCD:
        {
            action = FatalErrorCorrectiveAction::LOGOUT;
            break;
        }
        case FatalErrorCode::ERR_DB_INDEX_OVERFLOW:
        {
            action = FatalErrorCorrectiveAction::RELOAD;
            break;
        }
        case FatalErrorCode::ERR_NO_ERROR:
        {
            action = FatalErrorCorrectiveAction::NO_ACTION;
            break;
        }
    }
    return action;
}

FatalEventHandler::FatalErrorCorrectiveAction FatalEventHandler::getSecondaryAction() const
{
    FatalEventHandler::FatalErrorCorrectiveAction action = FatalErrorCorrectiveAction::NO_ACTION;
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
            action = FatalErrorCorrectiveAction::CONTACT_SUPPORT;
            break;
        }
        case FatalErrorCode::ERR_DB_IO_FAILURE:
        {
            action = FatalErrorCorrectiveAction::RESTART_APP;
            break;
        }
        case FatalErrorCode::ERR_DB_FULL:
        // Fallthrough
        case FatalErrorCode::ERR_NO_ERROR:
        // Fallthrough
        case FatalErrorCode::ERR_FAILURE_UNSERIALIZE_NODE:
        {
            action = FatalErrorCorrectiveAction::NO_ACTION;
            break;
        }
    }
    return action;
}

QString
    FatalEventHandler::getActionLabel(FatalEventHandler::FatalErrorCorrectiveAction action) const
{
    QString label;

    switch (action)
    {
        case FatalErrorCorrectiveAction::CONTACT_SUPPORT:
        {
            label = QCoreApplication::translate("MegaError", "Contact support");
            break;
        }
        case FatalErrorCorrectiveAction::RESTART_APP:
        {
            label = QCoreApplication::translate("MegaError", "Restart MEGA");
            break;
        }
        case FatalErrorCorrectiveAction::LOGOUT:
        {
            label = QCoreApplication::translate("MegaError", "Log out");
            break;
        }
        case FatalErrorCorrectiveAction::CHECK_PERMISSIONS:
        {
            label = QCoreApplication::translate("MegaError", "Check permissions");
            break;
        }
        case FatalErrorCorrectiveAction::RELOAD:
        {
            label = QCoreApplication::translate("MegaError", "Reload");
            break;
        }
        case FatalErrorCorrectiveAction::NO_ACTION:
        {
            // No associated user interaction
            break;
        }
    }
    return label;
}

QString FatalEventHandler::getActionIcon(FatalEventHandler::FatalErrorCorrectiveAction action) const
{
    QString icon;
    switch (action)
    {
        case FatalErrorCorrectiveAction::CONTACT_SUPPORT:
        {
            icon = QLatin1String("icons/headset.svg");
            break;
        }
        case FatalErrorCorrectiveAction::RESTART_APP:
        {
            icon = QLatin1String("icons/rotate_ccw.svg");
            break;
        }
        case FatalErrorCorrectiveAction::LOGOUT:
        {
            icon = QLatin1String("icons/log-out-01.svg");
            break;
        }
        case FatalErrorCorrectiveAction::CHECK_PERMISSIONS:
        {
            icon = QLatin1String("icons/file_edit.svg");
            break;
        }
        case FatalErrorCorrectiveAction::RELOAD:
        case FatalErrorCorrectiveAction::NO_ACTION:
        {
            // No associated user interaction
            break;
        }
    }
    return icon;
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
    QMegaMessageBox::MessageBoxInfo msgInfo;
    msgInfo.textFormat = Qt::RichText;
    msgInfo.title = QMegaMessageBox::fatalErrorTitle();
    msgInfo.iconPixmap = QPixmap(QLatin1String(":images/alert-triangle-small.png"));

    msgInfo.text = QLatin1String("<b>%1</b>").arg(getErrorTitle());
    msgInfo.informativeText = getErrorReason();

    auto url(getErrorReasonUrl());
    if (!url.isEmpty())
    {
        const Text::Link link(url);
        link.process(msgInfo.informativeText);
    }

    // Close button
    msgInfo.buttons = QMessageBox::Close;
    msgInfo.hideCloseButton = true;

    // Primary action button
    if (getDefaultAction() != FatalErrorCorrectiveAction::NO_ACTION)
    {
        msgInfo.buttons |= DEFAULT_ACTION_BUTTON;
        msgInfo.buttonsText.insert(DEFAULT_ACTION_BUTTON, getDefaultActionLabel());
        msgInfo.buttonsIcons.insert(DEFAULT_ACTION_BUTTON, QIcon());
        msgInfo.defaultButton = DEFAULT_ACTION_BUTTON;
    }

    // Eventual secondary action button
    if (getSecondaryAction() != FatalErrorCorrectiveAction::NO_ACTION)
    {
        msgInfo.buttons |= SECONDARY_ACTION_BUTTON;
        msgInfo.buttonsText.insert(SECONDARY_ACTION_BUTTON, getSecondaryActionLabel());
        msgInfo.buttonsIcons.insert(SECONDARY_ACTION_BUTTON, QIcon());
    }

    // User choice handling
    msgInfo.finishFunc = [this](QPointer<QMessageBox> msg)
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

    QMegaMessageBox::warning(msgInfo);
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
