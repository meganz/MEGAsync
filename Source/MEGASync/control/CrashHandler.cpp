#include "CrashHandler.h"
#include <QtCore/QDir>
#include <QtCore/QProcess>
#include <QtCore/QCoreApplication>
#include <QString>
#include "MegaApplication.h"

#if defined(Q_OS_MAC)
#include "client/mac/handler/exception_handler.h"
#elif defined(Q_OS_LINUX)
#include "client/linux/handler/exception_handler.h"
#elif defined(Q_OS_WIN32)
#include "client/windows/handler/exception_handler.h"
#endif

/************************************************************************/
/* CrashHandlerPrivate                                                  */
/************************************************************************/
class CrashHandlerPrivate
{
public:
    CrashHandlerPrivate()
    {
        pHandler = NULL;
    }

    ~CrashHandlerPrivate()
    {
        delete pHandler;
    }

    void InitCrashHandler(const QString& dumpPath);
    static google_breakpad::ExceptionHandler* pHandler;
    static bool bReportCrashesToSystem;
};

google_breakpad::ExceptionHandler* CrashHandlerPrivate::pHandler = NULL;
bool CrashHandlerPrivate::bReportCrashesToSystem = true;

/************************************************************************/
/* DumpCallback                                                         */
/************************************************************************/
#if defined(Q_OS_WIN32)
bool DumpCallback(const wchar_t* _dump_dir,const wchar_t* _minidump_id,void* context,EXCEPTION_POINTERS* exinfo,MDRawAssertionInfo* assertion,bool success)
#elif defined(Q_OS_LINUX)
bool DumpCallback(const google_breakpad::MinidumpDescriptor &md,void *context, bool success)
#elif defined(Q_OS_MAC)
bool DumpCallback(const char* _dump_dir,const char* _minidump_id,void *context, bool success)
#endif
{
    Q_UNUSED(context);
#if defined(Q_OS_WIN32)
    Q_UNUSED(_dump_dir);
    Q_UNUSED(_minidump_id);
    Q_UNUSED(assertion);
    Q_UNUSED(exinfo);
#endif
    qDebug("BreakpadQt crash");
    //QString app = MegaApplication::applicationFilePath();
    //QStringList args = QStringList();
    //args.append(QString::fromAscii("/reboot"));
    //QProcess::startDetached(app, args);
    //((MegaApplication *)qApp)->cleanAll();
    //qDebug("BreakpadQt restart");

    Preferences *preferences = Preferences::instance();
    preferences->setCrashed(true);

    if((QDateTime::currentMSecsSinceEpoch()-preferences->getLastReboot()) > Preferences::MIN_REBOOT_INTERVAL_MS)
    {
        LOG("Reboot");
        preferences->setLastReboot(QDateTime::currentMSecsSinceEpoch());
        QString app = MegaApplication::applicationFilePath();
        QStringList args = QStringList();
        args.append(QString::fromAscii("/reboot"));
        QProcess::startDetached(app, args);
    }
    else
    {
        LOG("No reboot");
    }

    return CrashHandlerPrivate::bReportCrashesToSystem ? success : false;
}

void CrashHandlerPrivate::InitCrashHandler(const QString& dumpPath)
{
    if ( pHandler != NULL )
        return;

#if defined(Q_OS_WIN32)
    std::wstring pathAsStr = (const wchar_t*)dumpPath.utf16();
    pHandler = new google_breakpad::ExceptionHandler(
        pathAsStr,
        /*FilterCallback*/ NULL,
        DumpCallback,
        /*context*/
        NULL,
        google_breakpad::ExceptionHandler::HANDLER_EXCEPTION
        );
#elif defined(Q_OS_LINUX)
    std::string pathAsStr = dumpPath.toStdString();
    google_breakpad::MinidumpDescriptor md(pathAsStr);
    pHandler = new google_breakpad::ExceptionHandler(
        md,
        /*FilterCallback*/ 0,
        DumpCallback,
        /*context*/ 0,
        true,
        -1
        );
#elif defined(Q_OS_MAC)
    std::string pathAsStr = dumpPath.toStdString();
    pHandler = new google_breakpad::ExceptionHandler(
        pathAsStr,
        /*FilterCallback*/ 0,
        DumpCallback,
        /*context*/
        0,
        true,
        NULL
        );
#endif
}

/************************************************************************/
/* CrashHandler                                                         */
/************************************************************************/
CrashHandler* CrashHandler::instance()
{
    static CrashHandler globalHandler;
    return &globalHandler;
}

CrashHandler::CrashHandler() : QObject()
{
    d = new CrashHandlerPrivate();
    crashPostTimer.setSingleShot(true);
    connect(&crashPostTimer, SIGNAL(timeout()), this, SLOT(onCrashPostTimeout()));
    networkManager = NULL;
}

CrashHandler::~CrashHandler()
{
    delete d;
}

void CrashHandler::setReportCrashesToSystem(bool report)
{
    d->bReportCrashesToSystem = report;
}

