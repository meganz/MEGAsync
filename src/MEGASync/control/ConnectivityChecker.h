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
    enum {
        METHOD_GET,
        METHOD_POST
    };

    explicit ConnectivityChecker(QString testURL, QObject *parent = 0);
    void setProxy(QNetworkProxy proxy);
    void setTimeout(int ms);
    void setTestString(QString testString);
    void setMethod(int method);
    void setHeader(QByteArray header, QByteArray value);
    void setPostData(QByteArray postData);
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
    QNetworkRequest testRequest;
    QByteArray postData;
    QNetworkReply *reply;
    QNetworkProxy proxy;
    QTimer *timer;
    int timeoutms;
    QString testURL;
    QString testString;
    int method;
};

#endif // CONNECTIVITYCHECKER_H
