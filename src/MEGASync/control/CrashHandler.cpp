#ifdef USE_BREAKPAD
#include "CrashHandler.h"

#include "MegaApplication.h"
#include "ServiceUrls.h"
#include "Utilities.h"
#include "Version.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QTimer>

#include <string>

using namespace mega;
using namespace std;

#if defined(Q_OS_MAC)
#include "client/mac/handler/exception_handler.h"
#elif defined(Q_OS_LINUX)
#include "client/linux/handler/exception_handler.h"
#elif defined(Q_OS_WINDOWS)
#include "client/windows/handler/exception_handler.h"
#endif

/************************************************************************/
/* CrashHandlerImpl                                                  */
/************************************************************************/

class CrashHandlerImpl
{
public:
    CrashHandlerImpl()
    {
        mHandler = NULL;
    }

    ~CrashHandlerImpl()
    {
        delete mHandler;
    }

    void initCrashHandler(const QString& dumpPath);
    static google_breakpad::ExceptionHandler* mHandler;
};

google_breakpad::ExceptionHandler* CrashHandlerImpl::mHandler = NULL;

/************************************************************************/
/* DumpCallback                                                         */
/************************************************************************/
#if defined(Q_OS_WINDOWS)
bool DumpCallback(const wchar_t*,
                  const wchar_t*,
                  void* context,
                  EXCEPTION_POINTERS*,
                  MDRawAssertionInfo*,
                  bool success)
#elif defined(Q_OS_LINUX)
bool DumpCallback(const google_breakpad::MinidumpDescriptor&, void*, bool success)
#elif defined(Q_OS_MAC)
bool DumpCallback(const char*, const char*, void*, bool success)
#endif
{
    if (g_megaSyncLogger)
    {
        g_megaSyncLogger->flushAndClose();
    }

    CrashHandler::tryReboot();
    return success;
}

/************************************************************************/
/* CrashHandler                                                         */
/************************************************************************/
void CrashHandlerImpl::initCrashHandler(const QString& dumpPath)
{
    if (mHandler != NULL)
        return;
#ifdef Q_OS_WINDOWS
    std::wstring pathAsStr = (const wchar_t*)dumpPath.utf16();
    mHandler =
        new google_breakpad::ExceptionHandler(pathAsStr,
                                              /*FilterCallback*/ NULL,
                                              DumpCallback,
                                              /*context*/
                                              NULL,
                                              google_breakpad::ExceptionHandler::HANDLER_EXCEPTION);
#else
    std::string pathAsStr = dumpPath.toUtf8().constData();
#ifdef Q_OS_MAC
    // In Macos breakpad and a debugger cannot work together
    // As both of them will try to get ownership of task exception port
    // But in Mac the exception ports only have a single owner
    mHandler = new google_breakpad::ExceptionHandler(pathAsStr,
                                                     /*FilterCallback*/ 0,
                                                     DumpCallback,
                                                     /*context*/
                                                     0,
                                                     true,
                                                     NULL);
#elif defined(Q_OS_LINUX)
    google_breakpad::MinidumpDescriptor md(pathAsStr);
    mHandler = new google_breakpad::ExceptionHandler(md,
                                                     /* filter */ 0,
                                                     DumpCallback,
                                                     /* context */ 0,
                                                     /* install handler */ true,
                                                     /* server FD */ -1);
#endif
#endif
}

CrashHandler* CrashHandler::instance()
{
    static CrashHandler globalHandler;
    return &globalHandler;
}

CrashHandler::CrashHandler():
    QObject(),
    mImpl(new CrashHandlerImpl())
{}

CrashHandler::~CrashHandler()
{
    delete mImpl;
}

void CrashHandler::init(const QString& reportPath)
{
    this->mDumpPath = reportPath;
    mImpl->initCrashHandler(reportPath);
}

void CrashHandler::tryReboot()
{
    auto preferences = Preferences::instance();
    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Setting isCrashed true: tryReboot (CrashHandler)");
    preferences->setCrashed(true);
    preferences->setCrashedUserID(
        QString::fromUtf8(dynamic_cast<MegaApplication*>(qApp)->getMegaApi()->getMyUserHandle()));
    if ((QDateTime::currentMSecsSinceEpoch() - preferences->getLastReboot()) >
        Preferences::MIN_REBOOT_INTERVAL_MS)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_WARNING, "Restarting app...");
        preferences->setLastReboot(QDateTime::currentMSecsSinceEpoch());

#ifndef Q_OS_MACOS
        QString app = MegaApplication::applicationFilePath();
        QProcess::startDetached(app, {});
