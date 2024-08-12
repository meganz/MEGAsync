#include "FatalEventHandler.h"

#include "DialogOpener.h"
#include "QMegaMessageBox.h"
#include "TextDecorator.h"
#include "Utilities.h"

#include <QMessageBox>
#include <QQmlEngine>
#include <QString>
#include <QVariant>

#include <megaapi.h>

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
        case FatalErrorCode::ERR_DB_INDEX_OVERFLOW: // TODO
        {
            errorTitle =
                QCoreApplication::translate("MegaError", "Fatal error: DB ondex overflow.");
            break;
        }
        case FatalErrorCode::ERR_NO_JSCD: // TODO
        {
            errorTitle =
                QCoreApplication::translate("MegaError", "Fatal error: No JSON Sync Config Data.");
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
            errorReason =
                QCoreApplication::translate("MegaError",
                                            "An error is causing the communication with MEGA"
                                            " to fail. Your syncs and backups are unable to"
                                            " update, and there may be further issues if you"
                                            " continue using this app without restarting. We "
                                            "strongly recommend immediately restarting the app"
                                            " to resolve this problem.");
            break;
        }
        case FatalErrorCode::ERR_FAILURE_UNSERIALIZE_NODE:
        {
            errorReason =
                QCoreApplication::translate("MegaError",
                                            "A serious issue has been detected in the MEGA "
                                            "software or the connection between this device "
                                            "and MEGA. Reinstall the app from "
                                            "[A]mega.io/desktop[/A] or contact support for"
                                            " further assistance.");
            break;
        }
        case FatalErrorCode::ERR_DB_FULL:
        {
            errorReason =
                QCoreApplication::translate("MegaError",
                                            "You need to make more space available in your"
                                            " local storage to be able to run MEGA.");
            break;
        }
        case FatalErrorCode::ERR_DB_IO_FAILURE:
        {
            errorReason =
                QCoreApplication::translate("MegaError",
                                            "Critical system files which are required by this "
                                            "app are unable to be reached. This may be the "
                                            "permissions of the folder the system files are "
                                            "in. You can also try restarting the app to see "
                                            "if this resolves the issue. If the folder "
                                            "permissions have please [A]contact support[/A]");
            break;
        }
        case FatalErrorCode::ERR_NO_JSCD: // TODO
        {
            errorReason = QCoreApplication::translate("MegaError", "Please [A]contact support[/A]");
            break;
        }
        case FatalErrorCode::ERR_DB_INDEX_OVERFLOW: // No message shown
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
        // Fallthrough
        case FatalErrorCode::ERR_NO_JSCD: // TODO
        {
            errorReasonUrl = Utilities::SUPPORT_URL;
            break;
        }
        case FatalErrorCode::ERR_UNKNOWN:
        // Fallthrough
        case FatalErrorCode::ERR_DB_FULL:
        // Fallthrough
        case FatalErrorCode::ERR_UNHANDLED:
        // Fallthrough
        case FatalErrorCode::ERR_NO_ERROR:
        // Fallthrough
        case FatalErrorCode::ERR_DB_INDEX_OVERFLOW: // TODO
        {
            // Do nothing
            break;
        }
    }
    return errorReasonUrl;
}

void FatalEventHandler::processFatalErrorEvent(std::unique_ptr<mega::MegaEvent> event,
                                               MegaSyncLogger* logger)
{
    mEvent = std::move(event);
    mLogger = logger;

    auto number = QVariant::fromValue(event->getNumber());

    // Make sure we can handle this error type, otherwise process as "Unhandled"
    mErrorCode = number.canConvert(qMetaTypeId<FatalEventHandler::FatalErrorCode>()) ?
                     number.value<FatalEventHandler::FatalErrorCode>() :
                     FatalErrorCode::ERR_UNHANDLED;

    const QString sdkErrorReason = QString::fromUtf8(mEvent->getText());

    mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_FATAL,
                       QString::fromUtf8("Fatal error %1 (%2): %3")
                           .arg(getErrorCodeString(), getErrorString(), sdkErrorReason)
                           .toUtf8()
                           .constData());
    // Process error
    if (mErrorCode == FatalErrorCode::ERR_DB_INDEX_OVERFLOW)
    {
        // This error can be silently fixed by reloading.
        // AppState::instance()->setAppState(AppState::RELOADING);
        emit requestAppState(AppState::RELOADING);
    }
    else
    {
        // AppState::instance()->setAppState(AppState::FATAL_ERROR);
        // showFatalErrorMessage();
        emit requestAppState(AppState::FATAL_ERROR);
    }
}

void FatalEventHandler::showFatalErrorBugReportDialog(bool respawnWarningDialog)
{
    if (mFatalErrorReportDialog == nullptr)
    {
        // Prepare report dialog
        mFatalErrorReportDialog = new BugReportDialog(nullptr, *mLogger);

        const QString reportTitle =
            QString::fromLatin1("%1 (%2)").arg(getErrorTitle(), getErrorString());
        mFatalErrorReportDialog->setReportObject(reportTitle);

        const QString reportText = QString::fromLatin1("FATAL ERROR CODE: %1 (%2)\n")
                                       .arg(QString::number(mErrorCode), getErrorString());
        mFatalErrorReportDialog->setReportText(reportText);

        if (respawnWarningDialog)
        {
            connect(mFatalErrorReportDialog,
                    &BugReportDialog::finished,
                    this,
                    &FatalEventHandler::showFatalErrorMessage);
        }
    }

    DialogOpener::showDialog(mFatalErrorReportDialog);
}

void FatalEventHandler::restartOnFatalError()
{
    emit requestRebootApp(false);
}

FatalEventHandler::FatalErrorCode FatalEventHandler::getErrorCode() const
{
    return mErrorCode;
}

