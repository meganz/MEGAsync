#include "HTTPServer.h"
#include "Preferences.h"
#include "Utilities.h"
#include "MegaApplication.h"

#include <iostream>


using namespace mega;

HTTPServer::HTTPServer(MegaApi *megaApi, quint16 port, bool sslEnabled)
    : QTcpServer(), disabled(false)
{
    this->megaApi = megaApi;
    this->sslEnabled = sslEnabled;
    this->isFirstWebDownloadDone = false;
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

    if (sslEnabled)
    {
        sslSocket = new QSslSocket(this);;
        s = sslSocket;
        connect(sslSocket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslErrors(QList<QSslError>)));
        connect(sslSocket, SIGNAL(peerVerifyError(QSslError)), this, SLOT(peerVerifyError(QSslError)));
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

    if (sslSocket)
    {
        sslSocket->setPeerVerifyMode(QSslSocket::VerifyNone);

        QSslKey key(Preferences::HTTPS_KEY.toUtf8(), QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey);
        if (key.isNull())
        {
            s->disconnectFromHost();
            return;
        }

#if QT_VERSION >= 0x050100
        QList<QSslCertificate> certificates;
        certificates.append(QSslCertificate(Preferences::HTTPS_CERT.toUtf8(), QSsl::Pem));
        certificates.append(QSslCertificate(Preferences::HTTPS_CERT_INTERMEDIATE.toUtf8(), QSsl::Pem));
        sslSocket->setLocalCertificateChain(certificates);
#else
        sslSocket->setLocalCertificate(QSslCertificate(Preferences::HTTPS_CERT.toUtf8(), QSsl::Pem));
#endif
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

        if (Preferences::HTTPS_ORIGIN_CHECK_ENABLED && !Preferences::HTTPS_ALLOWED_ORIGINS.isEmpty())
        {
            bool found = false;
            for (int i = 0; i < Preferences::HTTPS_ALLOWED_ORIGINS.size(); i++)
            {                
                QString check = QString::fromUtf8("Origin: %1").arg(Preferences::HTTPS_ALLOWED_ORIGINS.at(i));
                for (int j = 0; j < headers.size(); j++)
                {
                    if (!headers[j].compare(check, Qt::CaseInsensitive))
                    {
                       request->origin = i;
                       found = true;
                       break;
                    }
                }

                if (found)
                {
                    break;
                }
            }

            if (!found)
            {
                rejectRequest(socket);
                return;
            }
        }

        QString contentLengthId = QString::fromUtf8("Content-length: ");
        QStringList contentLengthHeader = headers.filter(QRegExp(contentLengthId, Qt::CaseInsensitive));
        if (!contentLengthHeader.size())
        {
            rejectRequest(socket);
            return;
        }

        bool ok;
        request->contentLength = contentLengthHeader[0].mid(contentLengthId.size(), contentLengthHeader[0].size() - contentLengthId.size()).toInt(&ok);
        if (!ok || request->contentLength < tokens[1].size())
        {
            rejectRequest(socket);
            return;
        }

        if (request->contentLength > tokens[1].size())
        {
            return;
        }

        request->data = tokens[1];
        processRequest(socket, *request);

        HTTPRequest *req = requests.value(socket, NULL);
        if (request == req)
        {
            requests.remove(socket);
            delete request;
        }
    }
}
void HTTPServer::discardClient()
{
    QAbstractSocket* socket = (QSslSocket*)sender();
    socket->deleteLater();

    HTTPRequest *request = requests.value(socket);
    if (request)
    {
        requests.remove(socket);
        delete request;
    }
}

void HTTPServer::rejectRequest(QAbstractSocket *socket, QString response)
{
    socket->write(QString::fromUtf8("HTTP/1.0 %1\r\n"
                  "\r\n").arg(response).toUtf8());
    socket->flush();
    socket->disconnectFromHost();
    socket->deleteLater();

    HTTPRequest *request = requests.value(socket);
    if (request)
    {
        requests.remove(socket);
        delete request;
    }
}

