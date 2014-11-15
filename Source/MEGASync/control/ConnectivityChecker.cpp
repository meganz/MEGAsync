#include "ConnectivityChecker.h"

ConnectivityChecker::ConnectivityChecker(QString testURL, QObject *parent) :
    QObject(parent)
{
    networkAccess = NULL;
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTestTimeout()));

    this->testURL = testURL;
}

ConnectivityChecker::~ConnectivityChecker()
{
    delete networkAccess;
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
    delete networkAccess;
    timer->stop();

    QNetworkRequest proxyTestRequest(testURL);
    proxyTestRequest.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QVariant( int(QNetworkRequest::AlwaysNetwork)));
    networkAccess = new QNetworkAccessManager();
    networkAccess->setProxy(proxy);

    connect(networkAccess, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(onTestFinished(QNetworkReply*)));
    connect(networkAccess, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)),
            this, SLOT(onProxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)));

    timer->start(timeoutms);
    networkAccess->get(proxyTestRequest);
}

void ConnectivityChecker::onTestFinished(QNetworkReply *reply)
{
    timer->stop();
    reply->deleteLater();
    if(networkAccess)
    {
        networkAccess->deleteLater();
        networkAccess = NULL;
    }

    QVariant statusCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    if (!statusCode.isValid() || (statusCode.toInt() != 200) || (reply->error() != QNetworkReply::NoError))
    {
        emit error();
        emit finished();
        return;
    }

    QString data = QString::fromUtf8(reply->readAll());
    if (!testString.isEmpty() && !data.contains(testString)) {
        emit error();
        emit finished();
        return;
    }

    emit success();
    emit finished();
}

void ConnectivityChecker::onTestTimeout()
{
    if(networkAccess)
    {
        networkAccess->deleteLater();
        networkAccess = NULL;
    }

    emit error();
    emit finished();
}

void ConnectivityChecker::onProxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *auth)
{
    if(!proxy.user().isEmpty())
    {
        auth->setUser(proxy.user());
        auth->setPassword(proxy.password());
    }
}
