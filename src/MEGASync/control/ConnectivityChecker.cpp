#include "ConnectivityChecker.h"

ConnectivityChecker::ConnectivityChecker(QString testURL, QObject *parent) :
    QObject(parent)
{
    networkAccess = new QNetworkAccessManager(this);
    timer = new QTimer(this);
    timer->setSingleShot(true);

    connect(timer, SIGNAL(timeout()), this, SLOT(onTestTimeout()));
    connect(networkAccess, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(onTestFinished(QNetworkReply*)));
    connect(networkAccess, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)),
            this, SLOT(onProxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)));

    this->testURL = testURL;
    this->reply = NULL;
    timeoutms = 10000;
}

void ConnectivityChecker::setProxy(QNetworkProxy proxy)
{
    this->proxy = proxy;
}

void ConnectivityChecker::setTimeout(int ms)
{
    this->timeoutms = ms;
}

void ConnectivityChecker::setTestString(QString testString)
{
    this->testString = testString;
}

void ConnectivityChecker::startCheck()
{    
    timer->stop();

    QNetworkRequest proxyTestRequest(testURL);
    proxyTestRequest.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QVariant( int(QNetworkRequest::AlwaysNetwork)));
    networkAccess->setProxy(proxy);

    timer->start(timeoutms);
    reply = networkAccess->get(proxyTestRequest);
}

void ConnectivityChecker::onTestFinished(QNetworkReply *reply)
{
    timer->stop();
    reply->deleteLater();
    this->reply = NULL;

    QVariant statusCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    if (!statusCode.isValid() || (statusCode.toInt() != 200) || (reply->error() != QNetworkReply::NoError))
    {
        emit testError();
        emit testFinished();
        return;
    }

    QString data = QString::fromUtf8(reply->readAll());
    if (!testString.isEmpty() && !data.contains(testString)) {
        emit testError();
        emit testFinished();
        return;
    }

    emit testSuccess();
    emit testFinished();
}

void ConnectivityChecker::onTestTimeout()
{
    if (reply)
    {
        reply->abort();
    }
}

void ConnectivityChecker::onProxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *auth)
{
    if (!proxy.user().isEmpty())
    {
        auth->setUser(proxy.user());
        auth->setPassword(proxy.password());
    }
}
