#ifndef CRASHHANDLER_H
#define CRASHHANDLER_H

#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QCryptographicHash>
#include <QEventLoop>
#include <QTimer>

class CrashHandlerPrivate;
class CrashHandler: public QObject
{
    Q_OBJECT

public:
    static CrashHandler* instance();
    static void tryReboot();
    void Init(const QString&  reportPath);
    void Disable();
    void setReportCrashesToSystem(bool report);
    bool writeMinidump();

    QStringList getPendingCrashReports();
    void sendPendingCrashReports(QString userMessage);
    void discardPendingCrashReports();

    QString getLastCrashHash() const;

private slots:
    void onPostFinished(QNetworkReply *reply);
    void onCrashPostTimeout();

private:
    void deletePendingCrashReports();

    CrashHandler();
    ~CrashHandler();
    CrashHandlerPrivate* d;
    QString dumpPath;
    QNetworkAccessManager *networkManager;
    QNetworkRequest request;
    QEventLoop loop;
    QTimer crashPostTimer;
    QString lastCrashHash;
};

#endif // CRASHHANDLER_H