#else
        QString app = MegaApplication::applicationDirPath();
        QString launchCommand = QString::fromUtf8("open");
        QStringList args = QStringList();

        QDir appPath(app);
        appPath.cdUp();
        appPath.cdUp();

        args.append(QString::fromLatin1("-n"));
        args.append(appPath.absolutePath());
        QProcess::startDetached(launchCommand, args);
#endif

#ifdef Q_OS_WINDOWS
        Sleep(2000);
#else
        sleep(2);
#endif
    }
    else
    {
        MegaApi::log(MegaApi::LOG_LEVEL_WARNING, "The app was recently restarted. Restart skipped");
    }
}

void CrashHandler::sendPendingCrashReports(QString userMessage, bool shouldSendLogs)
{
    QStringList crashes = getPendingCrashReports();
    if (!crashes.size())
    {
        return;
    }
    std::string url = CRASH_BACKEND_URL;
    std::map<std::string, std::string> parameters;

    parameters["sentry[release]"] = string{VER_FILEDESCRIPTION_STR} + "@" + VER_PRODUCTVERSION_STR;
    parameters["sentry[environment]"] = "production";
    parameters["sentry[logger]"] = "breakpad";
    parameters["sentry[level]"] = "fatal";
    parameters["sentry[platform]"] = "native";
    std::string deviceID = dynamic_cast<MegaApplication*>(qApp)->getMegaApi()->getDeviceId();
    parameters["sentry[user][id]"] = deviceID.empty() ?
                                         "invalid_user" :
                                         deviceID; // Used to properly increase affected users count
                                                   // for each installation (device ID is used).
    // Sentry Context section (these are not searchable)
    parameters["sentry[contexts][app][app_name]"] =
        QCoreApplication::applicationName().toStdString();
    parameters["sentry[contexts][app][logs_uploaded]"] = shouldSendLogs ? "Yes" : "No";
    parameters["sentry[contexts][app][build_type]"] =
#ifdef QT_DEBUG
        "debug";
#else
        "release";
#endif
    if (!userMessage.isEmpty())
    {
        parameters["sentry[contexts][user][feedback]"] = userMessage.toStdString();
    }
    // Sentry standard tags
    parameters["sentry[tags][os]"] = QSysInfo::prettyProductName().toStdString();
    parameters["sentry[contexts][os][kernel_version]"] = QSysInfo::kernelVersion().toStdString();

    // Custom tags (Use tags for indexing and searching the info in sentry)
    parameters["sentry[tags][build_id]"] = std::to_string(VER_BUILD_ID);
    parameters["sentry[tags][mega_sdk]"] = Preferences::SDK_ID.toStdString();
    parameters["sentry[tags][auto_update]"] =
        Preferences::instance()->updateAutomatically() ? "True" : "False";
    std::string UID = Preferences::instance()->crashedUserID().toStdString();
    parameters["sentry[tags][user-handle]"] = UID.empty() ? "invalid_user" : UID;

    QtConcurrent::run(
        [=]()
        {
            QString combinedCrashID;
            for (auto crash: crashes)
            {
                std::map<std::string, std::string> files;
                files["upload_file_minidump"] = crash.toStdString();
                auto crashID = sendCrashReport(url, parameters, files);
                if (!crashID.isEmpty())
                {
                    combinedCrashID += (crashID + QString::fromUtf8("-"));
                }
            }
            deletePendingCrashReports(crashes);
            if (shouldSendLogs && !combinedCrashID.isEmpty())
            {
                sendLogs(combinedCrashID);
            }
            sendOSNotification(!combinedCrashID.isEmpty());
        });
}

QStringList CrashHandler::getPendingCrashReports()
{
    QStringList dumpFiles;
    QDir dir(mDumpPath);
    QFileInfoList crashFileList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Time);
    for (int i = 0; i < crashFileList.size(); i++)
    {
        QFile file(crashFileList[i].absoluteFilePath());
        if (!file.fileName().endsWith(QString::fromLatin1(".dmp")))
        {
            continue;
        }
        dumpFiles.push_back(crashFileList[i].absoluteFilePath());
    }
    return dumpFiles;
}