bool CrashHandler::writeMinidump()
{
    bool res = d->pHandler->WriteMinidump();
    if (res) {
        qDebug("BreakpadQt: writeMinidump() successed.");
    } else {
        qWarning("BreakpadQt: writeMinidump() failed.");
    }
    return res;
}

QStringList CrashHandler::getPendingCrashReports()
{
    Preferences *preferences = Preferences::instance();
    QStringList previousCrashes = preferences->getPreviousCrashes();
    QStringList result;

    LOG("Getting pending crash repors");
    QDir dir(dumpPath);
    QFileInfoList fiList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Time);
    for(int i=0; i<fiList.size(); i++)
    {
        QFile file(fiList[i].absoluteFilePath());
        if(file.size()>16384)
            continue;

        if(!file.open(QIODevice::ReadOnly))
            continue;

        QString crashReport = QString::fromUtf8(file.readAll());
        file.close();

        QStringList lines = crashReport.split(QString::fromAscii("\n"));
        if((lines.size()<3) ||
         (lines.at(0) != QString::fromAscii("MEGAprivate ERROR DUMP")) ||
                (!lines.at(1).startsWith(QString::fromAscii("Application: ") + QApplication::applicationName())) ||
                (!lines.at(2).startsWith(QString::fromAscii("Version code: ") + QString::number(MegaApplication::VERSION_CODE))))
        {
            LOG(QString::fromAscii("Invalid or outdated dump file: ") + file.fileName());
            LOG(crashReport);
            file.remove();
            continue;
        }

        QString crashHash = QString::fromAscii(QCryptographicHash::hash(crashReport.toUtf8(),QCryptographicHash::Md5).toHex());
        if(!previousCrashes.contains(crashHash))
        {
            LOG(QString::fromAscii("New crash file: ") + file.fileName() + QString::fromAscii("  Hash: ") + crashHash);
            result.append(crashReport);
            previousCrashes.append(crashHash);
        }
        else
        {
            LOG("Already sent log file");
            file.remove();
        }
    }
    LOG(QString::fromAscii("Number of crash reports: ") + QString::number(result.size()));
    return result;
}

void CrashHandler::sendPendingCrashReports(QString userMessage)
{
    if(networkManager) return;

    QStringList crashes = getPendingCrashReports();
    if(!crashes.size())
    {
        LOG("No pending crashes");
        return;
    }

    crashes.append(userMessage);
    QString postString = crashes.join(QString::fromAscii("------------------------------\n"));
    postString.append(QString::fromAscii("\n------------------------------\n"));

    networkManager = new QNetworkAccessManager();
    connect(networkManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(onPostFinished(QNetworkReply*)), Qt::UniqueConnection);
    request.setUrl(Preferences::CRASH_REPORT_URL);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QString::fromAscii("application/x-www-form-urlencoded"));
    LOG("Sending crash reports");
    networkManager->post(request, postString.toUtf8());
    loop.exec();
}

void CrashHandler::discardPendingCrashReports()
{
    LOG("Discarding crashes");
    Preferences *preferences = Preferences::instance();
    QStringList crashes = getPendingCrashReports();
    QStringList previousCrashes = preferences->getPreviousCrashes();
    for(int i=0; i<crashes.size(); i++)
    {
        QString crashHash = QString::fromAscii(QCryptographicHash::hash(crashes[i].toUtf8(),QCryptographicHash::Md5).toHex());
        if(!previousCrashes.contains(crashHash))
        {
            LOG(QString::fromAscii("Discarding crash: ") + crashHash);
            previousCrashes.append(crashHash);
        }
    }
    preferences->setPreviousCrashes(previousCrashes);
    deletePendingCrashReports();
}

void CrashHandler::onPostFinished(QNetworkReply *reply)
{
    reply->deleteLater();
    crashPostTimer.stop();
    if(networkManager)
    {
        networkManager->deleteLater();
        networkManager = NULL;
    }
    loop.exit();
    QVariant statusCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    if (!statusCode.isValid() || (statusCode.toInt() != 200) || (reply->error() != QNetworkReply::NoError))
    {
        LOG("Error sending crash reports");
        return;
    }

    LOG("Crash reports sent");
    discardPendingCrashReports();
}

void CrashHandler::onCrashPostTimeout()
{
    loop.exit();
    if(networkManager)
    {
        networkManager->deleteLater();
        networkManager = NULL;
    }

    LOG("POST timeout");
    return;
}

void CrashHandler::deletePendingCrashReports()
{
    LOG("Deleting all crash reports");
    QDir dir(dumpPath);
    QFileInfoList fiList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Time);
    for(int i=0; i<fiList.size(); i++)
    {
        QFileInfo fi = fiList[i];
        if(fi.fileName().endsWith(QString::fromAscii(".dmp")))
            QFile::remove(fi.absoluteFilePath());
    }
}

void CrashHandler::Init( const QString& reportPath )
{
    this->dumpPath = reportPath;
    d->InitCrashHandler(reportPath);
}
