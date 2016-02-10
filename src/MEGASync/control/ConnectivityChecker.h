#ifndef CONNECTIVITYCHECKER_H
#define CONNECTIVITYCHECKER_H

#include <QObject>
#include <QNetworkProxy>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QAuthenticator>

class ConnectivityChecker : public QObject
{
    Q_OBJECT

public:
    explicit ConnectivityChecker(QString testURL, QObject *parent = 0);
    void setProxy(QNetworkProxy proxy);
    void setTimeout(int ms);
    void setTestString(QString testString);
    void startCheck();

signals:
    void testError();
    void testSuccess();
    void testFinished();

protected slots:
    void onTestTimeout();
    void onTestFinished(QNetworkReply*);
    void onProxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*);

protected:
    QNetworkAccessManager *networkAccess;
    QNetworkReply *reply;
    QNetworkProxy proxy;
    QTimer *timer;
    int timeoutms;
    QString testURL;
    QString testString;
};

#endif // CONNECTIVITYCHECKER_H