void CrashHandler::sendLogs(const QString& crashID)
{
    MegaApi::log(MegaApi::LOG_LEVEL_WARNING, "Uploaded logs after crash...");
    connect(&MegaSyncApp->getLogger(),
            &MegaSyncLogger::logReadyForReporting,
            this,
            [crashID]()
            {
                auto crashReportFilePath =
                    Utilities::joinLogZipFiles(MegaSyncApp->getMegaApi(), nullptr, crashID);
                if (!crashReportFilePath.isNull() && MegaSyncApp->getMegaApi() &&
                    MegaSyncApp->getMegaApi()->isLoggedIn())
                {
                    MegaSyncApp->getMegaApi()->startUploadForSupport(
                        QDir::toNativeSeparators(crashReportFilePath).toUtf8().constData(),
                        false);
                    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG,
                                 QString::fromUtf8("Uploaded logs: %1")
                                     .arg(crashReportFilePath)
                                     .toUtf8()
                                     .constData());
                    crashReportFilePath.clear();
                }
            });
    MegaSyncApp->getLogger().prepareForReporting();
}

QString CrashHandler::sendCrashReport(const std::string& url,
                                      const std::map<std::string, std::string>& parameters,
                                      const std::map<std::string, std::string>& files)
{
    const QUrl qurl(QString::fromStdString(url));
    std::unique_ptr<QHttpMultiPart> multiPart =
        std::make_unique<QHttpMultiPart>(QHttpMultiPart::FormDataType);

    for (const auto& [paramName, paramValue]: parameters)
    {
        QHttpPart part;
        part.setHeader(
            QNetworkRequest::ContentDispositionHeader,
            QVariant(
                QLatin1String("form-data; name=\"%1\"").arg(QString::fromStdString(paramName))));
        part.setBody(QByteArray::fromStdString(paramValue));
        multiPart->append(part);
    }

    for (const auto& [fileName, filePath]: files)
    {
        QHttpPart part;
        part.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant(QLatin1String("form-data; name=\"%1\"; filename=\"%2\"")
                                    .arg(QString::fromStdString(fileName),
                                         QFileInfo(QString::fromStdString(filePath)).fileName())));
        QFile* filePtr = new QFile(QString::fromStdString(filePath), multiPart.get());
        if (!filePtr->open(QIODevice::ReadOnly))
        {
            MegaApi::log(MegaApi::LOG_LEVEL_WARNING, "Failed to open crash dump file");

            return QString();
        }
        part.setBodyDevice(filePtr);
        multiPart->append(part);
    }

    QNetworkAccessManager networkManager;
    QNetworkRequest request(qurl);

    std::unique_ptr<QNetworkReply> reply(networkManager.post(request, multiPart.get()));

    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    timeoutTimer.start(Preferences::CRASH_REPORT_TIMEOUT_MS);
    connect(&timeoutTimer, &QTimer::timeout, reply.get(), &QNetworkReply::abort);

    QEventLoop loop;
    connect(reply.get(), &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    bool success = reply->error() == QNetworkReply::NoError;
    QString crashID;
    if (success)
    {
        crashID = QString::fromStdString(reply->readAll().toStdString());
        crashID.remove(QLatin1Char('-'));
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG,
                     QString::fromUtf8("Crash report sent successfully: %1")
                         .arg(crashID)
                         .toUtf8()
                         .constData());
    }
    else
    {
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG,
                     QString::fromUtf8("Crash report failed: %1")
                         .arg(reply->errorString())
                         .toUtf8()
                         .constData());
    }
    return crashID;
}

void CrashHandler::sendOSNotification(bool succeeded)
{
    DesktopNotifications::NotificationInfo notification;
    if (succeeded)
    {
        notification.title = tr("Error report sent");
        notification.message =
            tr("Your error report was successfully submitted. Thank you for your feedback!");
        MegaApi::log(MegaApi::LOG_LEVEL_WARNING, "Error report sent successfully");
    }
    else
    {
        MegaApi::log(MegaApi::LOG_LEVEL_WARNING, "Failed to send error report");

        notification.title = tr("Failed to send error report");
        notification.message =
            tr("Unable to send the error report. Please contact support for assistance.");
        notification.actions << tr("Contact support");
        notification.activatedFunction = [](DesktopAppNotificationBase::Action action)
        {
            if (action == DesktopAppNotificationBase::Action::firstButton)
            {
                // FIXME mega.app -- contact url
                Utilities::openUrl(ServiceUrls::instance()->getContactUrl());
            }
        };
    }
    MegaSyncApp->showInfoMessage(notification);
}

void CrashHandler::deletePendingCrashReports(const QStringList& crashes)
{
    for (const auto& crash: crashes)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_WARNING,
                     QString::fromUtf8("Removing crash dump: %1").arg(crash).toUtf8().constData());
        QFile::remove(crash);
    }
}
#endif