QString FatalEventHandler::getErrorCodeString() const
{
    return QString::number(mEvent ? mEvent->getNumber() : getErrorCode());
}

QString FatalEventHandler::getErrorString() const
{
    return QVariant::fromValue<FatalEventHandler::FatalErrorCode>(getErrorCode()).toString();
}

FatalEventHandler::FatalEventHandler():
    QObject{nullptr},
    mEvent{nullptr},
    mErrorCode{FatalErrorCode::ERR_NO_ERROR},
    mFatalErrorReportDialog{nullptr}
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

void FatalEventHandler::handleUserAction(int choice)
{
    switch (choice)
    {
        case QMessageBox::Yes:
        {
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO, "Fatal error event: user exits app");
            emit requestExitApp(true);
            break;
        }
        case QMessageBox::Retry:
        {
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO,
                               "Fatal error event: user restarts app");
            restartOnFatalError();
            break;
        }
        case QMessageBox::Ok:
        {
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO, "Fatal error event: user logs out");
            emit requestUnlink(true);
            break;
        }
        case QMessageBox::Help:
        {
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO,
                               "Fatal error event: user contacts support");
            connect(mFatalErrorReportDialog,
                    &BugReportDialog::finished,
                    this,
                    &FatalEventHandler::showFatalErrorMessage,
                    Qt::UniqueConnection);
            showFatalErrorBugReportDialog(true);
            break;
        }
        case QMessageBox::Apply:
        {
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO,
                               "Fatal error event: user opens app folder");
            Utilities::openAppDataPath();
            showFatalErrorMessage();
            break;
        }
        default:
        {
            break;
        }
    }
}

void FatalEventHandler::clear()
{
    mFatalErrorReportDialog.clear();
    mErrorCode = FatalErrorCode::ERR_NO_ERROR;
    mEvent.reset();
    mLogger = nullptr; // Do not delete mLogger, it belongs to another object!
}

void FatalEventHandler::showFatalErrorMessage()
{
    // TODO: button icons?
    QMegaMessageBox::MessageBoxInfo msgInfo;
    msgInfo.textFormat = Qt::RichText;
    msgInfo.title = QMegaMessageBox::fatalErrorTitle();
    msgInfo.buttons = QMessageBox::Close;
    msgInfo.buttonsText.insert(QMessageBox::Close,
                               QCoreApplication::translate("MegaError", "Close"));
    msgInfo.defaultButton = QMessageBox::Close;

    msgInfo.text = QLatin1String("<b>%1</b>").arg(getErrorTitle());
    msgInfo.informativeText = getErrorReason();

    auto url(getErrorReasonUrl());
    if (!url.isEmpty())
    {
        const Text::Link link(url);
        link.process(msgInfo.informativeText);
    }

    msgInfo.finishFunc = [this, msgInfo](QPointer<QMessageBox> msg)
    {
        handleUserAction(msg->result());
    };

    switch (getErrorCode())
    {
        case FatalErrorCode::ERR_UNHANDLED:
        // Fallthrough
        case FatalErrorCode::ERR_UNKNOWN:
        {
            msgInfo.buttons |= QMessageBox::Retry | QMessageBox::Help;
            msgInfo.buttonsText.insert(QMessageBox::Retry,
                                       QCoreApplication::translate("MegaError", "Restart MEGA"));
            msgInfo.buttonsText.insert(QMessageBox::Help,
                                       QCoreApplication::translate("MegaError", "Report issue"));
            msgInfo.defaultButton = QMessageBox::Retry;
            break;
        }
        case FatalErrorCode::ERR_FAILURE_UNSERIALIZE_NODE:
        {
            msgInfo.buttons |= QMessageBox::Help;
            msgInfo.buttonsText.insert(QMessageBox::Help,
                                       QCoreApplication::translate("MegaError", "Report issue"));
            msgInfo.defaultButton = QMessageBox::Help;
            break;
        }
        case FatalErrorCode::ERR_DB_FULL:
        {
            msgInfo.buttons |= QMessageBox::Retry;
            msgInfo.buttonsText.insert(QMessageBox::Retry,
                                       QCoreApplication::translate("MegaError", "Restart MEGA"));
            msgInfo.defaultButton = QMessageBox::Retry;
            break;
        }
        case FatalErrorCode::ERR_DB_IO_FAILURE:
        {
            msgInfo.buttons |= QMessageBox::Retry | QMessageBox::Apply;
            msgInfo.buttonsText.insert(QMessageBox::Retry,
                                       QCoreApplication::translate("MegaError", "Restart MEGA"));
            msgInfo.buttonsText.insert(
                QMessageBox::Apply,
                QCoreApplication::translate("MegaError", "Check permissions"));

            msgInfo.defaultButton = QMessageBox::Apply;
            break;
        }
        case FatalErrorCode::ERR_NO_JSCD: // TODO
        {
            msgInfo.buttons |= QMessageBox::Ok;
            msgInfo.buttonsText.insert(QMessageBox::Ok,
                                       QCoreApplication::translate("MegaError", "Logout"));
            msgInfo.defaultButton = QMessageBox::Ok;
            break;
        }
        case FatalErrorCode::ERR_DB_INDEX_OVERFLOW: // No message shown, silent reload
        // Fallthrough
        case FatalErrorCode::ERR_NO_ERROR:
        {
            // Do nothing
            return;
        }
    }
    QMegaMessageBox::warning(msgInfo);
}

void FatalEventHandler::onAppStateChanged(AppState::AppStates newAppState)
{
    if (newAppState == AppState::FATAL_ERROR)
    {
        showFatalErrorMessage();
    }
    else
    {
        clear();
    }
}
