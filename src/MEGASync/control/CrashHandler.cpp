#include "CrashHandler.h"
#include <QtCore/QDir>
#include <QtCore/QProcess>
#include <QtCore/QCoreApplication>
#include <QString>
#include <sstream>
#include "MegaApplication.h"

using namespace mega;
using namespace std;

#if defined(Q_OS_MAC)
#include "client/mac/handler/exception_handler.h"
#elif defined(Q_OS_LINUX)
#include "client/linux/handler/exception_handler.h"
#elif defined(Q_OS_WIN32)
#include "client/windows/handler/exception_handler.h"
#endif

#ifndef WIN32
    #ifndef CREATE_COMPATIBLE_MINIDUMPS

    #include <signal.h>
    #include <execinfo.h>
    #include <sys/utsname.h>

    string dump_path;

    // signal handler
    void signal_handler(int sig, siginfo_t *info, void *secret)
    {
        int dump_file = open(dump_path.c_str(),  O_WRONLY | O_CREAT, 0400);
        if (dump_file<0)
        {
            CrashHandler::tryReboot();
            exit(128+sig);
        }

        std::ostringstream oss;
        oss << "MEGAprivate ERROR DUMP\n";
        oss << "Application: " << QApplication::applicationName().toStdString() << "\n";
        oss << "Version code: " << QString::number(Preferences::VERSION_CODE).toStdString() <<
               "." << QString::number(Preferences::BUILD_ID).toStdString() << "\n";
        oss << "Module name: " << "megasync" << "\n";

        struct utsname osData;
        if (!uname(&osData))
        {
            oss << "Operating system: " << osData.sysname << "\n";
            oss << "System version:  " << osData.version << "\n";
            oss << "System release:  " << osData.release << "\n";
            oss << "System arch: " << osData.machine << "\n";
        }
        else
        {
            oss << "Operating system: Unknown\n";
            oss << "System version: Unknown\n";
            oss << "System release: Unknown\n";
            oss << "System arch: Unknown\n";
        }

        time_t rawtime;
        time(&rawtime);
        oss << "Error info:\n";
        if (info)
        {
            oss << sys_siglist[sig] << " (" << sig << ") at address " << std::showbase << std::hex << info->si_addr << std::dec << "\n";
        }
        else
        {
            oss << "Out of memory" << endl;
        }

        void *pnt = NULL;
        if (secret)
        {
            #if defined(__APPLE__)
                ucontext_t* uc = (ucontext_t*) secret;
                pnt = (void *)uc->uc_mcontext->__ss.__rip;
            #elif defined(__x86_64__)
                ucontext_t* uc = (ucontext_t*) secret;
                pnt = (void*) uc->uc_mcontext.gregs[REG_RIP] ;
            #elif (defined (__ppc__)) || (defined (__powerpc__))
                ucontext_t* uc = (ucontext_t*) secret;
                pnt = (void*) uc->uc_mcontext.regs->nip ;
            #elif defined(__sparc__)
                struct sigcontext* sc = (struct sigcontext*) secret;
                #if __WORDSIZE == 64
                    pnt = (void*) scp->sigc_regs.tpc ;
                #else
                    pnt = (void*) scp->si_regs.pc ;
                #endif
            #elif defined(__i386__)
                ucontext_t* uc = (ucontext_t*) secret;
                pnt = (void*) uc->uc_mcontext.gregs[REG_EIP];
            #elif defined(__arm__)
                ucontext_t* uc = (ucontext_t*) secret;
                pnt = (void*) uc->uc_mcontext.arm_pc
            #else
                pnt = NULL;
            #endif
        }

        oss << "Stacktrace:\n";
        void *stack[32];
        size_t size;
        size = backtrace(stack, 32);
        if (size > 1)
        {
            stack[1] = pnt;
            char **messages = backtrace_symbols(stack, size);
            for (unsigned int i = 1; i < size; i++)
            {
                oss << messages[i] << "\n";
            }
        }
        else
        {
            oss << "Error getting stacktrace\n";
        }

        write(dump_file, oss.str().c_str(), oss.str().size());
        close(dump_file);

        CrashHandler::tryReboot();
        exit(128+sig);
    }

    void mega_new_handler()
    {
        signal_handler(0, 0, 0);
    }

    #endif
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
bool DumpCallback(const google_breakpad::MinidumpDescriptor &,void *context, bool success)
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

    CrashHandler::tryReboot();
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
#else
    #ifdef CREATE_COMPATIBLE_MINIDUMPS
        #if defined(Q_OS_LINUX)
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
    #else
        srandom(time(NULL));
        uint32_t data1 = (uint32_t)random();
        uint16_t data2 = (uint16_t)random();
        uint16_t data3 = (uint16_t)random();
        uint32_t data4 = (uint32_t)random();
        uint32_t data5 = (uint32_t)random();

        char name[37];
        sprintf(name, "%08x-%04x-%04x-%08x-%08x", data1, data2, data3, data4, data5);
        dump_path = dumpPath.toStdString() + "/" + name + ".dmp";

        /* Install our signal handler */
        struct sigaction sa;
        sa.sa_sigaction = signal_handler;
        sigemptyset (&sa.sa_mask);
        sa.sa_flags = SA_RESTART | SA_SIGINFO;
        sigaction(SIGSEGV, &sa, NULL);
        sigaction(SIGBUS, &sa, NULL);
        sigaction(SIGILL, &sa, NULL);
        sigaction(SIGFPE, &sa, NULL);
        sigaction(SIGABRT, &sa, NULL);
        std::set_new_handler(mega_new_handler);
    #endif
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

