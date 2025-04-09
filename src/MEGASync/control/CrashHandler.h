#ifndef CRASHHANDLER_H
#define CRASHHANDLER_H

#ifdef USE_BREAKPAD
#include <QNetworkAccessManager>
#include <QObject>
#include <QString>

class CrashHandlerImpl;

class CrashHandler: public QObject
{
    Q_OBJECT

public:
    static CrashHandler* instance();

    void init(const QString& reportPath);
    static void tryReboot();
    void sendPendingCrashReports(QString userMessage, bool shouldSendLogs);
    QStringList getPendingCrashReports();
    void deletePendingCrashReports(const QStringList& crashes);

private:
    CrashHandler();
    ~CrashHandler();
    void sendLogs(const QString& crashID);
    QString sendCrashReport(const std::string& url,
                            const std::map<std::string, std::string>& parameters,
                            const std::map<std::string, std::string>& files);
    static void sendOSNotification(bool succeeded);

    CrashHandlerImpl* mImpl;
    QString mDumpPath;
};
#endif
#endif // CRASHHANDLER_H
