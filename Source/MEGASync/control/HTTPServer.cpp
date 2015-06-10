#include "HTTPServer.h"
#include "Preferences.h"
#include <iostream>

HTTPServer::HTTPServer(quint16 port, bool sslEnabled)
    : QTcpServer(), disabled(false)
{
    this->sslEnabled = sslEnabled;
    listen(QHostAddress::LocalHost, port);
}

#if QT_VERSION >= 0x050000
void HTTPServer::incomingConnection(qintptr socket)
#else
void HTTPServer::incomingConnection(int socket)
#endif
{
    if (disabled)
    {
        return;
    }

    QTcpSocket* s = NULL;
    QSslSocket *sslSocket = NULL;

    if(sslEnabled)
    {
        sslSocket = new QSslSocket(this);;
        s = sslSocket;
    }
    else
    {
        s = new QTcpSocket(this);
    }

    connect(s, SIGNAL(readyRead()), this, SLOT(readClient()));
    connect(s, SIGNAL(disconnected()), this, SLOT(discardClient()));
    connect(s, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));

    s->setSocketDescriptor(socket);
    requests.insert(s, new HTTPRequest());

    if(sslSocket)
    {
        sslSocket->setPeerVerifyMode(QSslSocket::VerifyNone);

        QSslKey key(Preferences::HTTPS_KEY.toUtf8(), QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey);
        if (key.isNull())
        {
            s->disconnectFromHost();
            return;
        }

        sslSocket->setLocalCertificate(QSslCertificate(Preferences::HTTPS_CERT.toUtf8(), QSsl::Pem));
        sslSocket->setPrivateKey(key);
        sslSocket->startServerEncryption();
    }
}

void HTTPServer::pause()
{
    disabled = true;
}

void HTTPServer::resume()
{
    disabled = false;
}

void HTTPServer::readClient()
{
    QAbstractSocket *socket = (QSslSocket*)sender();
    HTTPRequest *request = requests.value(socket);
    if (disabled || !request)
    {
        discardClient();
        return;
    }

    request->data.append(QString::fromUtf8(socket->readAll().data()));
    if (request->data.contains(QString::fromUtf8("\r\n\r\n")))
    {
        QStringList tokens = request->data.split(QString::fromUtf8("\r\n\r\n"));
        QStringList headers = tokens[0].split(QString::fromUtf8("\r\n"));
        if (!headers.size() || !headers[0].startsWith(QString::fromAscii("POST")))
        {
            rejectRequest(socket, QString::fromUtf8("405 Method Not Allowed"));
            return;
        }

        if(Preferences::HTTPS_ALLOWED_ORIGIN != QString::fromUtf8("*"))
        {
            QStringList result = headers.filter(QRegExp(QString::fromUtf8("Origin: %1").arg(Preferences::HTTPS_ALLOWED_ORIGIN), Qt::CaseInsensitive));
            if(!result.size())
            {
                rejectRequest(socket);
                return;
            }
        }

        QString contentLengthId = QString::fromUtf8("Content-length: ");
        QStringList contentLengthHeader = headers.filter(QRegExp(contentLengthId, Qt::CaseInsensitive));
        if(!contentLengthHeader.size())
        {
            rejectRequest(socket);
            return;
        }

        bool ok;
        request->contentLength = contentLengthHeader[0].mid(contentLengthId.size(), contentLengthHeader[0].size() - contentLengthId.size()).toInt(&ok);
        if(!ok || request->contentLength < tokens[1].size())
        {
            rejectRequest(socket);
            return;
        }

        if(request->contentLength > tokens[1].size())
        {
            return;
        }

        request->data = tokens[1];
        processRequest(socket, request);
    }
}
void HTTPServer::discardClient()
{
    QAbstractSocket* socket = (QSslSocket*)sender();
    socket->disconnectFromHost();
    socket->deleteLater();

    HTTPRequest *request = requests.value(socket);
    if(request)
    {
        requests.remove(socket);
        delete request;
    }
}

void HTTPServer::rejectRequest(QAbstractSocket *socket, QString response)
{
    socket->write(QString::fromUtf8("HTTP/1.0 %1\r\n"
                  "\r\n").arg(response).toUtf8());
    socket->disconnectFromHost();
    socket->deleteLater();

    HTTPRequest *request = requests.value(socket);
    if(request)
    {
        requests.remove(socket);
        delete request;
    }
}

void HTTPServer::processRequest(QAbstractSocket *socket, HTTPRequest *request)
{
    QString response;
    QRegExp openLinkRequest(QString::fromUtf8("\\{\"a\":\"l\",\"h\":\"(.*)\",\"k\":\"(.*)\"\\}"));

    if(request->data == QString::fromUtf8("{\"a\":\"v\"}"))
    {
        response = QString::fromUtf8("{\"v\":\"%1\"}").arg(Preferences::VERSION_STRING);
    }
    else if(openLinkRequest.exactMatch(request->data))
    {
        QStringList parameters = openLinkRequest.capturedTexts();
        if(parameters.size() == 3)
        {
            QString handle = parameters[1];
            QString key = parameters[2];
            if(handle.size() == 8 && key.size() == 43)
            {
                QString link = QString::fromUtf8("https://mega.nz/#!%1!%2").arg(handle).arg(key);
                emit onLinkReceived(link);
                response = QString::fromUtf8("0");
            }
        }
    }

    if(!response.size())
    {
        response = QString::fromUtf8("-2");
    }

    socket->write(QString::fromUtf8("HTTP/1.0 200 Ok\r\n"
        "Access-Control-Allow-Origin: %1\r\n"
        "Content-Type: text/html; charset=\"utf-8\"\r\n"
        "Content-Length: %2\r\n"
        "\r\n"
        "%3").arg(Preferences::HTTPS_ALLOWED_ORIGIN).arg(response.size()).arg(response).toUtf8());

    socket->flush();
    socket->disconnectFromHost();
    socket->deleteLater();
}

void HTTPServer::error(QAbstractSocket::SocketError)
{
    discardClient();
}