void CrashHandler::tryReboot()
{
    Preferences *preferences = Preferences::instance();
    preferences->setCrashed(true);

    if ((QDateTime::currentMSecsSinceEpoch()-preferences->getLastReboot()) > Preferences::MIN_REBOOT_INTERVAL_MS)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_WARNING, "Restarting app...");
        preferences->setLastReboot(QDateTime::currentMSecsSinceEpoch());

#ifndef __APPLE__
        QString app = MegaApplication::applicationFilePath();
        QProcess::startDetached(app);
#else
        QString app = MegaApplication::applicationDirPath();
        QString launchCommand = QString::fromUtf8("open");
        QStringList args = QStringList();

        QDir appPath(app);
        appPath.cdUp();
        appPath.cdUp();

        args.append(QString::fromAscii("-n"));
        args.append(appPath.absolutePath());
        QProcess::startDetached(launchCommand, args);
#endif

#ifdef WIN32
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

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Checking pending crash repors");
    QDir dir(dumpPath);
    QFileInfoList fiList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Time);
    for (int i = 0; i < fiList.size(); i++)
    {
        QFile file(fiList[i].absoluteFilePath());
        if (!file.fileName().endsWith(QString::fromAscii(".dmp")))
        {
            continue;
        }

        if (file.size() > 16384)
        {
            continue;
        }

        if (!file.open(QIODevice::ReadOnly))
        {
            continue;
        }

        QString crashReport = QString::fromUtf8(file.readAll());
        file.close();

        QStringList lines = crashReport.split(QString::fromAscii("\n"));
        if ((lines.size()<3)
                || (lines.at(0) != QString::fromAscii("MEGAprivate ERROR DUMP"))
                || (!lines.at(1).startsWith(QString::fromAscii("Application: ") + QApplication::applicationName()))
                || (!lines.at(2).startsWith(QString::fromAscii("Version code: ") + QString::number(Preferences::VERSION_CODE))))
        {
            MegaApi::log(MegaApi::LOG_LEVEL_WARNING, QString::fromUtf8("Invalid or outdated dump file: %1").arg(file.fileName()).toUtf8().constData());
            file.remove();
            continue;
        }

        QString crashHash = QString::fromAscii(QCryptographicHash::hash(crashReport.toUtf8(),QCryptographicHash::Md5).toHex());
        if (!previousCrashes.contains(crashHash))
        {
            MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("New crash file: %1  Hash: %2")
                         .arg(file.fileName()).arg(crashHash).toUtf8().constData());
            result.append(crashReport);
            previousCrashes.append(crashHash);
        }
        else
        {
            MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Already sent log file: %1").arg(file.fileName()).toUtf8().constData());
            file.remove();
        }
    }
    return result;
}

void CrashHandler::sendPendingCrashReports(QString userMessage)
{
    if (networkManager)
    {
        return;
    }

    QStringList crashes = getPendingCrashReports();
    if (!crashes.size())
    {
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

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Sending crash reports");
    networkManager->post(request, postString.toUtf8());
    loop.exec();
}

void CrashHandler::discardPendingCrashReports()
{
    Preferences *preferences = Preferences::instance();
    QStringList crashes = getPendingCrashReports();
    QStringList previousCrashes = preferences->getPreviousCrashes();
    for (int i = 0; i < crashes.size(); i++)
    {
        QString crashHash = QString::fromAscii(QCryptographicHash::hash(crashes[i].toUtf8(),QCryptographicHash::Md5).toHex());
        if (!previousCrashes.contains(crashHash))
        {
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
    if (networkManager)
    {
        networkManager->deleteLater();
        networkManager = NULL;
    }
    loop.exit();
    QVariant statusCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    if (!statusCode.isValid() || (statusCode.toInt() != 200) || (reply->error() != QNetworkReply::NoError))
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Error sending crash reports");
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Crash reports sent");
    discardPendingCrashReports();
}

void CrashHandler::onCrashPostTimeout()
{
    loop.exit();
    if (networkManager)
    {
        networkManager->deleteLater();
        networkManager = NULL;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Timeout sending crash reports");
    return;
}

void CrashHandler::deletePendingCrashReports()
{
    QDir dir(dumpPath);
    QFileInfoList fiList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Time);
    for (int i = 0; i < fiList.size(); i++)
    {
        QFileInfo fi = fiList[i];
        if (fi.fileName().endsWith(QString::fromAscii(".dmp")))
        {
            QFile::remove(fi.absoluteFilePath());
        }
    }
}

void CrashHandler::Init( const QString& reportPath )
{
    this->dumpPath = reportPath;
    d->InitCrashHandler(reportPath);
}

void CrashHandler::Disable()
{
    delete d;
    d = NULL;
}