void HTTPServer::processRequest(QAbstractSocket *socket, HTTPRequest request)
{
    QString response;
    QString openLinkRequestStart(QString::fromUtf8("{\"a\":\"l\","));
    QRegExp syncRequest(QString::fromUtf8("\\{\"a\":\"s\",\"h\":\"(.*)\"\\}"));
    QString externalDownloadRequestStart = QString::fromUtf8("{\"a\":\"d\",");
    QPointer<QAbstractSocket> safeSocket = socket;

    if (request.data == QString::fromUtf8("{\"a\":\"v\"}"))
    {
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "GetVersion command received from the webclient");
        char *myHandle = megaApi->getMyUserHandle();
        if (!myHandle)
        {
            response = QString::fromUtf8("{\"v\":\"%1\"}").arg(Preferences::VERSION_STRING);
        }
        else
        {
            response = QString::fromUtf8("{\"v\":\"%1\",\"u\":\"%2\"}")
                    .arg(Preferences::VERSION_STRING)
                    .arg(QString::fromUtf8(myHandle));
            delete myHandle;
        }
    }
    else if (request.data.startsWith(openLinkRequestStart))
    {
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "OpenLink command received from the webclient");
        QString handle = Utilities::extractJSONString(request.data, QString::fromUtf8("h"));
        QString key = Utilities::extractJSONString(request.data, QString::fromUtf8("k"));
        QString auth = Utilities::extractJSONString(request.data, QString::fromUtf8("esid"));

        if (key.size() > 43)
        {
            key.resize(43);
        }

        if (handle.size() == 8 && key.size() == 43)
        {
            QString link = QString::fromUtf8("https://mega.nz/#!%1!%2").arg(handle).arg(key);
            emit onLinkReceived(link, auth);
            response = QString::fromUtf8("0");

            Preferences *preferences = Preferences::instance();
            QString defaultPath = preferences->downloadFolder();

            if (preferences->hasDefaultDownloadFolder() && QFile(defaultPath).exists())
            {
                ((MegaApplication *)qApp)->showInfoMessage(tr("Your download has started"));
            }

            if (!isFirstWebDownloadDone && !Preferences::instance()->isFirstWebDownloadDone())
            {
                megaApi->sendEvent(99503, "MEGAsync first webclient download");
                isFirstWebDownloadDone = true;
            }
        }
        else if (key.size() && key.size() != 43)
        {
            response = QString::fromUtf8("-14");
        }
    }
    else if (syncRequest.exactMatch(request.data))
    {
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "SyncFolder command received from the webclient");
        QStringList parameters = syncRequest.capturedTexts();
        if (parameters.size() == 2)
        {
            QString handle = parameters[1];
            MegaHandle h = megaApi->base64ToHandle(handle.toUtf8().constData());
            MegaNode *node = megaApi->getNodeByHandle(h);
            if (!node)
            {
                if (!megaApi->isLoggedIn())
                {
                    response = QString::fromUtf8("-11");
                }
                else
                {
                    response = QString::fromUtf8("-9");
                }
            }
            else if (node->getType() == MegaNode::TYPE_FOLDER
                     || node->getType() == MegaNode::TYPE_ROOT
                     || node->getType() == MegaNode::TYPE_INCOMING)
            {
                emit onSyncRequested(h);
                response = QString::fromUtf8("0");
                delete node;
            }
        }
    }
    else if (request.data.startsWith(externalDownloadRequestStart))
    {
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "ExternalDownload command received from the webclient");
        int start = request.data.indexOf(QString::fromUtf8("\"f\":[")) + 5;
        if (start > 0)
        {
            QString privateAuth = Utilities::extractJSONString(request.data.mid(0, start), QString::fromUtf8("esid"));
            QString publicAuth = Utilities::extractJSONString(request.data.mid(0, start), QString::fromUtf8("en"));

            if (privateAuth.isEmpty() && publicAuth.isEmpty())
            {
                QString auth  = Utilities::extractJSONString(request.data.mid(0, start), QString::fromUtf8("auth"));
                if (auth.length() == 8)
                {
                    publicAuth = auth;
                }
                else
                {
                    privateAuth = auth;
                }
            }

            if (privateAuth.size() || publicAuth.size())
            {
                QQueue<mega::MegaNode *> downloadQueue;
                int end;
                bool firstnode = true;

                while (request.data[start] == QChar::fromAscii('{'))
                {
                    end = request.data.indexOf(QChar::fromAscii('}'), start);
                    if (end < 0)
                    {
                        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Error parsing webclient request");
                        qDeleteAll(downloadQueue);
                        downloadQueue.clear();
                        break;
                    }

                    end++;
                    QString file = request.data.mid(start, end - start);
                    start = end + 1;

                    int type = Utilities::extractJSONNumber(file, QString::fromUtf8("t"));
                    if (type < 0)
                    {
                        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Node without type in webclient request");
                        qDeleteAll(downloadQueue);
                        downloadQueue.clear();
                        break;
                    }

                    QString handle = Utilities::extractJSONString(file, QString::fromUtf8("h"));
                    if (handle.isEmpty())
                    {
                        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Node without handle in webclient request");
                        qDeleteAll(downloadQueue);
                        downloadQueue.clear();
                        break;
                    }

                    QString name = Utilities::extractJSONString(file, QString::fromUtf8("n"));
                    name.replace(QString::fromUtf8("-"), QString::fromUtf8("+"));
                    name.replace(QString::fromUtf8("_"), QString::fromUtf8("/"));
                    name = QString::fromUtf8(QByteArray::fromBase64(name.toUtf8().constData()).constData());
                    if (name.isEmpty())
                    {
                        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Node without name in webclient request");
                        qDeleteAll(downloadQueue);
                        downloadQueue.clear();
                        break;
                    }

                    MegaHandle h = megaApi->base64ToHandle(handle.toUtf8().constData());
                    MegaHandle p = INVALID_HANDLE;

                    if (!firstnode)
                    {
                        QString parentHandle = Utilities::extractJSONString(file, QString::fromUtf8("p"));
                        p = megaApi->base64ToHandle(parentHandle.toUtf8().constData());
                    }
                    else
                    {
                        firstnode = false;
                    }

                    if (type != MegaNode::TYPE_FILE)
                    {
                        MegaNode *node = megaApi->createForeignFolderNode(h, name.toUtf8().constData(), p,
                                                                         privateAuth.toUtf8().constData(),
                                                                         publicAuth.toUtf8().constData());
                        downloadQueue.append(node);
                    }
                    else
                    {
                        QString key = Utilities::extractJSONString(file, QString::fromUtf8("k"));
                        if (key.size() == 43)
                        {
                            long long size = Utilities::extractJSONNumber(file, QString::fromUtf8("s"));
                            long long mtime = Utilities::extractJSONNumber(file, QString::fromUtf8("ts"));

                            MegaNode *node = megaApi->createForeignFileNode(h, key.toUtf8().constData(),
                                                             name.toUtf8().constData(), size, mtime,
                                                             p, privateAuth.toUtf8().constData(),
                                                             publicAuth.toUtf8().constData());
                            downloadQueue.append(node);
                        }
                        else
                        {
                            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Node without key (or an invalid key) in webclient request");
                        }
                    }
                }

                if (downloadQueue.size())
                {
                    emit onExternalDownloadRequested(downloadQueue);
                    emit onExternalDownloadRequestFinished();
                    response = QString::fromUtf8("0");
                }
            }
        }
    }

    if (!response.size())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Invalid webclient request: %1").arg(request.data).toUtf8().constData());
        response = QString::fromUtf8("-2");
    }

    QString fullResponse = QString::fromUtf8("HTTP/1.0 200 Ok\r\n"
                                             "Access-Control-Allow-Origin: %1\r\n"
                                             "Content-Type: text/html; charset=\"utf-8\"\r\n"
                                             "Content-Length: %2\r\n"
                                             "\r\n"
                                             "%3")
            .arg((request.origin < 0 || request.origin >= Preferences::HTTPS_ALLOWED_ORIGINS.size())
                 ? QString::fromUtf8("*") : Preferences::HTTPS_ALLOWED_ORIGINS.at(request.origin))
            .arg(response.size())
            .arg(response);

    if (safeSocket)
    {
        safeSocket->write(fullResponse.toUtf8());
        safeSocket->flush();
        safeSocket->disconnectFromHost();
        safeSocket->deleteLater();
    }
}

void HTTPServer::error(QAbstractSocket::SocketError)
{
}

void HTTPServer::sslErrors(const QList<QSslError> &)
{
}

void HTTPServer::peerVerifyError(const QSslError &)
{
}
